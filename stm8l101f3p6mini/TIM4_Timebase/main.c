#include "stm8l10x.h"
#include "stm8l10x_it.h"    /* SDCC patch: required by SDCC for interrupts */

// leb between GND and PD0

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
  CLK_MasterPrescalerConfig(CLK_MasterPrescaler_HSIDiv1);
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);

  TIM4_DeInit();
  /* Time base configuration */
  TIM4_TimeBaseInit(TIM4_Prescaler_128, 124);
  TIM4_ITConfig(TIM4_IT_Update, ENABLE);
  GPIO_Init(GPIOD, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);

  enableInterrupts();
  TIM4_Cmd(ENABLE);

  while (1) {
    GPIO_ToggleBits(GPIOD, GPIO_Pin_0);
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
