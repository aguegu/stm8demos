#include "stm8l15x.h"
#include "stm8l15x_it.h"

void Delay(__IO uint32_t nTime);
void TimingDelay_Decrement(void);

__IO uint32_t TimingDelay;

void Delay(__IO uint32_t nTime) {
  TimingDelay = nTime;
  while (TimingDelay);
}

void TimingDelay_Decrement(void) {
  if (TimingDelay) {
    TimingDelay--;
  }
}


void main(void) {
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);

  GPIO_Init(GPIOE, GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);

  TIM4_DeInit();
  TIM4_TimeBaseInit(TIM4_Prescaler_128, 124);
  TIM4_ClearFlag(TIM4_FLAG_Update);
  TIM4_ITConfig(TIM4_IT_Update, ENABLE);
  enableInterrupts();

  TIM4_Cmd(ENABLE);

  while (1) {
    GPIO_ToggleBits(GPIOE, GPIO_Pin_7);
    Delay(1000);
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line) {
  (void) file;
  (void) line;
  while (1);
}

#endif
