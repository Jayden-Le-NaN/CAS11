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

//------------------------------- bit mask ------------------------
#define OIP_MASK                0x01

typedef enum {
    GD5F2GM7_DMA_Idle                               = 0x00,         // 空闲状态
    GD5F2GM7_DMA_Transmiting                        = 0x01,         // 正在传输状态
    GD5F2GM7_DMA_Receiving                          = 0x02,         // 正在接收状态
}GD5F2GM7_DMA_FSM;

typedef enum {
    GD5F2GM7_USE_TIM,
    GD5F2GM7_NOUSE_TIM
}GD5F2GM7_TIM;

/* Protection Register: |BRWD | Reserved | BP2 | BP1 | BP0 | INV | CMP | Reserved |
                        |  7  |     6    |  5  |  4  |  3  |  2  |  1  |     0    |
 */
typedef enum{
    NoneLock            = 0x00,
    Upper1_64Lock       = 0x08,
    Upper1_32Lock       = 0x10,
    Upper1_16Lock       = 0x18,
    Upper1_8Lock        = 0x20,
    Upper1_4Lock        = 0x28,
    Upper1_2Lock        = 0x30,
    AllLock             = 0x38,

    Lower1_64Lock       = 0x0A,
    Lower1_32Lock       = 0x12,
    Lower1_16Lock       = 0x1A,
    Lower1_8Lock        = 0x22,
    Lower1_4Lock        = 0x2A,
    Lower1_2Lock        = 0x32,

    Lower63_64Lock      = 0x0C,
    Lower31_32Lock      = 0x14,
    Lower15_16Lock      = 0x1C,
    Lower7_8Lock        = 0x24,
    Lower3_4Lock        = 0x2C,
    Block0Lock          = 0x34,

    Upper63_64Lock      = 0x0E,
    Upper31_32Lock      = 0x16,
    Upper15_16Lock      = 0x1E,
    Upper7_8Lock        = 0x26,
    Upper3_4Lock        = 0x2E
}BLOCKLOCK_Status;

typedef struct {
    //------------------------------用户配置部分------------------------------
    SPI_HandleTypeDef*          spi;
    TIM_HandleTypeDef*          tim;
    uint32_t                    cs_pin;
    GPIO_TypeDef*               cs_pin_type;
    uint16_t                    tim_ms;                // 读写函数执行完经过tim_ms后，触发tim中断
    GD5F2GM7_TIM                tim_usage;
    //------------------------------禁止用户配置------------------------------
    GD5F2GM7_DMA_FSM            _dma_fsm_state_transmit;
    GD5F2GM7_DMA_FSM            _dma_fsm_state_receive;
}GD5F2GM7_Info_Struct;

//------------------------------需要放入中断回调函数中的函数------------------------------
void GD5F2GM7_Transmit_IRQ_Handler(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi);
void GD5F2GM7_Receive_IRQ_Handler(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi);
//------------------------------需要放入中断回调函数中的函数------------------------------

void GD5F2GM7_Init(GD5F2GM7_Info_Struct* gd5f2gm7_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type, TIM_HandleTypeDef* tim, uint16_t tim_ms);
UTILS_Status GD5F2GM7_WriteEnable(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_WriteDisable(GD5F2GM7_Info_Struct* gd5f2gm7_obj);
UTILS_Status GD5F2GM7_Get_Features(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t reg_addr, uint8_t* rx_data);
UTILS_Status GD5F2GM7_Set_Features(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t reg_addr, uint8_t tx_data);
UTILS_Status GD5F2GM7_isBusy(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint8_t* bit_status);
UTILS_Status GD5F2GM7_PageRead_ToCache(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t addr);
UTILS_Status GD5F2GM7_ReadFromCache(GD5F2GM7_Info_Struct* gd5f2gm7_obj, uint32_t cache_addr, uint8_t* rx_data, uint32_t len, UTILS_CommunicationMode rx_mode, GD5F2GM7_TIM tim_usage);
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
