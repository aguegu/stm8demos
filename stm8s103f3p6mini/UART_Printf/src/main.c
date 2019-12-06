#include <stdio.h>
#include "stm8s.h"

// TEST LED between vcc and pb5

int putchar (int c) {
  UART1_SendData8(c);
  while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
  return (c);
}

int getchar (void) {
  int c = 0;
  while (UART1_GetFlagStatus(UART1_FLAG_RXNE) == RESET);
  c = UART1_ReceiveData8();
  return c;
}

void main(void) {
  char ans;

  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  UART1_DeInit();
  UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
              UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);

  printf("\n\rUART1 Example :retarget the C library printf()/getchar() functions to the UART\n\r");
  printf("\n\rEnter Text\n\r");

  while (1) {
    ans = getchar();
    printf("%c %02x \r\n", ans, ans);
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  (void) file;
  (void) line;

  while (1);
}
#endif
