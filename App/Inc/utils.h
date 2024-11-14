#ifndef UTILS_H_
#define UTILS_H_

// 头文件功能: 包含公共的头文件供APP中的文件使用
//           : 包含公共的宏共APP中的文件使用

//------------------------------公用宏------------------------------
// #define MCU_STM32F4XX
#define MCU_STM32L4XX

#define MCU_FREQUENCY_MHZ       168         // STM32 系统时钟主频
#define PRINT_BUFFER_SIZE       256         // printf 的缓冲区大小

//------------------------------标准库(C语言)------------------------------
#include "stdarg.h"
#include "stdint.h"

//------------------------------HAL库------------------------------
#ifdef MCU_STM32F4XX
#include "stm32f4xx.h"
#endif

#ifdef MCU_STM32L4XX
#include "stm32l4xx_hal.h"
#endif


typedef enum {
    UTILS_OK = 0x00,
    UTILS_ERROR = 0x01,
    UTILS_WORKING = 0x02,
    UTILS_BUSY  = 0x03,
}UTILS_Status;

typedef enum { 
    UTILS_BIT_SET = 0x01,
    UTILS_BIT_RESET = 0x00,
}UTILS_BitState;

typedef enum {
    UTILS_LOOP  = 0x00,
    UTILS_IT    = 0x01,
    UTILS_DMA   = 0x02,
}UTILS_CommunicationMode;

//------------------------------常用工具包(宏函数)------------------------------
#define UTILS_WriteBit(data, bit_pos, bit_state) \
    _Generic((data), \
            uint8_t*: UTILS_WriteBit_Byte, \
            uint16_t*: UTILS_WriteBit_Word, \
            uint32_t*: UTILS_WriteBit_32bit \
            )(data, bit_pos, bit_state)

#define UTILS_WriteBit_Zone(data, msb, lsb, value) \
    _Generic((data), \
            uint8_t*: UTILS_WriteBit_Zone_Byte,     \
            uint16_t*: UTILS_WriteBit_Zone_Word,    \
            uint32_t*: UTILS_WriteBit_Zone_32bit    \
            )(data, msb, lsb, value)

//------------------------------常用工具包(函数)------------------------------
UTILS_Status UTILS_RCC_GPIO_Enable(GPIO_TypeDef* GPIOx);
void UTILS_Delay_us(uint32_t us);
UTILS_Status UTILS_WriteBit_Byte(uint8_t* byte, uint8_t bit_pos, UTILS_BitState bit_state);
UTILS_Status UTILS_WriteBit_Word(uint16_t* word, uint8_t bit_pos, UTILS_BitState bit_state);
UTILS_Status UTILS_WriteBit_32bit(uint32_t* data, uint8_t bit_pos, UTILS_BitState bit_state);
UTILS_Status UTILS_WriteBit_Zone_Byte(uint8_t* byte, uint8_t msb, uint8_t lsb, uint8_t value);
UTILS_Status UTILS_WriteBit_Zone_Word(uint16_t* word, uint8_t msb, uint8_t lsb, uint16_t value);
UTILS_Status UTILS_WriteBit_Zone_32bit(uint32_t* data, uint8_t msb, uint8_t lsb, uint32_t val);

int32_t UTILS_Ceil(double data);
int32_t UTILS_Log2(uint32_t value);

uint32_t UTILS_GetSysTick(void);


void printf(const char *format, ...);

#endif
