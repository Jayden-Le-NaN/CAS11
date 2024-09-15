#ifndef GM5F2GM7XEXXG_H_
#define GM5F2GM7XEXXG_H_

#include "utils.h"

//------------------------------指令表------------------------------
#define GM5F2GM7XEXXG_CMD_WRITE_ENABLE                  (0x06)
#define GM5F2GM7XEXXG_CMD_WRITE_DISABLE                 (0x04)
#define GM5F2GM7XEXXG_CMD_GET_FEATURES                  (0x0F)
#define GM5F2GM7XEXXG_CMD_SET_FEATURES                  (0x1F)
#define GM5F2GM7XEXXG_CMD_PAGE_READ_TO_CACHE            (0x13)
#define GM5F2GM7XEXXG_CMD_READ_FROM_CACHE               (0x03)
#define GM5F2GM7XEXXG_CMD_READ_ID                       (0x9F)
#define GM5F2GM7XEXXG_CMD_READ_PARAMETER_PAGE           (0x13)
#define GM5F2GM7XEXXG_CMD_READ_UID                      (0x13)
#define GM5F2GM7XEXXG_CMD_PROGRAM_LOAD                  (0x02)
#define GM5F2GM7XEXXG_CMD_PROGRAM_EXECUTE               (0x10)
#define GM5F2GM7XEXXG_CMD_PROGRAM_LOAD_RANDOM_DATA      (0x84)
#define GM5F2GM7XEXXG_CMD_BLOCK_ERASE                   (0xD8)
#define GM5F2GM7XEXXG_CMD_RESET                         (0xFF) 
#define GM5F2GM7XEXXG_CMD_ENABLE_POWER_ON_RESET         (0x66)
#define GM5F2GM7XEXXG_CMD_POWER_ON_RESET                (0x99)
#define GM5F2GM7XEXXG_CMD_DEEP_POWER_DOWN               (0xB9)
#define GM5F2GM7XEXXG_CMD_RELEASE_DEEP_POWER_DOWN       (0xAB)

typedef enum {
    GM5F2GM7XEXXG_Idle                                  = 0x00,         // 空闲状态
    GM5F2GM7XEXXG_Transmiting                           = 0x01,         // 正在传输状态
    GM5F2GM7XEXXG_Receiving                             = 0x02,         // 正在接收状态
}GM5F2GM7XEXXG_FSM;

typedef enum {
    GM5F2GM7XEXXG_READ_Idle                             = 0x00,         // 读取数据空闲状态

    //------------------------------读page的状态------------------------------
    GM5F2GM7XEXXG_READ_PAGE_ADDR_Transmiting            = 0x10,         // 正在传输地址状态(page)
    GM5F2GM7XEXXG_READ_PAGE_ADDR_Transmit_Finish        = 0x11,         // 地址传输完成状态(page)
    GM5F2GM7XEXXG_READ_PAGE_Load_Finish                 = 0x12,         // 数据接收完成状态(page)
    GM5F2GM7XEXXG_READ_PAGE_Loading_Loop                = 0x13,         // 在阻塞通信模式下正在加载数据状态
    GM5F2GM7XEXXG_READ_PAGE_Load_Finish_Loop            = 0x14,         // 在阻塞通信模式下数据加载完成状态

    //------------------------------读cache的状态------------------------------
    GM5F2GM7XEXXG_READ_CACHE_ADDR_Transmiting           = 0x20,         // 正在传输地址状态(cache)
    GM5F2GM7XEXXG_READ_CACHE_ADDR_Transmit_Finish       = 0x21,         // 地址传输完成状态(cache)
    GM5F2GM7XEXXG_READ_CACHE_Receiving                  = 0x22,         // 数据正在接收状态(cache)
    GM5F2GM7XEXXG_READ_CACHE_Receive_Finish             = 0x23,         // 数据接收完成状态(cache)
    
    //------------------------------读寄存器的状态------------------------------
    GM5F2GM7XEXXG_READ_REGISTER_ADDR_Transmiting        = 0x30,         // 正在传输地址状态(寄存器)
    GM5F2GM7XEXXG_READ_REGISTER_ADDR_Transmit_Finish    = 0x31,         // 地址传输完成状态(寄存器)
    GM5F2GM7XEXXG_READ_REGISTER_Receiving               = 0x32,         // 数据正在接收状态(寄存器)
    GM5F2GM7XEXXG_READ_REGISTER_Receive_Finish          = 0x33,         // 数据接收完成状态(寄存器)

}GM5F2GM7XEXXG_READ_FSM;

typedef enum {
    GM5F2GM7XEXXG_PROGRAM_Idle                          = 0x00,         // 编写数据空闲状态

    //------------------------------直接编辑页数据的状态机------------------------------
    GM5F2GM7XEXXG_PROGRAM_Loading                       = 0x10,         // 正在将数据加载到cache中
    GM5F2GM7XEXXG_PROGRAM_Load_Finish                   = 0x11,         // 数据加载到cache完成
    GM5F2GM7XEXXG_PROGRAM_Executing                     = 0x12,         // 正在发送编程命令
    GM5F2GM7XEXXG_PROGRAM_Execute_Finish                = 0x13,         // 编程命令发送完成
    GM5F2GM7XEXXG_PROGRAM_Programing_Loop               = 0x14,         // 在阻塞通信模式下正在编写数据到存储器中


    //------------------------------内部数据迁移的状态机------------------------------
    GM5F2GM7XEXXG_DATA_MOVE_PageLoad_AddrSending        = 0x20,         // 正在发送加载到cache的page地址
    GM5F2GM7XEXXG_DATA_MOVE_PageLoad_AddrSend_Finish    = 0x21,         // page地址发送完成
    GM5F2GM7XEXXG_DATA_MOVE_PageLoad_Finish             = 0x22,         // page数据加载完成
    GM5F2GM7XEXXG_DATA_MOVE_Executing                   = 0x23,         // 正在发送随机写page命令
    GM5F2GM7XEXXG_DATA_MOVE_Execute_Finish              = 0x24,         // 写page命令发送完成
    GM5F2GM7XEXXG_DATA_MOVE_PageLoading_Loop            = 0x25,         // 在阻塞通信模式下数据正在加载到cache中
    GM5F2GM7XEXXG_DATA_MOVE_Programing_Loop             = 0x26,         // 在阻塞通信模式下数据正在编写到存储器中

    //------------------------------更改部分页数据状态机------------------------------
    GM5F2GM7XEXXG_REPLACE_PageLoad_AddrSending          = 0x30,         // 正在发送加载到cache的page地址
    GM5F2GM7XEXXG_REPLACE_PageLoad_AddrSend_Finish      = 0x30,         // page地址发送完成
    GM5F2GM7XEXXG_REPLACE_PageLoad_Finish               = 0x31,         // page数据加载完成
    GM5F2GM7XEXXG_REPLACE_RandomLoading                 = 0x32,         // 正在发送随机写命令
    GM5F2GM7XEXXG_REPLACE_RandomLoad_Finish             = 0x33,         // 随机写命令发送完成
    GM5F2GM7XEXXG_REPLACE_Executing                     = 0x34,         // 正在发送写page命令
    GM5F2GM7XEXXG_REPLACE_Execute_Finish                = 0x35,         // 写page命令发送完成
    GM5F2GM7XEXXG_REPLACE_PageLoading_Loop              = 0x36,         // 在阻塞通信模式下加载页数据
    GM5F2GM7XEXXG_REPLACE_Programing_Loop               = 0x37,         // 在阻塞通信模式下想内存中编程
}GM5F2GM7XEXXG_PROGRAM_FSM;


typedef struct {
    //------------------------------用户配置部分------------------------------
    SPI_HandleTypeDef*          spi;
    uint32_t                    cs_pin;
    GPIO_TypeDef*               cs_pin_type;
    UTILS_CommunicationMode     mode;
    //------------------------------禁止用户配置------------------------------
    GM5F2GM7XEXXG_FSM           _fsm_state_transmit;
    GM5F2GM7XEXXG_FSM           _fsm_state_receive;
    GM5F2GM7XEXXG_READ_FSM      _fsm_state_read;
    GM5F2GM7XEXXG_READ_FSM      _fsm_state_read_register;
    GM5F2GM7XEXXG_PROGRAM_FSM   _fsm_state_program;
    uint32_t                    _last_read_page_addr;                   // 判断是否需要重新加载数据到寄存器
    uint16_t                    _last_read_cache_addr;                  // 判断是否是同一个任务调用
    uint8_t                     _last_read_register_addr;               // 判断是否是同一个任务调用
    uint32_t                    _last_program_page_addr;
    uint32_t                    _last_program_cache_addr;
    uint32_t                    _last_data_move_src_addr;
    uint32_t                    _last_data_move_dst_addr;
    uint32_t                    _last_replace_src_addr;
    uint32_t                    _last_replace_dst_addr;
    uint16_t                    _last_replace_cache_addr;
}GM5F2GM7XEXXG_Info_Struct;

//------------------------------以下这些函数需要放到对应的回调函数中(如果是使用DMA/IT方式传输数据的话)------------------------------
void GM5F2GM7XEXXG_Transmit_IRQ_Handler(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, SPI_HandleTypeDef* spi);
void GM5F2GM7XEXXG_Receive_IRQ_Handler(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, SPI_HandleTypeDef* spi);
//------------------------------以上这些函数需要放到对应的回调函数中(如果是使用DMA/IT方式传输数据的话)------------------------------

//------------------------------用户函数接口------------------------------
UTILS_Status GM5F2GM7XEXXG_WriteEnable(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj);
UTILS_Status GM5F2GM7XEXXG_WriteDisable(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj);
UTILS_Status GM5F2GM7XEXXG_Reset_Soft(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj);
UTILS_Status GM5F2GM7XEXXG_Set_Features(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint8_t addr, uint8_t tx_data);
UTILS_Status GM5F2GM7XEXXG_Get_Features(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint8_t addr, uint8_t* rx_data);
UTILS_Status GM5F2GM7XEXXG_DeviceIsBusy(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj);
UTILS_Status GM5F2GM7XEXXG_ReadDataFromCache(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t page_addr, uint16_t cache_addr, uint8_t* rx_data, uint32_t len);
UTILS_Status GM5F2GM7XEXXG_Reset(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj);
UTILS_Status GM5F2GM7XEXXG_Sleep(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj);
UTILS_Status GM5F2GM7XEXXG_WakeUP(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj);
UTILS_Status GM5F2GM7XEXXG_Block_Erase(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t page_addr);
UTILS_Status GM5F2GM7XEXXG_Program(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t page_addr, uint16_t cache_addr, const uint8_t* tx_data, uint32_t len);
UTILS_Status GM5F2GM7XEXXG_DataMove(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t src_page_addr, uint32_t dst_page_addr);
UTILS_Status GM5F2GM7XEXXG_Replace(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, uint32_t src_page_addr, uint32_t dst_page_addr, uint16_t cache_addr, const uint8_t* tx_data, uint32_t len);
void GM5F2GM7XEXXG_Init(GM5F2GM7XEXXG_Info_Struct* gm5f2gm7xexxg_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type, UTILS_CommunicationMode mode);
//------------------------------用户函数接口------------------------------

#endif
