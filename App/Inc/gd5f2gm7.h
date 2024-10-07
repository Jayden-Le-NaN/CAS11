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

//------------------------------需要放入中断回调函数中的函数------------------------------
void GD5F2GM7_Transmit_IRQ_Handler(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi);
void GD5F2GM7_Receive_IRQ_Hanlder(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi);
//------------------------------需要放入中断回调函数中的函数------------------------------

void GD5F2GM7_Init(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type);
UTILS_Status GD5F2GM7_WriteEnable(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_WriteDisable(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_Get_Features(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t reg_addr, uint8_t* rx_data);
UTILS_Status GD5F2GM7_Set_Features(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t reg_addr, uint8_t tx_data);
UTILS_Status GD5F2GM7_PageRead_ToCache(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t addr);
UTILS_Status GD5F2GM7_ReadFromCache(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t cache_addr, uint8_t* rx_data, uint32_t len, UTILS_CommunicationMode rx_mode);
UTILS_Status GD5F2GM7_ReadID(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t* rx_data);
UTILS_Status GD5F2GM7_ReadParameterPage(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_ReadUID(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_ProgramLoad(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t cache_addr, uint8_t* tx_data, uint32_t len, UTILS_CommunicationMode tx_mode);
UTILS_Status GD5F2GM7_ProgramExecute(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t page_addr);
UTILS_Status GD5F2GM7_ProgramLoadRandomData(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t cache_addr, uint8_t* tx_data, uint32_t len, UTILS_CommunicationMode tx_mode);
UTILS_Status GD52GM7_BlockErase(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t page_addr);
UTILS_Status GD5F2GM7_Reset(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_EnablePowerOnReset(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_PowerOnReset(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_DeepPowerDown(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_ReleaseDeepPowerDown(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_DeviceIsBusy(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_DMA_TransmitIsBusy(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_DMA_ReceiveIsBusy(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
#endif
