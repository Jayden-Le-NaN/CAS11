#include "adf4252.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_spi.h"
#include "utils.h"

//------------------------------------------------------------
// Created Time     : 2024.09.27
// Author           : JaydenLee
// Note             : 驱动中,LE引脚的用法和SPI中CS引脚的用法相同
//------------------------------------------------------------


//------------------------------仅内部可用,外部不可用------------------------------
#define ADF4252_Transmit_Receive_Start()    HAL_GPIO_WritePin(adf4252_obj->le_pin_type, adf4252_obj->le_pin, GPIO_PIN_RESET);
#define ADF4252_Transmit_Receive_Stop()     HAL_GPIO_WritePin(adf4252_obj->le_pin_type, adf4252_obj->le_pin, GPIO_PIN_SET);
//------------------------------仅内部可用,外部不可用------------------------------

/*
 * @brief               把缓冲区中的数据打到片子上的锁存器中
 * @param adf4252_obj   adf4252 指定信息
 * @param reg_addr      需要打入的锁存器地址
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 地址填写错误或者传输总线被占用
 */
UTILS_Status ADF4252_Write_register(ADF4252_Info_Struct* adf4252_obj, uint8_t reg_addr) {
    // 数据的发送不适用DMA
    UTILS_Status status = UTILS_OK;
    uint8_t packet[3];
    uint32_t reg_data;
    do {
        //------------------------------数据获取------------------------------
        if (reg_addr == ADF4252_REG_RF_CONTROL) 
            reg_data = adf4252_obj->_val_rf_control;
        else if (reg_addr == ADF4252_REG_RF_R_DIVIDER)
            reg_data = adf4252_obj->_val_if_r_divider;
        else if (reg_addr == ADF4252_REG_RF_CONTROL)
            reg_data = adf4252_obj->_val_rf_control;
        else if (reg_addr == ADF4252_REG_MASTER)
            reg_data = adf4252_obj->_val_master;
        else if (reg_addr == ADF4252_REG_IF_N_DIVIDER)
            reg_data = adf4252_obj->_val_if_n_divider;
        else if (reg_addr == ADF4252_REG_IF_R_DIVIDER)
            reg_data = adf4252_obj->_val_if_r_divider;
        else if (reg_addr == ADF4252_REG_IF_CONTROL)
            reg_data = adf4252_obj->_val_if_r_divider;
        else {
            status = UTILS_ERROR;
            break;
        }

        //------------------------------数据装载------------------------------
        packet[0] = reg_data & 0xFF;
        packet[1] = (reg_data >> 8) & 0xFF;
        packet[2] = (reg_data >> 16) & 0xFF;

        //------------------------------把数据打出去------------------------------
        ADF4252_Transmit_Receive_Start();
        if (HAL_SPI_Transmit(adf4252_obj->spi, packet, 3, HAL_MAX_DELAY) != HAL_OK) 
            status = UTILS_ERROR;
        ADF4252_Transmit_Receive_Stop();

    } while(0);
    return status;
}

/*
 * @brief               把所有的数据都打到响应的寄存器中
 * @param adf4252_obj   adf4252 指定信息
 * @param reg_addr      需要打入的锁存器地址
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 传输总线被占用
 */
UTILS_Status ADF4252_Write_All_Registers(ADF4252_Info_Struct* adf4252_obj) {
    UTILS_Status status = UTILS_OK;
    status = ADF4252_Write_register(adf4252_obj, ADF4252_REG_RF_N_DIVIDER);
    status = ADF4252_Write_register(adf4252_obj, ADF4252_REG_RF_R_DIVIDER);
    status = ADF4252_Write_register(adf4252_obj, ADF4252_REG_RF_CONTROL);
    status = ADF4252_Write_register(adf4252_obj, ADF4252_REG_MASTER);
    status = ADF4252_Write_register(adf4252_obj, ADF4252_REG_IF_N_DIVIDER);
    status = ADF4252_Write_register(adf4252_obj, ADF4252_REG_IF_R_DIVIDER);
    status = ADF4252_Write_register(adf4252_obj, ADF4252_REG_IF_CONTROL);
    return status;
}

/*
 * @brief               设置某些Value或Counter的值
 * @param adf4252_obj   adf4252 指定信息
 * @param vc_code       某些value 或 counter 的编码
 * @param val           需要设置的值
 * @return              UTILS_OK    : 一切正常
 *                      UTILS_ERROR : vc_code 设置错误
 * @note                在这里没有对value值的合法性进行判断
 */
UTILS_Status ADF4252_VC_Set(ADF4252_Info_Struct* adf4252_obj, uint8_t vc_code, uint32_t val) {
    UTILS_Status status = UTILS_OK;   
    if (vc_code == ADF4252_VC_RF_INTEGER) {
        status = UTILS_WriteBit_Zone(&(adf4252_obj->_val_rf_n_divider), 22, 15, val);
    }
    else if (vc_code == ADF4252_VC_RF_FRACTIONAL) {
        status = UTILS_WriteBit_Zone(&(adf4252_obj->_val_rf_n_divider), 14, 3, val);
    }
    else if (vc_code == ADF4252_VC_RF_R) {
        status = UTILS_WriteBit_Zone(&(adf4252_obj->_val_rf_r_divider), 18, 15, val);
    }
    else if (vc_code == ADF4252_VC_INTERPOLATOR_MODULUS) {
        status = UTILS_WriteBit_Zone(&(adf4252_obj->_val_rf_r_divider), 14, 3, val);
    }
    else if (vc_code == ADF4252_VC_IF_B) {
        status = UTILS_WriteBit_Zone(&(adf4252_obj->_val_if_n_divider), 20, 9, val);
    }
    else if (vc_code == ADF4252_VC_IF_A) {
        status = UTILS_WriteBit_Zone(&(adf4252_obj->_val_if_n_divider), 8, 3, val);
    }
    else if (vc_code == ADF4252_VC_IF_R) {
        status = UTILS_WriteBit_Zone(&(adf4252_obj->_val_if_r_divider), 17, 3, val);
    }
    else 
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               对于某些状态位进行设置
 * @param adf4252_obj   adf4252 指定信息
 * @param bit_code      对应bit位的编码(参考头文件中的表单)
 * @param val           需要设置的值
 * @return              UTILS_OK    : 没有问题
 *                      UTILS_ERROR : bit_code 设置错误
 * @note                程序并没有判断数据的合法性,需要自行处理
 */
UTILS_Status ADF4252_Status_Set(ADF4252_Info_Struct* adf4252_obj, uint8_t bit_code, uint8_t val) {
    UTILS_Status status = UTILS_OK;
    if (bit_code == ADF4252_BIT_RF_REF_DOUBLER) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_rf_r_divider), 19, val);
    }
    else if (bit_code == ADF4252_BIT_RF_PD_POLARITY) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_rf_control), 7, val);
    }
    else if (bit_code == ADF4252_BIT_RF_POWER_DOWN) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_rf_control), 5, val);
    }
    else if (bit_code == ADF4252_BIT_RF_CP_THREE_STATE) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_rf_control), 4, val);
    }
    else if (bit_code == ADF4252_BIT_RF_COUNTER_RESET) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_rf_control), 3, val);
    }
    else if (bit_code == ADF4252_BIT_XO_DISABLE) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_master), 6, val);
    }
    else if (bit_code == ADF4252_BIT_POWER_DOWN) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_master), 5, val);
    }
    else if (bit_code == ADF4252_BIT_CP_THREE_STATE) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_master), 4, val);
    }
    else if (bit_code == ADF4252_BIT_COUNTER_RESET) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_master), 3, val);
    }
    else if (bit_code == ADF4252_BIT_IF_CP_GAIN) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_if_n_divider), 23, val);
    }
    else if (bit_code == ADF4252_BIT_IF_REF_DOUBLER) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_if_r_divider), 18, val);
    }
    else if (bit_code == ADF4252_BIT_RF_PHASE_RESYNC) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_if_control), 15, val);
        status = UTILS_WriteBit(&(adf4252_obj->_val_if_control), 14, val);
        status = UTILS_WriteBit(&(adf4252_obj->_val_if_control), 11, val);
    }
    else if (bit_code == ADF4252_BIT_IF_PD_POLARITY) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_if_control), 7, val);
    }
    else if (bit_code == ADF4252_BIT_IF_POWER_DOWN) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_if_control), 5, val);
    }
    else if (bit_code == ADF4252_BIT_IF_CP_THREE_STATE) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_if_control), 4, val);
    }
    else if (bit_code == ADF4252_BIT_IF_COUNTER_RESET) {
        status = UTILS_WriteBit(&(adf4252_obj->_val_if_control), 3, val);
    }
    else 
        status = UTILS_ERROR;
    return status;
}


/*
 * @brief               设置多路复用器
 * @param adf4252_obj   adf4252 指定信息
 * @param set_val       设置的值
 * @return              UTILS_OK    : 没有问题
 *                      UTILS_ERROR : 内部传参有问题(与用户无关)
 * @note                set_val 只能用定义的宏!!!
 */
UTILS_Status ADF4252_MUX_Set(ADF4252_Info_Struct* adf4252_obj, uint8_t set_val) {
    UTILS_Status status = UTILS_OK;
    status = UTILS_WriteBit_Zone(&(adf4252_obj->_val_master), 10, 7, set_val);
    return status;
}

/*
 * @brief               设置噪声和毛刺抑制程度
 * @param adf4252_obj   adf4252 指定信息
 * @param set_val       设置的值
 * @return              UTILS_OK    : 没有问题
 *                      UTILS_ERROR : 内部传参有问题(与用户无关)
 * @note                set_val 只能用定义的宏!!!
 */
UTILS_Status ADF4252_NAS_Set(ADF4252_Info_Struct* adf4252_obj, uint8_t set_val) {
    UTILS_Status status = UTILS_OK;
    status = UTILS_WriteBit(&(adf4252_obj->_val_rf_control), 6, (set_val) & 0x01);
    status = UTILS_WriteBit(&(adf4252_obj->_val_rf_control), 11, (set_val) & 0x02);
    status = UTILS_WriteBit(&(adf4252_obj->_val_rf_control), 15, (set_val) & 0x04);
    return status;
}

/*
 * @brief               设置IF/RF的预分频器
 * @param adf4252_obj   adf4252 指定信息
 * @param set_val       设置的值
 * @return              UTILS_OK    : 没有问题
 *                      UTILS_ERROR : 内部传参有问题(与用户无关)
 * @note                set_val 只能用定义的宏!!!
 */
UTILS_Status ADF4252_Prescaler_Set(ADF4252_Info_Struct* adf4252_obj, uint8_t set_val) {
    UTILS_Status status = UTILS_OK;
    if ((set_val & 0xF0) != 0) {            // 设置IF的预分频器
        status = UTILS_WriteBit_Zone(&(adf4252_obj->_val_if_n_divider), 14, 13, (set_val & 0x0F));
    } 
    else {                                  // 设置RF的预分频器
        status = UTILS_WriteBit(&(adf4252_obj->_val_rf_r_divider), 20, (set_val & 0x0F));
    }
    return status;
}

/*
 * @brief               初始化ADF4252
 * @param adf4252_obj   adf4252 指定信息
 * @param spi           使用的spi结构体
 * @param le_pin        le引脚 
 * @param le_pin_type   le引脚的类型
 * @return              无
 */
void ADF4252_Init(ADF4252_Info_Struct* adf4252_obj, SPI_HandleTypeDef* spi, uint32_t le_pin, GPIO_TypeDef* le_pin_type) {
    //------------------------------把数据挂载到结构体上------------------------------
    adf4252_obj->spi = spi;
    adf4252_obj->le_pin = le_pin;
    adf4252_obj->le_pin_type = le_pin_type;

    //------------------------------初始化各个寄存器的数据------------------------------
    adf4252_obj->_val_rf_n_divider  = 0;                
    adf4252_obj->_val_rf_r_divider  = 1;
    adf4252_obj->_val_rf_control    = 2;
    adf4252_obj->_val_master        = 3;
    adf4252_obj->_val_if_n_divider  = 4;
    adf4252_obj->_val_if_r_divider  = 5;
    adf4252_obj->_val_if_control    = 6;

    //------------------------------使能引脚时钟------------------------------
    UTILS_RCC_GPIO_Enable(adf4252_obj->le_pin_type);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    //------------------------------配置le引脚------------------------------
    // 数据会在LE引脚的上升沿从转移寄存器打到锁存器中,所以初始话是高电平还是低电平无所谓
    HAL_GPIO_WritePin(adf4252_obj->le_pin_type, adf4252_obj->le_pin, GPIO_PIN_SET);     
    GPIO_InitStruct.Pin = adf4252_obj->le_pin;
    HAL_GPIO_Init(adf4252_obj->le_pin_type, &GPIO_InitStruct);
}
