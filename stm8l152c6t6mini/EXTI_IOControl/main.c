#include "stm8l15x.h"
#include "stm8l15x_it.h"

void Delay (uint16_t nCount);

__IO uint16_t t = 0xf000;

void main(void) {
  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);

  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);
  GPIO_Init(GPIOC, GPIO_Pin_7, GPIO_Mode_In_FL_IT);
  EXTI_SetPinSensitivity(EXTI_Pin_7, EXTI_Trigger_Falling);

  enableInterrupts();

  while (1) {
    GPIO_ToggleBits(GPIOB, GPIO_Pin_0);
    Delay(t);
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
