#include "stm8s_conf.h"

// TEST LED between vcc and PB5
// Oscilloscope on PA3

void Delay(uint16_t t) {
  while (t--);
}

void main(void) {
  GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_SLOW);
  GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_LOW_SLOW);

  GPIO_WriteReverse(GPIOA, GPIO_PIN_3);

  while (1) {
    GPIO_WriteReverse(GPIOA, GPIO_PIN_3);
    GPIO_WriteReverse(GPIOB, GPIO_PIN_5);

    Delay(0xffff);
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  (void) file;
  (void) line;
  while (1);
}
#endif
