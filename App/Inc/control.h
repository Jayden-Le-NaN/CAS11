#ifndef CONTROL_H
#define CONTROL_H

// 开关上电
#define EN_TMUX4827_DCSW(_TMUX4827_) HAL_GPIO_WritePin((_TMUX4827_).TMUX4827_EN_Pin_Type, (_TMUX4827_).TMUX4827_EN_Pin, GPIO_PIN_SET);
// 开关下电
#define SHDN_TMUX4827_DCSW(_TMUX4827_) HAL_GPIO_WritePin((_TMUX4827_).TMUX4827_EN_Pin_Type, (_TMUX4827_).TMUX4827_EN_Pin, GPIO_PIN_RESET);
// 公共端Dx连接SxA
#define SEL_A_TMUX4827_DCSW(_TMUX4827_) HAL_GPIO_WritePin((_TMUX4827_).TMUX4827_Sel_Pin_Type, (_TMUX4827_).TMUX4827_Sel_Pin, GPIO_PIN_RESET);
// 公共端Dx连接SxB
#define SEL_B_TMUX4827_DCSW(_TMUX4827_) HAL_GPIO_WritePin((_TMUX4827_).TMUX4827_Sel_Pin_Type, (_TMUX4827_).TMUX4827_Sel_Pin, GPIO_PIN_SET);
typedef struct {
    uint32_t                    TMUX4827_Sel_Pin;                          // 引脚编号
    GPIO_TypeDef*               TMUX4827_Sel_Pin_Type;                     // 引脚字母

    uint32_t                    TMUX4827_EN_Pin;                          // 引脚编号
    GPIO_TypeDef*               TMUX4827_EN_Pin_Type;                     // 引脚字母
}TMUX4827_DCSW_Struct;

// TODO: input or exti
// 开关上电
#define EN_TPS16412_DCProtection(_TPS16412_) HAL_GPIO_WritePin((_TPS16412_).TPS16412_EN_Pin_Type, (_TPS16412_).TPS16412_EN_Pin, GPIO_PIN_SET);
// 开关下电
#define SHDN_TPS16412_DCProtection(_TPS16412_) HAL_GPIO_WritePin((_TPS16412_).TPS16412_EN_Pin_Type, (_TPS16412_).TPS16412_EN_Pin, GPIO_PIN_RESET);
typedef struct {
    uint32_t                    TPS16412_FLT_Pin;
    GPIO_TypeDef*               TPS16412_FLT_Pin_Type;

    uint32_t                    TPS16412_EN_Pin;
    GPIO_TypeDef*               TPS16412_EN_Pin_Type;
}TPS16412_DCProtection_Struct;

// 开关上电
#define EN_TPS22810_DCSW(_TPS22810_) HAL_GPIO_WritePin((_TPS22810_).TPS22810_EN_Pin_Type, (_TPS22810_).TPS22810_EN_Pin, GPIO_PIN_SET);
// 开关下电
#define SHDN_TPS22810_DCSW(_TPS22810_) HAL_GPIO_WritePin((_TPS22810_).TPS22810_EN_Pin_Type, (_TPS22810_).TPS22810_EN_Pin, GPIO_PIN_RESET);
typedef struct {
    uint32_t                    TPS22810_EN_Pin;
    GPIO_TypeDef*               TPS22810_EN_Pin_Type;
}TPS22810_DCSW_Struct;

// TODO: input or exti
typedef struct {
    uint32_t                    TPS259621_FLT_Pin;
    GPIO_TypeDef*               TPS259621_FLT_Pin_Type;
}TPS259621_DCProtection_Struct;

// 分立元件开关
// 开关上电
#define EN_Charger_DCSW(_Charger_) HAL_GPIO_WritePin((_Charger_).Charger_EN_Pin_Type, (_Charger_).Charger_EN_Pin, GPIO_PIN_SET);
// 开关下电
#define SHDN_Charger_DCSW(_Charger_) HAL_GPIO_WritePin((_Charger_).Charger_EN_Pin_Type, (_Charger_).Charger_EN_Pin, GPIO_PIN_RESET);
typedef struct {
    uint32_t                    Charger_EN_Pin;
    GPIO_TypeDef*               Charger_EN_Pin_Type;
}Charger_DCSW_Struct;

// 开关上电
#define EN_LT3436_Boost(_LT3436_) HAL_GPIO_WritePin((_LT3436_).LT3436_EN_Pin_Type, (_LT3436_).LT3436_EN_Pin, GPIO_PIN_SET);
// 开关下电
#define SHDN_LT3436_Boost(_LT3436_) HAL_GPIO_WritePin((_LT3436_).LT3436_EN_Pin_Type, (_LT3436_).LT3436_EN_Pin, GPIO_PIN_RESET);
typedef struct {
    uint32_t                    LT3436_EN_Pin;
    GPIO_TypeDef*               LT3436_EN_Pin_Type;
}LT3436_Boost_Struct;

// TODO: input or exti
typedef struct {
    uint32_t                    BQ25303_Stat_Pin;
    GPIO_TypeDef*               BQ25303_Stat_Pin_Type;
}BQ25303_Bat_Struct;

// 5V Buck上电
#define EN_LMR51606_BUCK(_LMR51606_) HAL_GPIO_WritePin((_LMR51606_).LMR51606_EN_Pin_Type, (_LMR51606_).LMR51606_EN_Pin, GPIO_PIN_SET);
// 5V Buck下电
#define SHDN_LMR51606_BUCK(_LMR51606_) HAL_GPIO_WritePin((_LMR51606_).LMR51606_EN_Pin_Type, (_LMR51606_).LMR51606_EN_Pin, GPIO_PIN_RESET);
typedef struct {
    uint32_t                    LMR51606_EN_Pin;
    GPIO_TypeDef*               LMR51606_EN_Pin_Type;
}LMR51606_Buck_Struct;

// RF开关 RF3 to RF1, RF4 to RF2
#define MXD8546F_13_24_RFSW(_MXD8546F_) HAL_GPIO_WritePin((_MXD8546F_).MXD8546F_RFSW_Pin_Type, (_MXD8546F_).MXD8546F_RFSW_Pin, GPIO_PIN_SET);
// RF开关 RF3 to RF2, RF4 to RF1
#define MXD8546F_14_23_RFSW(_MXD8546F_) HAL_GPIO_WritePin((_MXD8546F_).MXD8546F_RFSW_Pin_Type, (_MXD8546F_).MXD8546F_RFSW_Pin, GPIO_PIN_RESET);
typedef struct {
    uint32_t                    MXD8546F_RFSW_Pin;
    GPIO_TypeDef*               MXD8546F_RFSW_Pin_Type;
}MXD8546F_RFSW_Struct;

// RF开关 ANT to RF2
#define SKY13489_ANTto2_RFSW(_SKY13489_) HAL_GPIO_WritePin((_SKY13489_).SKY13489_RFSW_Pin_Type, (_SKY13489_).SKY13489_RFSW_Pin, GPIO_PIN_SET);
// RF开关 ANT to RF1
#define SKY13489_ANTto1_RFSW(_SKY13489_) HAL_GPIO_WritePin((_SKY13489_).SKY13489_RFSW_Pin_Type, (_SKY13489_).SKY13489_RFSW_Pin, GPIO_PIN_RESET);
typedef struct {
    uint32_t                    SKY13489_RFSW_Pin;
    GPIO_TypeDef*               SKY13489_RFSW_Pin_Type;
}SKY13489_RFSW_Struct;

// TODO: 多路ADC，采集要配合开关，在本文件实现

void control_output_init(void);
void control_exti_init(void);

#endif /* CONTROL_H */