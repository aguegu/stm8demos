#include "stm8l15x.h"
#include "stm8l15x_it.h"  // important

__IO uint32_t TimingDelay;

void Delay(__IO uint32_t nTime) {
  TimingDelay = nTime;
  while (TimingDelay) {
    wfi();
  }
}

void main(void) {
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);

  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);

  TIM4_DeInit();
  TIM4_TimeBaseInit(TIM4_Prescaler_128, 124);
  TIM4_ClearFlag(TIM4_FLAG_Update);
  TIM4_ITConfig(TIM4_IT_Update, ENABLE);

  enableInterrupts();
  TIM4_Cmd(ENABLE);

  while (1) {
    GPIO_ToggleBits(GPIOB, GPIO_Pin_0);
    Delay(50);
  }
}



#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line) {
  (void) file;
  (void) line;
  while (1);
}

#endif
