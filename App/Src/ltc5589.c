#include "ltc5589.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_spi.h"
#include "utils.h"
#include <stdint.h>

//------------------------------------------------------------
// Created Time     : 2024.09.20
// Author           : JaydenLee
//------------------------------------------------------------

//------------------------------仅内部可用,外部不可用------------------------------
#define LTC5589_Transmit_Receive_Start()    HAL_GPIO_WritePin(ltc5589_obj->cs_pin_type, ltc5589_obj->cs_pin, GPIO_PIN_RESET);
#define LTC5589_Transmit_Receive_Stop()     HAL_GPIO_WritePin(ltc5589_obj->cs_pin_type, ltc5589_obj->cs_pin, GPIO_PIN_SET);
//------------------------------仅内部可用,外部不可用------------------------------


/*
 * @brief               写寄存器数据
 * @param ltc5589_obj   ltc5589 指定信息
 * @param reg_addr      需要写入的寄存器地址
 * @param tx_data       需要发送的数据
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 */
static UTILS_Status LTC5589_Write_Register(LTC5589_Info_Struct* ltc5589_obj, uint8_t reg_addr, uint8_t tx_data) {
    // 数据的发送和读取不使用DMA    
    UTILS_Status status = UTILS_OK;
    uint8_t packet[2];
    packet[0] = (reg_addr << 1) & 0xFE;             // 把地址装载并设置为写模式
    packet[1] = tx_data;                            // 装载数据
    LTC5589_Transmit_Receive_Start();
    if (HAL_SPI_Transmit(ltc5589_obj->spi, packet, 2, HAL_MAX_DELAY) != HAL_OK) 
        status = UTILS_ERROR;
    LTC5589_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               读寄存器数据
 * @param ltc5589_obj   ltc5589 指定信息
 * @param reg_addr      需要读的寄存器地址
 * @param rx_data       读取数据存放的地址
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 */
static UTILS_Status LTC5589_Read_Register(LTC5589_Info_Struct* ltc5589_obj, uint8_t reg_addr, uint8_t* rx_data) {
    // 数据的发送和读取不适用DMA
    UTILS_Status status = UTILS_OK;
    uint8_t packet[1];
    packet[0] = (reg_addr << 1) | 0x01;
    LTC5589_Transmit_Receive_Start();
    do {
        // 发送数据地址
        if (HAL_SPI_Transmit(ltc5589_obj->spi, packet, 1, HAL_MAX_DELAY) != HAL_OK) {
            status = UTILS_ERROR;
            break;
        }

        // 接收数据
        if (HAL_SPI_Receive(ltc5589_obj->spi, rx_data, 1, HAL_MAX_DELAY) != HAL_OK) {
            status = UTILS_ERROR;
            break; 
        }
    } while(0);
    LTC5589_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               更改指定位的数据
 * @param ltc5589_obj   ltc5589 指定信息
 * @param reg_addr      需要修改寄存器的地址
 * @param bit_pos       需要修改的数据的位置
 * @param bit_val       需要修改的值
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 */
static UTILS_Status LTC5589_WriteBit_Register(LTC5589_Info_Struct* ltc5589_obj, uint8_t reg_addr, uint8_t bit_pos, uint8_t bit_val) {
    UTILS_Status status = UTILS_OK;
    uint8_t rx_data;
    do {
        // 读取原本的数据
        if (LTC5589_Read_Register(ltc5589_obj, reg_addr, &rx_data) != UTILS_OK) {
            status = UTILS_ERROR;
            break;
        }

        // 更改对应对应bit的数值
        UTILS_WriteBit(&rx_data, bit_pos, bit_val);

        // 把数据打回之前的寄存器
        if (LTC5589_Write_Register(ltc5589_obj, reg_addr, rx_data) != UTILS_OK) {
            status = UTILS_ERROR;
            break;
        }
    } while(0);
    return status;
}

/*
 * @brief               更改指定区域的数据
 * @param ltc5589_obj   ltc5589 指定信息
 * @param reg_addr      需要修改寄存器的地址
 * @param msb           需要修改的数据的高位
 * @param lsb           需要修改的数据的地位
 * @param val           需要修改的值
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 */
static UTILS_Status LTC5589_WriteBit_Zone_Register(LTC5589_Info_Struct* ltc5589_obj, uint8_t reg_addr, uint8_t msb, uint8_t lsb, uint8_t val) {
    UTILS_Status status = UTILS_OK;
    uint8_t rx_data;
    do {
        // 读取原本的数据
        if (LTC5589_Read_Register(ltc5589_obj, reg_addr, &rx_data) != UTILS_OK) {
            status = UTILS_ERROR;
            break;
        }

        // 更改对应对应区域的数据
        UTILS_WriteBit_Zone(&rx_data, msb, lsb, val);

        // 把数据打回之前的寄存器
        if (LTC5589_Write_Register(ltc5589_obj, reg_addr, rx_data) != UTILS_OK) {
            status = UTILS_ERROR;
            break;
        }
    } while(0);
    return status;
}

/*
 * @brief               设置频率
 * @param ltc5589_obj   ltc5589 指定信息
 * @param freq          需要设置的频率
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 * @note                设置频率的寄存器地址为 0x00, 范围为bit0 ~ bit6
 */
UTILS_Status LTC5589_Set_Frequency(LTC5589_Info_Struct* ltc5589_obj, uint8_t freq) {
    // 判断数据是否符合要求
    if (freq > 127 || freq < 5)
        return UTILS_ERROR;

    UTILS_Status status = UTILS_OK;
    status = LTC5589_WriteBit_Zone_Register(ltc5589_obj, 0x00, 6, 0, freq);
    return status;
}

/*
 * @brief               设置粗调增益
 * @param ltc5589_obj   ltc5589 指定信息
 * @param gain          需要设置的增益大小, 范围为 -19dB ~ 0dB
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 * @note                设置粗调增益的寄存器地址为 0x01, 范围为bit0 ~ bit4
 */
UTILS_Status LTC5589_Set_DigitalGain_Coarse(LTC5589_Info_Struct* ltc5589_obj, int8_t gain) {
    if (gain > 0 || gain < -19)
        return UTILS_ERROR;
    UTILS_Status status = UTILS_OK;
    do {
        if (LTC5589_WriteBit_Zone_Register(ltc5589_obj, 0x01, 4, 0, -gain) != UTILS_OK) {
            status = UTILS_ERROR;
            break;
        }

        // 如果是同步细调的话给一个同步细调的信号
        if (ltc5589_obj->_digital_gain_fine_mode == LTC5589_FINE_SYNCHRONIZATION) {
            HAL_GPIO_WritePin(ltc5589_obj->ttck_pin_type, ltc5589_obj->ttck_pin, GPIO_PIN_SET);
            UTILS_Delay_us(100);
            HAL_GPIO_WritePin(ltc5589_obj->ttck_pin_type, ltc5589_obj->ttck_pin, GPIO_PIN_RESET);
        }
    } while(0);
    return status;
}

/*
 * @brief               设置细调增益
 * @param ltc5589_obj   ltc5589 指定信息
 * @param gain          需要设置的增益大小
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 * @note                设置细调增益的寄存器地址为 0x07, 范围为bit0 ~ bit3
 */
UTILS_Status LTC5589_Set_DigitalGain_Fine(LTC5589_Info_Struct* ltc5589_obj, int8_t gain) {
    UTILS_Status status = UTILS_OK;
    if (ltc5589_obj->_digital_gain_fine_mode == LTC5589_FINE_MANUAL) {
        status = LTC5589_WriteBit_Zone_Register(ltc5589_obj, 0x07, 3, 0, gain);
    }
    else
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               使能数字增益的自动微调
 * @param ltc5589_obj   ltc5589 指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 * @note                标志位 TEMPCORR 位于寄存器0x08的2号位
 */
UTILS_Status LTC5589_Set_DigitalGain_Fine_Mode(LTC5589_Info_Struct* ltc5589_obj, LTC5589_DIGITAL_GAIN_FINE_MODE mode) {
    UTILS_Status status = UTILS_OK;
    do {
        if (mode == LTC5589_FINE_MANUAL)                // 手动
            status = LTC5589_WriteBit_Register(ltc5589_obj, 0x08, 2, 1);
        else if (mode == LTC5589_FINE_ASYNCHRONOUS) {   // 异步
            if (LTC5589_WriteBit_Register(ltc5589_obj, 0x08, 2, 0) != UTILS_OK) {
                status = UTILS_ERROR;
                break;
            }
            status = LTC5589_WriteBit_Register(ltc5589_obj, 0x01, 7, 0);
        }
        else {                                          // 同步
            if (LTC5589_WriteBit_Register(ltc5589_obj, 0x08, 2, 0) != UTILS_OK) {
                status = UTILS_ERROR;
                break;
            }
            status = LTC5589_WriteBit_Register(ltc5589_obj, 0x01, 7, 0);
        }
    } while(0);
    if (status == UTILS_OK)
        ltc5589_obj->_digital_gain_fine_mode = mode;
    return status;
}

/*
 * @brief               设置LTC5589的直流偏置
 * @param ltc5589_obj   ltc5589 指定信息
 * @param channel       需要设置的通道
 * @param offset        需要设置的偏置的值
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 * @note                设置I通道的直流偏置的寄存器
 */
UTILS_Status LTC5589_Set_DCOffset(LTC5589_Info_Struct* ltc5589_obj, LTC5589_CHANNEL channel, uint8_t offset) {
    if (offset == 0)
        return UTILS_ERROR;

    UTILS_Status status = UTILS_OK;
    if (channel == LTC5589_CHANNEL_I) {
        status = LTC5589_Write_Register(ltc5589_obj, 0x02, offset);
    }
    else {
        status = LTC5589_Write_Register(ltc5589_obj, 0x03, offset);
    }
    return status;
}

/*
 * @brief               设置相位差
 * @param ltc5589_obj   ltc5589 指定信息
 * @param phi           需要设置的偏移角度值
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 * @note                IQPHE位于寄存器0x05中,范围为bit7~bit5,用于粗调
 *                      IQPHSIGN位于寄存器寄存器0x00中,位置为bit7,用于粗调的符号
 *                      IQPHF位于寄存器0x05中,范围为bit4~bit0,用于细调
 */
UTILS_Status LTC5589_Set_Phase(LTC5589_Info_Struct* ltc5589_obj, double phi) {
    UTILS_Status status = UTILS_OK;
    int32_t M_ph = (int32_t)(phi * (-15));
    uint8_t IQPHSIGN = 0;                                   // 这种情况下是 -32 * N_ext
    if (M_ph > 15)
        IQPHSIGN = 1;                                       // 这种情况下是 32 * N_ext
    uint8_t N_ph = M_ph % 16 + 16;                          // 细调的值
    uint8_t N_ext = M_ph / (IQPHSIGN == 0 ? -32 : 32);      // 粗调的值
    
    do {
        // 发送符号的值
        if (LTC5589_WriteBit_Register(ltc5589_obj, 0x00, 7, IQPHSIGN) != UTILS_OK) {
            status = UTILS_ERROR;
            break;
        }

        // 发送粗调的值
        if (LTC5589_WriteBit_Zone_Register(ltc5589_obj, 0x05, 7, 5, N_ext) != UTILS_OK) {
            status = UTILS_ERROR;
            break;
        }

        // 发送细调的值
        if (LTC5589_WriteBit_Zone_Register(ltc5589_obj, 0x05, 4, 0, N_ph) != UTILS_OK) {
            status = UTILS_ERROR;
            break;
        }

    } while(0);
    return status;
}

/*
 * @brief               使能Q通道
 * @param ltc5589_obj   ltc5589 指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 * @note                标志位 QDISABLE 位于寄存器0x01的5号位
 */
UTILS_Status LTC5589_Q_Channel_Enable(LTC5589_Info_Struct* ltc5589_obj) {
    UTILS_Status status = UTILS_OK;
    status = LTC5589_WriteBit_Register(ltc5589_obj, 0x01, 5, 0);
    return status;
}

/*
 * @brief               禁止Q通道
 * @param ltc5589_obj   ltc5589 指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 * @note                标志位 QDISABLE 位于寄存器0x01的5号位
 */
UTILS_Status LTC5589_Q_Channel_Disable(LTC5589_Info_Struct* ltc5589_obj) {
    UTILS_Status status = UTILS_OK;
    status = LTC5589_WriteBit_Register(ltc5589_obj, 0x01, 5, 1);
    return status;
}

/*
 * @brief               初始化LTC5589
 * @param ltc5589_obj   ltc5589 指定信息
 * @param spi           使用的spi结构体
 * @param cs_pin        片选引脚
 * @param cs_pin_type   片选引脚的GPIO类型
 * @param ttck_pin      ttck引脚
 * @param ttck_pin_type ttck引脚的GPIO类型
 * @return              无
 */
void LTC5589_Init(LTC5589_Info_Struct* ltc5589_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type, uint32_t ttck_pin, GPIO_TypeDef* ttck_pin_type) {
    //------------------------------把数据挂载到结构体上------------------------------
    ltc5589_obj->spi = spi;    
    ltc5589_obj->cs_pin = cs_pin;
    ltc5589_obj->cs_pin_type = cs_pin_type;
    ltc5589_obj->ttck_pin = ttck_pin;
    ltc5589_obj->ttck_pin_type = ttck_pin_type;

    //------------------------------使能引脚时钟------------------------------
    UTILS_RCC_GPIO_Enable(ltc5589_obj->cs_pin_type);
    UTILS_RCC_GPIO_Enable(ltc5589_obj->ttck_pin_type);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    //------------------------------配置CS引脚------------------------------
    HAL_GPIO_WritePin(ltc5589_obj->cs_pin_type, ltc5589_obj->cs_pin, GPIO_PIN_SET);           // 默认把电平拉高
    GPIO_InitStruct.Pin = ltc5589_obj->cs_pin;
    HAL_GPIO_Init(ltc5589_obj->cs_pin_type, &GPIO_InitStruct);

    //------------------------------配置TTCK引脚------------------------------
    HAL_GPIO_WritePin(ltc5589_obj->ttck_pin_type, ltc5589_obj->ttck_pin, GPIO_PIN_RESET);     // 默认把电平拉低
    GPIO_InitStruct.Pin = ltc5589_obj->ttck_pin;
    HAL_GPIO_Init(ltc5589_obj->ttck_pin_type, &GPIO_InitStruct);
}
