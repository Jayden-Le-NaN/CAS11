#include "gd5f2gm7.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_spi.h"
#include "utils.h"

//------------------------------------------------------------
// Created Time     : 2024.10.06
// Author           : JaydenLee
// Note             : 只有接收数据和发送数据使用DMA,像发送指令这些使用阻塞式发送
//------------------------------------------------------------

//-------------------------------全局变量-------------------------------
extern Timemeter_Struct timemeter_obj;

//------------------------------仅内部使用,外部不可用------------------------------
#define GD5F2GM7_Transmit_Receive_Start()   HAL_GPIO_WritePin(gd5f2gm7_obj->cs_pin_type, gd5f2gm7_obj->cs_pin, GPIO_PIN_RESET)
#define GD5F2GM7_Transmit_Receive_Stop()    HAL_GPIO_WritePin(gd5f2gm7_obj->cs_pin_type, gd5f2gm7_obj->cs_pin, GPIO_PIN_SET)
#define _LOAD_TIM_REG(tim)                       do{tim->Instance->EGR = TIM_EGR_UG; if (HAL_IS_BIT_SET(tim->Instance->SR, TIM_FLAG_UPDATE)){CLEAR_BIT(tim->Instance->SR, TIM_FLAG_UPDATE);}}while(0)
//------------------------------仅内部使用,外部不可用------------------------------


//------------------------------需要放入中断回调函数中的函数------------------------------
void GD5F2GM7_Transmit_IRQ_Handler(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi) {
    if (gd5f2gm7_obj->spi == spi) {
        gd5f2gm7_obj->_dma_fsm_state_transmit = GD5F2GM7_DMA_Idle;
        if (gd5f2gm7_obj->_dma_fsm_state_receive == GD5F2GM7_DMA_Idle)
            GD5F2GM7_Transmit_Receive_Stop();
    }
}

void GD5F2GM7_Receive_IRQ_Handler(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi) {
    if (gd5f2gm7_obj->spi == spi) {
        gd5f2gm7_obj->_dma_fsm_state_receive = GD5F2GM7_DMA_Idle;
        if (gd5f2gm7_obj->_dma_fsm_state_transmit == GD5F2GM7_DMA_Idle)
            GD5F2GM7_Transmit_Receive_Stop();
    }
    if(gd5f2gm7_obj->tim_usage == GD5F2GM7_USE_TIM){
        gd5f2gm7_obj->tim_usage = GD5F2GM7_NOUSE_TIM;
        __HAL_TIM_DISABLE(gd5f2gm7_obj->tim);
        __HAL_TIM_URS_ENABLE(gd5f2gm7_obj->tim);    // 重要，否则_LOAD_TIM_REG会产生中断
        __HAL_TIM_SET_COUNTER(gd5f2gm7_obj->tim, 0);
        __HAL_TIM_SET_PRESCALER(gd5f2gm7_obj->tim, MCU_FREQUENCY_MHZ*1000 - 1);
        __HAL_TIM_SetAutoreload(gd5f2gm7_obj->tim, gd5f2gm7_obj->tim_ms);
        __HAL_TIM_ENABLE_IT(gd5f2gm7_obj->tim, TIM_IT_UPDATE);
        _LOAD_TIM_REG(gd5f2gm7_obj->tim);
        __HAL_TIM_ENABLE(gd5f2gm7_obj->tim);
        // HAL_TIM_Base_Start_IT(gd5f2gm7_obj->tim);
        // printf("ts\r\n");
    }
    // time_calculator_end(&time_calculator_obj);
    // printf("time: %d us\r\n", time_calculator_obj.interval);
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
    else
        gd5f2gm7_obj->_dma_fsm_state_transmit = GD5F2GM7_DMA_Transmiting;
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
    else
        gd5f2gm7_obj->_dma_fsm_state_receive = GD5F2GM7_DMA_Receiving;
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

// UTILS_Status GD52GM7_Set_BlockProtection(GD5F2GM7_Info_Struct* gd5f2gm7_obj, BLOCKLOCK_Status status){
//     uint8_t temp = 0x00;

//     if(GD5F2GM7_Get_Features(gd5f2gm7_obj, 0xA0, &temp) != UTILS_OK){
//         printf("GD5F2GM7_Get_Features err");
//         return UTILS_ERROR;
//     }
//     if(temp)
//     temp |= (uint8_t)status;

//     GD5F2GM7_Set_Features(gd5f2gm7_obj, )
// }

/*
 * @brief               设置寄存器中的值
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param reg_addr      寄存器的地址
 * @param tx_data       需要设置的地址
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
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

UTILS_Status GD5F2GM7_isBusy(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t* bit_status){
    if(GD5F2GM7_Get_Features(gd5f2gm7_obj, 0xC0, bit_status) != UTILS_OK){
        return UTILS_ERROR;
    }
    // printf("%x\r\n", (*bit_status));
    *bit_status &= OIP_MASK;
    return UTILS_OK;
}

/*
 * @brief               把指定页面的数据加载到cache中
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param page_addr     页的地址
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
UTILS_Status GD5F2GM7_PageRead_ToCache(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t page_addr) {
    uint8_t packet[4];
    uint8_t command = GD5F2GM7_CMD_PAGE_READ_TO_CACHE;
    packet[0] = command;
    for (uint8_t i = 1; i < 4; ++i)
        packet[i] = (uint8_t)(page_addr >> (24 - i * 8) & 0xFF);
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, packet, 4);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               从缓冲区读取数据 
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param cache_addr    需要读取的缓冲区地址
 * @param rx_data       存储数据的地址
 * @param len           读取数据的长度
 * @param rx_mode       接收数据的模式
 *                          UTILS_LOOP: 使用阻塞的方式接收
 *                          UTILS_DMA : 使用DMA的方式接收
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
UTILS_Status GD5F2GM7_ReadFromCache(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t cache_addr, uint8_t* rx_data, uint32_t len, UTILS_CommunicationMode rx_mode, GD5F2GM7_TIM tim_usage) {
    uint8_t packet[4];
    uint8_t command = GD5F2GM7_CMD_READ_FROM_CACHE;
    packet[0] = command;
    for (uint8_t i = 1; i < 3; ++i)
        packet[i] = (uint8_t)(cache_addr >> (16 - i * 8) & 0xFF);
    packet[3] = 0;
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, packet, 4);

    do {
        if (status != UTILS_OK)
            break;

        if (rx_mode == UTILS_LOOP) {
            status = GD5F2GM7_Receive_8bit_Array(gd5f2gm7_obj, rx_data, len);
            GD5F2GM7_Transmit_Receive_Stop();
        }
        else if (rx_mode == UTILS_DMA){
            gd5f2gm7_obj->tim_usage = tim_usage;
            status = GD5F2GM7_Receive_8bit_Array_DMA(gd5f2gm7_obj, rx_data, len);
        }
        else
            status = UTILS_ERROR;
    } while(0);
    
    return status;
}

/*
 * @brief               读取芯片ID
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param rx_data       存储数据的地址
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 * @note                rx_data[0] = MID, rx_data[1] = DID
 */
UTILS_Status GD5F2GM7_ReadID(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t* rx_data) {
    uint8_t packet[2];
    uint8_t command = GD5F2GM7_CMD_READ_ID;
    packet[0] = command;
    packet[1] = 0;

    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, packet, 2);
    if (status == UTILS_OK)
        status = GD5F2GM7_Receive_8bit_Array(gd5f2gm7_obj, rx_data, 2);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               把参数页读取到cache中
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
UTILS_Status GD5F2GM7_ReadParameterPage(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    uint8_t packet[4];
    uint8_t command = GD5F2GM7_CMD_READ_PARAMETER_PAGE;
    packet[0] = command;
    packet[1] = 0;
    packet[2] = 0;
    packet[3] = 0x01;

    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, packet, 4);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               把设备UID加载到cache中
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
UTILS_Status GD5F2GM7_ReadUID(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    uint8_t packet[4];
    uint8_t command = GD5F2GM7_CMD_READ_UID;
    packet[0] = command;
    packet[1] = 0;
    packet[2] = 0;
    packet[3] = 0;

    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, packet, 4);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               把整页数据打到缓冲区中
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param cache_addr    缓冲区地址
 * @param tx_data       需要发送的数据
 * @param len           发送数据的长度
 * @param tx_mode       发送数据的模式
 *                          UTILS_LOOP: 使用阻塞的方式接收
 *                          UTILS_DMA : 使用DMA的方式接收
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输或者是传输模式选择错误
 */
UTILS_Status GD5F2GM7_ProgramLoad(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t cache_addr, uint8_t* tx_data, uint32_t len, UTILS_CommunicationMode tx_mode) {
    uint8_t B0_reg = 0;
    GD5F2GM7_Get_Features(gd5f2gm7_obj, 0xB0, &B0_reg);
    uint8_t ECC_EN_feature = 0x10 & B0_reg;
    // printf("B0_reg: %x\r\n", B0_reg);
    // printf("ECC_EN_feature: %x\r\n", (!ECC_EN_feature));
    if(ECC_EN_feature && (len != 2112)){
        printf("Flash err: ECC enabled, len must be 2112\r\n");
        return UTILS_ERROR;
    }else if((!ECC_EN_feature) && len != 2176U){
        printf("Flash err: ECC disabled, len must be 2176\r\n");
        return UTILS_ERROR;
    }
    // if(cache_addr == 0)printf("%x\r\n", tx_data[3]);
    uint8_t packet[3];
    uint8_t command = GD5F2GM7_CMD_PROGRAM_LOAD;
    packet[0] = command;
    for (uint8_t i = 1; i < 3; ++i)
        packet[i] = (uint8_t)(cache_addr >> (16 - i * 8) & 0xFF);
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, packet, 3);

    do {
        if (status != UTILS_OK)
            break;

        if (tx_mode == UTILS_LOOP) {
            status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, tx_data, len);
            GD5F2GM7_Transmit_Receive_Stop();
        }
        else if (tx_mode == UTILS_DMA) 
            status = GD5F2GM7_Transmit_8bit_Array_DMA(gd5f2gm7_obj, tx_data, len);
        else
            status = UTILS_ERROR;
    } while(0);

    return status;
}

/*
 * @brief               把cache中的数据写入到指定的页中
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param page_addr     页的地址
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时
 */
UTILS_Status GD5F2GM7_ProgramExecute(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t page_addr) {
    uint8_t packet[4];
    uint8_t command = GD5F2GM7_CMD_PROGRAM_EXECUTE;
    packet[0] = command;
    for (uint8_t i = 1; i < 4; ++i)
        packet[i] = (uint8_t)(page_addr >> (24 - i * 8) & 0xFF);
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_WriteEnable(gd5f2gm7_obj);
    GD5F2GM7_Transmit_Receive_Stop();       // 中间要把cs拉高
    // TODO: check WEL bit
    if(status == UTILS_ERROR){
        return status;
    }
    GD5F2GM7_Transmit_Receive_Start();
    status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, packet, 4);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}



/*
 * @brief               把数据打到缓冲区中(对数据进行Replace, 保留cache中的其他数据)
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param cache_addr    缓冲区地址
 * @param tx_data       需要发送的数据
 * @param len           发送数据的长度
 * @param tx_mode       发送数据的模式
 *                          UTILS_LOOP: 使用阻塞的方式接收
 *                          UTILS_DMA : 使用DMA的方式接收
 * @return              UTILS_OK    : 正常
 *                      UTILS_ERROR : 发生错误,可能是操作超时或者是已经有数据正在传输
 */
UTILS_Status GD5F2GM7_ProgramLoadRandomData(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t cache_addr, uint8_t* tx_data, uint32_t len, UTILS_CommunicationMode tx_mode) {
    uint8_t packet[3];
    uint8_t command = GD5F2GM7_CMD_PROGRAM_LOAD_RANDOM_DATA;
    packet[0] = command;
    for (uint8_t i = 1; i < 3; ++i)
        packet[i] = (uint8_t)(cache_addr >> (16 - i * 8) & 0xFF);
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, packet, 3);

    do {
        if (status != UTILS_OK)
            break;

        if (tx_mode == UTILS_LOOP) {
            status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, tx_data, len);
            GD5F2GM7_Transmit_Receive_Stop();
        }
        else if (tx_mode == UTILS_DMA) 
            status = GD5F2GM7_Transmit_8bit_Array_DMA(gd5f2gm7_obj, tx_data, len);
        else
            status = UTILS_ERROR;
    } while(0);

    return status;
}

/*
 * @brief               擦除指定页的数据
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @param page_addr     页的地址
 * @return              UTILS_OK        : 设备空闲
 *                      UTILS_ERROR     : 发生错误,可能是操作超时
 * @note                该操作的执行时间较长
 */
UTILS_Status GD52GM7_BlockErase(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t page_addr) {
    uint8_t packet[4];
    uint8_t command = GD5F2GM7_CMD_BLOCK_ERASE;
    packet[0] = command;
    for (uint8_t i = 1; i < 4; ++i)
        packet[i] = (uint8_t)(page_addr >> (24 - i * 8) & 0xFF);
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_WriteEnable(gd5f2gm7_obj);
    GD5F2GM7_Transmit_Receive_Stop();   // 中间需要把cs拉高
    // TODO: check WEL bit
    if(status == UTILS_ERROR){
        return status;
    }
    GD5F2GM7_Transmit_Receive_Start();
    status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, packet, 4);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               重启设备
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK        : 设备空闲
 *                      UTILS_ERROR     : 发生错误,可能是操作超时
 * @note                该命令为停止所有操作, 但不会回到默认状态
 */
UTILS_Status GD5F2GM7_Reset(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    uint8_t command = GD5F2GM7_CMD_RESET;
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, &command, 1);
    if (status == UTILS_OK) {
        gd5f2gm7_obj->_dma_fsm_state_transmit = GD5F2GM7_DMA_Idle;
        gd5f2gm7_obj->_dma_fsm_state_receive  = GD5F2GM7_DMA_Idle;
    }
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               使能上电热重启
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK        : 设备空闲
 *                      UTILS_ERROR     : 发生错误,可能是操作超时
 */
UTILS_Status GD5F2GM7_EnablePowerOnReset(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    uint8_t command = GD5F2GM7_CMD_ENABLE_POWER_ON_RESET;
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, &command, 1);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               上电热重启
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK        : 设备空闲
 *                      UTILS_ERROR     : 发生错误,可能是操作超时
 * @note                该命令会停止所有操作并是设备回到默认状态
 */
UTILS_Status GD5F2GM7_PowerOnReset(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    uint8_t command = GD5F2GM7_CMD_POWER_ON_RESET;
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, &command, 1);
    if (status == UTILS_OK) {
        gd5f2gm7_obj->_dma_fsm_state_transmit = GD5F2GM7_DMA_Idle;
        gd5f2gm7_obj->_dma_fsm_state_receive  = GD5F2GM7_DMA_Idle;
    }
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}


/*
 * @brief               设备进入最低功耗模式
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK        : 设备空闲
 *                      UTILS_ERROR     : 发生错误,可能是操作超时
 * @note                该模式下除了ReleaseDeepPowerDown命令以外,所有其他命令都不再可用
 */
UTILS_Status GD5F2GM7_DeepPowerDown(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    uint8_t command = GD5F2GM7_CMD_DEEP_POWER_DOWN;
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, &command, 1);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}


/*
 * @brief               离开最低功耗模式
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK        : 设备空闲
 *                      UTILS_ERROR     : 发生错误,可能是操作超时
 */
UTILS_Status GD5F2GM7_ReleaseDeepPowerDown(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    uint8_t command = GD5F2GM7_CMD_RELEASE_DEEP_POWER_DOWN;
    GD5F2GM7_Transmit_Receive_Start();
    UTILS_Status status = GD5F2GM7_Transmit_8bit_Array(gd5f2gm7_obj, &command, 1);
    GD5F2GM7_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               检测设备是否忙
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK        : 设备空闲
 *                      UTILS_WORKING   : 设备忙
 *                      UTILS_ERROR     : 发生错误,可能是操作超时
 * @note                设备在执行 PROGRAM EXECUTE, PAGE READ, BLOCK ERASE or RESET 命令的时候设备忙
 */
UTILS_Status GD5F2GM7_DeviceIsBusy(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    UTILS_Status status = UTILS_OK;
    uint8_t reg_status; 
    status = GD5F2GM7_Get_Features(gd5f2gm7_obj, 0xC0, &reg_status);
    if (status == UTILS_OK)
        status = (reg_status & 0x01) == 1 ? UTILS_WORKING : UTILS_OK;
    return status;
}

/*
 * @brief               检查传输DMA是否正在忙
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK        : 设备空闲
 *                      UTILS_WORKING   : 设备忙
 */
UTILS_Status GD5F2GM7_DMA_TransmitIsBusy(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    UTILS_Status status = UTILS_OK;
    if (gd5f2gm7_obj->_dma_fsm_state_transmit == GD5F2GM7_DMA_Transmiting)
        status = UTILS_WORKING;
    return status;
}

/*
 * @brief               检查接收DMA是否正在忙
 * @param gd5f2gm7_obj  gd5f2gm7 指定信息
 * @return              UTILS_OK        : 设备空闲
 *                      UTILS_WORKING   : 设备忙
 */
UTILS_Status GD5F2GM7_DMA_ReceiveIsBusy(GD5F2GM7_Info_Struct* gd5f2gm7_obj) {
    UTILS_Status status = UTILS_OK;
    if (gd5f2gm7_obj->_dma_fsm_state_receive == GD5F2GM7_DMA_Receiving)
        status = UTILS_WORKING;
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
void GD5F2GM7_Init(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type, TIM_HandleTypeDef* tim, uint16_t tim_ms) {
    //------------------------------数据挂载------------------------------
    gd5f2gm7_obj->spi = spi;
    gd5f2gm7_obj->cs_pin = cs_pin;
    gd5f2gm7_obj->cs_pin_type = cs_pin_type;
    gd5f2gm7_obj->tim = tim;
    gd5f2gm7_obj->tim_usage = GD5F2GM7_NOUSE_TIM;
    gd5f2gm7_obj->tim_ms = tim_ms;

    //------------------------------初始化状态机----------------------------
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

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    HAL_GPIO_Init(gd5f2gm7_obj->cs_pin_type, &GPIO_InitStruct);
    HAL_GPIO_WritePin(gd5f2gm7_obj->cs_pin_type, GPIO_PIN_8, GPIO_PIN_SET);
}
