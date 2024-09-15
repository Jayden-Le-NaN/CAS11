#ifndef W25Q128_H_
#define W25Q128_H_

#include "utils.h"

// W25QXX 片选引脚
#define W25QXX_CS_GPIO_PIN      GPIO_PIN_14
#define W25QXX_CS_GPIO_TYPE     GPIOB

// W25X系列/Q系列芯片 ID
#define W25Q80                  0XEF13
#define W25Q16                  0XEF14
#define W25Q32                  0XEF15
#define W25Q64                  0XEF16
#define W25Q128                 0XEF17

#define NM25Q80 	            0X5213 	
#define NM25Q16 	            0X5214
#define NM25Q32 	            0X5215
#define NM25Q64 	            0X5216
#define NM25Q128	            0X5217
#define NM25Q256 	            0X5218

// 指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 


//------------------------------变量接口------------------------------
extern uint16_t W25QXX_TYPE;

//------------------------------函数接口------------------------------
#define W25QXX_CS_Enable()      HAL_GPIO_WritePin(W25QXX_CS_GPIO_TYPE, W25QXX_CS_GPIO_PIN, GPIO_PIN_RESET)
#define W25QXX_CS_Disable()     HAL_GPIO_WritePin(W25QXX_CS_GPIO_TYPE, W25QXX_CS_GPIO_PIN, GPIO_PIN_SET)

void W25QXX_Init(SPI_HandleTypeDef* hspi);
uint16_t W25QXX_ReadID(void);
uint8_t W25QXX_ReadSR(void);
void W25QXX_WriteSR(uint8_t byte);
void W25QXX_WriteEnable(void);
void W25QXX_WriteDisable(void);
void W25QXX_Read(uint8_t* p_buffer, uint32_t read_addr, uint16_t num_byte_to_read);
void W25QXX_WritePage(uint8_t* p_buffer, uint32_t write_addr, uint16_t num_byte_to_write);
void W25QXX_WriteNoCheck(uint8_t* p_buffer, uint32_t write_addr, uint16_t num_byte_to_write);
void W25QXX_Write(uint8_t* p_buffer, uint32_t write_addr, uint16_t num_byte_to_write);
void W25QXX_EraseChip(void);
void W25QXX_EraseSector(uint32_t dst_addr);
void W25QXX_PowerDown(void);
void W25QXX_WakeUP(void);
void W25QXX_WaitBusy(void);

#endif
