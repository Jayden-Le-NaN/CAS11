#include "sstv.h"
#include "photo_cat.h"

#define       _SSTV_SET_ARR(arr)        sstv_tim->Instance->ARR = (arr);
#define       _SSTV_SET_PSC(psc)        sstv_tim->Instance->PSC = (psc);
#define       _LOAD_TIM_REG             do{sstv_tim->Instance->EGR = TIM_EGR_UG; if (HAL_IS_BIT_SET(sstv_tim->Instance->SR, TIM_FLAG_UPDATE)){CLEAR_BIT(sstv_tim->Instance->SR, TIM_FLAG_UPDATE);}}while(0)
#define       _DMA_CLEAR_TC_FLAG(hdma)  (hdma)->DmaBaseAddress->IFCR = (DMA_ISR_TCIF1 << ((hdma)->ChannelIndex & 0x1CU));
#define       _DMA_CLEAR_HT_FLAG(hdma)  (hdma)->DmaBaseAddress->IFCR = DMA_ISR_HTIF1 << ((hdma)->ChannelIndex & 0x1CU);

extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim2;
extern Time_Calculator time_calculator_obj;

SPI_HandleTypeDef* sstv_tim_dma_spi = &hspi2;
SSTV_Info_Struct sstv_info;

TIM_HandleTypeDef* sstv_tim = &htim2;
uint16_t test_point[20];
uint16_t testflag[20];
uint8_t isbusy_flag;
uint16_t sstv_tx_buffer[1056];


static void gen_test_signal(uint16_t* arr, uint16_t len){
  uint16_t i = 0;
  uint16_t temp = 0;
  uint16_t temp2 = 0;
  uint16_t len_6 = len / 6;
  for(i=0;i<len;i++){
    temp = i/3;
    temp2 = temp/len_6;
    if(i == temp*3){
        arr[i] = AD9833_REG_B28;
    }else if(i == temp*3+1){
      if(len_6/2 < temp-temp2*len_6 && temp < len_6*2){
          arr[i] = 0x5936;
      }else{
          arr[i] = 0x7eea;
      }
    }else{
      if(len_6/2 < temp-temp2*len_6 && temp < len_6*2){
          arr[i] = 0x4001;
      }else{
          arr[i] = 0x4000;
      }
    }
    printf("%x ", arr[i]);
    if(i == temp*3+2){
      printf("\r\n");
    }
  }

  for(i = 0;i<20;i++){
    test_point[i] = 0;
    testflag[i] = 0;
  }
  test_point[19] = 0;
}



UTILS_Status gen_flash_data(GD5F2GM7_Info_Struct *gd5f2gm7_obj){
  if(1){// if(sstv_info.sstv_mode->sstv_mode == SCT1){
    uint16_t gd5_RA = 0;
    uint16_t gd5_RA_init = 0;


    uint16_t len = sstv_info.sstv_mode->sstv_dma_line_length;
    uint16_t i = 0;
    uint16_t temp = 0;
    uint16_t temp2 = 0;
    uint16_t len_6 = len / 6;

    

    if(GD5F2GM7_WriteEnable(gd5f2gm7_obj) != UTILS_OK){
      printf("GD5F2GM7_WriteEnable error\r\n");
      return UTILS_ERROR;
    };

    GD5F2GM7_Set_Features(gd5f2gm7_obj, 0xA0, 0x00);    // disable block protection
    GD5F2GM7_Set_Features(gd5f2gm7_obj, 0xB0, 0x10);    // enable ECC
    // GD52GM7_BlockErase(gd5f2gm7_obj, 64);
// #define BLOCK_ERASE_1    
#ifdef BLOCK_ERASE_1
    
    for(uint16_t i = 0; i < 256*3; i+=64){
      GD52GM7_BlockErase(gd5f2gm7_obj, i);
      HAL_Delay(20);
    }
    
    for(uint16_t i = sstv_info.sstv_mode->sstv_dma_line_cnt * sstv_info.sstv_mode->loop_num-1; i < 1056; i++){
      sstv_tx_buffer[i] = 0xFFFF;
    }
    for (gd5_RA = gd5_RA_init; gd5_RA < gd5_RA_init + sstv_info.sstv_mode->sstv_dma_line_cnt * sstv_info.sstv_mode->loop_num; gd5_RA++){//sstv_info.sstv_mode->sstv_dma_line_cnt * sstv_info.sstv_mode->loop_num
      if(gd5_RA/3 < 80){
        for(i=0;i<len;i++){
          temp = i/3;
          temp2 = temp/len_6;
          if(i == temp*3){
              sstv_tx_buffer[i] = AD9833_REG_B28;
          }else if(i == temp*3+1){
            if(len_6/2 < temp-temp2*len_6 && temp < len_6*2){
                sstv_tx_buffer[i] = 0x5936;
            }else{
                sstv_tx_buffer[i] = 0x7eea;
            }
          }else{
            if(len_6/2 < temp-temp2*len_6 && temp < len_6*2){
                sstv_tx_buffer[i] = 0x4001;
            }else{
                sstv_tx_buffer[i] = 0x4000;
            }
          }
          // printf("%x ", sstv_tx_buffer[i]);
          // if(i == temp*3+2){
          //   printf("\r\n");
          // }
        }
        // printf("cal time:%d\r\n", Calculate_ElapsedTime(start, end));
        if(GD5F2GM7_ProgramLoad(gd5f2gm7_obj, 0, (uint8_t*)sstv_tx_buffer, 2112, UTILS_LOOP) != UTILS_OK){
          printf("GD5F2GM7_ProgramLoad error\r\n");
          return UTILS_ERROR;
        };
        HAL_Delay(1);
        
        if(GD5F2GM7_ProgramExecute(gd5f2gm7_obj, gd5_RA) != UTILS_OK){
          printf("GD5F2GM7_ProgramExecute error\r\n");
          return UTILS_ERROR;
        };
        HAL_Delay(1);
        // uint8_t isbusy = 0xff;
        // GD5F2GM7_isBusy(gd5f2gm7_obj, &isbusy);
        // if(isbusy){
        //   printf("busy\r\n");
        // }
        
      }else if(gd5_RA/3 <160){
        for(i=0;i<len;i++){
          temp = i/3;
          temp2 = temp/len_6;
          if(i == temp*3){
              sstv_tx_buffer[i] = AD9833_REG_B28;
          }else if(i == temp*3+1){
            if(len_6/2 < temp-temp2*len_6 && temp < len_6*2){
                sstv_tx_buffer[i] = 0x7eea;
            }else{
                sstv_tx_buffer[i] = 0x5936;
            }
          }else{
            if(len_6/2 < temp-temp2*len_6 && temp < len_6*2){
                sstv_tx_buffer[i] = 0x4000;
            }else{
                sstv_tx_buffer[i] = 0x4001;
            }
          }
          // printf("%x ", sstv_tx_buffer[i]);
          // if(i == temp*3+2){
          //   printf("\r\n");
          // }
        }
        // printf("cal time:%d\r\n", Calculate_ElapsedTime(start, end));
        if(GD5F2GM7_ProgramLoad(gd5f2gm7_obj, 0, (uint8_t*)sstv_tx_buffer, 2112, UTILS_LOOP) != UTILS_OK){
          printf("GD5F2GM7_ProgramLoad error\r\n");
          return UTILS_ERROR;
        };
        HAL_Delay(1);
        if(GD5F2GM7_ProgramExecute(gd5f2gm7_obj, gd5_RA) != UTILS_OK){
          printf("GD5F2GM7_ProgramExecute error\r\n");
          return UTILS_ERROR;
        };
        HAL_Delay(1);
      }else{
        for(i=0;i<len;i++){
          temp = i/3;
          temp2 = temp/len_6;
          if(i == temp*3){
              sstv_tx_buffer[i] = AD9833_REG_B28;
          }else if(i == temp*3+1){
            if(len_6/2 < temp-temp2*len_6 && temp < len_6*2){
                sstv_tx_buffer[i] = 0x5936;
            }else{
                sstv_tx_buffer[i] = 0x7eea;
            }
          }else{
            if(len_6/2 < temp-temp2*len_6 && temp < len_6*2){
                sstv_tx_buffer[i] = 0x4001;
            }else{
                sstv_tx_buffer[i] = 0x4000;
            }
          }
          // printf("%x ", sstv_tx_buffer[i]);
          // if(i == temp*3+2){
          //   printf("\r\n");
          // }
        }
      
        // printf("cal time:%d\r\n", Calculate_ElapsedTime(start, end));
        if(GD5F2GM7_ProgramLoad(gd5f2gm7_obj, 0, (uint8_t*)sstv_tx_buffer, 2112, UTILS_LOOP) != UTILS_OK){
          printf("GD5F2GM7_ProgramLoad error\r\n");
          return UTILS_ERROR;
        };
        HAL_Delay(1);
        if(GD5F2GM7_ProgramExecute(gd5f2gm7_obj, gd5_RA) != UTILS_OK){
          printf("GD5F2GM7_ProgramExecute error\r\n");
          return UTILS_ERROR;
        };
        HAL_Delay(1);
      }
    }
    for(uint16_t i = 0;i<1056;i++){
      printf("%x ", sstv_tx_buffer[i]);
    }
    printf("\r\n");
#endif

// #define BLOCK_ERASE_2
#ifdef BLOCK_ERASE_2
  for(uint16_t i = gd5_RA_init; i < gd5_RA_init+256*3; i+=64){
    GD52GM7_BlockErase(gd5f2gm7_obj, i);
    HAL_Delay(20);
  }
  for(gd5_RA = gd5_RA_init; gd5_RA < gd5_RA_init + sstv_info.sstv_mode->sstv_dma_line_cnt * 3; gd5_RA++){//sstv_info.sstv_mode->sstv_dma_line_cnt * 3
    temp2 = (gd5_RA-gd5_RA_init)/3;
    if((gd5_RA-gd5_RA_init) == temp2*3){
      // G line
      printf("G\r\n");
      for(i=0;i<len;i++){
        temp = i/3;
        // printf("%d\r\n", (gd5_RA-gd5_RA_init)*sstv_info.sstv_mode->sstv_dma_line_length/9 + temp);
        if(i == temp*3){
          sstv_tx_buffer[i] = AD9833_REG_B28;
        }else if(i == temp*3+1){
          sstv_tx_buffer[i] = AD9833_REG_FREQ0;
          uint16_t raw_frq = (uint16_t)(1500 + ((float)G1[(gd5_RA-gd5_RA_init)*sstv_info.sstv_mode->sstv_dma_line_length/9 + temp] * 3.1372549));
          AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &raw_frq, NULL, &sstv_tx_buffer[i]);
        }else{
          sstv_tx_buffer[i] = AD9833_REG_FREQ0;
          uint16_t raw_frq = (uint16_t)(1500 + ((float)G1[(gd5_RA-gd5_RA_init)*sstv_info.sstv_mode->sstv_dma_line_length/9 + temp] * 3.1372549));
          AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &raw_frq, &sstv_tx_buffer[i], NULL);
        }
      }
      if(GD5F2GM7_ProgramLoad(gd5f2gm7_obj, 0, (uint8_t*)sstv_tx_buffer, 2112, UTILS_LOOP) != UTILS_OK){
        printf("GD5F2GM7_ProgramLoad error\r\n");
        return UTILS_ERROR;
      };
      HAL_Delay(1);
      
      if(GD5F2GM7_ProgramExecute(gd5f2gm7_obj, gd5_RA) != UTILS_OK){
        printf("GD5F2GM7_ProgramExecute error\r\n");
        return UTILS_ERROR;
      };
      HAL_Delay(1);
    }else if((gd5_RA-gd5_RA_init) == temp2*3+1){
      // B line
      printf("B\r\n");
      for(i=0;i<len;i++){
        temp = i/3;
        if(i == temp*3){
          sstv_tx_buffer[i] = AD9833_REG_B28;
        }else if(i == temp*3+1){
          sstv_tx_buffer[i] = AD9833_REG_FREQ0;
          uint16_t raw_frq = (uint16_t)(1500 + ((float)B1[(gd5_RA-gd5_RA_init)*sstv_info.sstv_mode->sstv_dma_line_length/9 + temp] * 3.1372549));
          AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &raw_frq, NULL, &sstv_tx_buffer[i]);
        }else{
          sstv_tx_buffer[i] = AD9833_REG_FREQ0;
          uint16_t raw_frq = (uint16_t)(1500 + ((float)B1[(gd5_RA-gd5_RA_init)*sstv_info.sstv_mode->sstv_dma_line_length/9 + temp] * 3.1372549));
          AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &raw_frq, &sstv_tx_buffer[i], NULL);
        }
      }
      if(GD5F2GM7_ProgramLoad(gd5f2gm7_obj, 0, (uint8_t*)sstv_tx_buffer, 2112, UTILS_LOOP) != UTILS_OK){
        printf("GD5F2GM7_ProgramLoad error\r\n");
        return UTILS_ERROR;
      };
      HAL_Delay(1);
      
      if(GD5F2GM7_ProgramExecute(gd5f2gm7_obj, gd5_RA) != UTILS_OK){
        printf("GD5F2GM7_ProgramExecute error\r\n");
        return UTILS_ERROR;
      };
      HAL_Delay(1);
    }else{
      // R line
      printf("R\r\n");
      for(i=0;i<len;i++){
        temp = i/3;
        if(i == temp*3){
          sstv_tx_buffer[i] = AD9833_REG_B28;
        }else if(i == temp*3+1){
          sstv_tx_buffer[i] = AD9833_REG_FREQ0;
          uint16_t raw_frq = (uint16_t)(1500 + ((float)R1[(gd5_RA-gd5_RA_init)*sstv_info.sstv_mode->sstv_dma_line_length/9 + temp] * 3.1372549));
          AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &raw_frq, NULL, &sstv_tx_buffer[i]);
        }else{
          sstv_tx_buffer[i] = AD9833_REG_FREQ0;
          uint16_t raw_frq = (uint16_t)(1500 + ((float)R1[(gd5_RA-gd5_RA_init)*sstv_info.sstv_mode->sstv_dma_line_length/9 + temp] * 3.1372549));
          AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &raw_frq, &sstv_tx_buffer[i], NULL);
        }
      }
      if(GD5F2GM7_ProgramLoad(gd5f2gm7_obj, 0, (uint8_t*)sstv_tx_buffer, 2112, UTILS_LOOP) != UTILS_OK){
        printf("GD5F2GM7_ProgramLoad error\r\n");
        return UTILS_ERROR;
      };
      HAL_Delay(1);
      
      if(GD5F2GM7_ProgramExecute(gd5f2gm7_obj, gd5_RA) != UTILS_OK){
        printf("GD5F2GM7_ProgramExecute error\r\n");
        return UTILS_ERROR;
      };
      HAL_Delay(1);
    }
  }
    
#endif
    uint32_t page_address = 2;
    uint32_t data_address = 0;
    HAL_Delay(1);

    // if(GD5F2GM7_PageRead_ToCache(gd5f2gm7_obj, page_address) != UTILS_OK){
    //   printf("GD5F2GM7_PageRead_ToCache error\r\n");
    //   return UTILS_ERROR;
    // }
    // HAL_Delay(10);

    uint16_t test[1056];
    for(uint16_t i = 0;i<1056;i++){
      test[i] = 0xAA;
    }

    // GD5F2GM7_Set_Features(gd5f2gm7_obj, 0xB0, 0x10);

    // if(GD5F2GM7_ProgramLoad(gd5f2gm7_obj, data_address, test, 2112, UTILS_LOOP) != UTILS_OK){
    //   printf("GD5F2GM7_ProgramLoad error\r\n");
    //   return UTILS_ERROR;
    // };
    // HAL_Delay(1);
    // if(GD5F2GM7_WriteEnable(gd5f2gm7_obj) != UTILS_OK){
    //   printf("GD5F2GM7_WriteEnable error\r\n");
    //   return UTILS_ERROR;
    // };

    // GD5F2GM7_Set_Features(gd5f2gm7_obj, 0xA0, 0x00);

    // if(GD5F2GM7_ProgramExecute(gd5f2gm7_obj, page_address) != UTILS_OK){
    //   printf("GD5F2GM7_ProgramExecute error\r\n");
    //   return UTILS_ERROR;
    // };
    // HAL_Delay(1);
    uint8_t sstv_rx_buffer[4] = {0xaa, 0xaa, 0xaa, 0xaa};
    // 读一点数据并打印
  
    if(GD5F2GM7_PageRead_ToCache(gd5f2gm7_obj, page_address) != UTILS_OK){
      printf("GD5F2GM7_PageRead_ToCache error\r\n");
      return UTILS_ERROR;
    }
    HAL_Delay(1);
    GD5F2GM7_ReadFromCache(gd5f2gm7_obj, data_address, (uint8_t*)test, 2112, UTILS_DMA, GD5F2GM7_NOUSE_TIM);
    // GD5F2GM7_ReadFromCache(sstv_info.gd5f2gm7_obj, 0, (uint8_t*)sstv_info.tx_buffer_ptr, 1920, UTILS_DMA);
    HAL_Delay(1);
    // time_calculator_start(&time_calculator_obj);
    // GD5F2GM7_ReadFromCache(sstv_info.gd5f2gm7_obj, 960, (uint8_t*)(sstv_info.tx_buffer_ptr+480), 960, UTILS_DMA, GD5F2GM7_USE_TIM);   // 实测80MHz主频，spi分频256，耗时29ms
    // HAL_Delay(50);
    // printf("RA: %d\r\n", sstv_info._flash_RA);
    // GD5F2GM7_ReadFromCache(sstv_info.gd5f2gm7_obj, 960, (uint8_t*)(sstv_info.tx_buffer_ptr+480), 960, UTILS_DMA);
    // printf("line 10:%x\r\n", sstv_rx_buffer[1]);
    for(uint16_t i = 0;i<1056;i++){
      printf("%x ", test[i]);
    }
    // printf("\r\n");
    // printf("%d\r\n", sstv_info.sstv_mode->sstv_dma_line_length/2 -1);
    // for(uint16_t i = 0; i<1056; i++){//sstv_info.sstv_mode->sstv_dma_line_length
    //   printf("%x ", sstv_info.tx_buffer_ptr[i]);
    // }
    // printf("\r\n");
    // if(gd5f2gm7_obj->_dma_fsm_state_receive != GD5F2GM7_DMA_Idle){
    //   printf("DMA not IDLE\r\n");
    // }else{
    //   printf("DMA IDLE\r\n");
    // }


    // if(GD5F2GM7_PageRead_ToCache(gd5f2gm7_obj, 90U) != UTILS_OK){
    //   printf("GD5F2GM7_PageRead_ToCache error\r\n");
    //   return UTILS_ERROR;
    // }
    // UTILS_Delay_us(100);
    // GD5F2GM7_ReadFromCache(gd5f2gm7_obj, 100-1, (uint8_t*)(&sstv_rx_buffer), 2, UTILS_LOOP);
    // printf("line 90:%x\r\n", sstv_rx_buffer);


    // if(GD5F2GM7_PageRead_ToCache(gd5f2gm7_obj, 200U) != UTILS_OK){
    //   printf("GD5F2GM7_PageRead_ToCache error\r\n");
    //   return UTILS_ERROR;
    // }
    // UTILS_Delay_us(100);
    // GD5F2GM7_ReadFromCache(gd5f2gm7_obj, 100-1, (uint8_t*)(&sstv_rx_buffer), 2, UTILS_LOOP);
    // printf("line 200:%x\r\n", sstv_rx_buffer);

  }else{
    printf("sstv mode no data\r\n");
    return UTILS_ERROR;
  }
  return UTILS_OK;
}


/**
 * @brief  关联结构体，ad9833需要初始化fsync引脚和时钟速率，spi在SSTV_Transmit函数中初始化
 *
 * @param  sstv_mode_struct      SSTV mode structure
 * @param  ad9833_i             AD9833 I channel structure
 * @param  ad9833_q             AD9833 Q channel structure
 * @retval UTILS_OK             SSTV module initialized successfully
 * @retval UTILS_ERROR           SSTV module initialization failed
 *
 * @note   This function should be called before any other SSTV functions
 *         SSTV module can only be initialized in IDLE state
 *         AD9833 I and Q channel can only be initialized in IDLE state
 */
UTILS_Status SSTV_Init(SSTV_MODE_Struct* sstv_mode_struct, AD9833_Info_Struct *ad9833_i, AD9833_Info_Struct *ad9833_q, GD5F2GM7_Info_Struct *gd5f2gm7_obj)
{
  if((ad9833_i->_dma_fsm_state_transmit != AD9833_DMA_Idle) | (ad9833_q->_dma_fsm_state_transmit != AD9833_DMA_Idle)){
    return UTILS_ERROR;
  }
  sstv_info._sstv_fsm = SSTV_FSM_Idle;
  sstv_info._sstv_tx_state = SSTV_Idle;
  sstv_info.sstv_mode = sstv_mode_struct;
  sstv_info.AD9833_I = ad9833_i;
  sstv_info.AD9833_Q = ad9833_q;
  sstv_info._header_index = 0;
  sstv_info._loop_index = 0;
  sstv_info._pulse_porch_index = 0;
  sstv_info._line_sended = 0;
  sstv_info._flash_RA = 0;
  sstv_info.gd5f2gm7_obj = gd5f2gm7_obj;
  sstv_info.tx_buffer_ptr = sstv_tx_buffer;
  return UTILS_OK;
}

#define SSTV_USE_FLASH
UTILS_Status SSTV_Transmit(void){
  if((sstv_info._sstv_tx_state != SSTV_Idle) | (sstv_info.AD9833_I->_dma_fsm_state_transmit != AD9833_DMA_Idle) | (sstv_info.AD9833_Q->_dma_fsm_state_transmit != AD9833_DMA_Idle)){
    return UTILS_ERROR;
  }
  sstv_info._sstv_tx_state = SSTV_Transmitting;
  sstv_info._sstv_fsm = SSTV_FSM_Header;
  //get buffer
  uint16_t tx_buffer[sstv_info.sstv_mode->sstv_dma_line_length];
#ifndef SSTV_USE_FLASH
  gen_test_signal(sstv_tx_buffer, sstv_info.sstv_mode->sstv_dma_line_length);
#endif
  sstv_info.tx_buffer_ptr = sstv_tx_buffer;
  //initiate
  AD9833_Init_Tx_DMA_TIM(sstv_info.AD9833_I, sstv_info.AD9833_Q, sstv_tim_dma_spi, sstv_tim);

  HAL_GPIO_WritePin(sstv_info.AD9833_I->fsync_pin_type, sstv_info.AD9833_I->fsync_pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(sstv_info.AD9833_Q->fsync_pin_type, sstv_info.AD9833_Q->fsync_pin, GPIO_PIN_RESET);
  HAL_Delay(1);

  // 上电后要复位dds
  uint16_t temp = AD9833_REG_CONTROL | AD9833_REG_RESET;
  // Write_Half_Word(&temp); // only for test
  SPI_Write_Half_Word(sstv_tim_dma_spi, &temp);
  HAL_Delay(1);
  
  // TODO: set i&q
  sstv_info.AD9833_I->spi->hdmatx->XferHalfCpltCallback = SSTV_DMA_HalfCplt_Callback;
  sstv_info.AD9833_I->spi->hdmatx->XferCpltCallback = SSTV_DMA_Cplt_Callback;

  // printf("%d, %d\r\n", sstv_info.sstv_mode->header_arr[sstv_info._header_index], sstv_info.sstv_mode->header_psc[sstv_info._header_index]);
#ifdef SSTV_USE_FLASH
  GD5F2GM7_PageRead_ToCache(sstv_info.gd5f2gm7_obj, sstv_info._flash_RA);
  HAL_Delay(1);
  GD5F2GM7_ReadFromCache(sstv_info.gd5f2gm7_obj, 0, (uint8_t*)(sstv_info.tx_buffer_ptr), 2112, UTILS_DMA, GD5F2GM7_NOUSE_TIM);
  HAL_Delay(1);
#endif
  // for(uint16_t i = 0; i<sstv_info.sstv_mode->sstv_dma_line_length; i++){
  //   printf("%x ", sstv_info.tx_buffer_ptr[i]);
  //   if(i == (i/3)*3+2){
  //     printf("\r\n");
  //   }
  // }
  printf("tx_buffer_ptr %x ", sstv_info.tx_buffer_ptr[0]);
  printf("\r\n");
  _SSTV_SET_ARR(sstv_info.sstv_mode->header_arr[sstv_info._header_index]);
  _SSTV_SET_PSC(sstv_info.sstv_mode->header_psc[sstv_info._header_index]);
  __HAL_TIM_URS_ENABLE(sstv_tim);    // 重要，否则_LOAD_TIM_REG会产生中断
  _LOAD_TIM_REG;
  
  uint16_t frqh = AD9833_REG_FREQ0;
  uint16_t frql = AD9833_REG_FREQ0;

  //  AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &sstv_info.sstv_mode->header_frq[sstv_info._header_index], &frqh, &frql);
  // uint16_t a = 2000;
  // AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &a, &frqh1, &frql1);    // only for test
  // a = 1000;
  // AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &a, &frqh2, &frql2);// only for test
  AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &sstv_info.sstv_mode->header_frq[0], &frqh, &frql);
  AD9833_Write_Whole_Frq(sstv_tim_dma_spi, &frqh, &frql);
  __HAL_TIM_ENABLE(sstv_tim);//enable timer，测试写时注释掉
  // while(1){// only for test
  //   AD9833_Write_Whole_Frq(sstv_tim_dma_spi, &frqh1, &frql1);  //在函数内重写了，改为低速IO操作
  //   HAL_Delay(1000);
  //   AD9833_Write_Whole_Frq(sstv_tim_dma_spi, &frqh2, &frql2);
  //   HAL_Delay(1000);
  // }
  
  // printf("%x, %x\r\n", frqh1, frql1);
  // printf("%x, %x\r\n", frqh2, frql2);
  // return UTILS_OK;      // only for test

  //printf("add\r\n");
  sstv_info._header_index += 1;

  //_SSTV_SET_ARR(sstv_info.sstv_mode->header_arr[sstv_info._header_index]);
  _SSTV_SET_PSC(sstv_info.sstv_mode->header_psc[sstv_info._header_index]);
  // sstv_info._header_index ++;
  return UTILS_OK;
}

void SSTV_TIM_Header_Callback(void){
  testflag[0] = 1;
  // printf("H\r\n");
  // printf("%d\r\n", sstv_info._header_index);

  uint16_t frqh = AD9833_REG_FREQ0;
  uint16_t frql = AD9833_REG_FREQ0;
  if(sstv_info._header_index < sstv_info.sstv_mode->header_num - 1){
    
    AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &sstv_info.sstv_mode->header_frq[sstv_info._header_index], &frqh, &frql);
    // printf("%x, %x\r\n", frqh, frql);
    AD9833_Write_Whole_Frq(sstv_tim_dma_spi, &frqh, &frql);
    _SSTV_SET_ARR(sstv_info.sstv_mode->header_arr[sstv_info._header_index]);
    _SSTV_SET_PSC(sstv_info.sstv_mode->header_psc[sstv_info._header_index+1]);
    sstv_info._header_index ++;
    //printf("%d\r\n", sstv_info._header_index-1);
    // if(sstv_info._header_index<2){
    //   printf("%d\r\n", sstv_tim->Instance->CNT);
    // }
    // test_point[sstv_info._header_index-3] = sstv_tim->Instance->CNT;
  }else if(sstv_info._header_index == sstv_info.sstv_mode->header_num - 1){
    //_SSTV_SET_ARR(sstv_info.sstv_mode->pulse_porch_arr_ptr[0][0]);
    AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &sstv_info.sstv_mode->header_frq[sstv_info._header_index-1], &frqh, &frql);
    AD9833_Write_Whole_Frq(sstv_tim_dma_spi, &frqh, &frql);
    _SSTV_SET_ARR(sstv_info.sstv_mode->header_arr[sstv_info._header_index]);
    _SSTV_SET_PSC(sstv_info.sstv_mode->pulse_porch_psc_ptr[0][0]);
    sstv_info._sstv_fsm = SSTV_FSM_Loop;
    sstv_info._pulse_porch_index ++;    //至此_header_index == header_num; _pulse_porch_index == 1

    // test_point[sstv_info.sstv_mode->header_num-2] = sstv_tim->Instance->CNT;
  }else{
    printf("tim header overflow");
  }
  
}

uint8_t flag_temp = 0;

void SSTV_TIM_Loop_Callback(void){
  testflag[1] = 1;
  // printf("L\r\n");

  // 从header到loop后通常会有pulse或porch，所以没考虑_pulse_porch_index为0的情况
  if(sstv_info._sstv_fsm == SSTV_FSM_Loop){
    uint16_t frqh = AD9833_REG_FREQ0;
    uint16_t frql = AD9833_REG_FREQ0;
    if(sstv_info._pulse_porch_index < sstv_info.sstv_mode->pulse_porch_num[sstv_info._loop_index]){
      // Loop to Loop
      
      test_point[15] = sstv_info.tx_buffer_ptr[320*3-1];
      _SSTV_SET_ARR(sstv_info.sstv_mode->pulse_porch_arr_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index-1]);
      _SSTV_SET_PSC(sstv_info.sstv_mode->pulse_porch_psc_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index]);

      
      AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &sstv_info.sstv_mode->pulse_porch_frq_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index-1], &frqh, &frql);
      // UTILS_Delay_us(100);
      
      AD9833_Write_Whole_Frq(sstv_tim_dma_spi, &frqh, &frql);

      // printf("a %d %d\r\n", sstv_info._pulse_porch_index, sstv_info.sstv_mode->pulse_porch_frq_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index-1]);
      // printf("a %d %x %x\r\n", sstv_info._loop_index, frqh, frql);
      test_point[17]++;
      // UTILS_Delay_us(100);
      // printf("1\r\n");
      sstv_info._pulse_porch_index ++;
    }else if(sstv_info._pulse_porch_index == sstv_info.sstv_mode->pulse_porch_num[sstv_info._loop_index]){
      // Loop to DMA
      
      _SSTV_SET_ARR(sstv_info.sstv_mode->pulse_porch_arr_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index-1] - sstv_info.sstv_mode->dma_arr*3);
      _SSTV_SET_PSC(sstv_info.sstv_mode->dma_psc);
      // TODO: 检测DMA传输是否完成
      test_point[16] = sstv_info.tx_buffer_ptr[320*3-1];
      AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &sstv_info.sstv_mode->pulse_porch_frq_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index-1], &frqh, &frql);
      AD9833_Write_Whole_Frq(sstv_tim_dma_spi, &frqh, &frql);
      
      sstv_info._sstv_fsm = SSTV_FSM_DMA;

      // printf("b %d %d\r\n", sstv_info._pulse_porch_index, sstv_info.sstv_mode->pulse_porch_frq_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index-1]);
      // printf("b %d %x %x\r\n", sstv_info._loop_index, frqh, frql);
      test_point[18]++;
      

      //重置_pulse_porch_index和 _loop_index++，进入DMA
      if(sstv_info._loop_index == sstv_info.sstv_mode->loop_num){
        sstv_info._loop_index = 0;
      }else{
        sstv_info._loop_index ++;
      }
      sstv_info._pulse_porch_index = 0;
      // printf("2\r\n");
      // UTILS_Delay_us(100);
      // if(1){//flag_temp == 0
      //   HAL_DMA_Start_IT(sstv_info.AD9833_I->spi->hdmatx, (uint32_t)sstv_info.tx_buffer_ptr, (uint32_t)&(sstv_info.AD9833_I->spi->Instance->DR), sstv_info.sstv_mode->sstv_dma_line_length);
      //   flag_temp = 1;
      // }else{
      //   __HAL_DMA_ENABLE(sstv_info.AD9833_I->spi->hdmatx);
      // }
      // HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);



      // TODO: 通用模式不能这么写
      // if(sstv_info._line_sended > 0){
      //   GD5F2GM7_isBusy(sstv_info.gd5f2gm7_obj, &isbusy_flag);
      //   sstv_info._flash_RA += 1;
      //   GD5F2GM7_PageRead_ToCache(sstv_info.gd5f2gm7_obj, sstv_info._flash_RA);
      //   printf("tc\r\n");
      // }
      
      
    }else{
      printf("pp index overflow");
    }
  }else if(sstv_info._sstv_fsm == SSTV_FSM_DMA){
    // DMA to DMA
    // __HAL_DMA_ENABLE(sstv_info.AD9833_I->spi->hdmatx);
    __HAL_TIM_URS_ENABLE(sstv_tim);
    _SSTV_SET_ARR(sstv_info.sstv_mode->dma_arr);
    __HAL_TIM_DISABLE_IT(sstv_tim, TIM_IT_UPDATE);
    _LOAD_TIM_REG;
    if(flag_temp == 0){//flag_temp == 0
      // uint32_t a = UTILS_GetSysTick();
      HAL_DMA_Start_IT(sstv_info.AD9833_I->spi->hdmatx, (uint32_t)sstv_info.tx_buffer_ptr, (uint32_t)&(sstv_info.AD9833_I->spi->Instance->DR), sstv_info.sstv_mode->sstv_dma_line_length);
      uint16_t test_pixel = sstv_info.tx_buffer_ptr[1];
      // printf("pixel val: %x\r\n", test_pixel);
      // uint32_t b = UTILS_GetSysTick();
      // printf("a: %d, b: %d\r\n", a, b);
      
      flag_temp = 1;
    }else{
      __HAL_DMA_ENABLE(sstv_info.AD9833_I->spi->hdmatx);
    }
    // printf("3\r\n");
    // printf("c\r\n");
  }else if(sstv_info._sstv_fsm == SSTV_FSM_END){
    // printf("SSTV END1\r\n");
    __HAL_TIM_URS_ENABLE(sstv_tim);
    __HAL_TIM_DISABLE_IT(sstv_tim, TIM_IT_UPDATE);
    _LOAD_TIM_REG;
    __HAL_TIM_DISABLE(sstv_tim);
    // printf("SSTV END2\r\n");
    uint16_t temp = AD9833_REG_CONTROL | AD9833_REG_RESET;
    SPI_Write_Half_Word(sstv_tim_dma_spi, &temp);
    sstv_info._sstv_fsm = SSTV_FSM_Idle;
    sstv_info._sstv_tx_state = SSTV_Idle;
    HAL_GPIO_WritePin(sstv_info.AD9833_I->fsync_pin_type, sstv_info.AD9833_I->fsync_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(sstv_info.AD9833_Q->fsync_pin_type, sstv_info.AD9833_Q->fsync_pin, GPIO_PIN_SET);

    #ifdef SSTV_TEST
    // for(uint16_t i = 0; i<sstv_info.sstv_mode->sstv_dma_line_length; i++){
    //   printf("%x ", sstv_info.tx_buffer_ptr[i]);
    //   if(i == (i/3)*3+2){
    //     printf("\r\n");
    //   }
    // }
    // printf("isbusyflag: %x\r\n", isbusy_flag);
    printf("max interval: %d us", time_calculator_obj.interval);
    printf("testpoint15: %x\r\n", test_point[13]);
    printf("testpoint15: %x\r\n", test_point[15]);
    printf("testpoint16: %x\r\n", test_point[16]);
    printf("RA: %d\r\n", sstv_info._flash_RA);
    // printf("CNT = %d\t\n", test_point[16]);
    // printf("%d %d\r\n", test_point[17], test_point[18]);
    printf("SSTV END3\r\n");
    #endif
    
  }else{
    printf("sstv loopcb err");
  }
}

void SSTV_DMA_Cplt_Callback(void){
  test_point[19] += 1;
  // printf("D %d\r\n", test_point[19]);
  sstv_info._line_sended += 1;
  if(sstv_info._line_sended < sstv_info.sstv_mode->sstv_dma_line_cnt * sstv_info.sstv_mode->loop_num){
    // 还没发完
    if(sstv_info._loop_index < sstv_info.sstv_mode->loop_num && sstv_info.sstv_mode->pulse_porch_num[sstv_info._loop_index] != 0){
      _SSTV_SET_ARR(sstv_info.sstv_mode->dma_arr * 3);        // 让最后一次dma传输时间完整
      __HAL_DMA_DISABLE(sstv_info.AD9833_I->spi->hdmatx);
      // _DMA_CLEAR_TC_FLAG(sstv_info.AD9833_I->spi->hdmatx);   // HAL_DMA_IRQHandler已经清了，这里清了似乎会导致SPI传输出问题，具体原理不太清楚
      __HAL_TIM_URS_ENABLE(sstv_tim);
      __HAL_TIM_ENABLE_IT(sstv_tim, TIM_IT_UPDATE);
      _LOAD_TIM_REG;
      _SSTV_SET_PSC(sstv_info.sstv_mode->pulse_porch_psc_ptr[sstv_info._loop_index][0])
      sstv_info._pulse_porch_index = 1;
      sstv_info._sstv_fsm = SSTV_FSM_Loop;
      // printf("a %d\r\n", sstv_info._loop_index);
    }else if(sstv_info._loop_index == sstv_info.sstv_mode->loop_num && sstv_info.sstv_mode->pulse_porch_num[0] != 0){
      // 从loop末尾到开头
      // printf("c %d\r\n", sstv_info._loop_index);
      _SSTV_SET_ARR(sstv_info.sstv_mode->dma_arr * 3);        // 让最后一次dma传输时间完整
      __HAL_DMA_DISABLE(sstv_info.AD9833_I->spi->hdmatx);
      // _DMA_CLEAR_TC_FLAG(sstv_info.AD9833_I->spi->hdmatx);    // HAL_DMA_IRQHandler已经清了，这里清了似乎会导致SPI传输出问题，具体原理不太清楚
      __HAL_TIM_URS_ENABLE(sstv_tim);
      __HAL_TIM_ENABLE_IT(sstv_tim, TIM_IT_UPDATE);
      _LOAD_TIM_REG;
      sstv_info._loop_index = 0;
      _SSTV_SET_PSC(sstv_info.sstv_mode->pulse_porch_psc_ptr[sstv_info._loop_index][0])
      sstv_info._pulse_porch_index = 1;
      sstv_info._sstv_fsm = SSTV_FSM_Loop;
      
    }else{
      // __HAL_DMA_DISABLE(sstv_info.AD9833_I->spi->hdmatx);
      // _DMA_CLEAR_TC_FLAG(sstv_info.AD9833_I->spi->hdmatx);    // HAL_DMA_IRQHandler已经清了，这里清了似乎会导致SPI传输出问题，具体原理不太清楚
      __HAL_DMA_ENABLE(sstv_info.AD9833_I->spi->hdmatx);      // 循环模式不用重新配置DMA，打开就能从头传
      printf("b %d\r\n", sstv_info._loop_index);
      if(sstv_info._loop_index == sstv_info.sstv_mode->loop_num){
        sstv_info._loop_index = 0;
      }else{
        sstv_info._loop_index ++;
      }
      //不用重新配置HAL_DMA_Start_IT(sstv_info.AD9833_I->spi->hdmatx, (uint32_t)sstv_info.tx_buffer_ptr, (uint32_t)&(sstv_info.AD9833_I->spi->Instance->DR), sstv_info.sstv_mode->sstv_dma_line_length);
    }
#ifdef SSTV_USE_FLASH
    // GD5F2GM7_isBusy(sstv_info.gd5f2gm7_obj, &isbusy_flag);
    GD5F2GM7_ReadFromCache(sstv_info.gd5f2gm7_obj, sstv_info.sstv_mode->sstv_dma_line_length, (uint8_t*)(sstv_info.tx_buffer_ptr+sstv_info.sstv_mode->sstv_dma_line_length/2), sstv_info.sstv_mode->sstv_dma_line_length, UTILS_DMA, GD5F2GM7_USE_TIM);
#endif
    // printf("fe\r\n");
  }else{
    // 发完了
    __HAL_DMA_DISABLE(sstv_info.AD9833_I->spi->hdmatx);
    _DMA_CLEAR_TC_FLAG(sstv_info.AD9833_I->spi->hdmatx);
    _SSTV_SET_ARR(sstv_info.sstv_mode->dma_arr * 3);        // 让最后一次dma传输时间完整
    __HAL_TIM_ENABLE_IT(sstv_tim, TIM_IT_UPDATE);
    _LOAD_TIM_REG;
    sstv_info._sstv_fsm = SSTV_FSM_END;
  }
  

  // TODO: 不是最后一行的话，写后半段数组
}

void SSTV_DMA_HalfCplt_Callback(void){
#ifdef SSTV_USE_FLASH
  // TODO: 不是最后一行的话，写前半段数组
  if(sstv_info._line_sended < sstv_info.sstv_mode->sstv_dma_line_cnt*sstv_info.sstv_mode->loop_num - 1){
    GD5F2GM7_ReadFromCache(sstv_info.gd5f2gm7_obj, 0, (uint8_t*)sstv_info.tx_buffer_ptr, sstv_info.sstv_mode->sstv_dma_line_length, UTILS_DMA, GD5F2GM7_NOUSE_TIM);
    // printf("fm\r\n");
  }
#endif
  return;
}