#ifndef SSTV_MODE_DEF_H_
#define SSTV_MODE_DEF_H_

#include "utils.h"

typedef enum {
    PD120,
    SCT1
}SSTV_MODE;


typedef struct{
    SSTV_MODE                    sstv_mode;                         
    uint16_t                     sstv_dma_line_cnt;                 // sstv的行数
    uint16_t                     sstv_dma_line_length;              // 每一行数据的长度
    uint16_t                     sstv_dma_one_line;                 // 每一行要使用DMA的次数
//-------------------VIS头配置信息-----------------
    uint16_t*                    header_arr;                        // VIS头（部分模式还有别的）对应TIM的arr值
    uint16_t*                    header_psc;                        // VIS头（部分模式还有别的）对应TIM的的psc值
    uint16_t*                    header_frq;                        // VIS头（部分模式还有别的）对应频率寄存器值
    uint16_t                     header_num;                        // sstv头长度，通常是13，有模式不是
    //---------循环中sync pulse和sync porch信息---------
    uint16_t*                    pulse_porch_arr_ptr[4];            // （最多）四次扫描前的同步脉冲对应arr值
    uint16_t*                    pulse_porch_psc_ptr[4];            // 四次扫描前的同步脉冲对应psc值
    uint16_t*                    pulse_porch_frq_ptr[4];            // 四次扫描前的同步脉冲对应频率寄存器
    uint16_t*                    pulse_porch_num;                // 四轮的pulse porch数
    uint16_t                     loop_num;                          // 轮数，一次扫一行的通常是3轮，一次扫两行的通常是4轮
    uint16_t                     dma_psc;
    uint16_t                     dma_arr;
}SSTV_MODE_Struct ;


#endif