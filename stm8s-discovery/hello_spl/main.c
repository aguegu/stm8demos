#include "stm8s.h"
#include "stdio.h"

int putchar(int c) {
  UART2_SendData8(c);
  while (UART2_GetFlagStatus(UART2_FLAG_TXE) == RESET);
  return c;
}

void main(void) {
  uint32_t t = 0;
  uint8_t i = 0;
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  UART2_DeInit();
  UART2_Init((uint32_t)9600, UART2_WORDLENGTH_8D, UART2_STOPBITS_1, UART2_PARITY_NO,
              UART2_SYNCMODE_CLOCK_DISABLE, UART2_MODE_TXRX_ENABLE);

  while(1) {
    printf("hello, world. %02x\r\n", i++);
    t = 147456;
    while(t--);
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  while (1);
}
#endif
