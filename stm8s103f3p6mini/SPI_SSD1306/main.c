#include "stm8s_conf.h"

#define HEIGHT 64
#define WIDTH 128

// TEST LED between vcc and PB5
// Oscilloscope on PA3
// SSD1306:
// SCK: PC5
// MOSI: PC6
// RST: PD2
// DC: PD3

void Delay(uint16_t t) {
  while (t--);
}

void OLED_All(u8 c) {
	u16 n;
  for (n=0; n < 256; n++) {
    SPI_SendData(n);
  }
  for (n=0; n < 256; n++) {
    SPI_SendData(~n);
  }

  for (n=0; n < 256; n++) {
    SPI_SendData(0x01 << (n / 8 % 8));
  }
	for (n=0; n < 256; n++) {
    SPI_SendData(c);
  }
}

void main(void) {
  u8 s[31] = {0xae, 0x40, 0x81, 0xcf, 0xa0, 0xc0, 0xa6, 0xa8, 63, 0xd3, 0x00, 0xd5, 0x80, 0xd9, 0xf1, 0xda, 0x12, 0xdb, 0x40, 0x20, 0x01, 0x21, 0x00, WIDTH - 1, 0x8d, 0x14, 0xa4, 0xaf, 0xb0, 0x00, 0x10};
  uint8_t c = 0;
  CLK_PeripheralClockConfig(CLK_PERIPHERAL_SPI, ENABLE);

  GPIO_Init(GPIOC, (GPIO_Pin_TypeDef)GPIO_PIN_5, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(GPIOC, (GPIO_Pin_TypeDef)GPIO_PIN_6, GPIO_MODE_OUT_PP_LOW_FAST);

  GPIO_Init(GPIOD, (GPIO_Pin_TypeDef)GPIO_PIN_2, GPIO_MODE_OUT_PP_LOW_SLOW);
  GPIO_Init(GPIOD, (GPIO_Pin_TypeDef)GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_SLOW);

  GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_SLOW);

  SPI_Init(SPI_FIRSTBIT_MSB, SPI_BAUDRATEPRESCALER_2, SPI_MODE_MASTER,
           SPI_CLOCKPOLARITY_HIGH, SPI_CLOCKPHASE_2EDGE, SPI_DATADIRECTION_1LINE_TX,
           SPI_NSS_SOFT, 0x07);
  SPI_Cmd(ENABLE);

  GPIO_WriteLow(GPIOD, GPIO_PIN_2);
  GPIO_WriteHigh(GPIOD, GPIO_PIN_2);

  GPIO_WriteLow(GPIOD, GPIO_PIN_3);
  for (u8 i=0; i<31; i++) {
    SPI_SendData(s[i]);
  }

  GPIO_WriteHigh(GPIOD, GPIO_PIN_3);
  while (1) {
    OLED_All(c++);
    GPIO_WriteReverse(GPIOA, GPIO_PIN_3);
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  (void) file;
  (void) line;
  while (1);
}
#endif
