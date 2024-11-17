#ifndef SSTV_H_
#define SSTV_H_

#include "utils.h"

/**
 * @todo 添加状态
 * @brief  枚举多种模式的SSTV的所有可能的状态
*/
typedef enum {
    SSTV_Idle                               = 0x00         /**空闲状态 */
}SSTV_FSM;

/**
 * @todo 添加SSTV模式
 * @brief  枚举所有支持的SSTV模式
*/
typedef enum {
    PD120                                   = 0x00         /**PD120模式 */
}SSTV_MODE;

/**
 * @brief SSTV_Info_Struct
 */
typedef struct {
    /// @public
    AD9833_Info_Struct*          AD9833_Dual_DDS;                   // 双路DDS
    SSTV_MODE                    sstv_mode;                         // SSTV模式
    void (*TxISR)(struct __SPI_HandleTypeDef *hspi);

    /// @private
    SSTV_FSM              _SSTV_tx_state;

}SSTV_Info_Struct;
void SSTV_SPI_Init(void);
#endif /* SSTV_H_ */