#ifndef PM004MNIA_H_
#define PM004MNIA_H_

#include "utils.h"

//------------------------------指令表------------------------------
#define PM004MNIA_CMD_MODE_REGISTER_WRITE           (0xB1)
#define PM004MNIA_CMD_MODE_REGISTER_READ            (0xB5)
#define PM004MNIA_CMD_WRITE                         (0x02)
#define PM004MNIA_CMD_READ                          (0x03)
#define PM004MNIA_CMD_WRITE_ENABLE                  (0x06)
#define PM004MNIA_CMD_WRITE_DISABLE                 (0x04)
#define PM004MNIA_CMD_ENTRY_TO_DEEP_POWER_DOWN      (0xB9)
#define PM004MNIA_CMD_EXIT_FROM_DEEP_POWER_DOWN     (0xAB)
#define PM004MNIA_CMD_READ_UNIQUE_ID_REGISTER       (0x9F)
#define PM004MNIA_CMD_RESET_ENABLE                  (0x66)
#define PM004MNIA_CMD_RESET                         (0x99)

typedef enum {
    PM004MNIA_Idle                          = 0x00,             // 空闲状态
    PM004MNIA_Transmiting                   = 0x01,             // 正在传输状态
    PM004MNIA_Receiving                     = 0x02,             // 正在接收状态
}PM004MNIA_FSM;

typedef enum {
    PM004MNIA_READ_Idle                     = 0x00,             // 读空闲状态
    PM004MNIA_READ_ADDR_Transmiting         = 0x01,             // 正在传输地址状态
    PM004MNIA_READ_ADDR_Transmit_Finish     = 0x02,             // 地址传输完成
    PM004MNIA_READ_Receiving                = 0x03,             // 数据正在接收
    PM004MNIA_READ_Receive_Finish           = 0x04,             // 数据接收完成
}PM004MNIA_READ_FSM;

typedef enum {
    PM004MNIA_Memory                        = 0x00,             // 存储器类型
    PM004MNIA_Register                      = 0x01,             // 寄存器类型
}PM004MNIA_MEM_TYPE;

typedef struct {
    //------------------------------用户配置部分------------------------------
    SPI_HandleTypeDef*          spi;
    uint32_t                    cs_pin;
    GPIO_TypeDef*               cs_pin_type;
    UTILS_CommunicationMode     mode;

    //------------------------------禁止用户配置------------------------------
    PM004MNIA_FSM               _fsm_state_transmit;
    PM004MNIA_FSM               _fsm_state_receive;
    PM004MNIA_READ_FSM          _fsm_state_read;
    uint16_t                    _data_read_result;
    uint8_t                     _register_read_result;
    uint32_t                    _data_read_addr;
}PM004MNIA_Info_Struct;

//------------------------------以下这些函数需要放到对应的回调函数中(如果是使用DMA/IT方式传输数据的话)------------------------------
void PM004MNIA_Transmit_IRQ_Handler(PM004MNIA_Info_Struct* pm004mnia_obj, SPI_HandleTypeDef* spi);
void PM004MNIA_Receive_IRQ_Handler(PM004MNIA_Info_Struct* pm004mnia_obj, SPI_HandleTypeDef* spi);
//------------------------------以上这些函数需要放到对应的回调函数中(如果是使用DMA/IT方式传输数据的话)------------------------------


UTILS_Status PM004MNIA_Reset(PM004MNIA_Info_Struct* pm004mnia_obj);
UTILS_Status PM004MNIA_WakeUP(PM004MNIA_Info_Struct* pm004mnia_obj);
uint16_t PM004MNIA_GetData_Register(PM004MNIA_Info_Struct* pm004mnia_obj);
uint16_t PM004MNIA_GetData(PM004MNIA_Info_Struct* pm004mnia_obj);
UTILS_Status PM004MNIA_Read(PM004MNIA_Info_Struct* pm004mnia_obj, uint32_t addr, PM004MNIA_MEM_TYPE mem_type);
UTILS_Status PM004MNIA_Write_Register(PM004MNIA_Info_Struct* pm004mnia_obj, uint32_t addr, uint8_t data);
UTILS_Status PM004MNIA_Write(PM004MNIA_Info_Struct* pm004mnia_obj, uint32_t addr, uint16_t data);
UTILS_Status PM004MNIA_WriteDisable(PM004MNIA_Info_Struct* pm004mnia_obj);
UTILS_Status PM004MNIA_WriteEnable(PM004MNIA_Info_Struct* pm004mnia_obj);
void PM004MNIA_Init(PM004MNIA_Info_Struct* pm004mnia_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type, UTILS_CommunicationMode mode);
#endif
