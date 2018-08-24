#include "stm8l15x.h"
#include <string.h>
// #include "stm8l15x_it.h"

// SCK: PB5
// MOSI: PB6
// DC: PB2
// RES: PB1
#define HEIGHT 64
#define WIDTH 128


void Delay (uint16_t nCount);
static u8 buff[1024];

void display() {
	u16 addr;
	for (u8 i=0; i<8; i++) {
		addr = (u16)(buff + (i << 7));

		DMA1_Channel2->CM0ARH = (u8)(addr >> 8);
		DMA1_Channel2->CM0ARL = (u8)addr;
		DMA1_Channel2->CNBTR = 128;

		DMA_Cmd(DMA1_Channel2, ENABLE);
		while (!DMA_GetFlagStatus(DMA1_FLAG_TC2));
		DMA_ClearFlag(DMA1_FLAG_TC2);
		DMA_Cmd(DMA1_Channel2, DISABLE);
	}
}

static void init(void) {
	DMA_GlobalDeInit();
	DMA_DeInit(DMA1_Channel2);
	DMA_SetTimeOut(0x3F);
	DMA_GlobalCmd(ENABLE);
	SPI_DMACmd(SPI1, SPI_DMAReq_TX, ENABLE);
	DMA_Init(DMA1_Channel2, (u16)(buff), (uint16_t)0x5204, \
					 128, DMA_DIR_MemoryToPeripheral, DMA_Mode_Normal, \
					 DMA_MemoryIncMode_Inc, DMA_Priority_High, DMA_MemoryDataSize_Byte);
}

void main(void) {
  u8 s[31] = {0xae, 0x40, 0x81, 0xcf, 0xa0, 0xc0, 0xa6, 0xa8, 63, 0xd3, 0x00, 0xd5, 0x80, 0xd9, 0xf1, 0xda, 0x12, 0xdb, 0x40, 0x20, 0x01, 0x21, 0x00, WIDTH - 1, 0x8d, 0x14, 0xa4, 0xaf, 0xb0, 0x00, 0x10};
  u8 c = 0;
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);

	CLK_PeripheralClockConfig(CLK_Peripheral_DMA1, ENABLE);
  CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, ENABLE);
  GPIO_ExternalPullUpConfig(GPIOB, GPIO_Pin_5 | GPIO_Pin_6, ENABLE);

  SPI_Init(SPI1, SPI_FirstBit_MSB, SPI_BaudRatePrescaler_2, SPI_Mode_Master,
           SPI_CPOL_Low, SPI_CPHA_1Edge, SPI_Direction_1Line_Tx,
           SPI_NSS_Soft, 0x07);

  GPIO_Init(GPIOB, GPIO_Pin_1, GPIO_Mode_Out_PP_Low_Slow);
  GPIO_Init(GPIOB, GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Slow);

  SPI_Cmd(SPI1, ENABLE);

  GPIO_WriteBit(GPIOB, GPIO_Pin_1, RESET);
  GPIO_WriteBit(GPIOB, GPIO_Pin_1, SET);

  GPIO_WriteBit(GPIOB, GPIO_Pin_2, RESET);
  for (u8 i=0; i<31; i++) {
    SPI_SendData(SPI1, s[i]);
  }

	init();

	GPIO_WriteBit(GPIOB, GPIO_Pin_2, SET);
  while (1) {
		for (u16 i=0; i<6; i++) {
			memset(buff + (i << 7), c++, 128);
		}
		for (u16 i=0;  i<256; i++) {
			buff[1023 - i] = c + i;
		}

		// memset(buff, c++, 1024);

    display();
		GPIO_ToggleBits(GPIOB, GPIO_Pin_0);

		Delay(0xff00);
		Delay(0xff00);
		Delay(0xff00);
		Delay(0xff00);
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
