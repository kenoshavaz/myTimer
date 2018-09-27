//Program: myTimer.c
//Name:    Kenosha Vaz
//Date:    15 Feb, 2018, Thursday

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#include "common.h"

ParserReturnVal_t CmdGpioInit(int action)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  if(action!=CMD_INTERACTIVE) return CmdReturnOk;

  /* Turn on clocks to I/O */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  /* Configure GPIO pins */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = 0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

 return CmdReturnOk;
}

ADD_CMD("gpioinit",CmdGpioInit,"                Initialise GPIO Pins");

static TIM_HandleTypeDef tim17;

ParserReturnVal_t CmdTimerInit(int action)
{
  if(action!=CMD_INTERACTIVE) return CmdReturnOk;

  __HAL_RCC_TIM17_CLK_ENABLE();
  
  tim17.Instance = TIM17;
  tim17.Init.Prescaler     = HAL_RCC_GetPCLK2Freq() / 1000000 - 1;
  tim17.Init.CounterMode   = TIM_COUNTERMODE_UP;
  tim17.Init.Period        = 0xffff;
  tim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  tim17.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&tim17);

  HAL_NVIC_SetPriority(TIM17_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(TIM17_IRQn);
  void TIM17_IRQHandler(void);

  HAL_TIM_Base_Start(&tim17);

  return CmdReturnOk;
}
ADD_CMD("timerinit",CmdTimerInit,"                Initialise Timer Delay");

ParserReturnVal_t CmdTimerDelay(int action)
{
  if(action!=CMD_INTERACTIVE) return CmdReturnOk;

  uint32_t rc,delayVal,pin; 
  uint bc,sr;

  rc = fetch_uint32_arg(&pin);
  if(rc)
    {
    printf("Must supply a pin number\n");
    return CmdReturnBadParameter1;
  }

     rc = fetch_uint32_arg(&delayVal);
  if(rc)
  {
    printf("Must supply a value\n");
    return CmdReturnBadParameter1;
  }

  bc=0x0001;

  sr=0x0001;

  if(pin<=15){
    sr=bc<<pin;
  }else{
    printf("Must be between 0-15 ONLY!\n");
  }

  while(1){
    HAL_GPIO_WritePin(GPIOA, sr, 1);    
    TIM17->CNT = 0;     /* Reset counter */
    while(TIM17->CNT < delayVal) {
      asm volatile ("nop\n");
    }

    HAL_GPIO_WritePin(GPIOA, sr, 0);
    
    TaskingRun();  /* Run all registered tasks */
    my_Loop();
  }



  return CmdReturnOk;
}
ADD_CMD("timerdelay",CmdTimerDelay," <PIN#> <DELAY> Sets Timer Delay");
