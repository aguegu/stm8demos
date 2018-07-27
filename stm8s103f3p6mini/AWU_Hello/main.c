#include <stdio.h>
#include "stm8s_conf.h"

// TEST LED between vcc and PB5
// Oscilloscope on PA3

void Delay(uint16_t t) {
  while (t--);
}

// uint32_t LSIMeasurment(void)
// {
//
//   uint32_t lsi_freq_hz = 0x0;
//   uint32_t fmaster = 0x0;
//   uint16_t ICValue1 = 0x0;
//   uint16_t ICValue2 = 0x0;
//
//   /* Get master frequency */
//   fmaster = CLK_GetClockFreq();
//
//   /* Enable the LSI measurement: LSI clock connected to timer Input Capture 1 */
//   AWU->CSR |= AWU_CSR_MSR;
//
//   /* Measure the LSI frequency with TIMER Input Capture 1 */
//
//   /* Capture only every 8 events!!! */
//   /* Enable capture of TI1 */
// 	TIM1_ICInit(TIM1_CHANNEL_1, TIM1_ICPOLARITY_RISING, TIM1_ICSELECTION_DIRECTTI, TIM1_ICPSC_DIV8, 0);
//
//   /* Enable TIM1 */
//   TIM1_Cmd(ENABLE);
//
//   /* wait a capture on cc1 */
//   while((TIM1->SR1 & TIM1_FLAG_CC1) != TIM1_FLAG_CC1);
//   /* Get CCR1 value*/
//   ICValue1 = TIM1_GetCapture1();
//   TIM1_ClearFlag(TIM1_FLAG_CC1);
//
//   /* wait a capture on cc1 */
//   while((TIM1->SR1 & TIM1_FLAG_CC1) != TIM1_FLAG_CC1);
//   /* Get CCR1 value*/
//   ICValue2 = TIM1_GetCapture1();
//   TIM1_ClearFlag(TIM1_FLAG_CC1);
//
//   /* Disable IC1 input capture */
//   TIM1->CCER1 &= (uint8_t)(~TIM1_CCER1_CC1E);
//   /* Disable timer2 */
//   TIM1_Cmd(DISABLE);
//
//   /* Compute LSI clock frequency */
//   lsi_freq_hz = (8 * fmaster) / (ICValue2 - ICValue1);
//
//   /* Disable the LSI measurement: LSI clock disconnected from timer Input Capture 1 */
//   AWU->CSR &= (uint8_t)(~AWU_CSR_MSR);
//
//   return (lsi_freq_hz);
// }

int putchar (int c) {
  UART1_SendData8(c);
  while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
  return (c);
}

void main(void) {
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_SLOW);
  GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_LOW_SLOW);

  UART1_DeInit();
  UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
              UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);

  // AWU_LSICalibrationConfig(LSIMeasurment());
  AWU_Init(AWU_TIMEBASE_2S);

  enableInterrupts();

  while (1) {
    GPIO_WriteHigh(GPIOA, GPIO_PIN_3);
    GPIO_WriteLow(GPIOB, GPIO_PIN_5);
    printf("Hello\n\r");
    Delay(0xffff);
    GPIO_WriteLow(GPIOA, GPIO_PIN_3);
    GPIO_WriteHigh(GPIOB, GPIO_PIN_5);
    // Delay(0xffff);

    halt();
  }
}



#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  (void) file;
  (void) line;
  while (1);
}
#endif
