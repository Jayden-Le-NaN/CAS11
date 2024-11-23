#ifndef AD9833_H_
#define AD9833_H_

#include "utils.h"

//------------------------------寄存器------------------------------
#define AD9833_REG_CONTROL                          (0x0 << 14)  
#define AD9833_REG_RESET                            (0x1 << 8 )  
#define AD9833_REG_B28                              (0x1 << 13)
#define AD9833_REG_FREQ0                            (0x1 << 14)
#define AD9833_REG_FREQ1                            (0x2 << 14)
#define AD9833_REG_PHASE0                           (0x6 << 13)
#define AD9833_REG_PHASE1                           (0x7 << 13)
//------------------------------频率设置模式------------------------------
#define AD9833_FREQ_ALL                             (0x00)  
#define AD9833_FREQ_MSB                             (0x01)
#define AD9833_FREQ_LSB                             (0x02)
#define AD9833_FREQ_NO_CHANGE                       (0x03)
//------------------------------波形------------------------------
#define AD9833_WAVE_SINUSOID                        (0 << 5 | 0 << 1 | 0 << 3)
#define AD9833_WAVE_UP_DOWN_RAMP                    (0 << 5 | 1 << 1 | 0 << 3)
#define AD9833_WAVE_DAC_DATA_MSB_HALF               (1 << 5 | 0 << 1 | 0 << 3)
#define AD9833_WAVE_DAC_DATA_MSB                    (1 << 1 | 0 << 1 | 1 << 3)
//------------------------------休眠模式------------------------------
#define AD9833_SLEEP_NO_PWERDOWN                    (0x00)
#define AD9833_SLEEP_DAC_POWERDOWN                  (0x01)
#define AD9833_SLEEP_INTERNAL_CLOCK_POWERDOWN       (0x02)
#define AD9833_SLEEP_ALL_POWERDOWN                  (0x03)
//------------------------------输出内容------------------------------
#define AD9833_OUT_FREQ0                            (0x00)
#define AD9833_OUT_FREQ1                            (0x01)
#define AD9833_OUT_PHASE0                           (0x02)
#define AD9833_OUT_PHASE1                           (0x03)


//------------------------------DMA状态机------------------------------
typedef enum {
    AD9833_DMA_Idle                                 = 0x00,         // 空闲状态
    AD9833_DMA_Transmiting                          = 0x01,         // 正在传输状态
}AD9833_DMA_FSM;

typedef struct {
    //------------------------------用户配置部分------------------------------
    SPI_HandleTypeDef*          spi;                                // 进行通信的spi结构体
    uint32_t                    fsync_pin;                          // 信号同步引脚
    GPIO_TypeDef*               fsync_pin_type;                     // 信号同步引脚的类型
    uint32_t                    crystal_oscillator_frequency;       // 晶振的频率

    //------------------------------禁止用户配置部分------------------------------
    AD9833_DMA_FSM              _dma_fsm_state_transmit;
    uint16_t                    _control_reg_data;
}AD9833_Info_Struct;



//------------------------------需要放入中断回调函数中的函数------------------------------
void AD9833_Transmit_IRQ_Handler(AD9833_Info_Struct* ad9833_obj, SPI_HandleTypeDef* spi);
//------------------------------需要放入中断回调函数中的函数------------------------------

//------------------------------外接函数------------------------------
void SPI_Write_Half_Word(SPI_HandleTypeDef* sstv_tim_dma_spi, uint16_t Data);
void AD9833_Write_Whole_Frq(SPI_HandleTypeDef* sstv_tim_dma_spi, uint16_t* frqh, uint16_t* frql);
void AD9833_FrequencyConversion_2Reg(AD9833_Info_Struct* ad9833_obj, uint16_t* raw_freq, uint16_t* frqh, uint16_t* frql);
UTILS_Status AD9833_FrequencySetMode(AD9833_Info_Struct* ad9833_obj, uint8_t freq_set_mode);
UTILS_Status AD9833_FrequencyOutSelect(AD9833_Info_Struct* ad9833_obj, uint8_t freq_out_sel);
UTILS_Status AD9833_PhaseOutSelect(AD9833_Info_Struct* ad9833_obj, uint8_t phase_out_sel);
UTILS_Status AD9833_SetFrequency(AD9833_Info_Struct* ad9833_obj, uint16_t freq_reg, uint8_t freq_set_mode, uint32_t* freq, uint32_t len, UTILS_CommunicationMode tx_mode);
UTILS_Status AD9833_SetPhase(AD9833_Info_Struct* ad9833_obj, uint16_t phase_reg, uint16_t* phase, uint32_t len, UTILS_CommunicationMode tx_mode);
UTILS_Status AD9833_SetWave(AD9833_Info_Struct* ad9833_obj, uint16_t wave_mode);
UTILS_Status AD9833_Sleep(AD9833_Info_Struct* ad9833_obj, uint16_t sleep_mode);
UTILS_Status AD9833_Reset(AD9833_Info_Struct* ad9833_obj);
UTILS_Status AD9833_Transmit_Is_Idle(AD9833_Info_Struct* ad9833_obj);
UTILS_Status AD9833_Init_Tx_DMA_TIM(AD9833_Info_Struct* ad9833_obj1, AD9833_Info_Struct* ad9833_obj2, SPI_HandleTypeDef* ad9833_tim_dma_spi, TIM_HandleTypeDef* ad9833_tim);
// void DMA_TIM_SPI_HalfTxCplt(void);
// void DMA_TIM_SPI_TxCplt(void);
void AD9833_Init(AD9833_Info_Struct* ad9833_obj, SPI_HandleTypeDef* spi, uint32_t fsync_pin, GPIO_TypeDef* fsync_pin_type, uint32_t crystal_oscillator_frequency);
//------------------------------外接函数------------------------------

#endif
