/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "at.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
GM5F2GM7XEXXG_Info_Struct gm5f2gm7xexxg_obj;
GD5F2GM7_Info_Struct gd5f2gm7_obj;  
PM004M_Info_Struct pm004m_obj;
LTC5589_Info_Struct ltc5589_obj;
AD9833_Info_Struct ad9833_obj;

// urc 表


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;
DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi2_tx;
DMA_HandleTypeDef hdma_spi3_rx;
DMA_HandleTypeDef hdma_spi3_tx;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* 接收相关ringbuffer ------------------------------------------------------------*/
ring_buffer_t uart1_rx_rb;
ring_buffer_t uart1_rx_event_rb;
uint8_t uart1_rx_rb_buffer[4096];
uint8_t uart1_rx_event_rb_buffer[4096];
uint8_t uart1_rx_buffer[256];

/* 发送相关ringbuffer ------------------------------------------------------------*/
ring_buffer_t uart1_tx_rb;
ring_buffer_t uart1_tx_event_rb;
uint8_t uart1_tx_rb_buffer[4096];
uint8_t uart1_tx_event_rb_buffer[4096];
uint8_t uart1_tx_buffer[256];

/* 串口1接收空闲中断处理 ------------------------------------------------------------*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    if (huart == &huart1) {
        ring_buffer_put(&uart1_rx_rb, uart1_rx_buffer, Size);       // 把数据放到环形缓冲区中
        ring_buffer_put(&uart1_rx_event_rb, (uint8_t*)&Size, 2);    // 存储事件的值到环形缓冲区中
        HAL_UARTEx_ReceiveToIdle_IT(&huart1, uart1_rx_buffer, sizeof(uart1_rx_buffer));
    }
}

uint32_t at_uart_read(void* buffer, uint32_t len) {
    return ring_buffer_get(&uart1_rx_rb, (uint8_t*)buffer, len);
}

uint32_t at_uart_write(const void* buffer, uint32_t len) {
    ring_buffer_put(&uart1_tx_event_rb, (uint8_t*)&len, 4);
    return ring_buffer_put(&uart1_tx_rb, (uint8_t*) buffer, len);
}

static uint8_t uart1_urcbuffer[256];        // urc 接收缓冲区
uint8_t uart1_recvbuffer[256];

void task1_handler(void* obj_t, struct at_obj* at, char* recvbuf, int32_t len);
void task2_handler(void* obj_t, struct at_obj* at, char* recvbuf, int32_t len);
void task3_handler(void* obj_t, struct at_obj* at, char* recvbuf, int32_t len);


static at_obj_t at;

static const urc_item_t urc_table[] = {     // urc 表
    {NULL, &at, "task1", task1_handler},
    {NULL, &at, "task2", task2_handler},
    {NULL, &at, "task3", task3_handler},
    {&ltc5589_obj, &at, "AT+5589", LTC5589_AT_Handler},
};

static const at_adapter_t at_adapter = {    // at 适配器
    .write = at_uart_write,
    .read = at_uart_read,
    .error = NULL,
    .urc_tbl = (urc_item_t*)urc_table,
    .urc_buf = uart1_urcbuffer,
    .recv_buf = uart1_recvbuffer,
    .urc_tbl_count = sizeof(urc_table) / sizeof(urc_table[0]),
    .urc_buf_size = sizeof(uart1_urcbuffer),
    .recv_buf_size = sizeof(uart1_recvbuffer)
};


void task1_handler(void* obj_t, struct at_obj* at, char* recvbuf, int32_t len) {
    char tx_buff[] = "Task1 Receive: ";
    at_send_data(at, (void*)tx_buff, sizeof(tx_buff));
    at_send_data(at, (void*)recvbuf, len);
}

void task2_handler(void* obj_t, struct at_obj* at, char* recvbuf, int32_t len) {
    uint8_t tx_buff[] = "\ntask2 receive: ";
    HAL_UART_Transmit(&huart1, tx_buff, sizeof(tx_buff), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t*)recvbuf, len, HAL_MAX_DELAY);
}

void task3_handler(void* obj_t, struct at_obj* at, char* recvbuf, int32_t len) {
    uint8_t tx_buff[] = "\ntask3 receive: ";
    HAL_UART_Transmit(&huart1, tx_buff, sizeof(tx_buff), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t*)recvbuf, len, HAL_MAX_DELAY);
}



void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    //------------------------------FLAH中断处理------------------------------
    // GD5F2GM7_Transmit_IRQ_Handler(&gd5f2gm7_obj, hspi);
    //------------------------------MRAM中断处理------------------------------
    // PM004M_Transmit_IRQ_Handler(&pm004m_obj, hspi);
    //------------------------------DDS中断处理------------------------------
    AD9833_Transmit_IRQ_Handler(&ad9833_obj, hspi);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    //------------------------------FLAH中断处理------------------------------
    GD5F2GM7_Receive_IRQ_Hanlder(&gd5f2gm7_obj, hspi);
    //------------------------------MRAM中断处理------------------------------
    PM004M_Receive_IRQ_Handler(&pm004m_obj, hspi);
}



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_SPI3_Init(void);
static void MX_DAC1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_DAC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);



  ring_buffer_init(&uart1_rx_rb, uart1_rx_rb_buffer, sizeof(uart1_rx_rb_buffer) / sizeof(uart1_rx_rb_buffer[0]));
  ring_buffer_init(&uart1_rx_event_rb, uart1_rx_event_rb_buffer, sizeof(uart1_rx_event_rb_buffer) / sizeof(uart1_rx_event_rb_buffer[0]));
  ring_buffer_init(&uart1_tx_rb, uart1_tx_buffer, sizeof(uart1_tx_rb_buffer) / sizeof(uart1_tx_rb_buffer[0]));
  ring_buffer_init(&uart1_tx_event_rb, uart1_tx_event_rb_buffer, sizeof(uart1_tx_event_rb_buffer) / sizeof(uart1_tx_event_rb_buffer[0]));

  HAL_UARTEx_ReceiveToIdle_IT(&huart1, uart1_rx_buffer, sizeof(uart1_rx_buffer) / sizeof(uart1_rx_buffer[0]));
  at_obj_init(&at, &at_adapter);


  uint32_t uart1_tx_data_size = 0;
  uint32_t uart1_rx_data_size = 0;

  //------------------------------MRAM驱动测试------------------------------
  // PM004M_Init(&pm004m_obj, &hspi1, GPIO_PIN_9, GPIOB);
  // PM004M_ModeRegisterWrite(&pm004m_obj, 0x0000, 0x80);
  // PM004M_ModeRegisterRead(&pm004m_obj, 0x0000, uart_buff);
    
  //------------------------------FLASH驱动测试------------------------------
  // uint8_t reg_status[256];
  // GD5F2GM7_Init(&gd5f2gm7_obj, &hspi1, GPIO_PIN_6, GPIOB);
  // GD5F2GM7_WriteEnable(&gd5f2gm7_obj);
  // GD5F2GM7_Get_Features(&gd5f2gm7_obj, 0xC0, reg_status);
  // HAL_UART_Transmit(&huart1, reg_status, 1, HAL_MAX_DELAY);
    

  //------------------------------本振ADF4252测试------------------------------
  // ADF4252_Info_Struct adf4252_obj;
  // ADF4252_Init(&adf4252_obj, &hspi2, GPIO_PIN_6, GPIOA);
  // (&adf4252_obj)->_val_rf_n_divider  = 0x7B0000;                
  // (&adf4252_obj)->_val_rf_r_divider  = 0x108009;
  // (&adf4252_obj)->_val_rf_control    = 0x88C2;
  // (&adf4252_obj)->_val_master        = 0x7C3;
  // (&adf4252_obj)->_val_if_n_divider  = 0x40A864;
  // (&adf4252_obj)->_val_if_r_divider  = 0x195;
  // (&adf4252_obj)->_val_if_control    = 0xA6;
  // ADF4252_Write_All_Registers(&adf4252_obj);
  // HAL_Delay(1000);

  //------------------------------生成DDS的直流电压------------------------------
  // HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 470);
  // HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);

  //------------------------------LTC5589驱动测试------------------------------

  // LTC5589_Init(&ltc5589_obj, &hspi3, GPIO_PIN_0, GPIOB, GPIO_PIN_1, GPIOB);
  // LTC5589_Set_Frequency(&ltc5589_obj, 0x40);
  
  // LTC5589_Set_DigitalGain_Coarse(&ltc5589_obj, 0);
  // LTC5589_Set_DCOffset(&ltc5589_obj, LTC5589_CHANNEL_I, 0x13);





    

  //------------------------------AD9833测试------------------------------
  // AD9833_Init(&ad9833_obj, &hspi3, GPIO_PIN_6, GPIOA, 25000000);
  // AD9833_SetWave(&ad9833_obj, AD9833_WAVE_SINUSOID);

  // uint32_t freq = 1000;
  // uint32_t freq_list[] = {1000, 10000, 100000, 10000};
  // AD9833_SetFrequency(&ad9833_obj, AD9833_REG_FREQ0, AD9833_FREQ_ALL, &freq, sizeof(freq), UTILS_LOOP);
  // AD9833_FrequencyOutSelect(&ad9833_obj, AD9833_OUT_FREQ0);
  // AD9833_SetWave(&ad9833_obj, AD9833_WAVE_UP_DOWN_RAMP);

    // 01_001000
    // 00110100

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // uint8_t i_dc_offset = 0x50;
  // uint8_t ltc_freq = 0x3d; 
  // uint8_t packet[256];
  // int8_t gain = -19;
  // uint8_t char_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  // uint8_t ratio = 1;

    // AD9833_WriteData(AD9833_RESET | AD9833_B28);            // 选择数据写入一次,B28位和RESET位为1
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    if (ring_buffer_get(&uart1_rx_event_rb, (uint8_t*)&uart1_rx_data_size, 4) != 0) {
        at_recv_task(&at, uart1_rx_data_size);
    }

    if (ring_buffer_get(&uart1_tx_event_rb, (uint8_t*)&uart1_tx_data_size, 4) != 0) {
        ring_buffer_get(&uart1_tx_rb, uart1_tx_buffer, uart1_tx_data_size);
        HAL_UART_Transmit(&huart1, uart1_tx_buffer, uart1_tx_data_size, HAL_MAX_DELAY);
    }
      /*
    AD9833_SetFrequency(&ad9833_obj, AD9833_REG_FREQ0, AD9833_FREQ_ALL, &freq_list[0], 1, UTILS_DMA);
    HAL_Delay(1000);
    AD9833_SetFrequency(&ad9833_obj, AD9833_REG_FREQ0, AD9833_FREQ_ALL, &freq_list[1], 1, UTILS_DMA);
    HAL_Delay(1000);
    AD9833_SetFrequency(&ad9833_obj, AD9833_REG_FREQ0, AD9833_FREQ_ALL, &freq_list[2], 1, UTILS_DMA);
    HAL_Delay(1000);
    AD9833_SetFrequency(&ad9833_obj, AD9833_REG_FREQ0, AD9833_FREQ_ALL, &freq_list[3], 1, UTILS_DMA);
    HAL_Delay(1000);
    */


    //------------------------------LTC5589测试------------------------------
    // LTC5589_Set_Frequency(&ltc5589_obj, ltc_freq);
    // LTC5589_Read_Register(&ltc5589_obj, 0x00, rx_buff);

    // LTC5589_Set_DigitalGain_Coarse(&ltc5589_obj, gain);

    /*
    LTC5589_Set_DCOffset(&ltc5589_obj, LTC5589_CHANNEL_I, i_dc_offset);
    packet[0] = char_map[(i_dc_offset & 0xF0) >> 4];
    packet[1] = char_map[(i_dc_offset & 0x0F)];
    packet[2] = '\n';
    i_dc_offset += 5;
    if (i_dc_offset >= 250) {
        i_dc_offset = 1;
    }
    HAL_UART_Transmit(&huart1, packet, 3, HAL_MAX_DELAY);
    HAL_Delay(1000);
    */

    /*
    LTC5589_Set_IQ_GainRatio(&ltc5589_obj, ratio);

    do {
        LTC5589_Set_DCOffset(&ltc5589_obj, LTC5589_CHANNEL_I, i_dc_offset);
        packet[0] = char_map[(i_dc_offset & 0xF0) >> 4];
        packet[1] = char_map[(i_dc_offset & 0x0F)];
        // packet[2] = '\n';
        packet[2] = ' ';
        packet[3] = char_map[(ratio & 0xF0) >> 4];
        packet[4] = char_map[(ratio & 0x0F)];
        packet[5] = '\n';
        HAL_UART_Transmit(&huart1, packet, 6, HAL_MAX_DELAY);

        i_dc_offset += 5;
        if (i_dc_offset >= 250) {
            i_dc_offset = 1;
            break;
        }
        
        packet[2] = ' ';
        // 扫增益
        packet[3] = '-';
        packet[4] = char_map[((-gain) & 0xF0) >> 4];
        packet[5] = char_map[((-gain) & 0x0F)];
        packet[6] = '\n';
        // 扫频
        // packet[3] = char_map[(ltc_freq & 0xF0) >> 4];
        // packet[4] = char_map[(ltc_freq & 0x0F)];
        // packet[5] = char_map[(rx_buff[0] & 0xF0) >> 4];
        // packet[6] = char_map[(rx_buff[0] & 0x0F)];
        // packet[7] = '\n';
        HAL_Delay(1000);
    } while(1);

    ratio += 5;
    if (ratio >= 250)
        ratio = 1;


    // ltc_freq += 1;
    // if (ltc_freq == 0x43)
    //     ltc_freq = 0x3d;

    // AD9833_SetFrequency(&ad9833_obj, AD9833_REG_FREQ0, AD9833_FREQ_NO_CHANGE, &freq, sizeof(freq), UTILS_LOOP);
    // AD9833_FrequencyOutSelect(&ad9833_obj, AD9833_OUT_FREQ0);
    HAL_Delay(1000);
    */

    //------------------------------LTC5589测试------------------------------
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 20;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 99;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA2_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel1_IRQn);
  /* DMA2_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_SET);

  /*Configure GPIO pins : PB6 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
