#ifndef ADF4252_H_
#define ADF4252_H_

#include "utils.h"

//------------------------------寄存器地址表(可更改值,但不建议)------------------------------
#define ADF4252_REG_RF_N_DIVIDER                (0x00)
#define ADF4252_REG_RF_R_DIVIDER                (0x01)
#define ADF4252_REG_RF_CONTROL                  (0x02)
#define ADF4252_REG_MASTER                      (0x03)
#define ADF4252_REG_IF_N_DIVIDER                (0x04)
#define ADF4252_REG_IF_R_DIVIDER                (0x05)
#define ADF4252_REG_IF_CONTROL                  (0x06)


//------------------------------寄存器的Value/Counter编码(可更改值,但不建议)------------------------------
#define ADF4252_VC_RF_INTEGER                   (0x10)
#define ADF4252_VC_RF_FRACTIONAL                (0x11)
#define ADF4252_VC_RF_R                         (0x12)
#define ADF4252_VC_INTERPOLATOR_MODULUS         (0x13)
#define ADF4252_VC_IF_B                         (0x14)
#define ADF4252_VC_IF_A                         (0x15)
#define ADF4252_VC_IF_R                         (0x16)

//------------------------------寄存器的状态位编码(可更改, 但不建议)------------------------------
#define ADF4252_BIT_RF_REF_DOUBLER              (0x20)
#define ADF4252_BIT_RF_PD_POLARITY              (0x21)
#define ADF4252_BIT_RF_POWER_DOWN               (0x22)
#define ADF4252_BIT_RF_CP_THREE_STATE           (0x23)
#define ADF4252_BIT_RF_COUNTER_RESET            (0x24)
#define ADF4252_BIT_XO_DISABLE                  (0x25)
#define ADF4252_BIT_POWER_DOWN                  (0x26)
#define ADF4252_BIT_CP_THREE_STATE              (0x27)
#define ADF4252_BIT_COUNTER_RESET               (0x28)
#define ADF4252_BIT_IF_CP_GAIN                  (0x29)
#define ADF4252_BIT_IF_REF_DOUBLER              (0x2A)
#define ADF4252_BIT_RF_PHASE_RESYNC             (0x2B)
#define ADF4252_BIT_IF_PD_POLARITY              (0x2C)
#define ADF4252_BIT_IF_POWER_DOWN               (0x2D)
#define ADF4252_BIT_IF_CP_THREE_STATE           (0x2E)
#define ADF4252_BIT_IF_COUNTER_RESET            (0x2F)

//------------------------------状态值(不可更改值)------------------------------
#define ADF4252_STATUS_DISABLED                 (0x00)
#define ADF4252_STATUS_ENABLED                  (0x01)
#define ADF4252_STATUS_NEGATIVE                 (0x00)
#define ADF4252_STATUS_POSITIVE                 (0x01)
#define ADF4252_STATUS_THREE_STATE              (0x01)
#define ADF4252_STATUS_XO_ENABLED               (0x00)
#define ADF4252_STATUS_XO_DISABLED              (0x01)

//------------------------------多路复用器的设置(不可更改值)------------------------------
#define ADF4252_MUX_IF_ANALOG_LOCK_DETECT       (0x01)
#define ADF4252_MUX_IF_R_DIVIDER_OUTPUT         (0x02)
#define ADF4252_MUX_IF_N_DIVIDER_OUTPUT         (0x03)
#define ADF4252_MUX_RF_ANALOG_LOCK_DETECT       (0x04)
#define ADF4252_MUX_RF_IF_ANALOG_LOCK_DETECT    (0x05)
#define ADF4252_MUX_IF_DIGITAL_LOCK_DETECT      (0x06)
#define ADF4252_MUX_RF_R_DIVIDER_HIGH           (0x08)
#define ADF4252_MUX_RF_N_DIVIDER_OUTPUT         (0x09)
#define ADF4252_MUX_THREE_STATE_OUTPUT          (0x0A)
#define ADF4252_MUX_RF_DIGITAL_LOCK_DETECT      (0x0C)
#define ADF4252_MUX_RF_IF_DIGITAL_LOCK_DETECT   (0x0D)
#define ADF4252_MUX_LOGIC_HIGH                  (0x0E)
#define ADF4252_MUX_LOGIC_LOW                   (0x0F)

//------------------------------噪声和毛刺抑制(不可更改值)------------------------------
#define ADF4252_NAS_LOWEST_SPUR                 (0x00)
#define ADF4252_NAS_LOW_NOISE_AND_SPUR          (0x01)
#define ADF4252_NAX_LOWEST_NOISE                (0x07)

//------------------------------RF预分频器设置------------------------------
#define ADF4252_RF_PRESCALER_4                  (0x00)
#define ADF4252_RF_PRESCALER_8                  (0x01)
//------------------------------IF预分频器设置------------------------------
#define ADF4252_IF_PRESCALER_8                  (0x10)
#define ADF4252_IF_PRESCALER_16                 (0x11)
#define ADF4252_IF_PRESCALER_32                 (0x12)
#define ADF4252_IF_PRESCALER_64                 (0x13)

typedef struct {
    //------------------------------用户配置部分------------------------------
    SPI_HandleTypeDef*              spi;                            // spi结构体
    uint32_t                        le_pin;                         // le引脚
    GPIO_TypeDef*                   le_pin_type;                    // le引脚类型

    //------------------------------禁止用户配置------------------------------
    uint32_t                        _val_rf_n_divider;              // rf_n_divider 寄存器的值
    uint32_t                        _val_rf_r_divider;              // rf_r_divider 寄存器的值
    uint32_t                        _val_rf_control;                // rf_control 寄存器的值
    uint32_t                        _val_master;                    // master 寄存器的值
    uint32_t                        _val_if_n_divider;              // if_n_divider 寄存器的值
    uint32_t                        _val_if_r_divider;              // if_r_divider 寄存器的值
    uint32_t                        _val_if_control;                // if_control 寄存器的值
}ADF4252_Info_Struct;


void ADF4252_Init(ADF4252_Info_Struct* adf4252_obj, SPI_HandleTypeDef* spi, uint32_t le_pin, GPIO_TypeDef* le_pin_type);
UTILS_Status ADF4252_Prescaler_Set(ADF4252_Info_Struct* adf4252_obj, uint8_t set_val);
UTILS_Status ADF4252_NAS_Set(ADF4252_Info_Struct* adf4252_obj, uint8_t set_val);
UTILS_Status ADF4252_MUX_Set(ADF4252_Info_Struct* adf4252_obj, uint8_t set_val);
UTILS_Status ADF4252_Status_Set(ADF4252_Info_Struct* adf4252_obj, uint8_t bit_code, uint8_t val);
UTILS_Status ADF4252_VC_Set(ADF4252_Info_Struct* adf4252_obj, uint8_t vc_code, uint32_t val);
UTILS_Status ADF4252_Write_All_Registers(ADF4252_Info_Struct* adf4252_obj);
UTILS_Status ADF4252_Write_register(ADF4252_Info_Struct* adf4252_obj, uint8_t reg_addr);

#endif
