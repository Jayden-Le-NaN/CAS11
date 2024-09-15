#ifndef AD9833_H_
#define AD9833_H_
#include "utils.h"


#define AD9833_FCLK         25000000                                    // AD9833 时钟频率(单位HZ)

// AD9833 片选引脚
#define AD9833_FSYN_GPIO_PIN   GPIO_PIN_7
#define AD9833_FSYN_GPIO_TYPE  GPIOC

// AD9833 波形模式
#define AD9833_OUT_SINUS    ((0 << 5) | (0 << 1) | (0 << 3))            // sin wave
#define AD9833_OUT_TRIANGLE ((0 << 5) | (1 << 1) | (0 << 3))            // triangle wave
#define AD9833_OUT_MSB      ((1 << 5) | (0 << 1) | (1 << 3))            // square wave
#define AD9833_OUT_MSB2     ((1 << 5) | (0 << 1) | (0 << 3))

// AD9833 寄存器
#define AD9833_REG_CMD      (0 << 14)                                   
#define AD9833_REG_FREQ0    (1 << 14)
#define AD9833_REG_FREQ1    (2 << 14)
#define AD9833_REG_PHASE0   (6 << 13)
#define AD9833_REG_PHASE1   (7 << 13)


// AD9833 指令控制
#define AD9833_B28          (1 << 13)
#define AD9833_HLB          (1 << 12)
#define AD9833_FSEL0        (0 << 11)
#define AD9833_FSEL1        (1 << 11)
#define AD9833_PSEL0        (0 << 10)
#define AD9833_PSEL1        (1 << 10)
#define AD9833_PIN_SW       (1 << 9)
#define AD9833_RESET        (1 << 8)
#define AD9833_CLEAR_RESET  (0 << 8)
#define AD9833_SLEEP1       (1 << 7)
#define AD9833_SLEEP12      (1 << 6)
#define AD9833_OPBITEN      (1 << 5)
#define AD9833_SIGN_PIB     (1 << 4)
#define AD9833_DIV2         (1 << 3)
#define AD9833_MODE         (1 << 1)


//------------------------------外接函数------------------------------
#define AD9833_FSYN_Enable()      HAL_GPIO_WritePin(AD9833_FSYN_GPIO_TYPE, AD9833_FSYN_GPIO_PIN, GPIO_PIN_RESET)
#define AD9833_FSYN_Disable()     HAL_GPIO_WritePin(AD9833_FSYN_GPIO_TYPE, AD9833_FSYN_GPIO_PIN, GPIO_PIN_SET)

void AD9833_Init(SPI_HandleTypeDef* hspi);
void AD9833_WriteData(uint16_t tx_data);
void AD9833_SetFrequency(uint16_t reg, double val);
void AD9833_SetPhase(uint16_t reg, uint16_t val);
void AD9833_SetWave(uint16_t wave_mode, uint16_t freq_sfr, uint16_t phase_sfr);
void AD9833_Setup(uint16_t freq_reg, double freq_val, uint16_t phase_reg, uint16_t phase_val,uint16_t wave_mode);

#endif
