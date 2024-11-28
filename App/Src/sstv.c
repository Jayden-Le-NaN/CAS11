#include "sstv.h"

#define       _SSTV_SET_ARR(arr)        sstv_tim.Instance->ARR = (arr);
#define       _SSTV_SET_PSC(psc)        sstv_tim.Instance->PSC = (psc);
#define       _LOAD_TIM_REG             do{sstv_tim.Instance->EGR = TIM_EGR_UG; if (HAL_IS_BIT_SET(sstv_tim.Instance->SR, TIM_FLAG_UPDATE)){CLEAR_BIT(sstv_tim.Instance->SR, TIM_FLAG_UPDATE);}}while(0)
#define       _DMA_CLEAR_TC_FLAG(hdma)  (hdma)->DmaBaseAddress->IFCR = (DMA_ISR_TCIF1 << ((hdma)->ChannelIndex & 0x1CU));
#define       _DMA_CLEAR_HT_FLAG(hdma)  (hdma)->DmaBaseAddress->IFCR = DMA_ISR_HTIF1 << ((hdma)->ChannelIndex & 0x1CU);

SSTV_Info_Struct sstv_info;
SPI_HandleTypeDef sstv_tim_dma_spi;
TIM_HandleTypeDef sstv_tim;
uint16_t test_point[20];
uint16_t testflag[20];


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
    printf("%x\r\n", arr[i]);
  }

  for(i = 0;i<20;i++){
    test_point[i] = 0xaaaa;
    testflag[i] = 0;
  }
  test_point[19] = 0;
}

/**
  * @brief  
  * @param  
  * @retval 
  */
UTILS_Status SSTV_Init(SSTV_MODE_Struct* sstv_mode_struct, AD9833_Info_Struct *ad9833_i, AD9833_Info_Struct *ad9833_q)
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
  // sstv_info->_sstv_dma_line_cnt = sstv_mode_struct->sstv_dma_line_cnt;
  // sstv_info->_sstv_dma_line_length = sstv_mode_struct->sstv_dma_line_length;
  // sstv_info->_sstv_dma_one_line = sstv_mode_struct->sstv_dma_one_line;
  // sstv_info->_header_arr = sstv_mode_struct->header_arr;
  // sstv_info->_header_psc = sstv_mode_struct->header_psc;
  // sstv_info->_header_frq = sstv_mode_struct->header_frq;
  // sstv_info->_header_num = sstv_mode_struct->header_num;

  // for(uint8_t i = 0;i<4;i++){
  //   sstv_info->_pulse_porch_arr_ptr[i] = sstv_mode_struct->pulse_porch_frq_ptr[i];
  //   sstv_info->_pulse_porch_psc_ptr[i] = sstv_mode_struct->pulse_porch_psc_ptr[i];
  //   sstv_info->_pulse_porch_frq_ptr[i] = sstv_mode_struct->pulse_porch_frq_ptr[i];
  //   sstv_info->_pulse_porch_num[i] = sstv_mode_struct->pulse_porch_num[i];
  // }

  return UTILS_OK;
}

UTILS_Status SSTV_Transmit(void){
  if((sstv_info._sstv_tx_state != SSTV_Idle) | (sstv_info.AD9833_I->_dma_fsm_state_transmit != AD9833_DMA_Idle) | (sstv_info.AD9833_Q->_dma_fsm_state_transmit != AD9833_DMA_Idle)){
    return UTILS_ERROR;
  }
  sstv_info._sstv_tx_state = SSTV_Transmitting;
  sstv_info._sstv_fsm = SSTV_FSM_Header;
  //get buffer
  uint16_t tx_buffer[sstv_info.sstv_mode->sstv_dma_line_length];
  gen_test_signal(tx_buffer, sstv_info.sstv_mode->sstv_dma_line_length);
  sstv_info.tx_buffer_ptr = tx_buffer;
  //initiate
  AD9833_Init_Tx_DMA_TIM(sstv_info.AD9833_I, sstv_info.AD9833_Q, &sstv_tim_dma_spi, &sstv_tim);

  HAL_GPIO_WritePin(sstv_info.AD9833_I->fsync_pin_type, sstv_info.AD9833_I->fsync_pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(sstv_info.AD9833_Q->fsync_pin_type, sstv_info.AD9833_Q->fsync_pin, GPIO_PIN_RESET);
  HAL_Delay(1);

  SPI_Write_Half_Word(&sstv_tim_dma_spi, AD9833_REG_CONTROL | AD9833_REG_RESET);
  
  // TODO: reset dds; set i&q
  sstv_info.AD9833_I->spi->hdmatx->XferHalfCpltCallback = SSTV_DMA_HalfCplt_Callback;
  sstv_info.AD9833_I->spi->hdmatx->XferCpltCallback = SSTV_DMA_Cplt_Callback;

  //printf("%d, %d\r\n", sstv_info.sstv_mode->header_arr[sstv_info._header_index], sstv_info.sstv_mode->header_psc[sstv_info._header_index]);
  _SSTV_SET_ARR(sstv_info.sstv_mode->header_arr[sstv_info._header_index]);
  _SSTV_SET_PSC(sstv_info.sstv_mode->header_psc[sstv_info._header_index]);
  __HAL_TIM_URS_ENABLE(&sstv_tim);    // 重要，否则_LOAD_TIM_REG会产生中断
  _LOAD_TIM_REG;
  
  uint16_t frqh = 0;
  uint16_t frql = 0;
  AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &sstv_info.sstv_mode->header_frq[sstv_info._header_index], &frqh, &frql);

  __HAL_TIM_ENABLE(&sstv_tim);//enable timer
  AD9833_Write_Whole_Frq(&sstv_tim_dma_spi, &frqh, &frql);
  ////printf("add\r\n");
  sstv_info._header_index += 1;

  //_SSTV_SET_ARR(sstv_info.sstv_mode->header_arr[sstv_info._header_index]);
  _SSTV_SET_PSC(sstv_info.sstv_mode->header_psc[sstv_info._header_index]);
  //_LOAD_TIM_REG;
  sstv_info._header_index ++;
  return UTILS_OK;
}

void SSTV_TIM_Header_Callback(void){
  testflag[0] = 1;
  //printf("H\r\n");

  uint16_t frqh = 0;
  uint16_t frql = 0;
  if(sstv_info._header_index < sstv_info.sstv_mode->header_num){
    
    AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &sstv_info.sstv_mode->header_frq[sstv_info._header_index-1], &frqh, &frql);
    ////printf("%d, %d\r\n", frqh, frql);
    AD9833_Write_Whole_Frq(&sstv_tim_dma_spi, &frqh, &frql);
    _SSTV_SET_ARR(sstv_info.sstv_mode->header_arr[sstv_info._header_index-1]);
    _SSTV_SET_PSC(sstv_info.sstv_mode->header_psc[sstv_info._header_index]);
    sstv_info._header_index ++;
    //printf("%d\r\n", sstv_info._header_index-1);
    if(sstv_info._header_index<2){
      //printf("%d\r\n", sstv_tim.Instance->CNT);
    }
    test_point[sstv_info._header_index-3] = sstv_tim.Instance->CNT;
  }else if(sstv_info._header_index == sstv_info.sstv_mode->header_num ){
    //_SSTV_SET_ARR(sstv_info.sstv_mode->pulse_porch_arr_ptr[0][0]);
    AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &sstv_info.sstv_mode->header_frq[sstv_info._header_index-1], &frqh, &frql);
    AD9833_Write_Whole_Frq(&sstv_tim_dma_spi, &frqh, &frql);
    _SSTV_SET_ARR(sstv_info.sstv_mode->header_arr[sstv_info._header_index-1]);
    _SSTV_SET_PSC(sstv_info.sstv_mode->pulse_porch_psc_ptr[0][0]);
    sstv_info._sstv_fsm = SSTV_FSM_Loop;
    sstv_info._pulse_porch_index ++;    //至此_header_index == header_num; _pulse_porch_index == 1

    test_point[sstv_info.sstv_mode->header_num-2] = sstv_tim.Instance->CNT;
  }else{
    printf("tim header overflow");
  }
  
}

void SSTV_TIM_Loop_Callback(void){
  testflag[1] = 1;
  //printf("L\r\n");

  // 从header到loop后通常会有pulse或porch，所以没考虑_pulse_porch_index为0的情况
  if(sstv_info._sstv_fsm == SSTV_FSM_Loop){
    uint16_t frqh = 0;
    uint16_t frql = 0;
    if(sstv_info._pulse_porch_index < sstv_info.sstv_mode->pulse_porch_num[sstv_info._loop_index]){
      AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &sstv_info.sstv_mode->pulse_porch_frq_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index-1], &frqh, &frql);
      AD9833_Write_Whole_Frq(&sstv_tim_dma_spi, &frqh, &frql);
      _SSTV_SET_ARR(sstv_info.sstv_mode->pulse_porch_arr_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index-1]);
      _SSTV_SET_PSC(sstv_info.sstv_mode->pulse_porch_psc_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index]);

      //printf("a %d\r\n", sstv_info._pulse_porch_index);
      //printf("a %d\r\n", sstv_info._loop_index);
      sstv_info._pulse_porch_index ++;
    }else if(sstv_info._pulse_porch_index == sstv_info.sstv_mode->pulse_porch_num[sstv_info._loop_index]){
      // 
      //_SSTV_SET_ARR(sstv_info.sstv_mode->dma_arr);
      AD9833_FrequencyConversion_2Reg(sstv_info.AD9833_I, &sstv_info.sstv_mode->pulse_porch_frq_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index-1], &frqh, &frql);
      AD9833_Write_Whole_Frq(&sstv_tim_dma_spi, &frqh, &frql);
      _SSTV_SET_ARR(sstv_info.sstv_mode->pulse_porch_arr_ptr[sstv_info._loop_index][sstv_info._pulse_porch_index-1]);
      _SSTV_SET_PSC(sstv_info.sstv_mode->dma_psc);
      sstv_info._sstv_fsm = SSTV_FSM_DMA;

      //printf("b %d\r\n", sstv_info._pulse_porch_index);
      //printf("b %d\r\n", sstv_info._loop_index);
      //重置_pulse_porch_index和 _loop_index++，进入DMA
      if(sstv_info._loop_index == sstv_info.sstv_mode->loop_num){
        sstv_info._loop_index = 0;
      }else{
        sstv_info._loop_index ++;
      }
      sstv_info._pulse_porch_index = 0;
      
      HAL_DMA_Start_IT(sstv_info.AD9833_I->spi->hdmatx, (uint32_t)sstv_info.tx_buffer_ptr, (uint32_t)&(sstv_info.AD9833_I->spi->Instance->DR), sstv_info.sstv_mode->sstv_dma_line_length);
    }else{
      printf("pp index overflow");
    }
  }else if(sstv_info._sstv_fsm == SSTV_FSM_DMA){
    __HAL_DMA_ENABLE(sstv_info.AD9833_I->spi->hdmatx);
    _SSTV_SET_ARR(sstv_info.sstv_mode->dma_arr);
    __HAL_TIM_DISABLE_IT(&sstv_tim, TIM_IT_UPDATE);
    _LOAD_TIM_REG;
    //printf("c\r\n");
  }else if(sstv_info._sstv_fsm == SSTV_FSM_END){
    __HAL_TIM_DISABLE_IT(&sstv_tim, TIM_IT_UPDATE);
    _LOAD_TIM_REG;
    __HAL_TIM_DISABLE(&sstv_tim);
    SPI_Write_Half_Word(&sstv_tim_dma_spi, AD9833_REG_CONTROL | AD9833_REG_RESET);
    sstv_info._sstv_fsm = SSTV_FSM_Idle;
    sstv_info._sstv_tx_state = SSTV_Idle;
    //printf("SSTV END\r\n");
  }else{
    printf("sstv loopcb err");
  }
}

void SSTV_DMA_Cplt_Callback(void){
  test_point[19] += 1;
  //printf("D %d\r\n", test_point[19]);
  sstv_info._line_sended += 1;
  if(sstv_info._line_sended < sstv_info.sstv_mode->sstv_dma_line_cnt * sstv_info.sstv_mode->loop_num){
    // 还没发完
    if(sstv_info._loop_index < sstv_info.sstv_mode->loop_num && sstv_info.sstv_mode->pulse_porch_num[sstv_info._loop_index] != 0){
      //printf("a %d\r\n", sstv_info._loop_index);
      _SSTV_SET_ARR(sstv_info.sstv_mode->dma_arr * 3);        // 让最后一次dma传输时间完整
      __HAL_DMA_DISABLE(sstv_info.AD9833_I->spi->hdmatx);
      _DMA_CLEAR_TC_FLAG(sstv_info.AD9833_I->spi->hdmatx);
      __HAL_TIM_ENABLE_IT(&sstv_tim, TIM_IT_UPDATE);
      _LOAD_TIM_REG;
      _SSTV_SET_PSC(sstv_info.sstv_mode->pulse_porch_psc_ptr[sstv_info._loop_index][0])
      sstv_info._pulse_porch_index = 1;
      sstv_info._sstv_fsm = SSTV_FSM_Loop;
    }else if(sstv_info._loop_index == sstv_info.sstv_mode->loop_num && sstv_info.sstv_mode->pulse_porch_num[0] != 0){
      //printf("c %d\r\n", sstv_info._loop_index);
      _SSTV_SET_ARR(sstv_info.sstv_mode->dma_arr * 3);        // 让最后一次dma传输时间完整
      __HAL_DMA_DISABLE(sstv_info.AD9833_I->spi->hdmatx);
      _DMA_CLEAR_TC_FLAG(sstv_info.AD9833_I->spi->hdmatx);
      __HAL_TIM_ENABLE_IT(&sstv_tim, TIM_IT_UPDATE);
      _LOAD_TIM_REG;
      sstv_info._loop_index = 0;
      _SSTV_SET_PSC(sstv_info.sstv_mode->pulse_porch_psc_ptr[sstv_info._loop_index][0])
      sstv_info._pulse_porch_index = 1;
      sstv_info._sstv_fsm = SSTV_FSM_Loop;
    }else{
      __HAL_DMA_DISABLE(sstv_info.AD9833_I->spi->hdmatx);
      _DMA_CLEAR_TC_FLAG(sstv_info.AD9833_I->spi->hdmatx);    // 循环模式下似乎会自动清
      __HAL_DMA_ENABLE(sstv_info.AD9833_I->spi->hdmatx);      // 循环模式不用重新配置DMA，打开就能从头传
      //printf("b %d\r\n", sstv_info._loop_index);
      if(sstv_info._loop_index == sstv_info.sstv_mode->loop_num){
        sstv_info._loop_index = 0;
      }else{
        sstv_info._loop_index ++;
      }
      //不用重新配置HAL_DMA_Start_IT(sstv_info.AD9833_I->spi->hdmatx, (uint32_t)sstv_info.tx_buffer_ptr, (uint32_t)&(sstv_info.AD9833_I->spi->Instance->DR), sstv_info.sstv_mode->sstv_dma_line_length);
    }
  }else{
    // 发完了
    __HAL_DMA_DISABLE(sstv_info.AD9833_I->spi->hdmatx);
    _DMA_CLEAR_TC_FLAG(sstv_info.AD9833_I->spi->hdmatx);
    _SSTV_SET_ARR(sstv_info.sstv_mode->dma_arr * 3);        // 让最后一次dma传输时间完整
    __HAL_TIM_ENABLE_IT(&sstv_tim, TIM_IT_UPDATE);
    _LOAD_TIM_REG;
    sstv_info._sstv_fsm = SSTV_FSM_END;
    //sstv_info._sstv_tx_state = SSTV_Idle;
    for(uint8_t i=0;i<20;i++){
      //printf("testpoint%d: %d\r\n", i+1, test_point[i]);
    }
  }
  

  // TODO: 不是最后一行的话，写后半段数组
}

void SSTV_DMA_HalfCplt_Callback(void){
  // TODO: 不是最后一行的话，写前半段数组
  return;
}