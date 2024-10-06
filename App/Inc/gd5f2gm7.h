#ifndef GD5F2GM7_H_
#define GD5F2GM7_H_

#include "utils.h"
//------------------------------指令表------------------------------
#define GD5F2GM7_CMD_WRITE_ENABLE                  (0x06)
#define GD5F2GM7_CMD_WRITE_DISABLE                 (0x04)
#define GD5F2GM7_CMD_GET_FEATURES                  (0x0F)
#define GD5F2GM7_CMD_SET_FEATURES                  (0x1F)
#define GD5F2GM7_CMD_PAGE_READ_TO_CACHE            (0x13)
#define GD5F2GM7_CMD_READ_FROM_CACHE               (0x03)
#define GD5F2GM7_CMD_READ_ID                       (0x9F)
#define GD5F2GM7_CMD_READ_PARAMETER_PAGE           (0x13)
#define GD5F2GM7_CMD_READ_UID                      (0x13)
#define GD5F2GM7_CMD_PROGRAM_LOAD                  (0x02)
#define GD5F2GM7_CMD_PROGRAM_EXECUTE               (0x10)
#define GD5F2GM7_CMD_PROGRAM_LOAD_RANDOM_DATA      (0x84)
#define GD5F2GM7_CMD_BLOCK_ERASE                   (0xD8)
#define GD5F2GM7_CMD_RESET                         (0xFF) 
#define GD5F2GM7_CMD_ENABLE_POWER_ON_RESET         (0x66)
#define GD5F2GM7_CMD_POWER_ON_RESET                (0x99)
#define GD5F2GM7_CMD_DEEP_POWER_DOWN               (0xB9)
#define GD5F2GM7_CMD_RELEASE_DEEP_POWER_DOWN       (0xAB)

typedef enum {
    GD5F2GM7_DMA_Idle                               = 0x00,         // 空闲状态
    GD5F2GM7_DMA_Transmiting                        = 0x01,         // 正在传输状态
    GD5F2GM7_DMA_Receiving                          = 0x02,         // 正在接收状态
}GD5F2GM7_DMA_FSM;

typedef struct {
    //------------------------------用户配置部分------------------------------
    SPI_HandleTypeDef*          spi;
    uint32_t                    cs_pin;
    GPIO_TypeDef*               cs_pin_type;
    //------------------------------禁止用户配置------------------------------
    GD5F2GM7_DMA_FSM            _dma_fsm_state_transmit;
    GD5F2GM7_DMA_FSM            _dma_fsm_state_receive;
}GD5F2GM7_Info_Struct;

#endif
