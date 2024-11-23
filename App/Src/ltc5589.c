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
UTILS_Status LTC5589_Read_Register(LTC5589_Info_Struct* ltc5589_obj, uint8_t reg_addr, uint8_t* rx_data) {
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
    ltc5589_obj->_freq = freq;
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
        else 
            ltc5589_obj->_coarse_dg = gain;

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
                break; }
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
 * @note                设置I/Q通道的直流偏置的寄存器
 */
UTILS_Status LTC5589_Set_DCOffset(LTC5589_Info_Struct* ltc5589_obj, LTC5589_CHANNEL channel, uint8_t offset) {
    if (offset == 0)
        return UTILS_ERROR;

    UTILS_Status status = UTILS_OK;
    if (channel == LTC5589_CHANNEL_I) {
        ltc5589_obj->_i_dc_offset = offset;
        status = LTC5589_Write_Register(ltc5589_obj, 0x02, offset);
    }
    else {
        ltc5589_obj->_q_dc_offset = offset;
        status = LTC5589_Write_Register(ltc5589_obj, 0x03, offset);
    }
    return status;
}

/*
 * @brief               设置I/Q增益的比值
 * @param ltc5589_obj   ltc5589 指定信息
 * @param ratio         I/Q增益比值
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 可能是总线正在被占用等一系列问题
 * @note                I/Q增益比值的寄存器为 0x04
 */
UTILS_Status LTC5589_Set_IQ_GainRatio(LTC5589_Info_Struct* ltc5589_obj, uint8_t ratio) {
    UTILS_Status status = UTILS_OK;
    status = LTC5589_Write_Register(ltc5589_obj, 0x04, ratio);
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

        ltc5589_obj->_phase = phi;
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

    //------------------------------初始化数据------------------------------
    ltc5589_obj->_freq = 0x3E;
    ltc5589_obj->_phase = 0;
    ltc5589_obj->_i_dc_offset = 0;
    ltc5589_obj->_q_dc_offset = 0;
    ltc5589_obj->_coarse_dg = -4;
    

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

/*
 * @brief               enable 指令的回调函数(没有写)
 * @param ltc5589_obj   ltc5589 结构体
 * @param recvbuf       数据缓冲区
 * @param len           数据缓冲区的长度
 * @return              AT_RES_OK           : 参数设置完成
 *                      AT_RES_NO_CMD       : 缓冲区中没有找到该指令
 *                      AT_RES_PARAM_ERROR  : 缓冲去中有该指令,但是参数设置不符合规范
 */
static at_response LTC5589_CMD_Enable_handler(LTC5589_Info_Struct* ltc5589_obj, char* recvbuf, uint32_t len) {
    // TODO: tbd
    at_response res = AT_RES_NO_CMD;
    return res; 
}


/*
 * @brief               Phase 指令的回调函数
 * @param ltc5589_obj   ltc5589 结构体
 * @param recvbuf       数据缓冲区
 * @param len           数据缓冲区的长度
 * @return              AT_RES_OK           : 参数设置完成
 *                      AT_RES_NO_CMD       : 缓冲区中没有找到该指令
 *                      AT_RES_PARAM_ERROR  : 缓冲去中有该指令,但是参数设置不符合规范
 * @note                命令格式: Phase, num (double), num 范围 -240 ~ 239
 */
static at_response LTC5589_CMD_Phase_handler(LTC5589_Info_Struct* ltc5589_obj, char* recvbuf, uint32_t len) {
    at_response res = AT_RES_OK; 
    do {
        // 查找是否有 DG 指令
        char* cmd_pos = strstr(recvbuf, "Phase");
        if (cmd_pos == NULL) {
            res = AT_RES_NO_CMD;
            break;
        }
        
        // 查找数据
        char* comma_pos = strchr(cmd_pos, ',');

        // 获取数据
        double param = atof(comma_pos + 1);            

        // 验证数据合法性
        if (param < -240 || param > 239) {
            res = AT_RES_PARAM_ERROR;
            break;
        }

        // 设置数据
        LTC5589_Set_Phase(ltc5589_obj, param);

    } while(0);

    return res;
}

/*
 * @brief               DG 指令的回调函数
 * @param ltc5589_obj   ltc5589 结构体
 * @param recvbuf       数据缓冲区
 * @param len           数据缓冲区的长度
 * @return              AT_RES_OK           : 参数设置完成
 *                      AT_RES_NO_CMD       : 缓冲区中没有找到该指令
 *                      AT_RES_PARAM_ERROR  : 缓冲去中有该指令,但是参数设置不符合规范
 * @note                命令格式: DG, num(int),  num 范围 -19 ~ 0
 */
static at_response LTC5589_CMD_DG_handler(LTC5589_Info_Struct* ltc5589_obj, char* recvbuf, uint32_t len) {
    at_response res = AT_RES_OK;
    do {
        // 查找是否有 DG 指令
        char* cmd_pos = strstr(recvbuf, "DG");
        if (cmd_pos == NULL) {
            res = AT_RES_NO_CMD;
            break;
        }
        
        // 查找数据
        char* comma_pos = strchr(cmd_pos, ',');

        // 获取数据
        int param = atoi(comma_pos + 1);            

        // 验证数据合法性
        if (param > 0 || param < -19) {
            res = AT_RES_PARAM_ERROR;
            break;
        }

        // 设置数据
        LTC5589_Set_DigitalGain_Coarse(ltc5589_obj, param);

    } while(0);
    return res;
}

/*
 * @brief               DCOffset 指令的回调函数
 * @param ltc5589_obj   ltc5589 结构体
 * @param recvbuf       数据缓冲区
 * @param len           数据缓冲区的长度
 * @return              AT_RES_OK           : 参数设置完成
 *                      AT_RES_NO_CMD       : 缓冲区中没有找到该指令
 *                      AT_RES_PARAM_ERROR  : 缓冲去中有该指令,但是参数设置不符合规范
 * @note                命令格式: DCOffset, I/Q, num(int),  num 范围 1 ~ 255
 */
static at_response LTC5589_CMD_DCOffset_handler(LTC5589_Info_Struct* ltc5589_obj, char* recvbuf, uint32_t len) {
    at_response res = AT_RES_OK; 

    do {
        // 查找是否有 DCOffset 指令
        char* cmd_pos = strstr(recvbuf, "DCOffset");
        if (cmd_pos == NULL) {
            res = AT_RES_NO_CMD;
            break;
        }
        
        char* comma_1_pos = strchr(cmd_pos, ',');
        char* comma_2_pos = strchr(comma_1_pos + 1, ',');
        size_t length = comma_2_pos - comma_1_pos;

        LTC5589_CHANNEL channel;
        // 检查是 I通道 直流偏置 还是 Q通道 直流偏置
        if (memchr(comma_1_pos, 'I', length) != NULL) {
            channel = LTC5589_CHANNEL_I; 
        }
        else if(memchr(comma_1_pos, 'Q', length) != NULL) {
            channel = LTC5589_CHANNEL_Q; 
        }
        else {
            res = AT_RES_PARAM_ERROR;
            break;
        }
    
        // 获取偏置数据
        int param = atoi(comma_2_pos + 1);

        if (param < 1 || param > 255) {
            res = AT_RES_PARAM_ERROR;
            break;
        }

        LTC5589_Set_DCOffset(ltc5589_obj, channel, param);
    } while(0);
    return res;
}

/*
 * @brief               Freq 指令的回调函数
 * @param ltc5589_obj   ltc5589 结构体
 * @param recvbuf       数据缓冲区
 * @param len           数据缓冲区的长度
 * @return              AT_RES_OK           : 参数设置完成
 *                      AT_RES_NO_CMD       : 缓冲区中没有找到该指令
 *                      AT_RES_PARAM_ERROR  : 缓冲去中有该指令,但是参数设置不符合规范
 * @note                命令格式: Freq, num(int),  num 范围 5 ~ 127
 */
static at_response LTC5589_CMD_Freq_Handler(LTC5589_Info_Struct* ltc5589_obj, char* recvbuf, uint32_t len) {
    at_response res = AT_RES_OK;

    do {
        // 查找是否有 Freq 指令
        char* cmd_pos = strstr(recvbuf, "Freq");
        if (cmd_pos == NULL) {
            res = AT_RES_NO_CMD;
            break;
        }
        
        // 查找数据
        char* comma_pos = strchr(cmd_pos, ',');

        // 获取数据
        int param = atoi(comma_pos + 1);            

        // 验证数据合法性
        if (param > 127 || param < 5) {
            res = AT_RES_PARAM_ERROR;
            break;
        }

        // 设置数据
        LTC5589_Set_Frequency(ltc5589_obj, param);

    } while(0);
    return res;
}

/*
 * @brief               AT 指令回调函数
 * @param ltc5589_obj   ltc5589 结构体
 * @param at            at 结构体
 * @param recvbuf       数据接收缓冲区
 * @param len           缓冲区中有效数据的长度
 * @return              无
 */
void LTC5589_AT_Handler(void* ltc5589_obj, at_obj_t* at, char* recvbuf, int32_t len) {
    LTC5589_Info_Struct* obj = (LTC5589_Info_Struct*)ltc5589_obj;

    //------------------------------AT+5589?指令------------------------------
    if (strncmp(recvbuf, "AT+5589?", sizeof("AT+5589?") - 1) == 0) {
        char buff[256];
        int param_len;
        at_send_data(at, "\r\n+5589: Param Value: \r\n", sizeof("\r\n+5589: Param Value: \r\n"));

        //------------------------------处理直流偏置(I)------------------------------
        param_len = snprintf(buff, sizeof(buff), "%d", obj->_i_dc_offset);
        at_send_data(at, "+5589: I DC Offset: ", sizeof("+5589: I DC Offset: "));
        at_send_data(at, buff, param_len);
        at_send_data(at, "\r\n", sizeof("\r\n"));

        //------------------------------处理直流偏置(Q)------------------------------
        param_len = snprintf(buff, sizeof(buff), "%d", obj->_q_dc_offset);
        at_send_data(at, "+5589: Q DC Offset: ", sizeof("+5589: Q DC Offset: "));
        at_send_data(at, buff, param_len);
        at_send_data(at, "\r\n", sizeof("\r\n"));

        //------------------------------处理直流增益------------------------------
        param_len = snprintf(buff, sizeof(buff), "%d", obj->_coarse_dg);
        at_send_data(at, "+5589: Coarse DG: ", sizeof("+5589: Coarse DG: "));
        at_send_data(at, buff, param_len);
        at_send_data(at, "\r\n", sizeof("\r\n"));

        //------------------------------处理相位------------------------------
        param_len = snprintf(buff, sizeof(buff), "%d", (int32_t)obj->_phase);
        at_send_data(at, "+5589: Phase: ", sizeof("+5589: Phase: "));
        at_send_data(at, buff, param_len);
        at_send_data(at, "\r\n", sizeof("\r\n"));

        //------------------------------处理频率------------------------------
        param_len = snprintf(buff, sizeof(buff), "%d", obj->_freq);
        at_send_data(at, "+5589: Freq: ", sizeof("+5589: Freq: "));
        at_send_data(at, buff, param_len);
        at_send_data(at, "\r\n", sizeof("\r\n"));
    }
    else if (strncmp(recvbuf, "AT+5589=", sizeof("AT+5589=") - 1) == 0) {
        at_send_data(at, "\n", sizeof("\n"));

        //------------------------------查询指令------------------------------
        at_response en_cmd_res = LTC5589_CMD_Enable_handler(ltc5589_obj, recvbuf, len);
        at_response phase_cmd_res = LTC5589_CMD_Phase_handler(ltc5589_obj, recvbuf, len);
        at_response dg_cmd_res = LTC5589_CMD_DG_handler(ltc5589_obj, recvbuf, len);
        at_response dc_offset_cmd_res = LTC5589_CMD_DCOffset_handler(ltc5589_obj, recvbuf, len);
        at_response freq_cmd_res = LTC5589_CMD_Freq_Handler(ltc5589_obj, recvbuf, len);
        
        //------------------------------Enable 指令反馈------------------------------
        if (en_cmd_res == AT_RES_PARAM_ERROR) {
            at_send_data(at, "+5589: Enable cmd param error\r\n", sizeof("+5589: Enable cmd param error\r\n"));
        }
        else if (en_cmd_res == AT_RES_OK) {
            at_send_data(at, "+5589: Enable cmd set ok\r\n", sizeof("+5589: Enable cmd set ok\r\n"));
        }

        //------------------------------Phase 指令反馈------------------------------
        if (phase_cmd_res == AT_RES_PARAM_ERROR) {
            at_send_data(at, "+5589: Phase cmd param error\r\n", sizeof("+5589: Phase cmd param error\r\n"));
        }
        else if (phase_cmd_res == AT_RES_OK) {
            at_send_data(at, "+5589: Pahse cmd set ok\r\n", sizeof("+5589: Pahse cmd set ok\r\n"));
        }

        //------------------------------DG(数字增益) 指令反馈------------------------------
        if (dg_cmd_res == AT_RES_PARAM_ERROR) {
            at_send_data(at, "+5589: DG cmd param error \r\n", sizeof("+5589: DG cmd param error \r\n"));
        }
        else if (dg_cmd_res == AT_RES_OK) {
            at_send_data(at, "+5589: DG cmd set ok\r\n", sizeof("+5589: DG cmd set ok\r\n"));
        }

        //------------------------------DCOffset 指令反馈------------------------------
        if (dc_offset_cmd_res == AT_RES_PARAM_ERROR) {
            at_send_data(at, "+5589: DCOffset cmd param error\r\n", sizeof("+5589: DCOffset cmd param error\r\n"));
        }
        else if (dc_offset_cmd_res == AT_RES_OK) {
            at_send_data(at, "+5589: DCOffset cmd set ok\r\n", sizeof("+5589: DCOffset cmd set ok\r\n"));
        }

        //------------------------------Freq 指令反馈------------------------------
        if (freq_cmd_res == AT_RES_PARAM_ERROR) {
            at_send_data(at, "+5589: Freq cmd param error\r\n", sizeof("+5589: Freq cmd param error\r\n"));
        }
        else if (freq_cmd_res == AT_RES_OK) {
            at_send_data(at, "+5589: Freq cmd set ok\r\n", sizeof("+5589: Freq cmd set ok\r\n"));
        }
    }
    else if (strncmp(recvbuf, "AT+5589:Help", sizeof("AT+5589:Help") - 1) == 0) {
        at_send_data(at, "\r\n+5589: CMD Format:\r", sizeof("\r\n+5589: CMD Format:\r"));
        at_send_data(at, "+5589: Enable, 0/1\r", sizeof("+5589: Enable, 0/1\r"));
        at_send_data(at, "+5589: Pahse, num(double)\r", sizeof("+5589: Pahse, num(double)\r"));
        at_send_data(at, "+5589: DG, num\r", sizeof("+5589: DG, num\r"));
        at_send_data(at, "+5589: DCOffset, I/Q, num\r", sizeof("+5589: \"DCOffset\", num\r"));
        at_send_data(at, "+5589: Freq, num\n", sizeof("+5589: Freq, num\n"));
    }
    else if (strncmp(recvbuf, "AT+5589:Hello", sizeof("AT+5589:Hello") - 1) == 0) {
        at_send_data(at, "\r\nJaydenLee: Hi\r\n", sizeof("\r\nJaydenLee: Hi\r\n"));
    }
    else {
        at_send_data(at, "\r\n+5589: CMD Error\r\n", sizeof("\r\n+5589: CMD Error\r\n"));
    }
}

