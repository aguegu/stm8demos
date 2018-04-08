// #include <stdint.h>
#include <stdio.h>
#include "stm8l10x.h"
// #include "stm8l10x_it.h"    /* SDCC patch: required by SDCC for interrupts */

// leb between GND and PD0

int putchar (int c) {
  USART_SendData8(c);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART_FLAG_TXE) == RESET);

  return (c);
}

// char getchar (void) {
//
// }

void Delay(__IO uint16_t nCount) {
  while (nCount) {
    nCount--;
  }
}

void main(void) {
  uint8_t x = 0;

  CLK_MasterPrescalerConfig(CLK_MasterPrescaler_HSIDiv1);

  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_2|GPIO_Pin_3, ENABLE);

  CLK_PeripheralClockConfig(CLK_Peripheral_USART, ENABLE);

  USART_DeInit();

  USART_Init((uint32_t)9600, USART_WordLength_8D, USART_StopBits_1,
              USART_Parity_No, (USART_Mode_TypeDef)(USART_Mode_Rx | USART_Mode_Tx));


  while (1) {
    printf("Hello World! %02x\r\n", x++);
    Delay(0xf000);
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line) {
  (void) file;
  (void) line;
  while (1);
}

#endif
