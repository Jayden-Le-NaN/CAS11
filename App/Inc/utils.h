#ifndef UTILS_H_
#define UTILS_H_

// 头文件功能: 包含公共的头文件供APP中的文件使用
//           : 包含公共的宏共APP中的文件使用

//------------------------------公用宏------------------------------
// #define MCU_STM32F4XX
#define MCU_STM32L4XX
#define TEST_MODE
// 终板v1
// #define BOARD_v1


#define PRINT_BUFFER_SIZE       256U        // printf 的缓冲区大小

//------------------------------标准库(C语言)------------------------------
#include "stdarg.h"
#include "stdint.h"
#include "stdbool.h"
#include <string.h>

//------------------------------HAL库------------------------------
#ifdef MCU_STM32F4XX
#define MCU_FREQUENCY_MHZ       168         // STM32 系统时钟主频
#include "stm32f4xx.h"
#endif

#ifdef MCU_STM32L4XX
#define MCU_FREQUENCY_MHZ       80          // STM32 系统时钟主频
#include "stm32l4xx_hal.h"
#endif


typedef enum {
    UTILS_OK = 0x00,
    UTILS_ERROR = 0x01,
    UTILS_WORKING = 0x02,
    UTILS_BUSY  = 0x03,
    UTILS_IDLE = 0x04,
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
#define UTILS_LOCK(__HANDLE__) do{ if((__HANDLE__)->Lock == HAL_LOCKED) { return UTILS_BUSY; } else { (__HANDLE__)->Lock = HAL_LOCKED; } }while (0)
#define UTILS_UNLOCK(__HANDLE__) do{ (__HANDLE__)->Lock = HAL_UNLOCKED; }while (0)

//------------------------------给ide使用------------------------------
extern int strncmp(const char *str1, const char *str2, size_t n);
extern char *strstr(const char *haystack, const char *needle);
extern char *strchr(const char *str, int c);
extern int atoi(const char *str);
extern void *memchr(const void *str, int c, size_t n);
extern double atof(const char *str);
extern int sprintf(char *str, const char *format, ...);
extern int snprintf ( char * str, size_t size, const char * format, ... );
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
//----------------------------------调试函数-----------------------------------
uint32_t UTILS_GetSysTick(void);
uint32_t Calculate_ElapsedTime(uint32_t start, uint32_t end);
void sendString(const char* str);

void printf(const char *format, ...);

#ifdef TEST_MODE

// #define CONTROL_TEST
#define CAL_TIME
// #define OSC_TRIGGER_TEST
// #define AT_TEST
// #define SSTV_TEST
// #define FLASH_TEST
// #define MRAM_TEST
// #define LTC5589_TEST
// #define LTC5589_SCAN_TEST
// #define ADF4252_TEST
#define AD9833_TEST
// #define AD9833_SCAN_TEST

//----------------------------------示波器触发源-----------------------------------
typedef enum{
    RISING_EDGE,
    FALLING_EDGE
}OSC_TRIGGER_EDGE;

// 给示波器采样的触发信号
typedef struct {
    uint32_t                    osc_trigger_pin;                          // 信号同步引脚
    GPIO_TypeDef*               osc_trigger_pin_type;                     // 信号同步引脚的类型
    TIM_HandleTypeDef*          osc_tim;
    OSC_TRIGGER_EDGE            trigger_edge;
}OSC_Trigger;
UTILS_Status osc_trigger_init(OSC_Trigger* osc_trigger_obj, uint32_t osc_trigger_pin, GPIO_TypeDef* osc_trigger_pin_type, OSC_TRIGGER_EDGE trigger_edge, TIM_HandleTypeDef* osc_tim);
UTILS_Status osc_trigger_prepare(OSC_Trigger* osc_trigger_obj);
UTILS_Status osc_trigger_start(OSC_Trigger* osc_trigger_obj);
UTILS_Status osc_trigger_end(OSC_Trigger* osc_trigger_obj);

//----------------------------------计算程序执行时间-----------------------------------
typedef struct{
    uint32_t start_time_us;
    uint32_t end_time_us;
    uint32_t start_time_ms;
    uint32_t end_time_ms;
    UTILS_Status status;
    uint32_t interval_us;
}Timemeter_Struct;
UTILS_Status Timemeter_Init(Timemeter_Struct* timemeter_obj);
UTILS_Status Timemeter_Start(Timemeter_Struct* timemeter_obj);
UTILS_Status Timemeter_End(Timemeter_Struct* timemeter_obj, bool enPrint);
uint32_t Timemeter_Get_Interval(Timemeter_Struct* timemeter_obj);
#endif

#endif
