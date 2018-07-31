#include "stm8l15x.h"
#include "stm8l15x_it.h"

// led between VCC and PB0
// SST223 touch button on PC7

void Delay (uint16_t nCount);

__IO uint16_t t = 0xf000;

void main(void) {
  uint16_t i;

  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);

  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);
  GPIO_Init(GPIOC, GPIO_Pin_7, GPIO_Mode_In_FL_IT);
  EXTI_SetPinSensitivity(EXTI_Pin_7, EXTI_Trigger_Falling);

  enableInterrupts();

  while (1) {
    i = 0x10;
    while (i--) {
      GPIO_ToggleBits(GPIOB, GPIO_Pin_0);
      Delay(t);
    }
    halt();
  }
}

void Delay(uint16_t nCount) {
  while (nCount--);
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line) {
  (void) file;
  (void) line;
  while (1);
}

#endif
