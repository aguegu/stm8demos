#include "stm8s.h"
#include "stm8s_it.h"

// TEST LED between vcc and pb5

void Delay(uint16_t t) {
  while (t--);
}

void main(void) {
  GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_LOW_SLOW);

  while (1) {
    GPIO_WriteReverse(GPIOB, GPIO_PIN_5);
    Delay(0x4000);
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  (void) file;
  (void) line;

  while (1);
}
#endif
