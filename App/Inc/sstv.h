#ifndef SSTV_H_
#define SSTV_H_

#include "utils.h"
#include "ad9833.h"
#include "stm32l4xx_hal_spi.h"
#include "sstv_mode_def.h"

#define     SSTV_UNUSED     0xffff                         //模式中未用到的配置信息赋值

/**
 * @todo 添加状态
 * @brief  枚举多种模式的SSTV的所有可能的状态
*/
typedef enum {
    SSTV_Idle,
    SSTV_Transmitting
}SSTV_STAT;
typedef enum {
    SSTV_FSM_Idle,
    SSTV_FSM_Header,
    SSTV_FSM_Loop,
    SSTV_FSM_DMA,
    SSTV_FSM_END
}SSTV_FSM;
/**
 * @todo 添加SSTV模式
 * @brief  枚举所有支持的SSTV模式
*/
// typedef enum {
//     PD120
// }SSTV_MODE;


// typedef struct{
//     SSTV_MODE                    sstv_mode;                         
//     uint16_t                     sstv_dma_line_cnt;                 // sstv的行数
//     uint16_t                     sstv_dma_line_length;              // 每一行数据的长度
//     uint16_t                     sstv_dma_one_line;                 // 每一行要使用DMA的次数
// //-------------------VIS头配置信息-----------------
//     uint16_t*                    header_arr;                        // VIS头（部分模式还有别的）对应TIM的arr值
//     uint16_t*                    header_psc;                        // VIS头（部分模式还有别的）对应TIM的的psc值
//     uint16_t*                    header_frq;                        // VIS头（部分模式还有别的）对应频率寄存器值
//     uint16_t                     header_num;                        // sstv头长度，通常是13，有模式不是
//     //---------循环中sync pulse和sync porch信息---------
//     uint16_t*                    pulse_porch_arr_ptr[4];            // （最多）四次扫描前的同步脉冲对应arr值
//     uint16_t*                    pulse_porch_psc_ptr[4];            // 四次扫描前的同步脉冲对应psc值
//     uint16_t*                    pulse_porch_frq_ptr[4];            // 四次扫描前的同步脉冲对应频率寄存器
//     uint16_t*                    pulse_porch_num;                // 四轮的pulse porch数
//     uint16_t                     loop_num;                          // 轮数，一次扫一行的通常是3轮，一次扫两行的通常是4轮
//     uint16_t                     dma_psc;
//     uint16_t                     dma_arr;
// }SSTV_MODE_Struct ;
// uint16_t PD120_header_psc[13] = {8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1, 8000-1};
// uint16_t PD120_header_arr[13] = {3000-1, 100-1, 3000-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1, 300-1};
// uint16_t PD120_header_frq[13] = {1900, 1200, 1900, 1200, 1100, 1100, 1100, 1100, 1100, 1300, 1100, 1300, 1200};
// uint16_t PD120_pulse_porch_arr_ptr1[2] = {200-1, 2080-190};
// uint16_t PD120_pulse_porch_psc_ptr1[2] = {8000-1, 80};
// uint16_t PD120_pulse_porch_frq_ptr1[2] = {1200, 1500};
// uint16_t PD120_pulse_porch_num[4] = {2, 0, 0, 0};
// const SSTV_MODE_Struct PD120_MODE = {
//     .sstv_mode = PD120,
//     .sstv_dma_line_cnt = (496)/2,           // including 16 line header
//     .sstv_dma_line_length = 640*3,

//     .header_psc = PD120_header_psc,//psc 8000 ->10kHz 100us
//     .header_arr = PD120_header_arr,
//     .header_frq = PD120_header_frq,
//     .header_num = 13,

//     .pulse_porch_arr_ptr = {PD120_pulse_porch_arr_ptr1, NULL, NULL, NULL}, //20ms  (2.8-0.19)ms
//     .pulse_porch_psc_ptr = {PD120_pulse_porch_psc_ptr1, NULL, NULL, NULL},      //100us 1us
//     .pulse_porch_frq_ptr = {PD120_pulse_porch_frq_ptr1, NULL, NULL, NULL},
//     .pulse_porch_num = PD120_pulse_porch_num,
//     .loop_num = 4,
//     //190ms per color pixel -> 3 spi writes
//     .dma_psc = 0,
//     .dma_arr = 5068-1               //80MHz情况下周期为63.3375us，比较难做，主时钟最好是3倍数
// };

/**
 * @brief SSTV_Info_Struct
 */
typedef struct {
    /// @public
    AD9833_Info_Struct*          AD9833_I;                          // 同相路DDS
    AD9833_Info_Struct*          AD9833_Q;                          // 正交路DDS
    SSTV_MODE_Struct*            sstv_mode;                         // SSTV模式
    uint16_t*                    tx_buffer_ptr;                     // 指向申请的buffer
    //void (*TxISR)(struct __SPI_HandleTypeDef *hspi);

    SSTV_STAT                    _sstv_tx_state;
    SSTV_FSM                     _sstv_fsm;
    uint8_t                      _header_index;
    uint8_t                      _loop_index;
    uint8_t                      _pulse_porch_index;
    uint16_t                     _line_sended;

//-------------------sstv模式配置信息-----------------
    // uint16_t                     _sstv_dma_line_cnt;                // sstv的行数
    // uint16_t*                    _tx_buffer;                        // 发送数据的缓冲区
    // uint16_t                     _sstv_dma_line_length;             // 每一行数据的长度
    // uint16_t                     _sstv_dma_one_line;                // 每一行要使用DMA的次数
    // //-------------------VIS头配置信息-----------------
    // uint16_t*                    _header_arr;                       // VIS头（部分模式还有别的）对应TIM的arr值
    // uint16_t*                    _header_psc;                       // VIS头（部分模式还有别的）对应TIM的的psc值
    // uint16_t*                    _header_frq;                       // VIS头（部分模式还有别的）对应频率寄存器值
    // uint16_t                     _header_num;                       // sstv头长度，通常是13，有模式不是
    // //---------循环中sync pulse和sync porch信息---------
    // uint16_t*                    _pulse_porch_arr_ptr[4];           // （最多）四次扫描前的同步脉冲对应arr值
    // uint16_t*                    _pulse_porch_psc_ptr[4];           // 四次扫描前的同步脉冲对应psc值
    // uint16_t*                    _pulse_porch_frq_ptr[4];           // 四次扫描前的同步脉冲对应频率寄存器
    // uint16_t*                    _pulse_porch_num[4];               // 四轮的pulse porch数
    // //循环内的tim信息
    // uint16_t                     dma_psc;
    // uint16_t                     dma_arr;

}SSTV_Info_Struct;



UTILS_Status SSTV_Init(SSTV_MODE_Struct* sstv_mode_struct, AD9833_Info_Struct *ad9833_i, AD9833_Info_Struct *ad9833_q);
UTILS_Status SSTV_Transmit(void);
void SSTV_TIM_Header_Callback(void);
void SSTV_TIM_Loop_Callback(void);
void SSTV_DMA_Cplt_Callback(void);
void SSTV_DMA_HalfCplt_Callback(void);

#endif /* SSTV_H_ */