#include "stm8s.h"
#include "stdio.h"

int putchar(int c) {
  UART2_SendData8(c);
  while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);
  return c;
}

volatile uint32_t TimingDelay = 0;

void Delay(uint32_t nTime) {
  TimingDelay = nTime;
  while (TimingDelay);
}

void main(void) {
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  UART2_DeInit();
  UART2_Init((uint32_t)9600, UART2_WORDLENGTH_8D, UART2_STOPBITS_1, UART2_PARITY_NO,
              UART2_SYNCMODE_CLOCK_DISABLE, UART2_MODE_TXRX_ENABLE);

  GPIO_Init(GPIOD, GPIO_PIN_0, GPIO_MODE_OUT_OD_LOW_SLOW);
  GPIO_WriteHigh(GPIOD, GPIO_PIN_0);
  printf("hello.\r\n");
  // 16000000 / 128 / (124 + 1) = 1000, TIM4 is 8-bit timer
  // TIM4_DeInit();
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  TIM4_Cmd(ENABLE);

  enableInterrupts();

  while (1) {
    // GPIO_WriteReverse(GPIOD, GPIO_PIN_0);
    // Delay(1000);
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  printf("Wrong parameters value: file %s on line %ld\r\n", file, line);
  while (1);
}
#endif
