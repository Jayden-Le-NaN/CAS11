#ifndef MAX5394_H_
#define MAX5394_H_
#include "utils.h"


// MAX5394 片选引脚
#define MAX5394_CS_GPIO_PIN         GPIO_PIN_14
#define MAX5394_CS_GPIO_TYPE        GPIOB

// 指令表
#define MAX5394_WIPER               0x00
#define MAX5394_SD_CLR              0x80
#define MAX5394_SD_H_WREG           0x90
#define MAX5394_SD_H_ZERO           0x91
#define MAX5394_SD_H_MID            0x92
#define MAX5394_SD_H_FULL           0x93
#define MAX5394_SD_L_WREG           0x88
#define MAX5394_SD_L_ZERO           0x89
#define MAX5394_SD_L_MID            0x8A
#define MAX5394_SD_L_FULL           0x8B
#define MAX5394_SD_W                0x84
#define MAX5394_QP_OFF              0xA0
#define MAX5394_QP_ON               0xA1
#define MAX5394_RST                 0xC0

//------------------------------结构体------------------------------
typedef enum {
    MAX5394_H = 0x00,               // H端子
    MAX5394_W = 0x01,               // W端子
    MAX5394_L = 0x02,               // L端子
}MAX5394_TERMINAL_TYPE;

typedef enum {
    MAX5394_ZERO = 0x00,            // 0位置
    MAX5394_MID = 0x01,             // 满位置
    MAX5394_FULL = 0x02,            // 中间位置
    MAX5394_WREG = 0x03,            // 寄存器设置位置
}MAX5394_WIPER_POS;



//------------------------------函数接口------------------------------
#define MAX5394_CS_Enable()         HAL_GPIO_WritePin(MAX5394_CS_GPIO_TYPE, MAX5394_CS_GPIO_PIN, GPIO_PIN_RESET)
#define MAX5394_CS_Disable()        HAL_GPIO_WritePin(MAX5394_CS_GPIO_TYPE, MAX5394_CS_GPIO_PIN, GPIO_PIN_SET)

void MAX5394_Init(SPI_HandleTypeDef* hspi);
void MAX9394_SetRegisterVal(uint8_t tx_data);
uint8_t MAX5394_GetRegisterVal(void);
UTILS_Status MAX5394_SetResistance(MAX5394_TERMINAL_TYPE terminal_type, MAX5394_WIPER_POS wiper_pos);
void MAX5394_Reset(void);
void MAX5394_QP_Start(void);
void MAX5394_QP_Stop(void);

#endif
