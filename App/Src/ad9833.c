#include "ad9833.h"
#include "stm32l4xx_hal_spi.h"
#include "stm32l4xx_hal_tim.h"
#include "utils.h"

//------------------------------------------------------------
// Created Time     : 2024.10.28
// Author           : JaydenLee
//------------------------------------------------------------


//------------------------------仅内部使用, 外部不可用------------------------------
#define AD9833_Transmit_Start()                 HAL_GPIO_WritePin(ad9833_obj->fsync_pin_type, ad9833_obj->fsync_pin, GPIO_PIN_RESET)
#define AD9833_Transmit_Stop()                  HAL_GPIO_WritePin(ad9833_obj->fsync_pin_type, ad9833_obj->fsync_pin, GPIO_PIN_SET)
#define AD9833_Cal_Whole_frq(ad9833_obj, raw_freq, frqh, frql)      AD9833_FrequencyConversion(ad9833_obj, raw_freq, 1, NULL, frqh, frql)
#define USE_TIM_DMA                     1
#define NO_USE_TIM_DMA                  0

//------------------------------仅内部使用, 外部不可用------------------------------


//--------------------------------------全局变量-----------------------------------
uint8_t ad9833_tim_dma_flag = NO_USE_TIM_DMA;

//------------------------------需要放入中断回调函数中的函数------------------------------
void AD9833_Transmit_IRQ_Handler(AD9833_Info_Struct* ad9833_obj, SPI_HandleTypeDef* spi) {
    if (ad9833_obj->spi == spi && ad9833_obj->_dma_fsm_state_transmit == AD9833_DMA_Transmiting) {
        ad9833_obj->_dma_fsm_state_transmit = AD9833_DMA_Idle;
        AD9833_Transmit_Stop();
    }
}
//------------------------------需要放入中断回调函数中的函数------------------------------


// void DMA_TIM_SPI_HalfTxCplt(void)
// {
//     printf("DMA TIM SPI HalfTxCplt\n");
// }

// void DMA_TIM_SPI_TxCplt(void){
//     printf("DMA TIM SPI TxCplt\n");
// }

/**
 * @brief   通过SPI的方式写16bit
 * @param   sstv_tim_dma_spi  :  SPI句柄
 * @param   Data              :  需要写入的数据
 * @retval  None
 */
void SPI_Write_Half_Word(SPI_HandleTypeDef* sstv_tim_dma_spi, uint16_t *Data){
    while(__HAL_SPI_GET_FLAG(sstv_tim_dma_spi, SPI_FLAG_TXE) == 0){}        // 等待上一轮传输完成
    sstv_tim_dma_spi->Instance->DR = *Data;
    // printf("spi %x\r\n", *Data);
}

#define AD9833_SCLK(level)      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, level);
#define AD9833_SDATA(level)     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, level);
#define AD9833_FSYNC(level)     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, level);

/**
 * @brief   通过低速IO口写16位数据
 * @param   Data              :  需要写入的数据
 * @retval  None
 */
void Write_Half_Word(uint16_t *Data){
    int i;
    uint16_t temp = *Data;
    AD9833_SCLK(GPIO_PIN_SET);
    HAL_Delay(1);
    AD9833_FSYNC(GPIO_PIN_SET);
    HAL_Delay(1);
    AD9833_FSYNC(GPIO_PIN_RESET);
    //写16位数据
    for(i=0;i<16;i++)
    {
        
        if (temp & 0x8000){
            AD9833_SDATA(GPIO_PIN_SET);
        } 
        else{
            AD9833_SDATA(GPIO_PIN_RESET);
        }
        HAL_Delay(1);
        AD9833_SCLK(GPIO_PIN_RESET);
        HAL_Delay(1);
        AD9833_SCLK(GPIO_PIN_SET);
        temp<<=1;
    }
    AD9833_FSYNC(GPIO_PIN_SET);
}


/**
 * @brief   通过SPI将DDS的整数和分数两个部分都写入到AD9833
 * @param   sstv_tim_dma_spi  :  SPI句柄
 * @param   frqh              :  频率寄存器高14位
 * @param   frql              :  频率寄存器高低14位
 * @retval  None
 */
void AD9833_Write_Whole_Frq(SPI_HandleTypeDef* sstv_tim_dma_spi, uint16_t *frqh, uint16_t *frql){
    uint16_t temp = AD9833_REG_B28;
    // 测试用的低速方式
    // Write_Half_Word(&temp);// 低速IO操作
    // Write_Half_Word(frql);
    // Write_Half_Word(frqh);
    // 高速方式
    SPI_Write_Half_Word(sstv_tim_dma_spi, &temp);// | AD9833_OUT_FREQ0 | AD9833_OUT_PHASE0
    SPI_Write_Half_Word(sstv_tim_dma_spi, frql);
    SPI_Write_Half_Word(sstv_tim_dma_spi, frqh);
    // UTILS_Delay_us(5);
}



/**
 * @brief  Initialize AD9833 SPI and TIM for DMA transfer.
 * @param  ad9833_obj1:  AD9833 object 1
 * @param  ad9833_obj2:  AD9833 object 2
 * @param  ad9833_tim_dma_spi:  SPI handle for DMA transfer
 * @param  ad9833_tim:  TIM handle for DMA transfer
 * @retval UTILS_OK:  Successfully initialized
 * @retval UTILS_ERROR:  Initialization failed
 */
UTILS_Status AD9833_Init_Tx_DMA_TIM(AD9833_Info_Struct* ad9833_obj1, AD9833_Info_Struct* ad9833_obj2, SPI_HandleTypeDef* ad9833_tim_dma_spi, TIM_HandleTypeDef* ad9833_tim) {
    // SPI配置
    if(ad9833_tim_dma_spi != NULL){
        if(HAL_SPI_DeInit(ad9833_tim_dma_spi) != HAL_OK){
            printf("HAL_SPI_DeInit error\r\n");
            return UTILS_ERROR;
        }   
    }
    printf("AD9833_Init_Tx_DMA_TIM\r\n");
    
    ad9833_tim_dma_flag = USE_TIM_DMA;                                      //标记使用TIM DMA，在HAL_SPI_MspInit中执行对应代码

    ad9833_obj1->spi = ad9833_tim_dma_spi;
    ad9833_obj2->spi = ad9833_tim_dma_spi;
    ad9833_tim_dma_spi->Instance = SPI2;
    ad9833_tim_dma_spi->Init.Mode = SPI_MODE_MASTER;
    ad9833_tim_dma_spi->Init.Direction = SPI_DIRECTION_1LINE;
    ad9833_tim_dma_spi->Init.DataSize = SPI_DATASIZE_16BIT;
    ad9833_tim_dma_spi->Init.CLKPolarity = SPI_POLARITY_HIGH;
    ad9833_tim_dma_spi->Init.CLKPhase = SPI_PHASE_1EDGE;
    ad9833_tim_dma_spi->Init.NSS = SPI_NSS_SOFT;
    ad9833_tim_dma_spi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    ad9833_tim_dma_spi->Init.FirstBit = SPI_FIRSTBIT_MSB;
    ad9833_tim_dma_spi->Init.TIMode = SPI_TIMODE_DISABLE;
    ad9833_tim_dma_spi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    ad9833_tim_dma_spi->Init.CRCPolynomial = 7;
    ad9833_tim_dma_spi->Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    ad9833_tim_dma_spi->Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    if (HAL_SPI_Init(ad9833_tim_dma_spi) != HAL_OK)
    {
        //Error_Handler();
        printf("HAL_SPI_Init error\r\n");
        return UTILS_ERROR;
    }
    // printf("HAL_SPI_Init ok\r\n");

    /// TIM配置
    if(HAL_TIM_Base_DeInit(ad9833_tim) != HAL_OK){
        printf("HAL_TIM_Base_DeInit error\r\n");
        return UTILS_ERROR;
    }
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    ad9833_tim->Instance = TIM2;
    //ad9833_tim.Init.Prescaler = Prescaler;
    ad9833_tim->Init.CounterMode = TIM_COUNTERMODE_UP;
    //ad9833_tim.Init.Period = Period;
    ad9833_tim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    ad9833_tim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(ad9833_tim) != HAL_OK)
    {
        //Error_Handler();
        printf("HAL_TIM_Base_Init error\r\n");
        return UTILS_ERROR;
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(ad9833_tim, &sClockSourceConfig) != HAL_OK)
    {
        //Error_Handler();
        printf("HAL_TIM_ConfigClockSource error\r\n");
        return UTILS_ERROR;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(ad9833_tim, &sMasterConfig) != HAL_OK)
    {
        //Error_Handler();
        printf("HAL_TIMEx_MasterConfigSynchronization error\r\n");
        return UTILS_ERROR;
    }
    
    
    //HAL_SPI_Transmit_DMA(ad9833_obj->spi, tx_data, 8);
    //((ad9833_tim).Instance->DIER |= ((0x1UL << (8U))));
    //HAL_TIM_Base_Start_IT(&ad9833_tim);


//---------------移植HAL_SPI_Transmit_DMA-----------------------
    //TODO: hal lock返回值与utils不一致，考虑再封装一层
    __HAL_LOCK(ad9833_tim_dma_spi);//(ad9833_tim_dma_spi).Lock = HAL_LOCKED;
    //do{ if((ad9833_tim_dma_spi).Lock == HAL_LOCKED) { return HAL_BUSY; } else { (ad9833_tim_dma_spi).Lock = HAL_LOCKED; } }while (0);//lock spi
    __HAL_SPI_DISABLE(ad9833_tim_dma_spi);//disable spi
    SPI_1LINE_TX(ad9833_tim_dma_spi);//spi tx 1 line

    /* Set the SPI TxDMA Half transfer complete callback */
    ad9833_tim_dma_spi->hdmatx->XferHalfCpltCallback = NULL;        //DMA_TIM_SPI_HalfTxCplt;

    /* Set the SPI TxDMA transfer complete callback */
    ad9833_tim_dma_spi->hdmatx->XferCpltCallback = NULL;            //DMA_TIM_SPI_TxCplt;

    /* Set the DMA error callback */
    ad9833_tim_dma_spi->hdmatx->XferErrorCallback = NULL;

    /* Set the DMA AbortCpltCallback */
    ad9833_tim_dma_spi->hdmatx->XferAbortCallback = NULL;


    CLEAR_BIT(ad9833_tim_dma_spi->Instance->CR2, SPI_CR2_LDMATX);//clear SPI_CR2_LDMATX

    __HAL_SPI_ENABLE(ad9833_tim_dma_spi);//enable spi
    __HAL_UNLOCK(ad9833_tim_dma_spi);//unlock spi
    //((ad9833_tim_dma_spi.Instance->CR2) |= ((0x1UL << (1U))));//enable dma tx request

//----------------------------configure timer-----------------
    //TODO: tim在sstv结束时解锁
    __HAL_LOCK(ad9833_tim);
    __HAL_TIM_DISABLE(ad9833_tim);
    __HAL_TIM_SetCounter(ad9833_tim, 0);
    __HAL_TIM_ENABLE_DMA(ad9833_tim, TIM_DMA_UPDATE);  //tim dma trigger
    //ad9833_tim->Instance->CR1 |= TIM_CR1_ARPE;        //arr preload enable; arr 在cnt溢出后才更新
    __HAL_TIM_ENABLE_IT(ad9833_tim, TIM_IT_UPDATE);//enable tim interrupt
    // __HAL_TIM_ENABLE(&ad9833_tim);//enable timer
    printf("here\r\n");
    return UTILS_OK;

}



UTILS_Status AD9833_Transmit_16bit_Array(AD9833_Info_Struct* ad9833_obj, uint16_t* tx_data, uint32_t len) {
    UTILS_Status status = UTILS_OK; 
    if (HAL_SPI_Transmit(ad9833_obj->spi, (const uint8_t *)tx_data, len, HAL_MAX_DELAY) != HAL_OK)
        status = UTILS_ERROR;
    return status;
}

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
    AD9833_Transmit_Start();
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

    for (uint32_t i = 0; i < len / 2; ++i) {
        data[i] = tx_data[i] | reg;
    }
    
    if (reg == AD9833_REG_FREQ0 || reg == AD9833_REG_FREQ1) {
        if ((ad9833_obj->_control_reg_data & (1 << 13)) != 0) {
            int8_t bias = 1;
            uint8_t data_buf = 0;

            for (uint32_t i = 0; i < len; ++i) {
                if (bias > 0) {
                    data_buf =  *((uint8_t*)data + i);
                    *((uint8_t*)data + i) = *((uint8_t*)data + i + bias);
                }
                else
                    *((uint8_t*)data + i) = data_buf;
                bias = -bias;
            }
        }
    }

    AD9833_Transmit_Start();
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

void AD9833_FrequencyConversion_2Reg(AD9833_Info_Struct* ad9833_obj, uint16_t* raw_freq, uint16_t* frqh, uint16_t* frql){
    if(raw_freq == NULL || ad9833_obj == NULL){
        return;
    }
    double freq_scal_factor = 268435456.0 / ad9833_obj->crystal_oscillator_frequency;
    uint32_t val = (uint32_t)(*raw_freq * freq_scal_factor);
    if(frqh != NULL){
        *frqh |= (uint16_t)((val & 0xFFFC000) >> 14);
    }
    if(frql != NULL){
        *frql |= (uint16_t)(val & 0x3FFF);
    }
}

/*
 * @brief               进行频率数值转换
 * @param ad9833_obj    ad9833 指定信息
 * @param raw_freq      原始的频率数据
 * @param len           数据的长度
 * @param converted_freq 
 *                      转换后的频率数据
 * @return              无
 * @note                len的值应该是数组的大小,而不是sizeof得到的大小,注意区分
 */
static void AD9833_FrequencyConversion(AD9833_Info_Struct* ad9833_obj, uint32_t* raw_freq, uint32_t len, uint32_t* converted_freq) {
    // 计算频率比例因子 
    double freq_scal_factor = 268435456.0 / ad9833_obj->crystal_oscillator_frequency;
    // 进行数据转换
    for (uint32_t i = 0; i < len; ++i)
        converted_freq[i] = (uint32_t)(raw_freq[i] * freq_scal_factor);
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
            
        //------------------------------数据转换------------------------------
        uint32_t converted_freq[len];
        AD9833_FrequencyConversion(ad9833_obj, freq, len, converted_freq);
        
        //------------------------------持续打入数据------------------------------
        len = len * sizeof(uint32_t);
        uint16_t tx_data[len / 2];
        uint32_t tx_len = len;
        if (((ad9833_obj->_control_reg_data >> 13) & 0x01) == 1) {          // AD9833_FREQ_ALL
            for (uint32_t i = 0; i < len / 2; ++i)
                tx_data[i] = (converted_freq[i / 2] >> 14 * (i % 2)) & 0x3FFF;
            tx_len = len;    
        }
        else {
            if (((ad9833_obj->_control_reg_data >> 12) & 0x01) == 1) {      // AD9833_FREQ_MSB
                for (uint32_t i = 0; i < len / 2; i += 2)
                    tx_data[i / 2] = (converted_freq[i / 2] >> 14) & 0x3FFF;
                tx_len = len / 2;
            }
            else {                                                          // AD9833_FREQ_LSB
                for (uint32_t i = 0; i < len / 2; i += 2)
                    tx_data[i / 2] = (converted_freq[i / 2]) & 0x3FFF;
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
 * @param fsync_pin_type
 *                      片选引脚的GPIO类型
 * @param crystal_oscillator_frequency
 *                      晶振频率
 * @return              无
 */
void AD9833_Init(AD9833_Info_Struct* ad9833_obj, SPI_HandleTypeDef* spi, uint32_t fsync_pin, GPIO_TypeDef* fsync_pin_type, uint32_t crystal_oscillator_frequency) {
    //------------------------------数据挂载------------------------------
    if(spi != NULL){
        ad9833_obj->spi = spi;
    }
    ad9833_obj->fsync_pin = fsync_pin;
    ad9833_obj->fsync_pin_type = fsync_pin_type;
    ad9833_obj->crystal_oscillator_frequency = crystal_oscillator_frequency;

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
