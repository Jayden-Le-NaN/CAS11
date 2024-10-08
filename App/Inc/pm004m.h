#ifndef PM004M_H_
#define PM004M_H_
#include "utils.h"

//------------------------------指令表------------------------------
#define PM004M_CMD_MODE_REGISTER_WRITE              (0xB1)
#define PM004M_CMD_MODE_REGISTER_READ               (0xB5)
#define PM004M_CMD_WRITE                            (0x02)
#define PM004M_CMD_READ                             (0x03)
#define PM004M_CMD_WRITE_ENABLE                     (0x06)
#define PM004M_CMD_WRITE_DISABLE                    (0x04)
#define PM004M_CMD_ENTRY_TO_DEEP_POWER_DOWN         (0xB9)
#define PM004M_CMD_EXIT_FROM_DEEP_POWER_DOWN        (0xAB)
#define PM004M_CMD_READ_UNIQUE_ID_REGISTER          (0x9F)
#define PM004M_CMD_RESET_ENABLE                     (0x66)
#define PM004M_CMD_RESET                            (0x99)

typedef struct {
    //------------------------------用户配置部分------------------------------
    SPI_HandleTypeDef*          spi;
    uint32_t                    cs_pin;
    GPIO_TypeDef*               cs_pin_type;
    //------------------------------禁止用户配置------------------------------

}PM004M_Info_struct;



#endif
