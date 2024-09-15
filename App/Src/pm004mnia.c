#include "pm004mnia.h"

//------------------------------------------------------------
//Created Time  : 2024.08.21
//Author        : JaydenLe
//------------------------------------------------------------

//------------------------------仅内部可用，外部不可用------------------------------
#define PM004MNIA_Transmit_Receive_Start()   HAL_GPIO_WritePin(pm004mnia_obj->cs_pin_type, pm004mnia_obj->cs_pin, GPIO_PIN_RESET)
#define PM004MNIA_Transmit_Receive_Stop()    HAL_GPIO_WritePin(pm004mnia_obj->cs_pin_type, pm004mnia_obj->cs_pin, GPIO_PIN_SET)
//------------------------------仅内部可用，外部不可用------------------------------


//------------------------------可能需要放入中断回调函数中的函数------------------------------
/*
 * @brief               pm004mnia 传输中断处理函数
 * @param pm004mnia_obj pm004mnia 指定信息
 * @param spi           使用的spi结构体
 * @return              无
 * @note                如何使用IT/DMA进行传输的话,需要把该函数丢到以下函数中去
 *                      void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
 */
void PM004MNIA_Transmit_IRQ_Handler(PM004MNIA_Info_Struct* pm004mnia_obj, SPI_HandleTypeDef* spi) {
    if (pm004mnia_obj->spi == spi) {
        //------------------------------更新Read的状态机------------------------------
        if (pm004mnia_obj->_fsm_state_read == PM004MNIA_READ_ADDR_Transmiting) 
            pm004mnia_obj->_fsm_state_read = PM004MNIA_READ_ADDR_Transmit_Finish;

        //------------------------------更新传输的状态机------------------------------
        pm004mnia_obj->_fsm_state_transmit = PM004MNIA_Idle;

        //------------------------------拉低片选引脚------------------------------
        if (pm004mnia_obj->_fsm_state_receive == PM004MNIA_Idle)          // 接收和传输都空闲了,把片选信号拉高
            PM004MNIA_Transmit_Receive_Stop();
    }
}

/*
 * @brief               pm004mnia 接收中断处理函数
 * @param pm004mnia_obj pm004mnia 指定信息
 * @param spi           使用的spi结构体
 * @return              无
 * @note                如何使用IT/DMA进行传输的话,需要把该函数丢到以下函数中去
 *                      void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
 */
void PM004MNIA_Receive_IRQ_Handler(PM004MNIA_Info_Struct* pm004mnia_obj, SPI_HandleTypeDef* spi) {
    if (pm004mnia_obj->spi == spi) {
        //------------------------------更新Read的状态机------------------------------
        if (pm004mnia_obj->_fsm_state_read == PM004MNIA_READ_Receiving) 
            pm004mnia_obj->_fsm_state_read = PM004MNIA_READ_Receive_Finish;

        //------------------------------更新接收的状态机------------------------------
        pm004mnia_obj->_fsm_state_receive = PM004MNIA_Idle;

        //------------------------------拉低片选引脚------------------------------
        if (pm004mnia_obj->_fsm_state_transmit == PM004MNIA_Idle)         // 接收和传输都空闲了,把片选信号拉高
            PM004MNIA_Transmit_Receive_Stop();
    }
}
//------------------------------可能需要放入中断回调函数中的函数------------------------------

/*
 * @brief               传输8bit数据
 * @param pm004mnia_obj pm004mnia 指定信息
 * @param tx_data       需要传输的数据
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
static UTILS_Status PM004MNIA_Transmit_8bit(PM004MNIA_Info_Struct* pm004mnia_obj, uint8_t tx_data) {
    UTILS_Status status = UTILS_OK;
    PM004MNIA_Transmit_Receive_Start();
    if (pm004mnia_obj->mode == UTILS_LOOP) {
        if (HAL_SPI_Transmit(pm004mnia_obj->spi, &tx_data, 1, HAL_MAX_DELAY) != HAL_OK)
            status = UTILS_ERROR;
        else {
            pm004mnia_obj->_fsm_state_transmit = PM004MNIA_Idle;
            if (pm004mnia_obj->_fsm_state_receive == PM004MNIA_Idle)
                PM004MNIA_Transmit_Receive_Stop();
        }
    }
    else if (pm004mnia_obj->mode == UTILS_IT) {
        if (HAL_SPI_Transmit_IT(pm004mnia_obj->spi, &tx_data, 1) != HAL_OK)
            status = UTILS_ERROR;
        else
            pm004mnia_obj->_fsm_state_transmit = PM004MNIA_Transmiting;
    }
    else if (pm004mnia_obj->mode == UTILS_DMA) {
        if (HAL_SPI_Transmit_DMA(pm004mnia_obj->spi, &tx_data, 1) != HAL_OK)
            status = UTILS_ERROR;
        else
            pm004mnia_obj->_fsm_state_transmit = PM004MNIA_Transmiting;
    }
    else
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               传输8bit数据
 * @param pm004mnia_obj pm004mnia 指定信息
 * @param tx_data       需要传输的数据
 * @param len           传输数据的长度
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
static UTILS_Status PM004MNIA_Transmit_8bit_Array(PM004MNIA_Info_Struct* pm004mnia_obj, uint8_t* tx_data, uint32_t len) {
    UTILS_Status status = UTILS_OK;
    PM004MNIA_Transmit_Receive_Start();
    if (pm004mnia_obj->mode == UTILS_LOOP) {
        if (HAL_SPI_Transmit(pm004mnia_obj->spi, tx_data, len, HAL_MAX_DELAY) != HAL_OK)
            status = UTILS_ERROR;
        else {
            pm004mnia_obj->_fsm_state_transmit = PM004MNIA_Idle;
            if (pm004mnia_obj->_fsm_state_receive == PM004MNIA_Idle)
                PM004MNIA_Transmit_Receive_Stop();
        }
    }
    else if (pm004mnia_obj->mode == UTILS_IT) {
        if (HAL_SPI_Transmit_IT(pm004mnia_obj->spi, tx_data, len) != HAL_OK)
            status = UTILS_ERROR;
        else
            pm004mnia_obj->_fsm_state_transmit = PM004MNIA_Transmiting;
    }
    else if (pm004mnia_obj->mode == UTILS_DMA) {
        if (HAL_SPI_Transmit_DMA(pm004mnia_obj->spi, tx_data, len) != HAL_OK)
            status = UTILS_ERROR;
        else
            pm004mnia_obj->_fsm_state_transmit = PM004MNIA_Transmiting;
    }
    else
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               接收数据(16bit)
 * @param pm004mnia_obj pm004mnia 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
static UTILS_Status PM004MNIA_Receive_Data(PM004MNIA_Info_Struct* pm004mnia_obj) {
    UTILS_Status status = UTILS_OK; 
    PM004MNIA_Transmit_Receive_Start();
    if (pm004mnia_obj->mode == UTILS_LOOP) {
        if (HAL_SPI_Receive(pm004mnia_obj->spi, (uint8_t*)&(pm004mnia_obj->_data_read_result), 2, HAL_MAX_DELAY) != HAL_OK)
            status = UTILS_ERROR;
        else {
            pm004mnia_obj->_fsm_state_receive = PM004MNIA_Idle;
            if (pm004mnia_obj->_fsm_state_transmit == PM004MNIA_Idle)
                PM004MNIA_Transmit_Receive_Stop();
        }
    }
    else if (pm004mnia_obj->mode == UTILS_IT) {
        if (HAL_SPI_Receive_IT(pm004mnia_obj->spi, (uint8_t*)&(pm004mnia_obj->_data_read_result), 2) != HAL_OK)
            status = UTILS_ERROR;
        else
            pm004mnia_obj->_fsm_state_receive = PM004MNIA_Receiving;
    }
    else if (pm004mnia_obj->mode == UTILS_DMA) {
        if (HAL_SPI_Receive_DMA(pm004mnia_obj->spi, (uint8_t*)&(pm004mnia_obj->_data_read_result), 2) != HAL_OK)
            status = UTILS_ERROR;
        else
            pm004mnia_obj->_fsm_state_receive = PM004MNIA_Receiving;
    }
    else
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               接收寄存器数据(8bit)
 * @param pm004mnia_obj pm004mnia 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
static UTILS_Status PM004MNIA_Receive_Data_Register(PM004MNIA_Info_Struct* pm004mnia_obj) {
    UTILS_Status status = UTILS_OK; 
    PM004MNIA_Transmit_Receive_Start();
    if (pm004mnia_obj->mode == UTILS_LOOP) {
        if (HAL_SPI_Receive(pm004mnia_obj->spi, (uint8_t*)&(pm004mnia_obj->_register_read_result), 2, HAL_MAX_DELAY) != HAL_OK)
            status = UTILS_ERROR;
        else {
            pm004mnia_obj->_fsm_state_receive = PM004MNIA_Idle;
            if (pm004mnia_obj->_fsm_state_transmit == PM004MNIA_Idle)
                PM004MNIA_Transmit_Receive_Stop();
        }
    }
    else if (pm004mnia_obj->mode == UTILS_IT) {
        if (HAL_SPI_Receive_IT(pm004mnia_obj->spi, (uint8_t*)&(pm004mnia_obj->_register_read_result), 2) != HAL_OK)
            status = UTILS_ERROR;
        else
            pm004mnia_obj->_fsm_state_receive = PM004MNIA_Receiving;
    }
    else if (pm004mnia_obj->mode == UTILS_DMA) {
        if (HAL_SPI_Receive_DMA(pm004mnia_obj->spi, (uint8_t*)&(pm004mnia_obj->_register_read_result), 2) != HAL_OK)
            status = UTILS_ERROR;
        else
            pm004mnia_obj->_fsm_state_receive = PM004MNIA_Receiving;
    }
    else
        status = UTILS_ERROR;
    return status;
}


/*
 * @brief               重启设备
 * @param pm004mnia_obj pm004mnia 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行
 */
UTILS_Status PM004MNIA_Reset(PM004MNIA_Info_Struct* pm004mnia_obj) {
    pm004mnia_obj->_fsm_state_receive = PM004MNIA_Idle;
    pm004mnia_obj->_fsm_state_transmit = PM004MNIA_Idle;
    pm004mnia_obj->_fsm_state_read = PM004MNIA_READ_Idle;
    pm004mnia_obj->_data_read_result = 0;
    pm004mnia_obj->_register_read_result = 0;
    pm004mnia_obj->_data_read_addr = 0xFFFFFFFF;

    uint8_t command[2];
    command[0] = PM004MNIA_CMD_RESET_ENABLE;
    command[1] = PM004MNIA_CMD_RESET;
    return PM004MNIA_Transmit_8bit_Array(pm004mnia_obj, command, 2);
}

/*
 * @brief               写使能
 * @param pm004mnia_obj pm004mnia 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行
 */
UTILS_Status PM004MNIA_WriteEnable(PM004MNIA_Info_Struct* pm004mnia_obj) {
    uint8_t command = PM004MNIA_CMD_WRITE_ENABLE;
    return PM004MNIA_Transmit_8bit(pm004mnia_obj, command);
}

/*
 * @brief               写失能
 * @param pm004mnia_obj pm004mnia 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行
 */
UTILS_Status PM004MNIA_WriteDisable(PM004MNIA_Info_Struct* pm004mnia_obj) {
    uint8_t command = PM004MNIA_CMD_WRITE_ENABLE;
    return PM004MNIA_Transmit_8bit(pm004mnia_obj, command);
}


/*
 * @brief               写数据
 * @param pm004mnia_obj pm004mnia 指定信息
 * @param addr          需要写数据的地址
 * @param data          需要写入的数据
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行,通过返回值判断是否传输成功
 */
UTILS_Status PM004MNIA_Write(PM004MNIA_Info_Struct* pm004mnia_obj, uint32_t addr, uint16_t data) {
    uint8_t command = PM004MNIA_CMD_WRITE;
    uint8_t packet[6];
    packet[0] = command;                            // 装载指令
    for (uint8_t i = 1; i < 4; ++i)                 // 装载地址
        packet[i] = (uint8_t)(addr >> (24 - i * 8) & 0xFF);
    packet[4] = (uint8_t)((data >> 8) & 0xFF);      // 装载数据
    packet[5] = (uint8_t)(data & 0xFF);      
    return PM004MNIA_Transmit_8bit_Array(pm004mnia_obj, packet, 6);
}

/*
 * @brief               向指定寄存器写数据
 * @param pm004mnia_obj pm004mnia 指定信息
 * @param addr          需要写数据的地址
 * @param data          需要写入的数据
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行,通过返回值判断是否传输成功
 */
UTILS_Status PM004MNIA_Write_Register(PM004MNIA_Info_Struct* pm004mnia_obj, uint32_t addr, uint8_t data) {
    uint8_t command = PM004MNIA_CMD_MODE_REGISTER_WRITE;
    uint8_t packet[5];
    packet[0] = command;                            // 装载指令
    for (uint8_t i = 1; i < 4; ++i)                 // 装载地址
        packet[i] = (uint8_t)(addr >> (24 - i * 8) & 0xFF);
    packet[4] = data;
    return PM004MNIA_Transmit_8bit_Array(pm004mnia_obj, packet, 5);
}

/*
 * @brief               读取指定地址的数据, 并将数据写到结构体的缓存中
 * @param pm004mnia_obj pm004mnia 指定信息
 * @param addr          需要读数据的地址
 * @param mem_type      需要读取的存储器类型
 * @return              UTILS_OK        : 读取完成
 *                      UTILS_WORKING   : 正在工作
 * @note                你需要通过函数 PM004MNIA_GetData(PM004MNIA_Info_Struct* pm004mnia_obj) 函数来获取具体结构
 * @example 
 * while (1) {
 *  // 主循环
 *  // 你的代码...
 *  if (PM004MNIA_Read(pm004mnia_obj, addr) == UTILS_OK) {
 *      uint16_t data = PM004MNIA_GetData(pm004mnia_obj);
 *      // 你的代码...
 *  }
 *  // 你的代码...
 * }
 */
UTILS_Status PM004MNIA_Read(PM004MNIA_Info_Struct* pm004mnia_obj, uint32_t addr, PM004MNIA_MEM_TYPE mem_type) {
    UTILS_Status status = UTILS_WORKING;
    if (pm004mnia_obj->mode == UTILS_DMA | pm004mnia_obj->mode == UTILS_IT) {
        if (pm004mnia_obj->_fsm_state_read == PM004MNIA_READ_Idle) {
            pm004mnia_obj->_data_read_addr = addr;
            uint8_t command = PM004MNIA_CMD_READ;
            uint8_t packet[4];
            packet[0] = command;                            // 装载指令
            for (uint8_t i = 1; i < 4; ++i)                 // 装载地址
                packet[i] = (uint8_t)(addr >> (24 - i * 8) & 0xFF);
            if (PM004MNIA_Transmit_8bit_Array(pm004mnia_obj, packet, 4) == UTILS_OK) {
                pm004mnia_obj->_fsm_state_read = PM004MNIA_READ_ADDR_Transmiting;
            }
            else 
                status = UTILS_ERROR;
        }

        if (pm004mnia_obj->_fsm_state_read == PM004MNIA_READ_ADDR_Transmit_Finish) {
            if (pm004mnia_obj->_data_read_addr != addr) {           // 通过 addr 来判断是否被被相同的对象调用
                status = UTILS_ERROR;
            }
            else {
                if (mem_type == PM004MNIA_Memory) {
                    if (PM004MNIA_Receive_Data(pm004mnia_obj) == UTILS_OK) 
                        pm004mnia_obj->_fsm_state_read = PM004MNIA_READ_Receiving;
                    else 
                        status = UTILS_ERROR;
                }
                else if (mem_type == PM004MNIA_Register) {
                    if (PM004MNIA_Receive_Data_Register(pm004mnia_obj) == UTILS_OK) 
                        pm004mnia_obj->_fsm_state_read = PM004MNIA_READ_Receiving;
                    else 
                        status = UTILS_ERROR;
                }
            }
        }

        if (pm004mnia_obj->_fsm_state_read == PM004MNIA_READ_Receive_Finish) {
            if (pm004mnia_obj->_data_read_addr != addr) {
                status = UTILS_ERROR;
            }
            else {
                pm004mnia_obj->_fsm_state_read = PM004MNIA_READ_Idle;
                status = UTILS_OK;
            }
        }
    }
    else if (pm004mnia_obj->mode == UTILS_LOOP) {
        uint8_t command = PM004MNIA_CMD_READ;
        uint8_t packet[4];
        packet[0] = command;                            // 装载指令
        for (uint8_t i = 1; i < 4; ++i)                 // 装载地址
            packet[i] = (uint8_t)(addr >> (24 - i * 8) & 0xFF);
        if (PM004MNIA_Transmit_8bit_Array(pm004mnia_obj, packet, 4) != UTILS_OK)
            return UTILS_ERROR;

        if (mem_type == PM004MNIA_Memory) {
            if (PM004MNIA_Receive_Data(pm004mnia_obj) != UTILS_OK)
                return UTILS_ERROR;
        }
        else if (mem_type == PM004MNIA_Register) {
            if (PM004MNIA_Receive_Data_Register(pm004mnia_obj) != UTILS_OK)
                return UTILS_ERROR;
        }

        status = UTILS_OK;
    }
    return status;
}

/*
 * @brief               获取存储缓冲区中的数据
 * @param pm004mnia_obj pm004mnia 指定信息
 * @return              读取到的数据
 */
uint16_t PM004MNIA_GetData(PM004MNIA_Info_Struct* pm004mnia_obj) {
    return pm004mnia_obj->_data_read_result;
}

/*
 * @brief               获取寄存器缓冲区中的数据
 * @param pm004mnia_obj pm004mnia 指定信息
 * @return              读取到的数据
 */
uint16_t PM004MNIA_GetData_Register(PM004MNIA_Info_Struct* pm004mnia_obj) {
    return pm004mnia_obj->_register_read_result;
}

/*
 * @brief               存储器进入休眠模式
 * @param pm004mnia_obj pm004mnia 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行,通过返回值判断是否传输成功
 */
UTILS_Status PM004MNIA_Sleep(PM004MNIA_Info_Struct* pm004mnia_obj) {
    uint8_t command = PM004MNIA_CMD_ENTRY_TO_DEEP_POWER_DOWN;
    return PM004MNIA_Transmit_8bit(pm004mnia_obj, command);
}

/*
 * @brief               唤醒存储器
 * @param pm004mnia_obj pm004mnia 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行,通过返回值判断是否传输成功
 */
UTILS_Status PM004MNIA_WakeUP(PM004MNIA_Info_Struct* pm004mnia_obj) {
    uint8_t command = PM004MNIA_CMD_EXIT_FROM_DEEP_POWER_DOWN;
    return PM004MNIA_Transmit_8bit(pm004mnia_obj, command);
}


/*
 * @brief               初始化一个pm004mnia硬件设备的配置信息
 * @param pm004mnia_obj pm004mnia 指定信息
 * @param spi           使用的spi结构体
 * @param cs_pin        片选引脚
 * @param cs_pin_type   片选引脚的GPIO类型
 * @return              无
 */
void PM004MNIA_Init(PM004MNIA_Info_Struct* pm004mnia_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type, UTILS_CommunicationMode mode) {
    //------------------------------挂载数据到结构体上------------------------------
    pm004mnia_obj->spi = spi;
    pm004mnia_obj->cs_pin = cs_pin;
    pm004mnia_obj->cs_pin_type = cs_pin_type;
    pm004mnia_obj->mode = mode;

    pm004mnia_obj->_fsm_state_receive = PM004MNIA_Idle;
    pm004mnia_obj->_fsm_state_transmit = PM004MNIA_Idle;
    pm004mnia_obj->_fsm_state_read = PM004MNIA_READ_Idle;
    pm004mnia_obj->_data_read_result = 0;
    pm004mnia_obj->_register_read_result = 0;
    pm004mnia_obj->_data_read_addr = 0xFFFFFFFF;
    //------------------------------配置CS引脚------------------------------
    UTILS_RCC_GPIO_Enable(pm004mnia_obj->cs_pin_type);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    HAL_GPIO_WritePin(pm004mnia_obj->cs_pin_type, pm004mnia_obj->cs_pin, GPIO_PIN_SET);
    
    GPIO_InitStruct.Pin = pm004mnia_obj->cs_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(pm004mnia_obj->cs_pin_type, &GPIO_InitStruct);
}
