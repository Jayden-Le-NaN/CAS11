#include "gd5f2gm7.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_spi.h"
#include "utils.h"

//------------------------------------------------------------
// Created Time     : 2024.10.06
// Author           : JaydenLee
// Note             : 只有接收数据和发送数据使用DMA,像发送指令这些使用阻塞式发送
//------------------------------------------------------------

//------------------------------仅内部使用,外部不可用------------------------------
#define GD5F2GM7_Transmit_Receive_Start()   HAL_GPIO_WritePin(gd5f2gm7_obj->cs_pin_type, gd5f2gm7_obj->cs_pin, GPIO_PIN_RESET)
#define GD5F2GM7_Transmit_Receive_Stop()    HAL_GPIO_WritePin(gd5f2gm7_obj->cs_pin_type, gd5f2gm7_obj->cs_pin, GPIO_PIN_SET)
//------------------------------仅内部使用,外部不可用------------------------------


//------------------------------需要放入中断回调函数中的函数------------------------------
void GD5F2GM7_Transmit_IRQ_Handler(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi) {
    if (gd5f2gm7_obj->spi == spi) {

    }
}

void GD5F2GM7_Receive_IRQ_Hanlder(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi) {
    if (gd5f2gm7_obj->spi == spi) {

    }
}
//------------------------------需要放入中断回调函数中的函数------------------------------

/*
 * @brief               使用阻塞的方式传输数组数据
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param tx_data       需要发送的数据
 * @param len           发送的数据长度
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
static UTILS_Status GD5F2GM7_Transmit_8bit_Array(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t* tx_data, uint32_t len) {
    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Transmit(gd5f2gm7_obj->spi, tx_data, len, HAL_MAX_DELAY) != HAL_OK)
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               使用DMA的方式传输数组数据
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param tx_data       需要发送的数据
 * @param len           需要发送的数据长度
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
static UTILS_Status GD5F2GM7_Transmit_8bit_Array_DMA(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t* tx_data, uint32_t len) {
    if (gd5f2gm7_obj->_dma_fsm_state_transmit != GD5F2GM7_DMA_Idle)
        return UTILS_ERROR;

    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Transmit_DMA(gd5f2gm7_obj->spi, tx_data, len) != HAL_OK)
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               使用阻塞的方式接收数据
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param rx_data       接收数据存放的地址
 * @param len           接收数据的长度
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
static UTILS_Status GD5F2GM7_Receive_8bit_Array(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t* rx_data, uint32_t len) {
    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Receive(gd5f2gm7_obj->spi, rx_data, len, HAL_MAX_DELAY) != HAL_OK)
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               使用DMA的方式接收数据
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param rx_data       接收数据存放的地址
 * @param len           接收数据的长度
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
static UTILS_Status GD5F2GM7_Receive_8bit_Array_DMA(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t* rx_data, uint32_t len) {
    if (gd5f2gm7_obj->_dma_fsm_state_receive != GD5F2GM7_DMA_Idle)
        return UTILS_ERROR;
    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Receive_DMA(gd5f2gm7_obj->spi, rx_data, len) != HAL_OK)
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               写使能
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
UTILS_Status GD5F2GM7_WriteEnable(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    uint8_t command = GD5F2GM7_CMD_WRITE_ENABLE;
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, &command, 1);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               写失能
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
UTILS_Status GD5F2GM7_WriteDisable(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    uint8_t command = GD5F2GM7_CMD_WRITE_DISABLE;
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, &command, 1);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               获取寄存器数据
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param reg_addr      寄存器的地址
 * @param rx_data       存储寄存器数据的地址
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
UTILS_Status GD5F2GM7_Get_Features(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t reg_addr, uint8_t* rx_data) {
    uint8_t packet[2];
    uint8_t command = GD5F2GM7_CMD_GET_FEATURES;
    packet[0] = command;
    packet[1] = reg_addr;
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, packet, 2);
    if (status == UTILS_OK)
        status = GD5F2GM7_Receive_8bit_Array(gd5f2gm7_obj, rx_data, 1);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               设置寄存器中的值
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param reg_addr      寄存器的地址
 * @param tx_data       需要设置的地址
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
UTILS_Status GD5F2GM7_Set_Features(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t reg_addr, uint8_t tx_data) {
    uint8_t packet[3];
    uint8_t command = GD5F2GM7_CMD_SET_FEATURES;
    packet[0] = command;
    packet[1] = reg_addr;
    packet[2] = tx_data;
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, packet, 3);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}


/*
 * @brief               初始化一个gd5f2gm7硬件设备的配置信息
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param spi           使用的spi结构体
 * @param cs_pin        片选引脚
 * @param cs_pin_type   片选引脚的GPIO类型
 * @return              无
 */
void GD5F2GM7_Init(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type) {
    //------------------------------数据挂载------------------------------
    gd5f2gm7_obj->spi = spi;
    gd5f2gm7_obj->cs_pin = cs_pin;
    gd5f2gm7_obj->cs_pin_type = cs_pin_type;

    //------------------------------初始化状态机------------------------------
    gd5f2gm7_obj->_dma_fsm_state_transmit = GD5F2GM7_DMA_Idle;
    gd5f2gm7_obj->_dma_fsm_state_receive  = GD5F2GM7_DMA_Idle;

    //------------------------------配置CS引脚------------------------------
    GD5F2GM7_Transmit_Receive_Stop();
    UTILS_RCC_GPIO_Enable(gd5f2gm7_obj->cs_pin_type);
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = gd5f2gm7_obj->cs_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(gd5f2gm7_obj->cs_pin_type, &GPIO_InitStruct);
}
