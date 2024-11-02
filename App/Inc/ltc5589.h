#ifndef LTC5589_H_
#define LTC5589_H_

#include "utils.h"

typedef enum {
    LTC5589_FINE_MANUAL             = 0x00,         // 手动微调
    LTC5589_FINE_ASYNCHRONOUS       = 0x01,         // 异步微调
    LTC5589_FINE_SYNCHRONIZATION    = 0x02,         // 同步微调
}LTC5589_DIGITAL_GAIN_FINE_MODE;

typedef enum {
    LTC5589_CHANNEL_I               = 0x00,         // I 通道
    LTC5589_CHANNEL_Q               = 0x01,         // Q 通道
}LTC5589_CHANNEL;
typedef struct {
    //------------------------------用户配置部分------------------------------
    SPI_HandleTypeDef*              spi;
    uint32_t                        cs_pin;
    GPIO_TypeDef*                   cs_pin_type;
    uint32_t                        ttck_pin;
    GPIO_TypeDef*                   ttck_pin_type;

    //------------------------------禁止用户配置------------------------------
    LTC5589_DIGITAL_GAIN_FINE_MODE  _digital_gain_fine_mode;    // 数字增益微调模式

}LTC5589_Info_Struct;

void LTC5589_Init(LTC5589_Info_Struct* ltc5589_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type, uint32_t ttck_pin, GPIO_TypeDef* ttck_pin_type);
UTILS_Status LTC5589_Read_Register(LTC5589_Info_Struct* ltc5589_obj, uint8_t reg_addr, uint8_t* rx_data);
UTILS_Status LTC5589_Set_Frequency(LTC5589_Info_Struct* ltc5589_obj, uint8_t freq);
UTILS_Status LTC5589_Set_DigitalGain_Coarse(LTC5589_Info_Struct* ltc5589_obj, int8_t gain);
UTILS_Status LTC5589_Set_DigitalGain_Fine(LTC5589_Info_Struct* ltc5589_obj, int8_t gain);
UTILS_Status LTC5589_Set_DigitalGain_Fine_Mode(LTC5589_Info_Struct* ltc5589_obj, LTC5589_DIGITAL_GAIN_FINE_MODE mode);
UTILS_Status LTC5589_Set_IQ_GainRatio(LTC5589_Info_Struct* ltc5589_obj, uint8_t ratio);
UTILS_Status LTC5589_Set_DCOffset(LTC5589_Info_Struct* ltc5589_obj, LTC5589_CHANNEL channel, uint8_t offset);
UTILS_Status LTC5589_Set_Phase(LTC5589_Info_Struct* ltc5589_obj, double phi);
UTILS_Status LTC5589_Q_Channel_Enable(LTC5589_Info_Struct* ltc5589_obj);
UTILS_Status LTC5589_Q_Channel_Disable(LTC5589_Info_Struct* ltc5589_obj);

#endif
