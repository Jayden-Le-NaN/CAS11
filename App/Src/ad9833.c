#include "ad9833.h"
#include "stm32l4xx_hal_spi.h"
#include "utils.h"

//------------------------------------------------------------
// Created Time     : 2024.10.28
// Author           : JaydenLee
//------------------------------------------------------------

//------------------------------仅内部使用, 外部不可用------------------------------
#define AD9833_Transmit_Strat()         HAL_GPIO_WritePin(ad9833_obj->fsync_pin_type, ad9833_obj->fsync_pin, GPIO_PIN_RESET)
#define AD9833_Transmit_Stop()          HAL_GPIO_WritePin(ad9833_obj->fsync_pin_type, ad9833_obj->fsync_pin, GPIO_PIN_SET)
//------------------------------仅内部使用, 外部不可用------------------------------

//------------------------------需要放入中断回调函数中的函数------------------------------
void AD9833_Transmit_IRQ_Handler(AD9833_Info_Struct* ad9833_obj, SPI_HandleTypeDef* spi) {
    if (ad9833_obj->spi == spi && ad9833_obj->_dma_fsm_state_transmit == AD9833_DMA_Transmiting) {
        ad9833_obj->_dma_fsm_state_transmit = AD9833_DMA_Idle;
        AD9833_Transmit_Stop();
    }
}
//------------------------------需要放入中断回调函数中的函数------------------------------

/*
 * @brief               使用阻塞的方式传输数组数据
 * @param ad9833_obj    ad9833 指定信息
 * @param tx_data       需要发送的数据
 * @param len           需要发送的数据长度
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
static UTILS_Status AD9833_Transmit_8bit_Array(AD9833_Info_Struct* ad9833_obj, uint8_t* tx_data, uint32_t len) {
    UTILS_Status status = UTILS_OK; 
    if (HAL_SPI_Transmit(ad9833_obj->spi, tx_data, len, HAL_MAX_DELAY) != HAL_OK)
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               使用DMA的方式传输数组数据
 * @param ad9833_obj    ad9833 指定信息
 * @param tx_data       需要发送的数据
 * @param len           需要发送的数据长度
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
static UTILS_Status AD9833_Transmit_8bit_Array_DMA(AD9833_Info_Struct* ad9833_obj, uint8_t* tx_data, uint32_t len) {
    if (ad9833_obj->_dma_fsm_state_transmit != AD9833_DMA_Idle) 
        return UTILS_ERROR;

    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Transmit_DMA(ad9833_obj->spi, tx_data, len) != HAL_OK)
        status = UTILS_ERROR;
    else
        ad9833_obj->_dma_fsm_state_transmit = AD9833_DMA_Transmiting;
    return status;
}

/*
 * @brief               写控制寄存器数据
 * @param ad9833_obj    ad9833 指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
static UTILS_Status AD9833_ControlRegisterWrite(AD9833_Info_Struct* ad9833_obj) {
    UTILS_Status status = UTILS_OK;
    uint8_t packet[2];
    packet[0] = ((AD9833_REG_CONTROL | ad9833_obj->_control_reg_data) & 0xFF00) >> 8;
    packet[1] = (ad9833_obj->_control_reg_data) & 0x00FF;
    AD9833_Transmit_Strat();
    status = AD9833_Transmit_8bit_Array(ad9833_obj, packet, 2);
    AD9833_Transmit_Stop();
    return status;
}

/*
 * @brief               写寄存器数据
 * @param ad9833_obj    ad9833 指定信息
 * @param reg           需要写入的寄存器数据
 * @param tx_data       需要发送的数据
 * @param len           数据的长度
 * @param tx_mode       数据发送的模式
 *                          UTILS_LOOP: 使用阻塞的方式接收
 *                          UTILS_DMA : 使用DMA的方式接收
 * @return              UTILS_OK    : 正常 
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输或者是传输模式选择错误
 * @note                关于参数len只能使用 sizeof(tx_data) 来传入数据, 不能是传入初始化时候的数组的大小
 *                      例如: uint16_t buff[512];
 *                      你应该传入 sizeof(buff);
 *                      而不是传入 512
 *                      因为 sizeof(buff) == 1024; --> 需要的是字节的数目
 */
static UTILS_Status AD9833_RegisterWrite(AD9833_Info_Struct* ad9833_obj, uint16_t reg, uint16_t* tx_data, uint32_t len, UTILS_CommunicationMode tx_mode) {
    UTILS_Status status = UTILS_OK;
    uint16_t data[len / 2];
    for (uint32_t i = 0; i < len / 2; ++i) 
        data[i] = tx_data[i] | reg;

    AD9833_Transmit_Strat();
    if (tx_mode == UTILS_LOOP) {
        status = AD9833_Transmit_8bit_Array(ad9833_obj, (uint8_t*)data, len);
        AD9833_Transmit_Stop();
    }
    else if (tx_mode == UTILS_DMA) {
        status = AD9833_Transmit_8bit_Array_DMA(ad9833_obj, (uint8_t*)data, len);
    }
    else 
        status = UTILS_ERROR;
    return status;
}


/*
 * @brief               设置写频率寄存器的模式
 * @param freq_set_mode 频率设置的模式
 * @return              UTILS_OK    : 正常 
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
UTILS_Status AD9833_FrequencySetMode(AD9833_Info_Struct* ad9833_obj, uint8_t freq_set_mode) {
    UTILS_Status status = UTILS_OK;
    if (freq_set_mode == AD9833_FREQ_ALL) {
        UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 13, 1);
        status = AD9833_ControlRegisterWrite(ad9833_obj);
    }
    else if (freq_set_mode == AD9833_FREQ_MSB) {
        UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 13, 0);
        UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 12, 1);
        status = AD9833_ControlRegisterWrite(ad9833_obj);
    }
    else if (freq_set_mode == AD9833_FREQ_LSB) {
        UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 13, 0);
        UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 12, 0);
        status = AD9833_ControlRegisterWrite(ad9833_obj);
    }
    else if (freq_set_mode == AD9833_FREQ_NO_CHANGE) {
        status = UTILS_OK;
    }
    else {
        status = UTILS_ERROR;
    }
    return status;
}


/*
 * @brief               频率输出选择
 * @param ad9833_obj    ad9833 指定信息
 * @param freq_out_sel  频率输出选择
 * @return              UTILS_OK    : 正常 
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
UTILS_Status AD9833_FrequencyOutSelect(AD9833_Info_Struct* ad9833_obj, uint8_t freq_out_sel) {
    UTILS_Status status = UTILS_OK;
    if (freq_out_sel == AD9833_OUT_FREQ0) {
        status = UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 11, 0);
    }
    else if (freq_out_sel == AD9833_OUT_FREQ1) {
        status = UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 11, 1);
    }
    else {
        status = UTILS_ERROR;
    }
    return status;
}

/*
 * @brief               相位输出选择
 * @param ad9833_obj    ad9833 指定信息
 * @param phase_out_sel 相位输出选择
 * @return              UTILS_OK    : 正常 
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
UTILS_Status AD9833_PhaseOutSelect(AD9833_Info_Struct* ad9833_obj, uint8_t phase_out_sel) {
    UTILS_Status status = UTILS_OK;
    if (phase_out_sel == AD9833_OUT_PHASE0) {
        status = UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 10, 0);
    }
    else if (phase_out_sel == AD9833_OUT_PHASE1) {
        status = UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 10, 0);
    }
    else {
        status = UTILS_ERROR;
    }
    return status;
}

/*
 * @brief               设置频率
 * @param ad9833_obj    ad9833 指定信息
 * @param freq_reg      选择写入的频率寄存器
 * @param freq_set_mode 频率设置的模式
 * @param freq          设置的频率
 * @param len           数据的长度
 * @param tx_mode       数据发送的模式
 *                          UTILS_LOOP: 使用阻塞的方式接收
 *                          UTILS_DMA : 使用DMA的方式接收
 *
 * @return              UTILS_OK    : 正常 
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输或者是传输模式选择错误
 * @note                关于参数len只能使用 sizeof(tx_data) 来传入数据, 不能是传入初始化时候的数组的大小
 *                      例如: uint32_t buff[256];
 *                      你应该传入 sizeof(buff);
 *                      而不是传入 256
 *                      因为 sizeof(buff) == 1024; --> 需要的是字节的数目
 */
UTILS_Status AD9833_SetFrequency(AD9833_Info_Struct* ad9833_obj, uint16_t freq_reg, uint8_t freq_set_mode, uint32_t* freq, uint32_t len, UTILS_CommunicationMode tx_mode) {
    UTILS_Status status = UTILS_OK;    
    do {
        //------------------------------频率设置模式选择------------------------------
        if (freq_set_mode == AD9833_FREQ_ALL) {
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 13, 1);
            status = AD9833_ControlRegisterWrite(ad9833_obj);
        }
        else if (freq_set_mode == AD9833_FREQ_MSB) {
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 13, 0);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 12, 1);
            status = AD9833_ControlRegisterWrite(ad9833_obj);
        }
        else if (freq_set_mode == AD9833_FREQ_LSB) {
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 13, 0);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 12, 0);
            status = AD9833_ControlRegisterWrite(ad9833_obj);
        }
        else if (freq_set_mode == AD9833_FREQ_NO_CHANGE) {
            status = UTILS_OK;
        }
        else {
            status = UTILS_ERROR;
            break;
        }
        
        //------------------------------控制寄存器设置状态检测------------------------------
        if (status != UTILS_OK)
            break;
            
        //------------------------------持续打入数据------------------------------
        uint16_t tx_data[len / 2];
        uint32_t tx_len = len;
        if (((ad9833_obj->_control_reg_data >> 13) & 0x01) == 1) {          // AD9833_FREQ_ALL
            for (uint32_t i = 0; i < len / 2; ++i)
                tx_data[i] = *((uint16_t*)freq + i);
            tx_len = len;    
        }
        else {
            if (((ad9833_obj->_control_reg_data >> 12) & 0x01) == 1) {      // AD9833_FREQ_MSB
                for (uint32_t i = 0; i < len / 2; i += 2)
                    tx_data[i / 2] = *((uint16_t*)freq + i);
                tx_len = len / 2;
            }
            else {                                                          // AD9833_FREQ_LSB
                for (uint32_t i = 0; i < len / 2; i += 2)
                    tx_data[i / 2] = *((uint16_t*)freq + i + 1);
                tx_len = len / 2;
            }
        }
        status = AD9833_RegisterWrite(ad9833_obj, freq_reg, tx_data, tx_len, tx_mode);
    } while(0);
    return status;
}

/*
 * @brief               设置相位
 * @param ad9833_obj    ad9833 指定信息
 * @param phase_reg     需要设置的相位寄存器
 * @param phase         需要设置的相位
 * @param len           数据的长度
 * @param tx_mode       数据发送的模式
 *                          UTILS_LOOP: 使用阻塞的方式接收
 *                          UTILS_DMA : 使用DMA的方式接收
 * @return              UTILS_OK    : 正常 
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输或者是传输模式选择错误
 * @note                关于参数len只能使用 sizeof(tx_data) 来传入数据, 不能是传入初始化时候的数组的大小
 *                      例如: uint32_t buff[256];
 *                      你应该传入 sizeof(buff);
 *                      而不是传入 256
 *                      因为 sizeof(buff) == 1024; --> 需要的是字节的数目
 */
UTILS_Status AD9833_SetPhase(AD9833_Info_Struct* ad9833_obj, uint16_t phase_reg, uint16_t* phase, uint32_t len, UTILS_CommunicationMode tx_mode) {
    UTILS_Status status = UTILS_OK;    
    status = AD9833_RegisterWrite(ad9833_obj, phase_reg, phase, len, tx_mode);
    return status;
}

/*
 * @brief               设置输出的波形
 * @param ad9833_obj    ad9833 指定信息
 * @param wave_mode     设置的波形
 * @return              UTILS_OK    : 正常 
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是波形选择错误
 * @note                寄存器的位置如下
                            OPBITEN bit  : D5
                            MODE bit     : D1
                            DIV2 bit     : D3
 */
UTILS_Status AD9833_SetWave(AD9833_Info_Struct* ad9833_obj, uint16_t wave_mode) {
    UTILS_Status status = UTILS_OK;
    do {
        if (wave_mode == AD9833_WAVE_SINUSOID) {
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 5, 0);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 1, 0);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 3, 0);
        }
        else if (wave_mode == AD9833_WAVE_UP_DOWN_RAMP) {
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 5, 0);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 1, 1);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 3, 0);
        }
        else if (wave_mode == AD9833_WAVE_DAC_DATA_MSB_HALF) {
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 5, 1);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 1, 0);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 3, 0);
        }
        else if (wave_mode == AD9833_WAVE_DAC_DATA_MSB) {
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 5, 1);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 1, 0);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 3, 1);
        }
        else {
            status = UTILS_ERROR;
            break;
        }
        status = AD9833_ControlRegisterWrite(ad9833_obj);
    } while(0);
    return status;
}


/*
 * @brief               设置睡眠模式
 * @param ad9833_obj    ad9833 指定信息
 * @param sleep_mode    设置的睡眠模式
 * @return              UTILS_OK    : 正常 
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是波形选择错误
 * @note                寄存器的位置如下
                            SLEEP1 bit  : D7
                            SLEEP2 bit  : D6
 */
UTILS_Status AD9833_Sleep(AD9833_Info_Struct* ad9833_obj, uint16_t sleep_mode) {
    UTILS_Status status = UTILS_OK;
    do {
        if (sleep_mode == AD9833_SLEEP_NO_PWERDOWN) {
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 7, 0);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 6, 0);
        }
        else if (sleep_mode == AD9833_SLEEP_DAC_POWERDOWN) {
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 7, 0);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 6, 1);
        }
        else if (sleep_mode == AD9833_SLEEP_INTERNAL_CLOCK_POWERDOWN) {
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 7, 1);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 6, 0);
        }
        else if (sleep_mode == AD9833_SLEEP_ALL_POWERDOWN) {
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 7, 1);
            UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 6, 1);
        }
        else {
            status = UTILS_ERROR;
            break;
        }
        status = AD9833_ControlRegisterWrite(ad9833_obj);
    } while(0);
    return status;
}

/*
 * @brief               内部寄存器设置为0
 * @param ad9833_obj    ad9833 指定信息
 * @return              UTILS_OK    : 正常 
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是波形选择错误
 * @note                寄存器的位置如下
                            RESET bit  : D8
 */
UTILS_Status AD9833_Reset(AD9833_Info_Struct* ad9833_obj) {
    UTILS_Status status = UTILS_OK;
    UTILS_WriteBit(&(ad9833_obj->_control_reg_data), 8, 1);
    status = AD9833_ControlRegisterWrite(ad9833_obj);
    return status;
}

/*
 * @brief               判断AD9833传输是否空闲
 * @param ad9833_obj    ad9833 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_WORKING   : 正在传输数据
 */
UTILS_Status AD9833_Transmit_Is_Idle(AD9833_Info_Struct* ad9833_obj) {
    UTILS_Status status = UTILS_OK; 
    if (ad9833_obj->_dma_fsm_state_transmit == AD9833_DMA_Transmiting)
        status = UTILS_WORKING;
    return status;
}


/*
 * @brief               初始化一个ad9833硬件设备的配置信息
 * @param ad9833_obj    ad9833 指定信息
 * @param spi           使用的spi结构体
 * @param fsync_pin     片选引脚
 * @param fsync_pin_type片选引脚的GPIO类型
 * @return              无
 */
void AD9833_Init(AD9833_Info_Struct* ad9833_obj, SPI_HandleTypeDef* spi, uint32_t fsync_pin, GPIO_TypeDef* fsync_pin_type) {
    //------------------------------数据挂载------------------------------
    ad9833_obj->spi = spi;
    ad9833_obj->fsync_pin = fsync_pin;
    ad9833_obj->fsync_pin_type = fsync_pin_type;

    //------------------------------默认数据处理------------------------------
    ad9833_obj->_dma_fsm_state_transmit = AD9833_DMA_Idle;
    ad9833_obj->_control_reg_data = 0x0;

    //------------------------------配置FSYNC引脚------------------------------
    AD9833_Transmit_Stop();
    UTILS_RCC_GPIO_Enable(ad9833_obj->fsync_pin_type);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = ad9833_obj->fsync_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ad9833_obj->fsync_pin_type, &GPIO_InitStruct);
}
