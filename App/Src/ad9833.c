#include "ad9833.h"

static SPI_HandleTypeDef* ad9833_spi;
// static DMA_HandleTypeDef* ad9833_dma_spi_tx;

// 频率比例因子
static const double ad9833_freq_scale_factor = 268435456.0 / AD9833_FCLK;

/*
 * @brief               初始化ad9833
 * @param hspi          用来传输的spi结构体
 * @param hdma_spi_tx   用来发送数据的dma结构体
 * @return              无
 */
void AD9833_Init(SPI_HandleTypeDef* hspi) {
    //------------------------------初始化gpio------------------------------

    //时钟使能
    UTILS_RCC_GPIO_Enable(AD9833_FSYN_GPIO_TYPE);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    // 初始设置为高电平
    HAL_GPIO_WritePin(AD9833_FSYN_GPIO_TYPE, AD9833_FSYN_GPIO_PIN, GPIO_PIN_SET);

    // 配置GPIO
    GPIO_InitStruct.Pin = AD9833_FSYN_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(AD9833_FSYN_GPIO_TYPE, &GPIO_InitStruct);

    //------------------------------挂载spi------------------------------
    ad9833_spi = hspi;    
}


/*
 * @brief               通过spi传输数据(dma)
 * @param tx_data       需要传输的数据
 * @return              无
 */
void AD9833_WriteData(uint16_t tx_data) {
    uint8_t data[2];
    data[0] = (uint8_t)((tx_data >> 8) & 0xFF);
    data[1] = (uint8_t)(tx_data & 0xFF);
    AD9833_FSYN_Enable();
    // HAL_SPI_Transmit_DMA(ad9833_spi, data, sizeof(data));            //TODO: 完成DMA功能的实现
    HAL_SPI_Transmit(ad9833_spi, data, sizeof(data), 0xFF);
    AD9833_FSYN_Disable();
}


/*
 * @brief               设置ad9833的信号频率
 * @param reg           需要设置的频率寄存器
 * @param val           需要设置的频率
 * @return              无
 */ 
void AD9833_SetFrequency(uint16_t reg, double val) {
    uint16_t freq_high = reg;
    uint16_t freq_low = reg;
    uint32_t freq = val * ad9833_freq_scale_factor;

    freq_low |= (freq & 0x3FFF);
    freq_high |= ((freq >> 14) & 0x3FFF);

    AD9833_WriteData(AD9833_RESET | AD9833_B28);
    AD9833_WriteData(freq_low);
    AD9833_WriteData(freq_high);
}

/*
 * @brief               设置ad9833的信号相位
 * @param reg           需要设置的相位寄存器
 * @param val           需要设置的相位
 * @return              无
 */
void AD9833_SetPhase(uint16_t reg, uint16_t val) {
    uint16_t phase = reg;
    phase |= val;
    AD9833_WriteData(phase);
}


/*
 * @brief               设置ad9833的波形
 * @param wave_mode     波形模式
 * @param freq_sfr      输出的频率寄存器选择
 * @param phase_sfr     输出的相位寄存器选择
 * @return              无
 */
void AD9833_SetWave(uint16_t wave_mode, uint16_t freq_sfr, uint16_t phase_sfr) {
    uint16_t val = 0;
    val = (val | wave_mode | freq_sfr | phase_sfr);
    AD9833_WriteData(val);
}

/*
 * @brief               ad9833总体设置
 * @param freq_reg      设置的频率寄存器
 * @param freq_val      设置的频率值
 * @param phase_reg     设置的相位寄存器
 * @param phase_val     设置的相位的值
 * @param wave_mode     设置的波形输出模式
 * @return              无
 */
void AD9833_Setup(uint16_t freq_reg, double freq_val, uint16_t phase_reg, uint16_t phase_val, uint16_t wave_mode) {

    uint16_t freq_sfr = (freq_reg == AD9833_REG_FREQ0 ? AD9833_FSEL0 : AD9833_FSEL1);
    uint16_t phase_sfr = (phase_reg == AD9833_REG_PHASE0 ? AD9833_PSEL0 : AD9833_PSEL1);

    AD9833_WriteData(AD9833_RESET);
    AD9833_WriteData(AD9833_RESET | AD9833_B28);            // 选择数据写入一次,B28位和RESET位为1
    AD9833_SetFrequency(freq_reg, freq_val);
    AD9833_SetPhase(phase_reg, phase_val);
    AD9833_SetWave(wave_mode, freq_sfr, phase_sfr);
}
