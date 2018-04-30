#include "stm8s_conf.h"

#define HEIGHT 64
#define WIDTH 128

/****************时锟斤拷*********************/
#define OLED_SCLK_PORT  (GPIOC)
#define OLED_SCLK_PINS  (GPIO_PIN_5)

/****************锟斤拷锟斤拷*********************/
#define OLED_SDIN_PORT  (GPIOC)
#define OLED_SDIN_PINS  (GPIO_PIN_6)

/****************锟斤拷位*********************/
#define OLED_RST_PORT  (GPIOD)
#define OLED_RST  (GPIO_PIN_2)

/****************锟斤拷锟斤拷/锟斤拷锟斤拷*********************/
#define OLED_DC_PORT  (GPIOD)
#define OLED_DC_PINS  (GPIO_PIN_3)

#define OLED_SCLK_Clr() GPIO_WriteLow(OLED_SCLK_PORT, OLED_SCLK_PINS)
#define OLED_SCLK_Set() GPIO_WriteHigh(OLED_SCLK_PORT, OLED_SCLK_PINS)

#define OLED_SDIN_Clr() GPIO_WriteLow(OLED_SDIN_PORT, OLED_SDIN_PINS)
#define OLED_SDIN_Set() GPIO_WriteHigh(OLED_SDIN_PORT, OLED_SDIN_PINS)

#define OLED_RST_Clr() GPIO_WriteLow(OLED_RST_PORT, OLED_RST)
#define OLED_RST_Set() GPIO_WriteHigh(OLED_RST_PORT, OLED_RST)

#define OLED_DC_Clr() GPIO_WriteLow(OLED_DC_PORT, OLED_DC_PINS)
#define OLED_DC_Set() GPIO_WriteHigh(OLED_DC_PORT, OLED_DC_PINS)

void GPIO_SendData(u8 dat) {
	u8 i;
	for (i=0; i<8; i++) {
		OLED_SCLK_Clr();
		if (dat&0x80) {
		  OLED_SDIN_Set();
		} else {
			OLED_SDIN_Clr();
		}
		OLED_SCLK_Set();
		dat<<=1;
	}
}

void Delay(uint16_t nCount) {
	while (nCount--);
}

void OLED_Init(void) {
	// u8 s[25] = {0xae, 0x40, 0x81, 0xcf, 0xa0, 0xc0, 0xa6, 0xa8, 63, 0xd3, 0x00, 0xd5, 0x80, 0xd9, 0xf1, 0xda, 0x12, 0xdb, 0x40, 0x20, 0x02, 0x8d, 0x14, 0xa4, 0xaf};
	u8 s[31] = {0xae, 0x40, 0x81, 0xcf, 0xa0, 0xc0, 0xa6, 0xa8, 63, 0xd3, 0x00, 0xd5, 0x80, 0xd9, 0xf1, 0xda, 0x12, 0xdb, 0x40, 0x20, 0x01, 0x21, 0x00, WIDTH - 1, 0x8d, 0x14, 0xa4, 0xaf, 0xb0, 0x00, 0x10};

  GPIO_Init(OLED_SCLK_PORT, (GPIO_Pin_TypeDef)OLED_SCLK_PINS, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(OLED_SDIN_PORT, (GPIO_Pin_TypeDef)OLED_SDIN_PINS, GPIO_MODE_OUT_PP_LOW_FAST);

  GPIO_Init(OLED_RST_PORT, (GPIO_Pin_TypeDef)OLED_RST, GPIO_MODE_OUT_PP_LOW_SLOW);
  GPIO_Init(OLED_DC_PORT, (GPIO_Pin_TypeDef)OLED_DC_PINS, GPIO_MODE_OUT_PP_LOW_SLOW);

	GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_SLOW);

	OLED_RST_Clr();
	OLED_RST_Set();

	// OLED_WR_Byte(0xAE, OLED_CMD);//--turn off oled panel
	// // OLED_WR_Byte(0x00, OLED_CMD);//---set low column address
	// // OLED_WR_Byte(0x10, OLED_CMD);//---set high column address
	// OLED_WR_Byte(0x40, OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	// OLED_WR_Byte(0x81, OLED_CMD);//--set contrast control register
	// OLED_WR_Byte(0xCF, OLED_CMD); // Set SEG Output Current Brightness
	// OLED_WR_Byte(0xA0, OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	// OLED_WR_Byte(0xC0, OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	// OLED_WR_Byte(0xA6, OLED_CMD);//--set normal display
	//
	// OLED_WR_Byte(0xA8, OLED_CMD);//--set multiplex ratio(1 to 64)
	// OLED_WR_Byte(HEIGHT - 1, OLED_CMD);//--1/64 duty
	//
	// OLED_WR_Byte(0xD3, OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	// OLED_WR_Byte(0x00, OLED_CMD);//-not offset
	//
	// OLED_WR_Byte(0xd5, OLED_CMD);//--set display clock divide ratio/oscillator frequency
	// OLED_WR_Byte(0x80, OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
	//
	// OLED_WR_Byte(0xD9, OLED_CMD);//--set pre-charge period
	// OLED_WR_Byte(0xF1, OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	//
	// OLED_WR_Byte(0xDA, OLED_CMD);//--set com pins hardware configuration
	// OLED_WR_Byte(0x12, OLED_CMD);
	//
	// OLED_WR_Byte(0xDB, OLED_CMD);//--set vcomh
	// OLED_WR_Byte(0x40, OLED_CMD);//Set VCOM Deselect Level
	//
	// OLED_WR_Byte(0x20, OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
	// OLED_WR_Byte(0x02, OLED_CMD);//
	//
	// OLED_WR_Byte(0x8D, OLED_CMD);//--set Charge Pump enable/disable
	// OLED_WR_Byte(0x14, OLED_CMD);//--set(0x10) disable
	// OLED_WR_Byte(0xA4, OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
	// // OLED_WR_Byte(0xA6, OLED_CMD);// Disable Inverse Display On (0xa6/a7)
	// OLED_WR_Byte(0xAF, OLED_CMD);//--turn on oled panel

	OLED_DC_Clr();
	for (u8 i=0; i<31; i++) {
    GPIO_SendData(s[i]);
  }
}


void OLED_All(u8 c) {
	// for (u16 n=0; n<1024; n++)
	// 	OLED_WR_Byte(c);

	u16 n;
  for (n=0; n < 256; n++) {
    GPIO_SendData(n);
  }
  for (n=0; n < 256; n++) {
    GPIO_SendData(~n);
  }

  for (n=0; n < 256; n++) {
    GPIO_SendData(0x01 << (n / 8 % 8));
  }
	for (n=0; n < 256; n++) {
    GPIO_SendData(c);
  }
}

void main(void) {
	u8 t = 0;
	OLED_Init();

	OLED_DC_Set();
	while (1) {
		OLED_All(t++);
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
