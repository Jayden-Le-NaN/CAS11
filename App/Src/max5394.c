#include "max5394.h"

static SPI_HandleTypeDef* max5394_spi;      // 挂载的spi

/*
 * @brief               传输一字节数据
 * @param tx_data       需要传输的数据
 * @return              传输状态:   UTILS_Ok - 没有问题
 *                                  UTILS_ERROR - 有问题
 */
static UTILS_Status MAX5394_TransmitByte(uint8_t tx_data) {
    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Transmit(max5394_spi, &tx_data, sizeof(tx_data), 0xFFFF) != HAL_OK)
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               接收一字节数据
 * @param rx_data       需要接收数据的地址
 * @return              传输状态:   UTILS_Ok - 没有问题
 *                                  UTILS_ERROR - 有问题
 */
static UTILS_Status MAX5394_ReceiveByte(uint8_t* rx_data) {
    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Receive(max5394_spi, rx_data, 0x01, 0xFFFF) != HAL_OK)
        status = UTILS_ERROR;
    return status;

}

/*
 * @brief               传输和接收一字节数据
 * @param tx_data       需要传输的数据
 * @param rx_data       需要接收数据的地址
 * @return              传输状态:   UTILS_Ok - 没有问题
 *                                  UTILS_ERROR - 有问题
 */
static UTILS_Status MAX5394_TransmitReceiveByte(uint8_t tx_data, uint8_t* rx_data) {
    UTILS_Status status = UTILS_OK;
    status = MAX5394_TransmitByte(tx_data);
    status = MAX5394_ReceiveByte(rx_data);
    return status;
}

/*
 * @brief               初始化max5394
 * @param hspi          用来传输的spi结构体
 * @return              无
 */
void MAX5394_Init(SPI_HandleTypeDef* hspi) {
    // 时钟使能
    UTILS_RCC_GPIO_Enable(MAX5394_CS_GPIO_TYPE);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    HAL_GPIO_WritePin(MAX5394_CS_GPIO_TYPE, MAX5394_CS_GPIO_PIN, GPIO_PIN_SET);

    // 配置GPIO
    GPIO_InitStruct.Pin = MAX5394_CS_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MAX5394_CS_GPIO_TYPE, &GPIO_InitStruct);
    max5394_spi = hspi; 
}

/*
 * @brief               设置寄存器的值
 * @param tx_data       需要设置的阻值
 * @return              无
 */
void MAX9394_SetRegisterVal(uint8_t tx_data) {
    MAX5394_CS_Enable();
    MAX5394_TransmitByte(MAX5394_SD_CLR);
    MAX5394_TransmitByte(MAX5394_WIPER);
    MAX5394_TransmitByte(tx_data);
    MAX5394_CS_Disable();
}

/*
 * @brief               获取寄存器的值
 * @return              寄存器的值
 * @attention           
 */
uint8_t MAX5394_GetRegisterVal(void) {
    uint8_t val = 0;
    MAX5394_CS_Enable();
    MAX5394_TransmitReceiveByte(MAX5394_SD_CLR, &val);
    MAX5394_CS_Disable();
    return val;
}

/*
 * @brief               设置滑动变阻器的工作模式
 * @param terminal_type 选择的端子类型
 * @param wiper_pos     端子的位置
 * @return              设置状态:   UTILS_Ok - 没有问题
 *                                  UTILS_ERROR - 有问题
 */
UTILS_Status MAX5394_SetResistance(MAX5394_TERMINAL_TYPE terminal_type, MAX5394_WIPER_POS wiper_pos) {
    UTILS_Status status = UTILS_OK;
    MAX5394_CS_Enable();
    switch (terminal_type) {
        case MAX5394_H:
            switch (wiper_pos) {
                case MAX5394_ZERO:
                    MAX5394_TransmitByte(MAX5394_SD_H_ZERO);
                    break;
                case MAX5394_MID:
                    MAX5394_TransmitByte(MAX5394_SD_H_MID);
                    break;
                case MAX5394_FULL:
                    MAX5394_TransmitByte(MAX5394_SD_H_FULL);
                    break;
                case MAX5394_WREG:
                    MAX5394_TransmitByte(MAX5394_SD_H_WREG);
                    break;
                default:
                    status = UTILS_ERROR;
                    break;
            }
            break;
        case MAX5394_W:
            switch (wiper_pos) {
                case MAX5394_WREG:
                    MAX5394_TransmitByte(MAX5394_SD_W);
                    break;
                default:
                    status = UTILS_ERROR;
                    break;
            }
            break;
        case MAX5394_L:
            switch (wiper_pos) {
                case MAX5394_ZERO:
                    MAX5394_TransmitByte(MAX5394_SD_L_ZERO);
                    break;
                case MAX5394_MID:
                    MAX5394_TransmitByte(MAX5394_SD_L_MID);
                    break;
                case MAX5394_FULL:
                    MAX5394_TransmitByte(MAX5394_SD_L_FULL);
                    break;
                case MAX5394_WREG:
                    MAX5394_TransmitByte(MAX5394_SD_L_WREG);
                    break;
                default:
                    status = UTILS_ERROR;
                    break;
            }
            break;
        default:
            status = UTILS_ERROR;
            break;
    }    
    MAX5394_CS_Disable();
    return status;
}

/*
 * @brief               重启
 */
void MAX5394_Reset(void) {
    MAX5394_CS_Enable();
    MAX5394_TransmitByte(MAX5394_RST);
    MAX5394_CS_Disable();
}

/*
 * @brief               开启电荷泵
 */
void MAX5394_QP_Start(void) {
    MAX5394_CS_Enable();
    MAX5394_TransmitByte(MAX5394_QP_ON);
    MAX5394_CS_Disable();
}

/*
 * @brief               关闭电荷泵
 */
void MAX5394_QP_Stop(void) {
    MAX5394_CS_Enable();
    MAX5394_TransmitByte(MAX5394_QP_OFF);
    MAX5394_CS_Disable();
}
