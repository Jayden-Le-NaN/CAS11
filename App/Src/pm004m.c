#include "pm004m.h"
#include "utils.h"

//------------------------------------------------------------
// Created Time     : 2024.10.08
// Author           : JaydenLee
//------------------------------------------------------------

//------------------------------仅内部使用, 外部不可用------------------------------
#define PM004M_Transmit_Receive_Strat()     HAL_GPIO_WritePin(pm004m_obj->cs_pin_type, pm004m_obj->cs_pin, GPIO_PIN_RESET)
#define PM004M_Transmit_Receive_Stop()      HAL_GPIO_WritePin(pm004m_obj->cs_pin_type, pm004m_obj->cs_pin, GPIO_PIN_SET)
//------------------------------仅内部使用, 外部不可用------------------------------


//------------------------------需要放入中断回调函数中的函数------------------------------
void PM004M_Transmit_IRQ_Handler(PM004M_Info_Struct* pm004m_obj, SPI_HandleTypeDef* spi) {
    if (pm004m_obj->spi == spi) {
        pm004m_obj->_dma_fsm_state_transmit = PM004M_DMA_Idle;
        if (pm004m_obj->_dma_fsm_state_receive == PM004M_DMA_Idle)
            PM004M_Transmit_Receive_Stop();
    }
}

void PM004M_Receive_IRQ_Handler(PM004M_Info_Struct* pm004m_obj, SPI_HandleTypeDef* spi) {
    if (pm004m_obj->spi == spi) {
        pm004m_obj->_dma_fsm_state_receive = PM004M_DMA_Idle;
        if (pm004m_obj->_dma_fsm_state_transmit == PM004M_DMA_Idle)
            PM004M_Transmit_Receive_Stop();
    }
}
//------------------------------需要放入中断回调函数中的函数------------------------------


/*
 * @brief               使用阻塞的方式传输数组数据
 * @param pm004m_obj    pm004m 指定信息
 * @param tx_data       需要发送的数据
 * @param len           发送的数据长度
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
static UTILS_Status PM004M_Transmit_8bit_Array(PM004M_Info_Struct* pm004m_obj, uint8_t* tx_data, uint32_t len) {
    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Transmit(pm004m_obj->spi, tx_data, len, HAL_MAX_DELAY) != HAL_OK)
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               使用DMA的方式传输数组数据
 * @param pm004m_obj    pm004m 指定信息
 * @param tx_data       需要发送的数据
 * @param len           需要发送的数据长度
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
static UTILS_Status PM004M_Transmit_8bit_Array_DMA(PM004M_Info_Struct* pm004m_obj, uint8_t* tx_data, uint32_t len) {
    if (pm004m_obj->_dma_fsm_state_transmit != PM004M_DMA_Idle)
        return UTILS_ERROR;

    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Transmit_DMA(pm004m_obj->spi, tx_data, len) != HAL_OK)
        status = UTILS_ERROR;
    else
        pm004m_obj->_dma_fsm_state_transmit = PM004M_DMA_Transmiting;
    return status;
}

/*
 * @brief               使用阻塞的方式接收数据
 * @param pm004m_obj    pm004m 指定信息
 * @param rx_data       接收数据存放的地址
 * @param len           接收数据的长度
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
static UTILS_Status PM004M_Receive_8bit_Array(PM004M_Info_Struct* pm004m_obj, uint8_t* rx_data, uint32_t len) {
    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Receive(pm004m_obj->spi, rx_data, len, HAL_MAX_DELAY) != HAL_OK)
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               使用DMA的方式接收数据
 * @param pm004m_obj    pm004m 指定信息
 * @param rx_data       接收数据存放的地址
 * @param len           接收数据的长度
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
static UTILS_Status PM004M_Receive_8bit_Array_DMA(PM004M_Info_Struct* pm004m_obj, uint8_t* rx_data, uint32_t len) {
    if (pm004m_obj->_dma_fsm_state_receive != PM004M_DMA_Idle)
        return UTILS_ERROR;
    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Receive_DMA(pm004m_obj->spi, rx_data, len) != HAL_OK)
        status = UTILS_ERROR;
    else
        pm004m_obj->_dma_fsm_state_receive = PM004M_DMA_Receiving;
    return status;
}

/*
 * @brief               写寄存器中的值
 * @param pm004m_obj    pm004m 指定信息
 * @param reg_addr      寄存器的地址
 * @param tx_data       写入寄存器的数据
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 * @note                寄存器的地址范围为: 0x0000 r/w, 0x0001 r/w, 0x0002 r
 */
UTILS_Status PM004M_ModeRegisterWrite(PM004M_Info_Struct* pm004m_obj, uint32_t reg_addr, uint8_t tx_data) {
    uint8_t packet[5];
    uint8_t command = PM004M_CMD_MODE_REGISTER_WRITE;
    packet[0] = command;
    for (uint8_t i = 1; i < 4; ++i) 
        packet[i] = (uint8_t)(reg_addr >> (24 - i * 8)) & 0xFF;
    packet[4] = tx_data;
    PM004M_Transmit_Receive_Strat();
    UTILS_Status status = PM004M_Transmit_8bit_Array(pm004m_obj, packet, 5);
    PM004M_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               读寄存器中的值
 * @param pm004m_obj    pm004m 指定信息
 * @param reg_addr      寄存器的地址
 * @param rx_data       存储寄存器数据的地址
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 * @note                寄存器的地址范围为: 0x0000 r/w, 0x0001 r/w, 0x0002 r
 */
UTILS_Status PM004M_ModeRegisterRead(PM004M_Info_Struct* pm004m_obj, uint32_t reg_addr, uint8_t* rx_data) {
    uint8_t packet[4];
    uint8_t command = PM004M_CMD_MODE_REGISTER_READ;
    packet[0] = command;
    for (uint8_t i = 1; i < 4; ++i) 
        packet[i] = (uint8_t)(reg_addr >> (24 - i * 8)) & 0xFF;
    PM004M_Transmit_Receive_Strat();
    UTILS_Status status = PM004M_Transmit_8bit_Array(pm004m_obj, packet, 4);
    if (status == UTILS_OK)
        status = PM004M_Receive_8bit_Array(pm004m_obj, rx_data, 1);
    PM004M_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               写数据
 * @param pm004m_obj    pm004m 指定信息
 * @param addr          存储区地址
 * @param tx_data       需要发送的数据
 * @param len           发送数据的长度
 * @param tx_mode       发送数据的模式
 *                          UTILS_LOOP: 使用阻塞的方式接收
 *                          UTILS_DMA : 使用DMA的方式接收
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输或者是传输模式选择错误
 */
UTILS_Status PM004M_Write(PM004M_Info_Struct* pm004m_obj, uint32_t addr, uint8_t* tx_data, uint32_t len, UTILS_CommunicationMode tx_mode) {
    uint8_t packet[4];
    uint8_t command = PM004M_CMD_WRITE;
    packet[0] = command;
    for (uint8_t i = 1; i < 4; ++i)
        packet[i] = (uint8_t)(addr >> (24 - 8 * i)) & 0xFF;
    PM004M_Transmit_Receive_Strat();
    UTILS_Status status = PM004M_Transmit_8bit_Array(pm004m_obj, packet, 4);

    do {
        if (status != UTILS_OK)
            break;

        if (tx_mode == UTILS_LOOP) {
            status = PM004M_Transmit_8bit_Array(pm004m_obj, tx_data, len);
            PM004M_Transmit_Receive_Stop();
        }
        else if (tx_mode == UTILS_DMA) {
            status = PM004M_Transmit_8bit_Array_DMA(pm004m_obj, tx_data, len);
        }
        else 
            status = UTILS_ERROR;
    } while(0);

    return status;
}

/*
 * @brief               读数据
 * @param pm004m_obj    pm004m 指定信息
 * @param addr          存储区地址
 * @param rx_data       存储读取数据的地址
 * @param len           读取数据的长度
 * @param rx_mode       读取数据的模式
 *                          UTILS_LOOP: 使用阻塞的方式接收
 *                          UTILS_DMA : 使用DMA的方式接收
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输或者是传输模式选择错误
 */
UTILS_Status PM004M_Read(PM004M_Info_Struct* pm004m_obj, uint32_t addr, uint8_t* rx_data, uint32_t len, UTILS_CommunicationMode rx_mode) {
    uint8_t packet[4 + pm004m_obj->_lt];
    packet[3 + pm004m_obj->_lt] = 0;
    uint8_t command = PM004M_CMD_READ;
    packet[0] = command;
    for (uint8_t i = 1; i < 4; ++i)
        packet[i] = (uint8_t)(addr >> (24 - i * 8)) & 0xFF;

    PM004M_Transmit_Receive_Strat();
    UTILS_Status status = PM004M_Transmit_8bit_Array(pm004m_obj, packet, 4 + pm004m_obj->_lt);

    do {
        if (status != UTILS_OK)
            break;
        
        if (rx_mode == UTILS_LOOP) {
            status = PM004M_Receive_8bit_Array(pm004m_obj, rx_data, len);
            PM004M_Transmit_Receive_Stop();
        }
        else if (rx_mode == UTILS_DMA) 
            status = PM004M_Receive_8bit_Array_DMA(pm004m_obj, rx_data, len);
        else
            status = UTILS_ERROR;
    } while(0);

    return status;
}

/*
 * @brief               写使能
 * @param pm004m_obj    pm004m指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
UTILS_Status PM004M_WriteEnable(PM004M_Info_Struct* pm004m_obj) {
    uint8_t command = PM004M_CMD_WRITE_ENABLE;
    PM004M_Transmit_Receive_Strat();
    UTILS_Status status = PM004M_Transmit_8bit_Array(pm004m_obj, &command, 1);
    PM004M_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               写失能
 * @param pm004m_obj    pm004m指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
UTILS_Status PM004M_WriteDisable(PM004M_Info_Struct* pm004m_obj) {
    uint8_t command = PM004M_CMD_WRITE_DISABLE;
    PM004M_Transmit_Receive_Strat();
    UTILS_Status status = PM004M_Transmit_8bit_Array(pm004m_obj, &command, 1);
    PM004M_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               进入低功耗模式
 * @param pm004m_obj    pm004m指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
UTILS_Status PM004M_EntryToDeepPowerDown(PM004M_Info_Struct* pm004m_obj) {
    uint8_t command = PM004M_CMD_ENTRY_TO_DEEP_POWER_DOWN;
    PM004M_Transmit_Receive_Strat();
    UTILS_Status status = PM004M_Transmit_8bit_Array(pm004m_obj, &command, 1);
    PM004M_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               退出低功耗模式
 * @param pm004m_obj    pm004m指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
UTILS_Status PM004M_ExitFromDeepPowerDown(PM004M_Info_Struct* pm004m_obj) {
    uint8_t command = PM004M_CMD_EXIT_FROM_DEEP_POWER_DOWN;
    PM004M_Transmit_Receive_Strat();
    UTILS_Status status = PM004M_Transmit_8bit_Array(pm004m_obj, &command, 1);
    PM004M_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               读取芯片的uid
 * @param pm004m_obj    pm004m指定信息
 * @param rx_data       接收数据的地址(内存大小需要大于16)
 * @param rx_mode       接收数据的模式
 *                          UTILS_LOOP: 使用阻塞的方式接收
 *                          UTILS_DMA : 使用DMA的方式接收
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输或者是传输模式选择错误
 * @note                需要注意: rx_data的大小要大于 16字节!!!
 *                      数据构成: MFID + KGD + 112 bit UID
 */
UTILS_Status PM004M_ReadUniqueIDRegister(PM004M_Info_Struct* pm004m_obj, uint8_t* rx_data, UTILS_CommunicationMode rx_mode) {
    uint8_t packet[4];
    uint8_t command = PM004M_CMD_READ_UNIQUE_ID_REGISTER;
    packet[0] = command;
    for (uint8_t i = 1; i < 4; ++i)
        packet[i] = 0;

    PM004M_Transmit_Receive_Strat();
    UTILS_Status status = PM004M_Transmit_8bit_Array(pm004m_obj, packet, 4);

    do {
        if (status != UTILS_OK)
            break;
        
        if (rx_mode == UTILS_LOOP) {
            status = PM004M_Receive_8bit_Array(pm004m_obj, rx_data, 16);
            PM004M_Transmit_Receive_Stop();
        }
        else if (rx_mode == UTILS_DMA) 
            status = PM004M_Receive_8bit_Array_DMA(pm004m_obj, rx_data, 16);
        else
            status = UTILS_ERROR;
    } while(0);

    return status;
}

/*
 * @brief               使能重启
 * @param pm004m_obj    pm004m指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
UTILS_Status PM004M_ResetEnable(PM004M_Info_Struct* pm004m_obj) {
    uint8_t command = PM004M_CMD_RESET_ENABLE;
    PM004M_Transmit_Receive_Strat();
    UTILS_Status status = PM004M_Transmit_8bit_Array(pm004m_obj, &command, 1);
    PM004M_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               使能重启
 * @param pm004m_obj    pm004m指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
UTILS_Status PM004M_Reset(PM004M_Info_Struct* pm004m_obj) {
    uint8_t command = PM004M_CMD_RESET;
    PM004M_Transmit_Receive_Strat();
    UTILS_Status status = PM004M_Transmit_8bit_Array(pm004m_obj, &command, 1);
    PM004M_Transmit_Receive_Stop();
    if (status == UTILS_OK) {
        pm004m_obj->_dma_fsm_state_transmit = PM004M_DMA_Idle;
        pm004m_obj->_dma_fsm_state_receive = PM004M_DMA_Idle;
        pm004m_obj->_lt = 0;        // dummy clk 的数量为 0
    }
    return status;
}

/*
 * @brief               初始化一个pm004m硬件设备的配置信息
 * @param pm004m_obj    pm004m 指定信息
 * @param spi           使用的spi结构体
 * @param cs_pin        片选引脚
 * @param cs_pin_type   片选引脚的GPIO类型
 * @return              无
 */
void PM004M_Init(PM004M_Info_Struct* pm004m_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type) {
    //------------------------------数据挂载------------------------------
    pm004m_obj->spi = spi;
    pm004m_obj->cs_pin = cs_pin;
    pm004m_obj->cs_pin_type = cs_pin_type;

    //------------------------------默认数据处理------------------------------
    pm004m_obj->_dma_fsm_state_transmit = PM004M_DMA_Idle;
    pm004m_obj->_dma_fsm_state_receive = PM004M_DMA_Idle;
    pm004m_obj->_lt = 0;        // dummy clk 的数量为 0

    //------------------------------配置CS引脚------------------------------
    PM004M_Transmit_Receive_Stop();
    UTILS_RCC_GPIO_Enable(pm004m_obj->cs_pin_type);
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = pm004m_obj->cs_pin;    
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(pm004m_obj->cs_pin_type, &GPIO_InitStruct);
}

