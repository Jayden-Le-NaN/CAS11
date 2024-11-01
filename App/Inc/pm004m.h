#ifndef PM004M_H_
#define PM004M_H_
#include "utils.h"

//------------------------------指令表------------------------------
#define PM004M_CMD_MODE_REGISTER_WRITE              (0xB1) 
#define PM004M_CMD_MODE_REGISTER_READ               (0xB5)
#define PM004M_CMD_WRITE                            (0x02)
#define PM004M_CMD_READ                             (0x03)
#define PM004M_CMD_WRITE_ENABLE                     (0x06)
#define PM004M_CMD_WRITE_DISABLE                    (0x04)
#define PM004M_CMD_ENTRY_TO_DEEP_POWER_DOWN         (0xB9)
#define PM004M_CMD_EXIT_FROM_DEEP_POWER_DOWN        (0xAB)
#define PM004M_CMD_READ_UNIQUE_ID_REGISTER          (0x9F)
#define PM004M_CMD_RESET_ENABLE                     (0x66)
#define PM004M_CMD_RESET                            (0x99)

typedef enum {
    PM004M_DMA_Idle                                 = 0x00,         // 空闲状态
    PM004M_DMA_Transmiting                          = 0x01,         // 正在传输状态
    PM004M_DMA_Receiving                            = 0x02,         // 正在接收状态
}PM004M_DMA_FSM;

typedef struct {
    //------------------------------用户配置部分------------------------------
    SPI_HandleTypeDef*          spi;
    uint32_t                    cs_pin;
    GPIO_TypeDef*               cs_pin_type;
    //------------------------------禁止用户配置------------------------------
    PM004M_DMA_FSM              _dma_fsm_state_transmit;
    PM004M_DMA_FSM              _dma_fsm_state_receive;
    uint8_t                     _lt;            // 接收数据时候的dummy clk的数量 / 8, 只能设置为 0 或 1(虽然芯片支持 0, 4, 8, 12个dummy clk, 但是 4, 12个dummy clk 硬件spi不好处理)
}PM004M_Info_Struct;

//------------------------------需要放入中断回调函数中的函数------------------------------
void PM004M_Transmit_IRQ_Handler(PM004M_Info_Struct* pm004m_obj, SPI_HandleTypeDef* spi);
void PM004M_Receive_IRQ_Handler(PM004M_Info_Struct* pm004m_obj, SPI_HandleTypeDef* spi);
//------------------------------需要放入中断回调函数中的函数------------------------------

UTILS_Status PM004M_ModeRegisterWrite(PM004M_Info_Struct* pm004m_obj, uint32_t reg_addr, uint8_t tx_data);
UTILS_Status PM004M_ModeRegisterRead(PM004M_Info_Struct* pm004m_obj, uint32_t reg_addr, uint8_t* rx_data);
UTILS_Status PM004M_Write(PM004M_Info_Struct* pm004m_obj, uint32_t addr, uint8_t* tx_data, uint32_t len, UTILS_CommunicationMode tx_mode);
UTILS_Status PM004M_Read(PM004M_Info_Struct* pm004m_obj, uint32_t addr, uint8_t* rx_data, uint32_t len, UTILS_CommunicationMode rx_mode);
UTILS_Status PM004M_WriteEnable(PM004M_Info_Struct* pm004m_obj);
UTILS_Status PM004M_WriteDisable(PM004M_Info_Struct* pm004m_obj);
UTILS_Status PM004M_EntryToDeepPowerDown(PM004M_Info_Struct* pm004m_obj);
UTILS_Status PM004M_ExitFromDeepPowerDown(PM004M_Info_Struct* pm004m_obj);
UTILS_Status PM004M_ReadUniqueIDRegister(PM004M_Info_Struct* pm004m_obj, uint8_t* rx_data, UTILS_CommunicationMode rx_mode);
UTILS_Status PM004M_ResetEnable(PM004M_Info_Struct* pm004m_obj);
UTILS_Status PM004M_Reset(PM004M_Info_Struct* pm004m_obj);
void PM004M_Init(PM004M_Info_Struct* pm004m_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type);
#endif
