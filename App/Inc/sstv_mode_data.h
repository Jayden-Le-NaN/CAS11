#ifndef SSTV_MODE_DATA_H_
#define SSTV_MODE_DEF_H_

#include "utils.h"
#include "sstv_mode_def.h"

//-------------------PD120 模式------------------
uint16_t PD120_header_psc[13] = {8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1};
uint16_t PD120_header_arr[13] = {3000-1, 100-1, 3000-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1};
uint16_t PD120_header_frq[13] = {1900, 1200, 1900, 1200, 1100, 1100, 1100, 1100, 1100, 1300, 1100, 1300, 1200};
uint16_t PD120_pulse_porch_arr_ptr1[2] = {200-1, 2080-190};
uint16_t PD120_pulse_porch_psc_ptr1[2] = {8000-1, 80};
uint16_t PD120_pulse_porch_frq_ptr1[2] = {1200, 1500};
uint16_t PD120_pulse_porch_num[4] = {2, 0, 0, 0};
SSTV_MODE_Struct PD120_MODE = {
    .sstv_mode = PD120,
    .sstv_dma_line_cnt = (496)/2,           // including 16 line header
    .sstv_dma_line_length = 640*3,

    .header_psc = PD120_header_psc,//psc 8000 ->10kHz 100us
    .header_arr = PD120_header_arr,
    .header_frq = PD120_header_frq,
    .header_num = 13,

    .pulse_porch_arr_ptr = {PD120_pulse_porch_arr_ptr1, NULL, NULL, NULL}, //20ms  (2.8-0.19)ms
    .pulse_porch_psc_ptr = {PD120_pulse_porch_psc_ptr1, NULL, NULL, NULL},      //100us 1us
    .pulse_porch_frq_ptr = {PD120_pulse_porch_frq_ptr1, NULL, NULL, NULL},
    .pulse_porch_num = PD120_pulse_porch_num,
    .loop_num = 4,
    //190us per color pixel -> 3 spi writes
    .dma_psc = 0,
    .dma_arr = 5068-1               //80MHz情况下周期为63.3375us，比较难做，主时钟最好是3倍数
};

//-------------------Scottie 1 模式------------------
uint16_t SCT1_header_psc[14] = {8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1};
uint16_t SCT1_header_arr[14] = {3000-1, 100-1, 3000-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 90-1};
uint16_t SCT1_header_frq[14] = {1900, 1200, 1900, 1200, 1300, 1300, 1100, 1100, 1100, 1100, 1300, 1300, 1200, 1200};
uint16_t SCT1_pulse_porch_arr_ptr1[1] = {6000-1+0};     // 1.5ms TODO: 15-1
uint16_t SCT1_pulse_porch_psc_ptr1[1] = {20-1};
uint16_t SCT1_pulse_porch_frq_ptr1[1] = {1500};
uint16_t SCT1_pulse_porch_arr_ptr2[1] = {6000-1-0};     // 1.5ms
uint16_t SCT1_pulse_porch_psc_ptr2[1] = {20-1};
uint16_t SCT1_pulse_porch_frq_ptr2[1] = {1500};
uint16_t SCT1_pulse_porch_arr_ptr3[2] = {36000-1, 6000-1};//-1800
uint16_t SCT1_pulse_porch_psc_ptr3[2] = {20-1, 20-1};
uint16_t SCT1_pulse_porch_frq_ptr3[2] = {1200, 1500};
uint16_t SCT1_pulse_porch_num[4] = {1, 1, 2, 0};
SSTV_MODE_Struct SCT1_MODE = {
    .sstv_mode = SCT1,
    .sstv_dma_line_cnt = 256,           //256 including 16 line header TODO:
    .sstv_dma_line_length = 320*3,

    .header_psc = SCT1_header_psc,//psc 8000 ->10kHz 100us
    .header_arr = SCT1_header_arr,
    .header_frq = SCT1_header_frq,
    .header_num = 14,

    .pulse_porch_arr_ptr = {SCT1_pulse_porch_arr_ptr1, SCT1_pulse_porch_arr_ptr2, SCT1_pulse_porch_arr_ptr3, NULL}, //20ms  (2.8-0.19)ms
    .pulse_porch_psc_ptr = {SCT1_pulse_porch_psc_ptr1, SCT1_pulse_porch_psc_ptr2, SCT1_pulse_porch_psc_ptr3, NULL},      //100us 1us
    .pulse_porch_frq_ptr = {SCT1_pulse_porch_frq_ptr1, SCT1_pulse_porch_frq_ptr2, SCT1_pulse_porch_frq_ptr3, NULL},
    .pulse_porch_num = SCT1_pulse_porch_num,
    .loop_num = 3,
    //432us per color pixel -> 3 spi writes
    .dma_psc = 20-1,        // 1us
    .dma_arr = 576-1
};

#endif