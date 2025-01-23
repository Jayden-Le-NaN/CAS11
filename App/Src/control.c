#include "utils.h"
#include "control.h"

#ifdef BOARD_v1
#define RFSW_SEL_MAIN_23dBm(_MXD8546F_, _SKY13489_) do{MXD8546F_14_23_RFSW(_MXD8546F_);SKY13489_ANTto1_RFSW(_SKY13489_)}while(0)
#define RFSW_SEL_MAIN_27dBm(_MXD8546F_, _SKY13489_) do{MXD8546F_13_24_RFSW(_MXD8546F_);SKY13489_ANTto2_RFSW(_SKY13489_)}while(0)
#define RFSW_SEL_DSP_23dBm(_MXD8546F_, _SKY13489_) do{MXD8546F_13_24_RFSW(_MXD8546F_);SKY13489_ANTto1_RFSW(_SKY13489_)}while(0)


// 两路5V选择开关
TMUX4827_DCSW_Struct tmux4827_5V_DCSW_obj = {
    .TMUX4827_EN_Pin = GPIO_PIN_8,
    .TMUX4827_EN_Pin_Type = GPIOC,

    .TMUX4827_Sel_Pin = GPIO_PIN_15,
    .TMUX4827_Sel_Pin_Type = GPIOE
};

// 两路PA供电选择开关 3V3 和5V
TMUX4827_DCSW_Struct tmux4827_PA_DCSW_obj = {
    .TMUX4827_EN_Pin = GPIO_PIN_6,
    .TMUX4827_EN_Pin_Type = GPIOA,

    .TMUX4827_Sel_Pin = GPIO_PIN_7,
    .TMUX4827_Sel_Pin_Type = GPIOA
};

// 5V PA 过流过压保护
TPS16412_DCProtection_Struct tps16412_5V_PA_obj = {
    .TPS16412_EN_Pin = GPIO_PIN_14,
    .TPS16412_EN_Pin_Type = GPIOE,

    .TPS16412_FLT_Pin = GPIO_PIN_13,
    .TPS16412_FLT_Pin_Type = GPIOE
};

// 3V PA 过流过压保护
TPS16412_DCProtection_Struct tps16412_3V3_PA_obj = {
    .TPS16412_EN_Pin = GPIO_PIN_15,
    .TPS16412_EN_Pin_Type = GPIOD,

    .TPS16412_FLT_Pin = GPIO_PIN_6,
    .TPS16412_FLT_Pin_Type = GPIOC
};

// VCO 过流过压保护
TPS16412_DCProtection_Struct tps16412_VCO_obj = {
    .TPS16412_EN_Pin = GPIO_PIN_10,
    .TPS16412_EN_Pin_Type = GPIOD,

    .TPS16412_FLT_Pin = GPIO_PIN_11,
    .TPS16412_FLT_Pin_Type = GPIOD
};

// AVDD (Mixer&DDS) 过流过压保护
TPS16412_DCProtection_Struct tps16412_AVDD_obj = {
    .TPS16412_EN_Pin = GPIO_PIN_12,
    .TPS16412_EN_Pin_Type = GPIOA,

    .TPS16412_FLT_Pin = GPIO_PIN_12,
    .TPS16412_FLT_Pin_Type = GPIOD
};

// AVDD PLL 过流过压保护
TPS16412_DCProtection_Struct tps16412_AVDD_PLL_obj = {
    .TPS16412_EN_Pin = GPIO_PIN_13,
    .TPS16412_EN_Pin_Type = GPIOD,

    .TPS16412_FLT_Pin = GPIO_PIN_14,
    .TPS16412_FLT_Pin_Type = GPIOD
};

// 5V 测量线开关
TPS22810_DCSW_Struct tps22810_5V_Measure_obj = {
    .TPS22810_EN_Pin = GPIO_PIN_11,
    .TPS22810_EN_Pin_Type = GPIOB
};

// TODO: 3V3 测量线开关 终板v1没连！
// TPS22810_DCSW_Struct tps22810_3V3_Measure_obj = {
//     .TPS22810_EN_Pin = GPIO_PIN_12,
//     .TPS22810_EN_Pin_Type = GPIOB
// };

// DVDD 过流过压保护
TPS259621_DCProtection_Struct tps259621_DVDD_obj = {
    .TPS259621_FLT_Pin = GPIO_PIN_7,
    .TPS259621_FLT_Pin_Type = GPIOC
};

// 充电分立元件开关
Charger_DCSW_Struct charger_obj = {
    .Charger_EN_Pin = GPIO_PIN_11,
    .Charger_EN_Pin_Type = GPIOA
};

// 电池5V使能
LT3436_Boost_Struct lt3436_obj = {
    .LT3436_EN_Pin = GPIO_PIN_8,
    .LT3436_EN_Pin_Type = GPIOA
};

// 充电芯片状态
BQ25303_Bat_Struct bq25303_obj = {
    .BQ25303_Stat_Pin = GPIO_PIN_9,
    .BQ25303_Stat_Pin_Type = GPIOC
};

// 5V Buck使能
LMR51606_Buck_Struct lmr51606_obj = {
    .LMR51606_EN_Pin = GPIO_PIN_10,
    .LMR51606_EN_Pin_Type = GPIOB
};

// 信号来源&功放选择DPDT射频开关
MXD8546F_RFSW_Struct mxd8546f_obj = {
    .MXD8546F_RFSW_Pin = GPIO_PIN_11,
    .MXD8546F_RFSW_Pin_Type = GPIOB
};

// TODO: 功放到天线SPDT射频开关
SKY13489_RFSW_Struct sky13489_obj = {   
    .SKY13489_RFSW_Pin = GPIO_PIN_0,
    .SKY13489_RFSW_Pin_Type = GPIOE
};

/*
tmux4827_5V_DCSW_obj
tmux4827_PA_DCSW_obj
tps16412_5V_PA_obj
tps16412_3V3_PA_obj
tps16412_VCO_obj
tps16412_AVDD_obj
tps16412_AVDD_PLL_obj
tps22810_5V_Measure_obj
TODO: tps22810_3V3_Measure_obj
charger_obj
lt3436_obj
lmr51606_obj
mxd8546f_obj
sky13489_obj
*/
void control_output_init(void){
    UTILS_RCC_GPIO_Enable(GPIOA);
    UTILS_RCC_GPIO_Enable(GPIOB);
    UTILS_RCC_GPIO_Enable(GPIOC);
    UTILS_RCC_GPIO_Enable(GPIOD);
    UTILS_RCC_GPIO_Enable(GPIOE);

    // 关闭所有器件
    SHDN_TMUX4827_DCSW(tmux4827_5V_DCSW_obj);
    SHDN_TMUX4827_DCSW(tmux4827_PA_DCSW_obj);

    SHDN_TPS16412_DCProtection(tps16412_5V_PA_obj);
    SHDN_TPS16412_DCProtection(tps16412_3V3_PA_obj);
    SHDN_TPS16412_DCProtection(tps16412_VCO_obj);
    SHDN_TPS16412_DCProtection(tps16412_AVDD_obj);
    SHDN_TPS16412_DCProtection(tps16412_AVDD_PLL_obj);

    SHDN_TPS22810_DCSW(tps22810_5V_Measure_obj);
    // TODO: SHDN_TPS22810_DCSW(tps22810_3V3_Measure_obj);
        
    SHDN_Charger_DCSW(charger_obj);

    SHDN_LT3436_Boost(lt3436_obj);

    SHDN_LMR51606_BUCK(lmr51606_obj);

    RFSW_SEL_MAIN_23dBm(mxd8546f_obj, sky13489_obj);


    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = tmux4827_5V_DCSW_obj.TMUX4827_EN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(tmux4827_5V_DCSW_obj.TMUX4827_EN_Pin_Type, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = tmux4827_PA_DCSW_obj.TMUX4827_EN_Pin;
    HAL_GPIO_Init(tmux4827_PA_DCSW_obj.TMUX4827_EN_Pin_Type, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = tps16412_5V_PA_obj.TPS16412_EN_Pin;
    HAL_GPIO_Init(tps16412_5V_PA_obj.TPS16412_EN_Pin_Type, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = tps16412_3V3_PA_obj.TPS16412_EN_Pin;
    HAL_GPIO_Init(tps16412_3V3_PA_obj.TPS16412_EN_Pin_Type, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = tps16412_VCO_obj.TPS16412_EN_Pin;
    HAL_GPIO_Init(tps16412_VCO_obj.TPS16412_EN_Pin_Type, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = tps16412_AVDD_obj.TPS16412_EN_Pin;
    HAL_GPIO_Init(tps16412_AVDD_obj.TPS16412_EN_Pin_Type, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = tps16412_AVDD_PLL_obj.TPS16412_EN_Pin;
    HAL_GPIO_Init(tps16412_AVDD_PLL_obj.TPS16412_EN_Pin_Type, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = tps22810_5V_Measure_obj.TPS22810_EN_Pin;
    HAL_GPIO_Init(tps22810_5V_Measure_obj.TPS22810_EN_Pin_Type, &GPIO_InitStruct);
    //   TODO:
    //   GPIO_InitStruct.Pin = tps22810_3V3_Measure_obj.TPS22810_EN_Pin;
    //   HAL_GPIO_Init(tps22810_3V3_Measure_obj.TPS22810_EN_Pin_Type, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = charger_obj.Charger_EN_Pin;
    HAL_GPIO_Init(charger_obj.Charger_EN_Pin_Type, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = lmr51606_obj.LMR51606_EN_Pin;
    HAL_GPIO_Init(lmr51606_obj.LMR51606_EN_Pin_Type, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = mxd8546f_obj.MXD8546F_RFSW_Pin;
    HAL_GPIO_Init(mxd8546f_obj.MXD8546F_RFSW_Pin_Type, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = sky13489_obj.SKY13489_RFSW_Pin;
    HAL_GPIO_Init(sky13489_obj.SKY13489_RFSW_Pin_Type, &GPIO_InitStruct);
}
void control_exti_init(void){
    ;
}

#endif