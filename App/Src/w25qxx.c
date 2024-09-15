#include "w25qxx.h"

uint16_t W25QXX_TYPE = W25Q80;              // 芯片的型号
uint8_t w25qxx_buffer[4096];                // 数据缓冲区
static SPI_HandleTypeDef* w25qxx_spi;       // 挂载的spi


/*
 * @brief               传输一字节数据
 * @param tx_data       需要传输的数据
 * @return              传输状态:   UTILS_Ok - 没有问题
 *                                  UTILS_ERROR - 有问题
 */
static UTILS_Status W25QXX_TransmitByte(uint8_t tx_data) {
    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Transmit(w25qxx_spi, &tx_data, sizeof(tx_data), 0xFFFFFFFF) != HAL_OK)
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               接收一字节数据
 * @param rx_data       需要接收数据的地址
 * @return              传输状态:   UTILS_Ok - 没有问题
 *                                  UTILS_ERROR - 有问题
 */
static UTILS_Status W25QXX_ReceiveByte(uint8_t* rx_data) {
    UTILS_Status status = UTILS_OK;
    if (HAL_SPI_Receive(w25qxx_spi, rx_data, 0x01, 0xFFFF) != HAL_OK)
        status = UTILS_ERROR;
    return status;
}

/*
 * @brief               传输和接收一字节数据
 * @param tx_data       需要传输的数据
 * @param rx_data       需要接收数据的地址
 * @return              传输状态:   UTILS_Ok - 没有问题
 *                                  UTILS_ERROR - 有问题
 */ 
static UTILS_Status W25QXX_TransmitReceiveByte(uint8_t tx_data, uint8_t* rx_data) {
    UTILS_Status status = UTILS_OK;
    status = W25QXX_TransmitByte(tx_data);
    status = W25QXX_ReceiveByte(rx_data);
    return status;
}

/*
 * @brief               初始化FLASH
 * @param hspi          用来传输的spi结构体
 * @return              无
 */
void W25QXX_Init(SPI_HandleTypeDef* hspi) {

    // 时钟使能
    UTILS_RCC_GPIO_Enable(W25QXX_CS_GPIO_TYPE);
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    HAL_GPIO_WritePin(W25QXX_CS_GPIO_TYPE, W25QXX_CS_GPIO_PIN, GPIO_PIN_SET);

    // 配置GPIO
    GPIO_InitStruct.Pin = W25QXX_CS_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(W25QXX_CS_GPIO_TYPE, &GPIO_InitStruct);
    
    //------------------------------获取指定spi的芯片信息------------------------------
    w25qxx_spi = hspi;
    W25QXX_TYPE = W25QXX_ReadID();
}


/*
 * @brief               读状态寄存器
 * @return              状态寄存器的数据
 * @attention           W25QXX的状态寄存器如下
 *  BIT7    6   5   4   3   2   1   0
 *  SPR     RV  TB BP2 BP1 BP0 WEL BUSY
 *  SPR: 默认为0, 状态寄存器保护位,配合WP使用
 *  TB, BP2, BP1, BP0: FLASH 区域写保护设置
 *  BUSY: 忙标志位
 *  默认: 0x00
 */
uint8_t W25QXX_ReadSR(void) {
    uint8_t byte = 0;
    W25QXX_CS_Enable();                 // 使能器件
    W25QXX_TransmitReceiveByte(W25X_ReadStatusReg, &byte);
    W25QXX_CS_Disable();                // 取消片选
    return byte;
}

/*
 * @brief               写W25QXX状态寄存器
 * @param byte          需要写入的数据
 * @return              无
 */
void W25QXX_WriteSR(uint8_t byte) {
    W25QXX_CS_Enable(); 
    W25QXX_TransmitByte(W25X_WriteStatusReg);
    W25QXX_TransmitByte(byte);
    W25QXX_CS_Disable();
}


/*
 * @brief               写使能
 * @return              无
 */
void W25QXX_WriteEnable(void) {
    W25QXX_CS_Enable(); 
    W25QXX_TransmitByte(W25X_WriteEnable);
    W25QXX_CS_Disable();
}

/*
 * @brief               写禁止
 * @return              无
 */
void W25QXX_WriteDisable(void) {
    W25QXX_CS_Enable(); 
    W25QXX_TransmitByte(W25X_WriteDisable);
    W25QXX_CS_Disable();
}

/*
 * @brief               阅读芯片ID
 * @return              芯片的ID号
 * @attention           芯片的ID号如下所示
 *                      0xEF13: W25Q80
 *                      0XEF13: W25Q80  
 *                      0XEF14: W25Q16  
 *                      0XEF15: W25Q32  
 *                      0XEF16: W25Q64  
 *                      0XEF17: W25Q128 
 */
uint16_t W25QXX_ReadID(void) {
    uint8_t rx_data;
    uint16_t tmp = 0;
    W25QXX_CS_Enable(); 
    W25QXX_TransmitByte(W25X_ManufactDeviceID);
    W25QXX_TransmitByte(0x00);
    W25QXX_TransmitByte(0x00);
    W25QXX_TransmitByte(0x00);
    W25QXX_ReceiveByte(&rx_data);
    tmp |= (rx_data << 8);
    W25QXX_ReceiveByte(&rx_data);
    tmp |= rx_data;
    W25QXX_CS_Disable();
    return tmp;
}

/*
 * @brief               读取flash数据
 * @param p_buffer      缓冲区地址
 * @param read_addr     读取的地址
 * @param num_byte_to_read  
 *                      读取的字节数
 * @return              无
 */
void W25QXX_Read(uint8_t* p_buffer, uint32_t read_addr, uint16_t num_byte_to_read) {
    W25QXX_CS_Enable();                     // 使能器件
    W25QXX_TransmitByte(W25X_ReadData);     // 发送读取指令
    // 发送24bit地址
    W25QXX_TransmitByte((uint8_t)((read_addr >> 16) & 0xFF));
    W25QXX_TransmitByte((uint8_t)((read_addr >> 8)) & 0xFF);
    W25QXX_TransmitByte((uint8_t)((read_addr)) & 0xFF);
    for (uint16_t i = 0; i < num_byte_to_read; ++i)
        W25QXX_ReceiveByte(&p_buffer[i]);
    
    W25QXX_CS_Disable();                    // 禁止器件
}

/*
 * @brief               写数据
 * @param p_buffer      需要写入的数据
 * @param write_addr    需要写入的地址
 * @param num_byte_to_write
 *                      需要写入数据数量
 * @return              无
 */
void W25QXX_WritePage(uint8_t* p_buffer, uint32_t write_addr, uint16_t num_byte_to_write) {
    W25QXX_WriteEnable();                   // 写使能
    W25QXX_CS_Enable();                     // 使能器件
    W25QXX_TransmitByte(W25X_PageProgram);  // 发送写命令
    W25QXX_TransmitByte((uint8_t)((write_addr >> 16) & 0xFF));
    W25QXX_TransmitByte((uint8_t)((write_addr >> 8)) & 0xFF);
    W25QXX_TransmitByte((uint8_t)((write_addr)) & 0xFF);
    for (uint16_t i = 0; i < num_byte_to_write; ++i)
        W25QXX_TransmitByte(p_buffer[i]);
    W25QXX_CS_Disable();                    // 禁止器件
    W25QXX_WriteDisable();
    W25QXX_WaitBusy();                     // 等待写入结束
}

/*
 * @brief               无须检验写spi flash
 * @param p_buffer      存储数据的首地址
 * @param write_addr    需要写入的flash的地址(24bit)
 * @param num_byte_to_write
 *                      需要写入的字节数(最大65535)
 * @return              无
 */
void W25QXX_WriteNoCheck(uint8_t* p_buffer, uint32_t write_addr, uint16_t num_byte_to_write) {
    // 单页剩余的字节数
    uint16_t page_remain = 256 - write_addr % 256; 
    if (num_byte_to_write <= page_remain)   
        page_remain = num_byte_to_write;
    while (1) {
        W25QXX_WritePage(p_buffer, write_addr, page_remain);
        if (num_byte_to_write == page_remain)       // 写入结束
            break;
        else {
            p_buffer += page_remain;                // 指针移动
            write_addr += page_remain;
            num_byte_to_write -= page_remain;
            if (num_byte_to_write > 256)
                page_remain = 256;
            else
                page_remain = num_byte_to_write;
        }
    }
}

/*
 * @brief               写flash
 * @param p_buffer      需要写入的数据起始地址
 * @param write_addr    需要写入的地址(24bit)
 * @param num_byte_to_write 
 *                      需要写入的字节数(最大65535)
 * return               无
 */
void W25QXX_Write(uint8_t* p_buffer, uint32_t write_addr, uint16_t num_byte_to_write) {
    uint8_t* w25qxx_buf = w25qxx_buffer;                // 暂存区
    uint32_t sec_pos = write_addr / 4096;               // 扇区位置
    uint16_t sec_offset = write_addr % 4096;            // 扇区内偏移
    uint16_t sec_remain = 4096 - sec_offset;            // 扇区剩余的空间
    uint16_t i;

     
    if (num_byte_to_write <= sec_remain)                // 要写入的字节数量小于扇区中剩余的容量
        sec_remain = num_byte_to_write;
    while (1) {
        W25QXX_Read(w25qxx_buf, sec_pos * 4096, 4096);  // 读取整个扇区的内容
        for (i = 0; i < sec_remain; ++i) {
            if (w25qxx_buf[sec_offset + i] != 0XFF)     // 需要进行擦除
                break;
        }
        if (i < sec_remain) {                           // 需要进行擦除
            W25QXX_EraseSector(sec_pos);                // 擦除并写入整个扇区数据
            for (i = 0; i < sec_remain; ++i)
                w25qxx_buf[sec_offset + i] = p_buffer[i];
            W25QXX_WriteNoCheck(w25qxx_buf, sec_pos * 4096, 4096);
        }
        else 
            W25QXX_WriteNoCheck(p_buffer, write_addr, sec_remain);

        if (num_byte_to_write == sec_remain)            // 写入结束
            break;
        else {                                          // 写入未结束
            ++sec_pos;                                  // 扇区地址增1
            sec_offset = 0;                             // 偏移位置置0
            num_byte_to_write -= sec_remain;            // 字节数递减
            if (num_byte_to_write > 4096)
                sec_remain = 4096;
            else
                sec_remain = num_byte_to_write;
        }
    }
}

/*
 * @brief               擦除整个芯片(等待时间较长)
 * @return              无
 */
void W25QXX_EraseChip(void) {
    W25QXX_WriteEnable();
    W25QXX_WaitBusy();
    W25QXX_CS_Enable();
    W25QXX_TransmitByte(W25X_ChipErase);
    W25QXX_CS_Disable();
    W25QXX_WaitBusy();
}

/*
 * @brief               擦除一个扇区
 * @param dst_addr      目标擦除扇区地址
 * @return              无
 * @attention           擦除一个扇区的最少时间是: 150ms
 *                      注意这里的地址是扇区地址,而不是其他函数的字节地址
 */
void W25QXX_EraseSector(uint32_t dst_addr) {
    dst_addr *= 4096;   // 把扇区地址转换为扇区起始字节的地址
    W25QXX_WriteEnable();
    W25QXX_WaitBusy();
    W25QXX_CS_Enable();

    W25QXX_TransmitByte(W25X_SectorErase);
    W25QXX_TransmitByte((uint8_t)((dst_addr >> 16) & 0xFF));
    W25QXX_TransmitByte((uint8_t)((dst_addr >> 8)) & 0xFF);
    W25QXX_TransmitByte((uint8_t)((dst_addr)) & 0xFF);

    W25QXX_CS_Disable();
    W25QXX_WaitBusy();
}

/*
 * @brief               进入掉电模式
 * @return              无
 */
void W25QXX_PowerDown(void) {
    W25QXX_CS_Enable();
    W25QXX_TransmitByte(W25X_PowerDown);
    W25QXX_CS_Disable();
    UTILS_Delay_us(3);              // 等待TPD
}

/*
 * @brief               唤醒
 * @return              无
 */
void W25QXX_WakeUP(void) {
    W25QXX_CS_Enable();
    W25QXX_TransmitByte(W25X_ReleasePowerDown);
    W25QXX_CS_Disable();
    UTILS_Delay_us(3);
}

/*
 * @brief               等待busy清空
 * @return              无
 */
void W25QXX_WaitBusy(void) {
    // 等待BUSY位清空
    while((W25QXX_ReadSR() & 0x01) == 0x01);
}

