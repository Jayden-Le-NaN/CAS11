#include "pm004m.h"

//------------------------------------------------------------
// Created Time     : 2024.10.08
// Autho            : JaydenLee
//------------------------------------------------------------

void PM004M_Init(PM004M_Info_struct* pm004m_obj, SPI_HandleTypeDef* spi, uint32_t cs_pin, GPIO_TypeDef* cs_pin_type) {
    //------------------------------数据挂载------------------------------
    pm004m_obj->spi = spi;
    pm004m_obj->cs_pin = cs_pin;
    pm004m_obj->cs_pin_type = cs_pin_type;

    //------------------------------配置CS引脚------------------------------
}
