#include "stm8s_conf.h"

// TEST LED between vcc and PB5
// Oscilloscope on PA3

void Delay(uint16_t t) {
  while (t--);
}

void main(void) {
  GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_SLOW);
  GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_LOW_SLOW);

  // LSI = 128kHz / 2 = 64kHz
  // IWDG counter clock = 64 kHz / 256 = 250 Hz = 1 / 4ms
  // Reload value = 1s * 250Hz = 250 = 1s / (4ms) = 250

  /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_Enable();
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  IWDG_SetPrescaler(IWDG_Prescaler_256);
  IWDG_SetReload(250);  //

  // 125: 500ms
  // 250: 1000ms

  IWDG_ReloadCounter();

  while (1) {
    GPIO_WriteHigh(GPIOA, GPIO_PIN_3);
    GPIO_WriteLow(GPIOB, GPIO_PIN_5);

    Delay(0xffff);

    GPIO_WriteLow(GPIOA, GPIO_PIN_3);
    GPIO_WriteHigh(GPIOB, GPIO_PIN_5);

    Delay(0xffff);
    Delay(0xffff);
    Delay(0xffff);
    Delay(0xffff);
    Delay(0xffff);
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
