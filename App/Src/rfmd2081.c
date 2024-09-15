#include "rfmd2081.h"

//------------------------------通信函数------------------------------
/*
 * @brief               使能数据写入
 * @return              无
 * @author              Chosifew & JaydenLe
 */
static void RFMD2081_ENX_Enable(void) {
    HAL_GPIO_WritePin(RFMD2081_ENX_GPIO_TYPE, RFMD2081_ENX_GPIO_PIN, GPIO_PIN_RESET);
    int32_t delay_us = UTILS_Ceil((double)1000000 / (RFMD2081_CLK_FREQUENCY * 2));
    delay_us = delay_us >= 0 ? delay_us : 0;
    UTILS_Delay_us(delay_us);
}

/*
 * @brief               失能数据写入
 * @return              无
 * @author              Chosifew & JaydenLe
 */
static void RFMD2081_ENX_Disable(void) {
    HAL_GPIO_WritePin(RFMD2081_ENX_GPIO_TYPE, RFMD2081_ENX_GPIO_PIN, GPIO_PIN_SET);
}

/*
 * @brief               更改数据线的状态
 * @param mode          数据线的状态:
 *                          GPIO_MODE_OUTPUT_PP - 输出状态
 *                          GPIO_MODE_INPUT     - 输入状态
 * @return              无
 * @author              Chosifew & JaydenLe
 */
static void RFMD2081_ChangeDataLineMode(uint32_t mode) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode = mode;
    GPIO_InitStruct.Pin = RFMD2081_DATA_GPIO_PIN;
    HAL_GPIO_Init(RFMD2081_DATA_GPIO_TYPE, &GPIO_InitStruct);
}

/*
 * @brief               提供一拍时钟
 * @return              无
 * @author              Chosifew & JaydenLe
 */
static void RFMD2081_EmptyCLK(void) {
    int32_t delay_us = UTILS_Ceil((double)1000000 / (RFMD2081_CLK_FREQUENCY * 2));
    delay_us = delay_us >= 0 ? delay_us : 0;

    // 提供一拍时钟
    UTILS_Delay_us(delay_us);
    HAL_GPIO_WritePin(RFMD2081_CLK_GPIO_TYPE, RFMD2081_CLK_GPIO_PIN, GPIO_PIN_SET);
    UTILS_Delay_us(delay_us);
    HAL_GPIO_WritePin(RFMD2081_CLK_GPIO_TYPE, RFMD2081_CLK_GPIO_PIN, GPIO_PIN_RESET);
}

/*
 * @brief               写入一个bit数据
 * @param bit           需要写入的比特
 * @return              无
 * @author              Chosifew & JaydenLe
 */
static void RFMD2081_SendBit(uint8_t bit) {
    int32_t delay_us = UTILS_Ceil((double)1000000 / (RFMD2081_CLK_FREQUENCY * 2));
    delay_us = delay_us >= 0 ? delay_us : 0;

#if RFMD2081_THREE_WIRE_BUS                             // 三线模式
    // 放数据
    HAL_GPIO_WritePin(RFMD2081_DATA_GPIO_TYPE, RFMD2081_DATA_GPIO_PIN, (bit == 0) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    UTILS_Delay_us(delay_us);

    // 提供一拍时钟
    HAL_GPIO_WritePin(RFMD2081_CLK_GPIO_TYPE, RFMD2081_CLK_GPIO_PIN, GPIO_PIN_SET);
    UTILS_Delay_us(delay_us);
    HAL_GPIO_WritePin(RFMD2081_CLK_GPIO_TYPE, RFMD2081_CLK_GPIO_PIN, GPIO_PIN_RESET);

#endif
}

/*
 * @brief               接收1bit数据
 * @return              0: 高电平, 1: 低电平
 * @author              Chosifew & JaydenLe
 */
static uint16_t RFMD2081_ReceiveBit(void) {
    int32_t delay_us = UTILS_Ceil((double)1000000 / (RFMD2081_CLK_FREQUENCY * 2));
    delay_us = delay_us >= 0 ? delay_us : 0;
    GPIO_PinState pin_state = GPIO_PIN_RESET;

#if RFMD2081_THREE_WIRE_BUS                             // 三线模式
    // 拼接时钟
    UTILS_Delay_us(delay_us);

    // 提供一拍时钟
    HAL_GPIO_WritePin(RFMD2081_CLK_GPIO_TYPE, RFMD2081_CLK_GPIO_PIN, GPIO_PIN_SET);
    UTILS_Delay_us(delay_us);
    HAL_GPIO_WritePin(RFMD2081_CLK_GPIO_TYPE, RFMD2081_CLK_GPIO_PIN, GPIO_PIN_RESET);

    // 读取数据
    pin_state = HAL_GPIO_ReadPin(RFMD2081_DATA_GPIO_TYPE, RFMD2081_DATA_GPIO_PIN);
#endif
    return (pin_state == GPIO_PIN_RESET ? 0 : 1);
}

/*
 * @brief               发送地址
 * @param mode          模式选择: RFMD2081_DATA_READ, RFMD2081_DATA_WRITE
 * @param addr          需要写入的地址
 * @return              无
 * @author              Chosifew & JaydenLe
 */
static void RFMD2081_SendAddr(RFMD2081_DATA_MODE mode, uint8_t addr) {
    RFMD2081_ChangeDataLineMode(GPIO_MODE_OUTPUT_PP);
    // 发送一个无意义数据
    RFMD2081_SendBit(0);
    // 发送读写模式
    RFMD2081_SendBit(mode);
    // 发送地址
    for (int8_t i = 6; i >= 0; --i)
        RFMD2081_SendBit((addr >> i) & 0x01);
}

/*
 * @brief               发送数据
 * @param data          需要发送的数据
 * @return              无
 * @author              Chosifew & JaydenLe
 */
static void RFMD2081_SendData(uint16_t data) {
    RFMD2081_ChangeDataLineMode(GPIO_MODE_OUTPUT_PP);
    // 发送数据
    for (int8_t i = 15; i >= 0; --i)
        RFMD2081_SendBit((data >> i) & 0x01);
}

/*
 * @brief               接收数据
 * @return              需要接收的数据
 * @author              Chosifew & JaydenLe
 */
static uint16_t RFMD2081_ReceiveData(void) {
    RFMD2081_ChangeDataLineMode(GPIO_MODE_INPUT);
    uint16_t ret_data = 0;
    for (int8_t i = 15; i >= 0; --i)
        ret_data |= RFMD2081_ReceiveBit() << i;
    return ret_data;
}


/*
 * @brief               写数据
 * @param addr          需要写数据的寄存器地址
 * @param data          需要写入的数据
 * @return              无
 * @author              Chosifew & JaydenLe
 */
void RFMD2081_Write(uint8_t addr, uint16_t data) {
    RFMD2081_ENX_Enable(); 
    RFMD2081_SendAddr(RFMD2081_DATA_WRITE, addr);
    RFMD2081_SendData(data);
    RFMD2081_ENX_Disable(); 
}

/*
 * @brief               读数据
 * @param addr          需要读数据的寄存器地址
 * @return              获取到的数据
 * @author              Chosifew & JaydenLe
 */
uint16_t RFMD2081_Read(uint8_t addr) {
    uint16_t ret_data = 0;
    RFMD2081_ENX_Enable(); 
    RFMD2081_SendAddr(RFMD2081_DATA_READ, addr);
    RFMD2081_EmptyCLK();
    ret_data = RFMD2081_ReceiveData();
    RFMD2081_ENX_Disable(); 
    return ret_data;
}


/*
 * @brief               单独设置某一比特的值
 * @param addr          需要设置值的寄存器的地址
 * @param bit_pos       bit的位置
 * @param bit_val       需要设置的bit的值
 * @return              无
 * @author              Chosifew & JaydenLe
 */
void RFMD2081_WriteBit(uint8_t addr, uint8_t bit_pos, uint8_t bit_val) {
    uint16_t raw_data = RFMD2081_Read(addr);
    UTILS_WriteBit(&raw_data, bit_pos, (bit_val == 0 ? UTILS_BIT_RESET : UTILS_BIT_SET));
    RFMD2081_Write(addr, raw_data);
}

/*
 * @brief               设置某一区域的值
 * @param addr          需要设置值的寄存器的地址
 * @param msb           目标位的高位索引
 * @param lsb           目标位的低位索引
 * @param value         需要设置的值
 * @return              无
 * @author              Chosifew & JaydenLe
 */
static void RFMD2081_WriteBitZone(uint8_t addr, uint8_t msb, uint8_t lsb, uint16_t value) {
    uint16_t raw_data = RFMD2081_Read(addr);
    UTILS_WriteBit_Zone(&raw_data, msb, lsb, value);
    RFMD2081_Write(addr, raw_data);
}

//------------------------------芯片设置函数------------------------------

/*
 * @brief               软件使能设备
 * @return              无
 * @author              Chosifew & JaydenLe
 */
void RFMD2081_Device_Enable(void) {
    RFMD2081_WriteBit(RFMD2081_REG_SDI_CTRL, 14, 1);
}

/*
 * @brief               软件失能设备
 * @return              无
 * @author              Chosifew & JaydenLe
 */
void RFMD2081_Device_Disable(void) {
    RFMD2081_WriteBit(RFMD2081_REG_SDI_CTRL, 14, 0);
}

/*
 * @brief               重启设备
 * @return              无
 * @author              Chosifew & JaydenLe
 */
void RFMD2081_Device_Reset(void) {
    RFMD2081_WriteBit(RFMD2081_REG_SDI_CTRL, 1, 1);
}

/*
 * @brief               设置控制方式
 * @param mode          RFMD2081_HARDWARE_CONTROL: 硬件控制
 *                      RFMD2081_SOFTWARE_CONTROL: 软件控制
 * @return              无
 * @author              Chosifew & JaydenLe
 */
void RFMD2081_SetControlMode(RFMD2081_CONTROL_MODE mode) {
    RFMD2081_WriteBit(RFMD2081_REG_SDI_CTRL, 15, mode);
}

/*
 * @brief               选择工作的锁相环
 * @param pll_x         选择的锁相环
 * @return              无
 * @author              Chosifew & JaydenLe
 */
void RFMD2081_PLL_Select(RFMD2081_PLLx pll_x) {
    RFMD2081_WriteBit(RFMD2081_REG_SDI_CTRL, 13, pll_x);
}

/*
 * @brief               选择工作的压控振荡器
 * @param pll_x         选择设置的PLL路径
 * @param voc_x         RFMD2081_AUTO   : 自动选择
 *                      RFMD2081_VCO_1  : 选择VOC1
 *                      RFMD2081_VCO_2  : 选择VOC2
 *                      RFMD2081_VCO_3  : 选择VOC3
 * @return              无
 * @author              Chosifew & JaydenLe
 */
void RFMD2081_VCO_Select(RFMD2081_PLLx pll_x, RFMD2081_VCOx vco_x) {
    if (vco_x == RFMD2081_VCO_AUTO) 
        RFMD2081_WriteBit(RFMD2081_REG_VCO_AUTO, 15, 1);
    else {
        if (pll_x == RFMD2081_PLL_1) 
            RFMD2081_WriteBitZone(RFMD2081_REG_P1_FREQ1, 1, 0, vco_x);
        else if (pll_x == RFMD2081_PLL_2)
            RFMD2081_WriteBitZone(RFMD2081_REG_P2_FREQ1, 1, 0, vco_x);
    }
}

/*
 * @brief               设置并开启VCO粗调
 * @param pll_x         需要设置的PLL路径
 * @param coarse_tuning_val
 *                      VCO粗调的默认值
 * @return              UTILS_OK    : 参数设置的没有问题
 *                      UTILS_ERROR : 参数传输的有问题
 * @author              Chosifew & JaydenLe
 */
UTILS_Status RFMD2081_VCO_CoaresTune(RFMD2081_PLLx pll_x, uint8_t coarse_tuning_val) {
    UTILS_Status status = UTILS_OK;
    do {
        if (coarse_tuning_val > 127) {              // 设置的值只允许在 0 ~ 127
            status = UTILS_ERROR;
            break;
        }

        uint8_t val = 128 + coarse_tuning_val;      // 把第7位数据设置为1以开启VCO粗调
        if (pll_x == RFMD2081_PLL_1)  
            RFMD2081_WriteBitZone(RFMD2081_REG_CT_CAL1, 7, 0, val);
        else if (pll_x == RFMD2081_PLL_2)
            RFMD2081_WriteBitZone(RFMD2081_REG_CT_CAL2, 7, 0, val);
        else
            status = UTILS_ERROR;
    } while(0);
    return status;
}


/*
 * @brief               RFMD2081初始化
 * @param control_mode  设置控制的模式
 * @return              无
 * @author              Chosifew & JaydenLe
 */
void RFMD2081_SetUP(RFMD2081_CONTROL_MODE control_mode) {
    RFMD2081_WriteBitZone(RFMD2081_REG_P2_FREQ1, 1, 0, 0);
    RFMD2081_WriteBitZone(RFMD2081_REG_VCO_AUTO, 7, 1, RFMD2081_SET_CT_MIN);
    RFMD2081_WriteBitZone(RFMD2081_REG_VCO_AUTO, 14, 8, RFMD2081_SET_CT_MAX);
    RFMD2081_WriteBitZone(RFMD2081_REG_CT_CAL1, 12, 8, RFMD2081_SET_P1_CTV);
    RFMD2081_WriteBitZone(RFMD2081_REG_CT_CAL2, 12, 8, RFMD2081_SET_P2_CTV);
    RFMD2081_WriteBit(RFMD2081_REG_TEST, 2, 1);

    RFMD2081_SetControlMode(control_mode);
}

void RFMD2081_SetAdditionalFeatures(void) {
    //------------------------------设置通信模式------------------------------
#if RFMD2081_THREE_WIRE_BUS
    RFMD2081_WriteBit(RFMD2081_REG_SDI_CTRL, 12, 0);
#elif RFMD2081_FOUR_WIRE_BUS
    RFMD2081_WriteBit(RFMD2081_REG_SDI_CTRL, 12, 1);
#endif
}

/*
 * @brief               设置输出频率
 * @param pll_x         需要设置的PLL路径
 * @param lo_freq       需要设置的输出频率,单位为MHz,范围为45MHz ~ 2700MHz
 * @return              UTILS_OK    : 参数设置的没有问题
 *                      UTILS_ERROR : 参数设置的有问题
 * @author              Chosifew & JaydenLe
 */
UTILS_Status RFMD2081_SetFrequency(RFMD2081_PLLx pll_x, double lo_freq) {
    UTILS_Status status = UTILS_OK;
    do {
        if (lo_freq > 2700 || lo_freq < 45) {
            status = UTILS_ERROR;
            break;
        }

        //------------------------------计算需要设置的值------------------------------
        // 计算过程参考 <IQ Modulator Programming Guide>
        uint32_t n_lo = UTILS_Log2((uint32_t)(RFMD2081_VCO_MAX_FREQUENCY / (2 * lo_freq)));
        uint16_t lo_div = (1 << n_lo);
        double f_vco = 2 * lo_div * lo_freq;
        uint16_t fbkdiv = (f_vco > 3200 ? 4 : 2);
        double n_div = f_vco / fbkdiv / RFMD2081_REF_FREQUENCY;
        uint16_t n = (uint16_t)n_div;
        uint32_t nummsb = (uint32_t)((1 << 16) * (n_div - n));
        uint32_t numlsb = (uint32_t)((1 << 8) * ((1 << 16) * (n_div - 1) - nummsb));

        //------------------------------进行设置------------------------------
        // 使用VCO自动选择, 此时不再设置VCO粗调
        RFMD2081_VCO_Select(pll_x, RFMD2081_VCO_AUTO);

        // 如果频率大于3.2GHz,把pllcpl设置为3
        if (fbkdiv == 4)
            RFMD2081_WriteBitZone(RFMD2081_REG_LF, 2, 0, 3);
        
        // 设置对应的寄存器
        if (pll_x == RFMD2081_PLL_1) {
            RFMD2081_WriteBitZone(RFMD2081_REG_P1_FREQ1, 15, 7, n);
            RFMD2081_WriteBitZone(RFMD2081_REG_P1_FREQ1, 6, 4, lo_div);
            RFMD2081_Write(RFMD2081_REG_P1_FREQ2, nummsb);
            RFMD2081_WriteBitZone(RFMD2081_REG_P1_FREQ3, 15, 8, numlsb);
        }
        else if (pll_x == RFMD2081_PLL_2) {
            RFMD2081_WriteBitZone(RFMD2081_REG_P2_FREQ1, 15, 7, n);
            RFMD2081_WriteBitZone(RFMD2081_REG_P2_FREQ1, 6, 4, lo_div);
            RFMD2081_Write(RFMD2081_REG_P2_FREQ2, nummsb);
            RFMD2081_WriteBitZone(RFMD2081_REG_P2_FREQ3, 15, 8, numlsb);
        }
        else 
            status = UTILS_ERROR;
    } while(0);
    return status;
}

/*
 * @brief               使能环路滤波器校准, 这个就是数据手册中的"SET LOOP FILTER CALIBRATION MODE"
 * @return              无
 * @author              Chosifew & JaydenLe
 */
void RFMD2081_LoopFilterCal_Enable(void) {
    RFMD2081_WriteBit(RFMD2081_REG_PLL_CAL1, 15, 1);
    RFMD2081_WriteBit(RFMD2081_REG_PLL_CAL2, 15, 1);
}


/*
 * @brief               优化相位噪声
 * @param pll_x         需要设置的锁相环路径
 * @param lo_freq       设置的输出频率
 * @return              UTILS_OK    : 参数设置的没有问题
 *                      UTILS_ERROR : 参数设置的有问题
 * @author              Chosifew & JaydenLe
 * @attention           该函数只能在设置频率完成,设备启动以后才能进行使用
 */
UTILS_Status RFMD2081_OptimizingPhaseNoise(RFMD2081_PLLx pll_x, double lo_freq) {
    UTILS_Status status = UTILS_OK;
    do {
        if (lo_freq > 2700 || lo_freq < 45) {
            status = UTILS_ERROR;
            break;
        }

        //------------------------------计算需要设置的值------------------------------
        // 计算过程参考 <IQ Modulator Programming Guide>
        uint32_t n_lo = UTILS_Log2((uint32_t)(RFMD2081_VCO_MAX_FREQUENCY / (2 * lo_freq)));
        uint16_t lo_div = (1 << n_lo);
        double f_vco = 2 * lo_div * lo_freq;
        uint16_t fbkdiv = (2);
        double n_div = f_vco / fbkdiv / RFMD2081_REF_FREQUENCY;
        uint16_t n = (uint16_t)n_div;
        uint32_t nummsb = (uint32_t)((1 << 16) * (n_div - n));
        uint32_t numlsb = (uint32_t)((1 << 8) * ((1 << 16) * (n_div - 1) - nummsb));

        //------------------------------进行设置------------------------------
        // 设置对应的寄存器
        if (pll_x == RFMD2081_PLL_1) {
            RFMD2081_WriteBitZone(RFMD2081_REG_P1_FREQ1, 15, 7, n);
            RFMD2081_Write(RFMD2081_REG_P1_FREQ2, nummsb);
            RFMD2081_WriteBitZone(RFMD2081_REG_P1_FREQ3, 15, 8, numlsb);
        }
        else if (pll_x == RFMD2081_PLL_2) {
            RFMD2081_WriteBitZone(RFMD2081_REG_P2_FREQ1, 15, 7, n);
            RFMD2081_Write(RFMD2081_REG_P2_FREQ2, nummsb);
            RFMD2081_WriteBitZone(RFMD2081_REG_P2_FREQ3, 15, 8, numlsb);
        }
        else 
            status = UTILS_ERROR;
    } while(0);
    return status;
}

/*
 * @brief               rfmd2081初始化
 * @return              无
 * @author              Chosifew & JaydenLe
 */
void RFMD2081_Init(void) {
    
    //------------------------------时钟使能------------------------------
    UTILS_RCC_GPIO_Enable(RFMD2081_ENX_GPIO_TYPE);
    UTILS_RCC_GPIO_Enable(RFMD2081_CLK_GPIO_TYPE);

#if RFMD2081_THREE_WIRE_BUS                             // 三线模式
    UTILS_RCC_GPIO_Enable(RFMD2081_DATA_GPIO_TYPE);
#elif RFMD2081_FOUR_WIRE_BUS                            // 四线模式
    UTILS_RCC_GPIO_Enable(RFMD2081_DI_GPIO_TYPE);
    UTILS_RCC_GPIO_Enable(RFMD2081_DO_GPIO_TYPE);
#endif

    //------------------------------引脚默认值设置------------------------------
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    HAL_GPIO_WritePin(RFMD2081_ENX_GPIO_TYPE, RFMD2081_ENX_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RFMD2081_CLK_GPIO_TYPE, RFMD2081_CLK_GPIO_PIN, GPIO_PIN_RESET);

#if RFMD2081_THREE_WIRE_BUS                             // 三线模式
    HAL_GPIO_WritePin(RFMD2081_DATA_GPIO_TYPE, RFMD2081_DATA_GPIO_PIN, GPIO_PIN_RESET);
#elif RFMD2081_FOUR_WIRE_BUS                            // 四线模式
    HAL_GPIO_WritePin(RFMD2081_DI_GPIO_TYPE, RFMD2081_DI_GPIO_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RFMD2081_DO_GPIO_TYPE, RFMD2081_DO_GPIO_PIN, GPIO_PIN_SET);
#endif

    //------------------------------引脚GPIO设置------------------------------
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = RFMD2081_ENX_GPIO_PIN;
    HAL_GPIO_Init(RFMD2081_ENX_GPIO_TYPE, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RFMD2081_CLK_GPIO_PIN;
    HAL_GPIO_Init(RFMD2081_CLK_GPIO_TYPE, &GPIO_InitStruct);
    
#if RFMD2081_THREE_WIRE_BUS                             // 三线模式
    GPIO_InitStruct.Pin = RFMD2081_DATA_GPIO_PIN;
    HAL_GPIO_Init(RFMD2081_DATA_GPIO_TYPE, &GPIO_InitStruct);
#elif RFMD2081_FOUR_WIRE_BUS                            // 四线模式
    GPIO_InitStruct.Pin = RFMD2081_DATA_GPIO_PIN;
    HAL_GPIO_Init(RFMD2081_DATA_GPIO_TYPE, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = RFMD2081_DATA_GPIO_PIN;
    HAL_GPIO_Init(RFMD2081_DATA_GPIO_TYPE, &GPIO_InitStruct);
#endif
}


