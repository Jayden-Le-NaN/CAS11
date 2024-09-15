#include "gd5f2gm7xexxg.h"
#include "stm32l4xx_hal_spi.h"

//------------------------------------------------------------
//Created Time  : 2024.08.22
//Author        : JaydenLe
//------------------------------------------------------------

//------------------------------仅内部使用, 外部不可用------------------------------
#define GM5F2GM7XEXXG_Transmit_Receive_Start()  HAL_GPIO_WritePin(gm5f2gm7xexxg_obj->cs_pin_type, gm5f2gm7xexxg_obj->cs_pin, GPIO_PIN_RESET)
#define GM5F2GM7XEXXG_Transmit_Receive_Stop()   HAL_GPIO_WritePin(gm5f2gm7xexxg_obj->cs_pin_type, gm5f2gm7xexxg_obj->cs_pin, GPIO_PIN_SET)
//------------------------------仅内部使用, 外部不可用------------------------------


//------------------------------可能需要放入中断回调函数中的函数------------------------------
/*
 * @brief               gm5f2gm7xexxg 传输中断处理函数
 * @param gm5f2gm7xexxg_obj 
 *                      gm5f2gm7xexxg 指定信息
 * @param spi           使用的spi结构体
 * @return              无
 * @note                如何使用IT/DMA进行传输的话,需要把该函数丢到以下函数中去
 *                      void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
 */
void GM5F2GM7XEXXG_Transmit_IRQ_Handler(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, SPI_HandleTypeDef* spi) {
    if (gm5f2gm7xexxg_obj->spi == spi) {
        //------------------------------更新传输的状态机------------------------------
        gm5f2gm7xexxg_obj->_fsm_state_transmit = GM5F2GM7XEXXG_Idle;
        
        //------------------------------更新load page状态机(read)------------------------------
        if (gm5f2gm7xexxg_obj->_fsm_state_read == GM5F2GM7XEXXG_READ_PAGE_ADDR_Transmiting) {
            gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_PAGE_ADDR_Transmit_Finish;
        }

        //------------------------------更新读cache状态机(read)------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_read == GM5F2GM7XEXXG_READ_CACHE_ADDR_Transmiting) {
            gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_CACHE_ADDR_Transmit_Finish;
        }

        //------------------------------更新读寄存器状态机(read)------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_read_register == GM5F2GM7XEXXG_READ_REGISTER_ADDR_Transmiting) {
            gm5f2gm7xexxg_obj->_fsm_state_read_register = GM5F2GM7XEXXG_READ_REGISTER_ADDR_Transmit_Finish;
        }

        //------------------------------更新program load状态机------------------------------
        if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_PROGRAM_Loading) {
            gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Load_Finish;
        }

        else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_PROGRAM_Executing) {
            gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Execute_Finish;
        }

        //------------------------------更新DataMove状态机------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_DATA_MOVE_PageLoad_AddrSending) {
            gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_DATA_MOVE_PageLoad_AddrSend_Finish;
        }

        //------------------------------写数据的命令发送完成(DataMove)------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_DATA_MOVE_Executing) {
            gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_DATA_MOVE_Execute_Finish;
        }

        //------------------------------更新替换数据的状态机------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_REPLACE_PageLoad_AddrSending) {
            gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_REPLACE_PageLoad_AddrSend_Finish;
        }

        //------------------------------更新随机加载的状态机------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_REPLACE_RandomLoading) {
            gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_REPLACE_RandomLoad_Finish;
        }

        //------------------------------写页数据命令发送完成------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_REPLACE_Executing) {
            gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_REPLACE_Execute_Finish;
        }
        
        //------------------------------拉低片选引脚------------------------------
        if (gm5f2gm7xexxg_obj->_fsm_state_receive == GM5F2GM7XEXXG_Idle)
            GM5F2GM7XEXXG_Transmit_Receive_Stop();
    }
}

/*
 * @brief               gm5f2gm7xexxg 接收中断处理函数
 * @param gm5f2gm7xexxg_obj 
 *                      gm5f2gm7xexxg 指定信息
 * @param spi           使用的spi结构体
 * @return              无
 * @note                如何使用IT/DMA进行传输的话,需要把该函数丢到以下函数中去
 *                      void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
 */
void GM5F2GM7XEXXG_Receive_IRQ_Handler(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, SPI_HandleTypeDef* spi) {
    if (gm5f2gm7xexxg_obj->spi == spi) {
        //------------------------------更新传输的状态机------------------------------
        gm5f2gm7xexxg_obj->_fsm_state_receive = GM5F2GM7XEXXG_Idle;

        //------------------------------更新读cache状态机------------------------------
        if (gm5f2gm7xexxg_obj->_fsm_state_read == GM5F2GM7XEXXG_READ_CACHE_Receiving) {
            gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_CACHE_Receive_Finish;
        }

        //------------------------------更新读寄存器状态机------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_read_register == GM5F2GM7XEXXG_READ_REGISTER_Receiving) {
            gm5f2gm7xexxg_obj->_fsm_state_read_register = GM5F2GM7XEXXG_READ_REGISTER_Receive_Finish;
        }

        //------------------------------拉低片选引脚------------------------------
        if (gm5f2gm7xexxg_obj->_fsm_state_transmit == GM5F2GM7XEXXG_Idle)
            GM5F2GM7XEXXG_Transmit_Receive_Stop();
    }
}
//------------------------------可能需要放入中断回调函数中的函数------------------------------

/*
 * @brief               传输8bit数组
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param tx_data       需要传输的数据
 * @param len           需要传输数据的长度
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
static UTILS_Status GM5F2GM7XEXXG_Transmit_8bit_Array(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint8_t* tx_data, uint32_t len) {
    if (gm5f2gm7xexxg_obj->_fsm_state_transmit != GM5F2GM7XEXXG_Idle)
        return UTILS_ERROR;

    UTILS_Status status = UTILS_OK; 
    if (gm5f2gm7xexxg_obj->mode == UTILS_LOOP) {
        if (HAL_SPI_Transmit(gm5f2gm7xexxg_obj->spi, tx_data, len, HAL_MAX_DELAY) != HAL_OK)
            status = UTILS_ERROR;
        else {
            gm5f2gm7xexxg_obj->_fsm_state_transmit = GM5F2GM7XEXXG_Idle;
        }
    }
    else if (gm5f2gm7xexxg_obj->mode == UTILS_IT) {
        gm5f2gm7xexxg_obj->_fsm_state_transmit = GM5F2GM7XEXXG_Transmiting;
        if (HAL_SPI_Transmit_IT(gm5f2gm7xexxg_obj->spi, tx_data, len) != HAL_OK) {
            gm5f2gm7xexxg_obj->_fsm_state_transmit = GM5F2GM7XEXXG_Idle;
            status = UTILS_ERROR;
        }
    }
    else if (gm5f2gm7xexxg_obj->mode == UTILS_DMA) {
        gm5f2gm7xexxg_obj->_fsm_state_transmit = GM5F2GM7XEXXG_Transmiting;
        if (HAL_SPI_Transmit_DMA(gm5f2gm7xexxg_obj->spi, tx_data, len) != HAL_OK) {
            gm5f2gm7xexxg_obj->_fsm_state_transmit = GM5F2GM7XEXXG_Idle;
            status = UTILS_ERROR;
        }
    }
    else 
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               传输8bit数据
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param tx_data       需要传输的数据
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
static UTILS_Status GM5F2GM7XEXXG_Transmit_8bit(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint8_t tx_data) {
    return GM5F2GM7XEXXG_Transmit_8bit_Array(gm5f2gm7xexxg_obj, &tx_data, 1);
}

/*
 * @brief               接收数据
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param rx_data       接收数据存放的首地址
 * @param len           接收数据的长度
 * @return              utils_ok        : 正常
 *                      utils_error     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
static UTILS_Status GM5F2GM7XEXXG_Receive_Data(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint8_t* rx_data, uint32_t len) {
    if (gm5f2gm7xexxg_obj->_fsm_state_receive != GM5F2GM7XEXXG_Idle)
        return UTILS_ERROR;
    UTILS_Status status = UTILS_OK;
    if (gm5f2gm7xexxg_obj->mode == UTILS_LOOP) {
        if (HAL_SPI_Receive(gm5f2gm7xexxg_obj->spi, rx_data, len, HAL_MAX_DELAY) != HAL_OK)
            status = UTILS_ERROR;
        else {
            gm5f2gm7xexxg_obj->_fsm_state_receive = GM5F2GM7XEXXG_Idle;
        }
    }
    else if (gm5f2gm7xexxg_obj->mode == UTILS_IT) {
        if (HAL_SPI_Receive_IT(gm5f2gm7xexxg_obj->spi, rx_data, len) != HAL_OK)
            status = UTILS_ERROR;
        else
            gm5f2gm7xexxg_obj->_fsm_state_receive = GM5F2GM7XEXXG_Idle;
    }
    else if (gm5f2gm7xexxg_obj->mode == UTILS_DMA) {
        gm5f2gm7xexxg_obj->_fsm_state_receive = GM5F2GM7XEXXG_Receiving;
        if (HAL_SPI_Receive_DMA(gm5f2gm7xexxg_obj->spi, rx_data, len) != HAL_OK) {
            gm5f2gm7xexxg_obj->_fsm_state_receive = GM5F2GM7XEXXG_Idle;
            status = UTILS_ERROR;
        }
    }
    else
        status = UTILS_ERROR;
    return status;
}


/*
 * @brief               写使能
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
UTILS_Status GM5F2GM7XEXXG_WriteEnable(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj) {
    uint8_t command = GM5F2GM7XEXXG_CMD_WRITE_ENABLE;
    GM5F2GM7XEXXG_Transmit_Receive_Start();
    HAL_StatusTypeDef status = HAL_SPI_Transmit(gm5f2gm7xexxg_obj->spi, &command, 1, HAL_MAX_DELAY);
    GM5F2GM7XEXXG_Transmit_Receive_Stop();
    if (status != HAL_OK)
        return UTILS_ERROR;
    return UTILS_OK;
}

/*
 * @brief               写失能
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
UTILS_Status GM5F2GM7XEXXG_WriteDisable(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj) {
    uint8_t command = GM5F2GM7XEXXG_CMD_WRITE_DISABLE;
    GM5F2GM7XEXXG_Transmit_Receive_Start();
    HAL_StatusTypeDef status = HAL_SPI_Transmit(gm5f2gm7xexxg_obj->spi, &command, 1, HAL_MAX_DELAY);
    GM5F2GM7XEXXG_Transmit_Receive_Stop();
    if (status != HAL_OK)
        return UTILS_ERROR;
    return UTILS_OK;
}

/*
 * @brief               reset, 但是该模式只是停止所有的操作,寄存器的数据不会更改
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
UTILS_Status GM5F2GM7XEXXG_Reset_Soft(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj) {
    //------------------------------重置状态------------------------------
    gm5f2gm7xexxg_obj->_fsm_state_transmit = GM5F2GM7XEXXG_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_receive = GM5F2GM7XEXXG_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_read_register = GM5F2GM7XEXXG_READ_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Idle;
    gm5f2gm7xexxg_obj->_last_read_page_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_read_cache_addr = 0xFFFF;
    gm5f2gm7xexxg_obj->_last_read_register_addr = 0xFF;
    gm5f2gm7xexxg_obj->_last_data_move_dst_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_data_move_src_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_replace_cache_addr = 0xFFFF;
    gm5f2gm7xexxg_obj->_last_replace_dst_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_replace_src_addr = 0xFFFFFFFF;

    //------------------------------发送重置命令------------------------------
    uint8_t command = GM5F2GM7XEXXG_CMD_RESET;
    GM5F2GM7XEXXG_Transmit_Receive_Start();
    HAL_StatusTypeDef status = HAL_SPI_Transmit(gm5f2gm7xexxg_obj->spi, &command, 1, HAL_MAX_DELAY);
    GM5F2GM7XEXXG_Transmit_Receive_Stop();
    if (status != HAL_OK)
        return UTILS_ERROR;
    return UTILS_OK;
}

/*
 * @brief               配置存储器的寄存器
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param addr          需要配置的寄存器的地址
 * @param data          需要配置的数据
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
UTILS_Status GM5F2GM7XEXXG_Set_Features(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint8_t addr, uint8_t tx_data) {
    uint8_t packet[3];
    uint8_t command = GM5F2GM7XEXXG_CMD_SET_FEATURES; 
    packet[0] = command;
    packet[1] = addr;
    packet[2] = tx_data;
    GM5F2GM7XEXXG_Transmit_Receive_Start();
    UTILS_Status status = GM5F2GM7XEXXG_Transmit_8bit_Array(gm5f2gm7xexxg_obj, packet, 3);
    GM5F2GM7XEXXG_Transmit_Receive_Stop();
    return status;
}

/*
 * @brief               获取寄存器数据
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param addr          需要获取寄存器的地址
 * @param rx_data       存储寄存器数据的地址
 * @return              UTILS_OK        : 数据接收完毕
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 *                      UTILS_WORKING   : 状态机正在工作
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
UTILS_Status GM5F2GM7XEXXG_Get_Features(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint8_t addr, uint8_t* rx_data) {
    UTILS_Status status = UTILS_WORKING;

    //------------------------------防止同时读/同时写的情况出现------------------------------
    // if (gm5f2gm7xexxg_obj->_fsm_state_program != GM5F2GM7XEXXG_PROGRAM_Idle)
    //     return UTILS_ERROR;

    if (gm5f2gm7xexxg_obj->mode == UTILS_IT || gm5f2gm7xexxg_obj->mode == UTILS_DMA) {
        if (gm5f2gm7xexxg_obj->_fsm_state_read_register == GM5F2GM7XEXXG_READ_Idle) {
            gm5f2gm7xexxg_obj->_fsm_state_read_register = GM5F2GM7XEXXG_READ_REGISTER_ADDR_Transmiting;

            //------------------------------发送数据---------------------------------------
            uint8_t packet[2];
            uint8_t command = GM5F2GM7XEXXG_CMD_GET_FEATURES;
            packet[0] = command;
            packet[1] = addr;
            if (GM5F2GM7XEXXG_Transmit_8bit_Array(gm5f2gm7xexxg_obj, packet, 2) != UTILS_OK) {
                gm5f2gm7xexxg_obj->_fsm_state_read_register = GM5F2GM7XEXXG_READ_Idle; 
                return UTILS_ERROR;
            }
            //------------------------------更新相关寄存器------------------------------
            gm5f2gm7xexxg_obj->_last_read_register_addr = addr;
        }
        
        else if (gm5f2gm7xexxg_obj->_fsm_state_read_register == GM5F2GM7XEXXG_READ_REGISTER_ADDR_Transmit_Finish) {
            if (addr != gm5f2gm7xexxg_obj->_last_read_register_addr) {
                status = UTILS_ERROR;
            }
            else {
                gm5f2gm7xexxg_obj->_fsm_state_read_register = GM5F2GM7XEXXG_READ_REGISTER_Receiving;
                if (GM5F2GM7XEXXG_Receive_Data(gm5f2gm7xexxg_obj, rx_data, 1) != UTILS_OK) {
                    gm5f2gm7xexxg_obj->_fsm_state_read_register = GM5F2GM7XEXXG_READ_REGISTER_ADDR_Transmit_Finish;
                    return UTILS_ERROR;
                }
            }
        }

        else if (gm5f2gm7xexxg_obj->_fsm_state_read_register == GM5F2GM7XEXXG_READ_REGISTER_Receive_Finish) {
            if (addr != gm5f2gm7xexxg_obj->_last_read_register_addr) {
                status = UTILS_ERROR;
            }
            else {
                status = UTILS_OK;
                gm5f2gm7xexxg_obj->_fsm_state_read_register = GM5F2GM7XEXXG_READ_Idle;
            }
        }
    }
    else if (gm5f2gm7xexxg_obj->mode == UTILS_LOOP) {
        uint8_t packet[2];
        uint8_t command = GM5F2GM7XEXXG_CMD_GET_FEATURES;
        packet[0] = command;
        packet[1] = addr;
        GM5F2GM7XEXXG_Transmit_Receive_Start();

        UTILS_Status inner_status = GM5F2GM7XEXXG_Transmit_8bit_Array(gm5f2gm7xexxg_obj, packet, 2);
        if (inner_status != UTILS_OK) {
            GM5F2GM7XEXXG_Transmit_Receive_Stop();
            return UTILS_ERROR;
        }

        inner_status = GM5F2GM7XEXXG_Receive_Data(gm5f2gm7xexxg_obj, rx_data, 1);
        GM5F2GM7XEXXG_Transmit_Receive_Stop();
        if (inner_status != UTILS_OK) 
            return UTILS_ERROR;
        status = UTILS_OK;
    }
    return status;
}

/*
 * @brief               检查设备是否在执行某一项命令
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @return              UTILS_OK        : 设备不忙
 *                      UTILS_BUSY      : 设备忙
 *                      UTILS_WORKING   : 正在检测
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
UTILS_Status GM5F2GM7XEXXG_DeviceIsBusy(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj) {
    UTILS_Status status = UTILS_OK;
    uint8_t rx_data;
    status = GM5F2GM7XEXXG_Get_Features(gm5f2gm7xexxg_obj, 0xc0, &rx_data);
    if (status == UTILS_OK) {
        if (rx_data & 0x01)
            status = UTILS_BUSY;
        else
            status = UTILS_OK;
    }
    return status;
}

/*
 * @brief               把指定的页面的数据加载到cache中
 * @param gm5f2gm7xexxg_obj     
 *                      gm5f2gm7xexxg 指定信息
 * @param addr          页的地址 (24为 ra地址)
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
static UTILS_Status GM5F2GM7XEXXG_PageRead_ToCache(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t addr) {
    uint8_t packet[4];
    uint8_t command = GM5F2GM7XEXXG_CMD_PAGE_READ_TO_CACHE;
    packet[0] = command;
    for (uint8_t i = 1; i < 4; ++i)                 // 装载地址
        packet[i] = (uint8_t)(addr >> (24 - i * 8) & 0xFF);
    return GM5F2GM7XEXXG_Transmit_8bit_Array(gm5f2gm7xexxg_obj, packet, 4);
}

/*
 * @brief               从缓冲区读取数据-发送地址部分
 * @param gm5f2gm7xexxg_obj     
 *                      gm5f2gm7xexxg 指定信息
 * @param addr          需要读取的数据的地址
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
static UTILS_Status GM5F2GM7XEXXG_ReadDataFromCache_TransmitAddr(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint16_t addr) {
    uint8_t packet[4];
    uint8_t command = GM5F2GM7XEXXG_CMD_READ_FROM_CACHE;
    uint8_t dummy = 0;
    packet[0] = command;
    packet[1] = (addr >> 8) & 0xFF;
    packet[2] = (addr & 0xFF);
    packet[3] = dummy;
    return GM5F2GM7XEXXG_Transmit_8bit_Array(gm5f2gm7xexxg_obj, packet, 4);
}

/*
 * @brief               从缓冲区读取数据-接收数据部分
 * @param gm5f2gm7xexxg_obj     
 *                      gm5f2gm7xexxg 指定信息
 * @param rx_data       接收数据的首地址
 * @param len           接收数据的长度
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 */
static UTILS_Status GM5F2GM7XEXXG_ReadDataFromCache_ReceiveData(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint8_t* rx_data, uint32_t len) {
    return GM5F2GM7XEXXG_Receive_Data(gm5f2gm7xexxg_obj, rx_data, len);
}


/*
 * @brief               从缓冲区读取数据
 * @param gm5f2gm7xexxg_obj     
 *                      gm5f2gm7xexxg 指定信息
 * @param page_addr     需要接收数据的页地址
 * @param cache_addr    需要接收数据的缓冲区地址
 * @param rx_data       存储接收数据的地址
 * @param len           接收数据的长度
 * @return              UTILS_OK        : 数据接收完毕
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 *                      UTILS_WORKING   : 状态机正在工作
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
UTILS_Status GM5F2GM7XEXXG_ReadDataFromCache(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t page_addr, uint16_t cache_addr, uint8_t* rx_data, uint32_t len) {
    UTILS_Status status = UTILS_WORKING;  

    //------------------------------防止同时读/同时写的情况出现------------------------------
    // if (gm5f2gm7xexxg_obj->_fsm_state_program != GM5F2GM7XEXXG_PROGRAM_Idle)
    //     return UTILS_ERROR;

    if (gm5f2gm7xexxg_obj->mode == UTILS_DMA || gm5f2gm7xexxg_obj->mode == UTILS_IT) {
        //------------------------------从page中加载数据到cache中------------------------------
        if (gm5f2gm7xexxg_obj->_fsm_state_read == GM5F2GM7XEXXG_READ_Idle) {
            if (page_addr != gm5f2gm7xexxg_obj->_last_read_page_addr) {
                gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_PAGE_ADDR_Transmiting;
                // 需要page中的数据加载到cache中
                if (GM5F2GM7XEXXG_PageRead_ToCache(gm5f2gm7xexxg_obj, page_addr) != UTILS_OK) {
                    gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_Idle;
                    return UTILS_ERROR;
                }
                gm5f2gm7xexxg_obj->_last_read_page_addr = page_addr; 
            }
            else {
                // 不需要将page中的数据加载到cache中
                gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_PAGE_Load_Finish;
            }
        }

        //------------------------------判断把page中的数据加载到cahce中是否完成------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_read == GM5F2GM7XEXXG_READ_PAGE_ADDR_Transmit_Finish && page_addr == gm5f2gm7xexxg_obj->_last_read_page_addr) {
            if (GM5F2GM7XEXXG_DeviceIsBusy(gm5f2gm7xexxg_obj) == UTILS_OK) {
                gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_PAGE_Load_Finish;
            }
            else
                status = UTILS_ERROR;
        }
        
        //------------------------------发送需要读取数据的地址------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_read == GM5F2GM7XEXXG_READ_PAGE_Load_Finish) {
            gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_CACHE_ADDR_Transmiting;
            if (GM5F2GM7XEXXG_ReadDataFromCache_TransmitAddr(gm5f2gm7xexxg_obj, cache_addr) != UTILS_OK) {
                status = UTILS_ERROR;
                gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_PAGE_Load_Finish;
            }
            else {
                gm5f2gm7xexxg_obj->_last_read_cache_addr = cache_addr;
            }
        }

        //------------------------------接收数据------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_read == GM5F2GM7XEXXG_READ_CACHE_ADDR_Transmit_Finish && gm5f2gm7xexxg_obj->_last_read_cache_addr == cache_addr) {
            gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_CACHE_Receiving;
            if (GM5F2GM7XEXXG_ReadDataFromCache_ReceiveData(gm5f2gm7xexxg_obj, rx_data, len) != UTILS_OK) {
                gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_CACHE_ADDR_Transmit_Finish;
                status = UTILS_ERROR;
            }
        }

        //------------------------------数据接收完成------------------------------
        else if (gm5f2gm7xexxg_obj->_fsm_state_read == GM5F2GM7XEXXG_READ_CACHE_Receive_Finish && gm5f2gm7xexxg_obj->_last_read_cache_addr == cache_addr) {
            gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_Idle;
            status = UTILS_OK;
        }
    }
    else if (gm5f2gm7xexxg_obj->mode == UTILS_LOOP) {
        do {
            //------------------------------判断是否需要重新加载page中的数据到cache中------------------------------
            if (gm5f2gm7xexxg_obj->_fsm_state_read == GM5F2GM7XEXXG_READ_Idle) {
                if (gm5f2gm7xexxg_obj->_last_read_page_addr != page_addr) {
                    //------------------------------发送把数据加载到cache的指令------------------------------
                    if (GM5F2GM7XEXXG_PageRead_ToCache(gm5f2gm7xexxg_obj, page_addr) != UTILS_OK) {
                        status = UTILS_ERROR; 
                        break;
                    }
                    gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_PAGE_Loading_Loop;
                    gm5f2gm7xexxg_obj->_last_read_page_addr = page_addr;
                }
                else {
                    gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_PAGE_Load_Finish_Loop;
                }
            }
            else if (gm5f2gm7xexxg_obj->_fsm_state_read == GM5F2GM7XEXXG_READ_PAGE_Loading_Loop && gm5f2gm7xexxg_obj->_last_read_page_addr == page_addr) {
                UTILS_Status dev_status = GM5F2GM7XEXXG_DeviceIsBusy(gm5f2gm7xexxg_obj);
                if (dev_status == UTILS_OK) {
                    gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_PAGE_Load_Finish_Loop;
                }
                else if (dev_status == UTILS_ERROR) {
                    status = UTILS_ERROR;
                    break;
                }
            }
            else if (gm5f2gm7xexxg_obj->_fsm_state_read == GM5F2GM7XEXXG_READ_PAGE_Load_Finish_Loop) {
                //------------------------------传输地址------------------------------
                if (GM5F2GM7XEXXG_ReadDataFromCache_TransmitAddr(gm5f2gm7xexxg_obj, cache_addr) != UTILS_OK) {
                    status = UTILS_ERROR;
                    break;
                }

                //------------------------------加载数据------------------------------
                if (GM5F2GM7XEXXG_ReadDataFromCache_ReceiveData(gm5f2gm7xexxg_obj, rx_data, len) != UTILS_OK) {
                    status = UTILS_ERROR;
                    break;
                }
                
                gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_Idle;
                status = UTILS_OK;
            }
        } while(0);

    }
    else
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               reset
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
UTILS_Status GM5F2GM7XEXXG_Reset(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj) {
    //------------------------------重置状态------------------------------
    gm5f2gm7xexxg_obj->_fsm_state_transmit = GM5F2GM7XEXXG_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_receive = GM5F2GM7XEXXG_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_read_register = GM5F2GM7XEXXG_READ_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Idle;
    gm5f2gm7xexxg_obj->_last_read_page_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_read_cache_addr = 0xFFFF;
    gm5f2gm7xexxg_obj->_last_read_register_addr = 0xFF;
    gm5f2gm7xexxg_obj->_last_data_move_dst_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_data_move_src_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_replace_cache_addr = 0xFFFF;
    gm5f2gm7xexxg_obj->_last_replace_dst_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_replace_src_addr = 0xFFFFFFFF;

    //------------------------------发送重置命令------------------------------
    uint8_t command[2];
    // TODO: 需要测试该方法的可行性
    command[0] = GM5F2GM7XEXXG_CMD_ENABLE_POWER_ON_RESET;
    command[1] = GM5F2GM7XEXXG_CMD_POWER_ON_RESET;
    return GM5F2GM7XEXXG_Transmit_8bit_Array(gm5f2gm7xexxg_obj, command, 2);
}

/*
 * @brief               使存储器进入休眠模式
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
UTILS_Status GM5F2GM7XEXXG_Sleep(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj) {
    uint8_t command;
    command = GM5F2GM7XEXXG_CMD_DEEP_POWER_DOWN;
    return GM5F2GM7XEXXG_Transmit_8bit(gm5f2gm7xexxg_obj, command);
}

/*
 * @brief               唤醒存储器
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
UTILS_Status GM5F2GM7XEXXG_WakeUP(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj) {
    uint8_t command;
    command = GM5F2GM7XEXXG_CMD_RELEASE_DEEP_POWER_DOWN;
    return GM5F2GM7XEXXG_Transmit_8bit(gm5f2gm7xexxg_obj, command);
}


/*
 * @brief               清除page地址所在的block
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param page_addr     page地址
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
UTILS_Status GM5F2GM7XEXXG_Block_Erase(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t page_addr) {
    uint8_t command;
    command = GM5F2GM7XEXXG_CMD_BLOCK_ERASE;
    uint8_t packet[4];
    packet[0] = command;
    for (uint8_t i = 1; i < 4; ++i)                 // 装载地址
        packet[i] = (uint8_t)(page_addr >> (24 - i * 8) & 0xFF);
    return GM5F2GM7XEXXG_Transmit_8bit_Array(gm5f2gm7xexxg_obj, packet, 4);
}

/*
 * @brief               从指定地址开始设置cache的数据(超过一页的数据将被忽略)
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param cache_addr    写数据开始的地址
 * @param tx_data       需要写入的数据
 * @param len           写入数据的长度
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用dma/it模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
static UTILS_Status GM5F2GM7XEXXG_Program_Load(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint16_t cache_addr, const uint8_t* tx_data, uint32_t len) {
    uint8_t command = GM5F2GM7XEXXG_CMD_PROGRAM_LOAD;
    uint8_t packet[3 + len];
    packet[0] = command;
    packet[1] = ((cache_addr >> 8) & 0xFF);
    packet[2] = (cache_addr & 0xFF);
    for (uint32_t i = 3; i < len + 3; ++i)
        packet[i] = tx_data[i - 3];
    return GM5F2GM7XEXXG_Transmit_8bit_Array(gm5f2gm7xexxg_obj, packet, len + 3);
}

/*
 * @brief               从指定地址开始设置cache的数据(超过一页的数据将被忽略), 不更改地址之前的数据
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param cache_addr    写数据开始的地址
 * @param tx_data       需要写入的数据
 * @param len           写入数据的长度
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用dma/it模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
static UTILS_Status GM5F2GM7XEXXG_Program_Load_Random(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint16_t cache_addr, const uint8_t* tx_data, uint32_t len) {
    uint8_t command = GM5F2GM7XEXXG_CMD_PROGRAM_LOAD_RANDOM_DATA;
    uint8_t packet[3 + len];
    packet[0] = command;
    packet[1] = ((cache_addr >> 8) & 0xFF);
    packet[2] = (cache_addr & 0xFF);
    for (uint32_t i = 3; i < len + 3; ++i)
        packet[i] = tx_data[i - 3];
    return GM5F2GM7XEXXG_Transmit_8bit_Array(gm5f2gm7xexxg_obj, packet, len + 3);
}

/*
 * @brief               把cache中的数据编写到内存中
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param page_addr     page地址
 * @return              UTILS_OK        : 正常
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
static UTILS_Status GM5F2GM7XEXXG_Program_Execute(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t page_addr) {
    uint8_t command = GM5F2GM7XEXXG_CMD_PROGRAM_EXECUTE;
    uint8_t packet[4];
    packet[0] = command;
    for (uint8_t i = 1; i < 4; ++i)                 // 装载地址
        packet[i] = (uint8_t)(page_addr >> (24 - i * 8) & 0xFF);
    return GM5F2GM7XEXXG_Transmit_8bit_Array(gm5f2gm7xexxg_obj, packet, 4);
}

/*
 * @brief               将tx_data数据编写到指定的页中
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param page_addr     page地址
 * @param cache_addr    cache地址
 * @param tx_data       需要编写进去的数据
 * @param len           数据的长度
 * @return              UTILS_OK        : 数据接收完毕
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 *                      UTILS_WORKING   : 状态机正在工作
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
UTILS_Status GM5F2GM7XEXXG_Program(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t page_addr, uint16_t cache_addr, const uint8_t* tx_data, uint32_t len) {
    UTILS_Status status = UTILS_WORKING; 
    //------------------------------防止同时读/同时写的情况出现------------------------------
    if (gm5f2gm7xexxg_obj->_fsm_state_read != GM5F2GM7XEXXG_READ_Idle)
        return UTILS_ERROR;

    do {
        if (gm5f2gm7xexxg_obj->mode == UTILS_DMA || gm5f2gm7xexxg_obj->mode == UTILS_IT) {
            //------------------------------状态机处于空闲状态------------------------------
            if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_PROGRAM_Idle) {
                gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Loading;
                if (GM5F2GM7XEXXG_Program_Load(gm5f2gm7xexxg_obj, cache_addr, tx_data, len) != UTILS_OK) {
                    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Idle;
                    status = UTILS_ERROR;
                    break;
                }
                else {
                    gm5f2gm7xexxg_obj->_last_program_page_addr = page_addr;
                    gm5f2gm7xexxg_obj->_last_program_cache_addr = cache_addr;
                }
            }
            //------------------------------数据加载到cache完成------------------------------
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_PROGRAM_Load_Finish && gm5f2gm7xexxg_obj->_last_program_cache_addr == cache_addr && gm5f2gm7xexxg_obj->_last_program_page_addr == page_addr){
                gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Executing;
                if (GM5F2GM7XEXXG_Program_Execute(gm5f2gm7xexxg_obj, page_addr) != UTILS_OK) {
                    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Load_Finish;
                    status = UTILS_ERROR;
                    break;
                }
            }
            //------------------------------编程命令发送完成------------------------------
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_PROGRAM_Execute_Finish && gm5f2gm7xexxg_obj->_last_program_cache_addr == cache_addr && gm5f2gm7xexxg_obj->_last_program_page_addr == page_addr){
                uint32_t dev_status = GM5F2GM7XEXXG_DeviceIsBusy(gm5f2gm7xexxg_obj);
                if (dev_status == UTILS_OK) {
                    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Idle;
                    status = UTILS_OK;
                }
                else if (dev_status == UTILS_ERROR) {
                    status = UTILS_ERROR;
                    break;
                }
            }
        }
        else if (gm5f2gm7xexxg_obj->mode == UTILS_LOOP) {
            if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_PROGRAM_Idle) {
                GM5F2GM7XEXXG_Transmit_Receive_Start();
                //------------------------------将数据加载到cache------------------------------
                if (GM5F2GM7XEXXG_Program_Load(gm5f2gm7xexxg_obj, cache_addr, tx_data, len) != UTILS_OK) {
                    GM5F2GM7XEXXG_Transmit_Receive_Stop();
                    status = UTILS_ERROR;
                    break;
                }
                //------------------------------发送编程命令------------------------------
                if (GM5F2GM7XEXXG_Program_Execute(gm5f2gm7xexxg_obj, page_addr) != UTILS_OK) {
                    status = UTILS_ERROR;
                    break;
                }
                else {
                    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Programing_Loop;
                }
                GM5F2GM7XEXXG_Transmit_Receive_Stop();
            }
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_PROGRAM_Programing_Loop) {
                status = GM5F2GM7XEXXG_DeviceIsBusy(gm5f2gm7xexxg_obj);
                if (status == UTILS_OK) {
                    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Idle;
                }
            }
            else
                status = UTILS_ERROR;
        }
        else
            status = UTILS_ERROR;
    } while (0);

    return status;
}

/*
 * @brief               将内存中原地址的数据搬运到目标地址中
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param src_page_addr 源数据页地址
 * @param dst_page_addr 目标页地址
 * @return              UTILS_OK        : 数据接收完毕
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 *                      UTILS_WORKING   : 状态机正在工作
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
UTILS_Status GM5F2GM7XEXXG_DataMove(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t src_page_addr, uint32_t dst_page_addr) {
    if (gm5f2gm7xexxg_obj->_fsm_state_read != GM5F2GM7XEXXG_READ_Idle)
        return UTILS_ERROR;

    UTILS_Status status = UTILS_WORKING;
    do {
        if (gm5f2gm7xexxg_obj->mode == UTILS_DMA || gm5f2gm7xexxg_obj->mode == UTILS_IT) {
            //------------------------------发送数据状态机机处于空闲状态------------------------------
            if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_PROGRAM_Idle) {
                if (GM5F2GM7XEXXG_PageRead_ToCache(gm5f2gm7xexxg_obj, src_page_addr) != UTILS_OK) {
                    status = UTILS_ERROR;
                    break;
                }
                gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_DATA_MOVE_PageLoad_AddrSending;
                gm5f2gm7xexxg_obj->_last_data_move_src_addr = src_page_addr;
                gm5f2gm7xexxg_obj->_last_data_move_dst_addr = dst_page_addr;
            }
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_DATA_MOVE_PageLoad_AddrSend_Finish && gm5f2gm7xexxg_obj->_last_data_move_src_addr == src_page_addr && gm5f2gm7xexxg_obj->_last_data_move_dst_addr == dst_page_addr) {
                if (GM5F2GM7XEXXG_Program_Execute(gm5f2gm7xexxg_obj, dst_page_addr) != UTILS_OK) {
                    status = UTILS_ERROR;
                    break;
                }
                gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_DATA_MOVE_Executing;
            }
            //------------------------------发送写数据命令完成------------------------------
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_DATA_MOVE_Execute_Finish) {
                status = GM5F2GM7XEXXG_DeviceIsBusy(gm5f2gm7xexxg_obj);
                if (status == UTILS_OK) 
                    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Idle;
            }
        }
        else if (gm5f2gm7xexxg_obj->mode == UTILS_LOOP) {
            if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_PROGRAM_Idle) {
                //------------------------------将数据加载到cache------------------------------
                if (GM5F2GM7XEXXG_PageRead_ToCache(gm5f2gm7xexxg_obj, src_page_addr) != UTILS_OK) {
                    status = UTILS_ERROR;
                    break;
                }
                gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_DATA_MOVE_PageLoading_Loop;
                gm5f2gm7xexxg_obj->_last_data_move_src_addr = src_page_addr;
                gm5f2gm7xexxg_obj->_last_data_move_dst_addr = dst_page_addr; 
            }
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_DATA_MOVE_PageLoading_Loop && gm5f2gm7xexxg_obj->_last_data_move_src_addr == src_page_addr && gm5f2gm7xexxg_obj->_last_data_move_dst_addr == dst_page_addr) {
                UTILS_Status dev_status = GM5F2GM7XEXXG_DeviceIsBusy(gm5f2gm7xexxg_obj);
                if (dev_status == UTILS_OK) {
                    //------------------------------发送编程命令------------------------------
                    if (GM5F2GM7XEXXG_Program_Execute(gm5f2gm7xexxg_obj, dst_page_addr) != UTILS_OK) {
                        status = UTILS_ERROR;
                        break;
                    }
                    else {
                        gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_DATA_MOVE_Programing_Loop;
                    }
                }
                else
                    status = dev_status;
            }
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_DATA_MOVE_Programing_Loop && gm5f2gm7xexxg_obj->_last_data_move_src_addr == src_page_addr && gm5f2gm7xexxg_obj->_last_data_move_dst_addr == dst_page_addr) {
                status = GM5F2GM7XEXXG_DeviceIsBusy(gm5f2gm7xexxg_obj);
                if (status == UTILS_OK) {
                    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Idle;
                }
            }
            else
                status = UTILS_ERROR;
        }
        else 
            status = UTILS_ERROR;
    }while(0);

    return status;
}

/*
 * @brief               更改指定page中的数据并放到指定的地址中
 * @param gm5f2gm7xexxg_obj
 *                      gm5f2gm7xexxg 指定信息
 * @param src_page_addr 源数据页地址
 * @param dst_page_addr 目标页地址
 * @param cache_addr    开始更改数据的cache地址
 * @param tx_data       需要更改的数据
 * @param len           数据的长度
 * @return              UTILS_OK        : 数据接收完毕
 *                      UTILS_ERROR     : 发生错误,可能是操作超时/spi正在忙于另外一个传输等
 *                      UTILS_WORKING   : 状态机正在工作
 * @note                如果使用DMA/IT模式进行数据传输的话,最好将本函数放在主循环中执行, 通过返回值判断是否执行成功
 */
UTILS_Status GM5F2GM7XEXXG_Replace(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t src_page_addr, uint32_t dst_page_addr, uint16_t cache_addr, const uint8_t* tx_data, uint32_t len) {
    if (gm5f2gm7xexxg_obj->_fsm_state_read != GM5F2GM7XEXXG_READ_Idle)
        return UTILS_ERROR;

    UTILS_Status status = UTILS_WORKING;
    do {
        if (gm5f2gm7xexxg_obj->mode == UTILS_DMA || gm5f2gm7xexxg_obj->mode == UTILS_IT) {
            //------------------------------编程状态机处于空闲状态------------------------------
            if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_PROGRAM_Idle) {
                if (GM5F2GM7XEXXG_PageRead_ToCache(gm5f2gm7xexxg_obj, src_page_addr) != UTILS_OK) {
                    status = UTILS_ERROR;
                    break;
                }
                gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_REPLACE_PageLoad_AddrSending;
                gm5f2gm7xexxg_obj->_last_replace_dst_addr = dst_page_addr;
                gm5f2gm7xexxg_obj->_last_replace_src_addr = src_page_addr;
                gm5f2gm7xexxg_obj->_last_replace_cache_addr = cache_addr;
            }

            //------------------------------等待数据加载到缓存中------------------------------
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_REPLACE_PageLoad_AddrSend_Finish && gm5f2gm7xexxg_obj->_last_replace_src_addr == src_page_addr && gm5f2gm7xexxg_obj->_last_replace_dst_addr == dst_page_addr && gm5f2gm7xexxg_obj->_last_replace_cache_addr == cache_addr) {
                UTILS_Status dev_status = GM5F2GM7XEXXG_DeviceIsBusy(gm5f2gm7xexxg_obj);
                if (dev_status == UTILS_OK) {
                    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_REPLACE_PageLoad_Finish;
                }
                else  {
                    status = dev_status;
                    break;
                }
            }
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_REPLACE_PageLoad_Finish && gm5f2gm7xexxg_obj->_last_replace_src_addr == src_page_addr && gm5f2gm7xexxg_obj->_last_replace_dst_addr == dst_page_addr && gm5f2gm7xexxg_obj->_last_replace_cache_addr == cache_addr) {
                if (GM5F2GM7XEXXG_Program_Load_Random(gm5f2gm7xexxg_obj, cache_addr, tx_data, len) != UTILS_OK) {
                    status = UTILS_ERROR; 
                    break;
                }
                gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_REPLACE_RandomLoading;
            }
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_REPLACE_RandomLoad_Finish && gm5f2gm7xexxg_obj->_last_replace_src_addr == src_page_addr && gm5f2gm7xexxg_obj->_last_replace_dst_addr == dst_page_addr && gm5f2gm7xexxg_obj->_last_replace_cache_addr == cache_addr) {
                if (GM5F2GM7XEXXG_Program_Execute(gm5f2gm7xexxg_obj, dst_page_addr) != UTILS_OK) {
                    status = UTILS_ERROR;
                    break;
                }
                gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_REPLACE_Executing;
            }
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_REPLACE_Execute_Finish && gm5f2gm7xexxg_obj->_last_replace_src_addr == src_page_addr && gm5f2gm7xexxg_obj->_last_replace_dst_addr == dst_page_addr && gm5f2gm7xexxg_obj->_last_replace_cache_addr == cache_addr) {
                status = GM5F2GM7XEXXG_DeviceIsBusy(gm5f2gm7xexxg_obj);
                if (status == UTILS_OK) {
                    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Idle;
                }
            }
            else 
                status = UTILS_ERROR;
        }
        else if (gm5f2gm7xexxg_obj->mode == UTILS_LOOP) {
            if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_PROGRAM_Idle) {
                //------------------------------将数据加载到cache------------------------------
                if (GM5F2GM7XEXXG_PageRead_ToCache(gm5f2gm7xexxg_obj, src_page_addr) != UTILS_OK) {
                    status = UTILS_ERROR;
                    break;
                }
                gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_REPLACE_PageLoading_Loop;
                gm5f2gm7xexxg_obj->_last_replace_src_addr = src_page_addr;
                gm5f2gm7xexxg_obj->_last_replace_dst_addr = dst_page_addr; 
                gm5f2gm7xexxg_obj->_last_replace_cache_addr = cache_addr;
            }
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_REPLACE_PageLoading_Loop && gm5f2gm7xexxg_obj->_last_replace_src_addr == src_page_addr && gm5f2gm7xexxg_obj->_last_replace_dst_addr == dst_page_addr && gm5f2gm7xexxg_obj->_last_replace_cache_addr == cache_addr) {
                UTILS_Status dev_status = GM5F2GM7XEXXG_DeviceIsBusy(gm5f2gm7xexxg_obj);
                if (dev_status == UTILS_OK) {
                    //------------------------------更改cache中的数据------------------------------
                    if (GM5F2GM7XEXXG_Program_Load_Random(gm5f2gm7xexxg_obj, cache_addr, tx_data, len) != UTILS_OK) {
                        status = UTILS_ERROR;
                        break;
                    }
                    
                    //------------------------------发送编程命令------------------------------
                    if (GM5F2GM7XEXXG_Program_Execute(gm5f2gm7xexxg_obj, dst_page_addr) != UTILS_OK) {
                        status = UTILS_ERROR;
                        break;
                    }
                    else {
                        gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_REPLACE_Programing_Loop;
                    }
                }
                else
                    status = dev_status;
            }
            else if (gm5f2gm7xexxg_obj->_fsm_state_program == GM5F2GM7XEXXG_REPLACE_Programing_Loop && gm5f2gm7xexxg_obj->_last_replace_src_addr == src_page_addr && gm5f2gm7xexxg_obj->_last_replace_dst_addr == dst_page_addr && gm5f2gm7xexxg_obj->_last_replace_cache_addr == cache_addr) {
                status = GM5F2GM7XEXXG_DeviceIsBusy(gm5f2gm7xexxg_obj);
                if (status == UTILS_OK) {
                    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Idle;
                }
            }
            else
                status = UTILS_ERROR;
        }
        else
            status = UTILS_ERROR;
    }while(0);

    return status;
}

/*
 * @brief               初始化一个gm5f2gm7xexxg硬件设备的配置信息
 * @param gm5f2gm7xexxg_obj 
 *                      gm5f2gm7xexxg 指定信息
 * @param spi           使用的spi结构体
 * @param cs_pin        片选引脚
 * @param cs_pin_type   片选引脚的GPIO类型
 * @return              无
 */
void GM5F2GM7XEXXG_Init(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type, UTILS_CommunicationMode mode) {
    //------------------------------数据挂载到结构体上------------------------------
    gm5f2gm7xexxg_obj->spi = spi;
    gm5f2gm7xexxg_obj->cs_pin = cs_pin;
    gm5f2gm7xexxg_obj->cs_pin_type = cs_pin_type;
    gm5f2gm7xexxg_obj->mode = mode;

    gm5f2gm7xexxg_obj->_fsm_state_transmit = GM5F2GM7XEXXG_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_receive = GM5F2GM7XEXXG_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_read = GM5F2GM7XEXXG_READ_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_read_register = GM5F2GM7XEXXG_READ_Idle;
    gm5f2gm7xexxg_obj->_fsm_state_program = GM5F2GM7XEXXG_PROGRAM_Idle;
    gm5f2gm7xexxg_obj->_last_read_page_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_read_cache_addr = 0xFFFF;
    gm5f2gm7xexxg_obj->_last_read_register_addr = 0xFF;
    gm5f2gm7xexxg_obj->_last_data_move_dst_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_data_move_src_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_replace_cache_addr = 0xFFFF;
    gm5f2gm7xexxg_obj->_last_replace_dst_addr = 0xFFFFFFFF;
    gm5f2gm7xexxg_obj->_last_replace_src_addr = 0xFFFFFFFF;

    //------------------------------配置CS引脚------------------------------
    UTILS_RCC_GPIO_Enable(gm5f2gm7xexxg_obj->cs_pin_type);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    HAL_GPIO_WritePin(gm5f2gm7xexxg_obj->cs_pin_type, gm5f2gm7xexxg_obj->cs_pin, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = gm5f2gm7xexxg_obj->cs_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(gm5f2gm7xexxg_obj->cs_pin_type, &GPIO_InitStruct);

    //------------------------------关闭芯片数据保护------------------------------
    // GM5F2GM7XEXXG_Transmit_Receive_Start();
    // uint8_t command = 0x1F;
    // uint8_t packet[3];
    // packet[0] = command;
    // packet[1] = 0xA0;
    // packet[2] = 0;
    // HAL_SPI_Transmit(gm5f2gm7xexxg_obj->spi, packet, 3, HAL_MAX_DELAY);
    // GM5F2GM7XEXXG_Transmit_Receive_Stop();
    // HAL_Delay(1);
}
