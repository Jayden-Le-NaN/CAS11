#include "sstv.h"
#include "stm32l4xx_hal_spi.h"

static SPI_HandleTypeDef sstv_spi;


void SSTV_GPIO_Init(void){
    
}

/**
  * @brief  Initialize the SPI2 according to the specified parameters
  *         in the SPI2_HandleTypeDef and initialize the associated handle.
  * @param  None
  * @retval None
  */
void SSTV_SPI2_Init(void)
{
  /* SPI2 parameter configuration*/
  sstv_spi.Instance = SPI2;
  sstv_spi.Init.Mode = SPI_MODE_MASTER;
  sstv_spi.Init.Direction = SPI_DIRECTION_1LINE;
  sstv_spi.Init.DataSize = SPI_DATASIZE_16BIT;
  sstv_spi.Init.CLKPolarity = SPI_POLARITY_LOW;
  sstv_spi.Init.CLKPhase = SPI_PHASE_2EDGE;
  sstv_spi.Init.NSS = SPI_NSS_SOFT;
  sstv_spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  sstv_spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  sstv_spi.Init.TIMode = SPI_TIMODE_DISABLE;
  sstv_spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  sstv_spi.Init.CRCPolynomial = 7;
  sstv_spi.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  sstv_spi.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&sstv_spi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}