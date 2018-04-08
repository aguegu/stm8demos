#include "stm8l10x.h"
// #include "stm8l10x_it.h"    /* SDCC patch: required by SDCC for interrupts */

// leb between GND and PD0

void Delay(__IO uint16_t nCount) {
  while (nCount) {
    nCount--;
  }
}

void main(void) {
  GPIO_Init(GPIOD, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);
  while (1) {
    GPIO_ToggleBits(GPIOD, GPIO_Pin_0);
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
