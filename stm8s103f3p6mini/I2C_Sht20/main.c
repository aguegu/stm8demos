#include <stdio.h>
#include "stm8s.h"
#include "stm8s_it.h"

// TEST LED between vcc and PB5, no use
// Oscilloscope on PA3
// UART1_TX on PD5
// UART1_RX on PD6
// I2C SDA on PB5
// I2C SCL on PB4

#define delay(a)          { TIM4_tout = a; while(TIM4_tout); }
#define tout()            (TIM4_tout)
#define set_tout_ms(a)    { TIM4_tout = a; }

volatile u16 TIM4_tout = 0;

int putchar (int c) {
  UART1_SendData8(c);
  while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
  return (c);
}

// int getchar (void) {
//   int c = 0;
//   while (UART1_GetFlagStatus(UART1_FLAG_RXNE) == RESET);
//   c = UART1_ReceiveData8();
//   return c;
// }

uint16_t POLYNOMIAL = 0x131; //P(x)=x^8+x^5+x^4+1 = 100110001

uint8_t checkCrc(uint8_t *data, uint8_t len) {
  uint8_t crc = 0;

  for (uint8_t i = 0; i < len; ++i) {
    crc ^= (data[i]);
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ POLYNOMIAL;
      } else {
        crc = (crc << 1);
      }
    }
  }
  return (crc);
}

void putFloat(int32_t x) {
  int16_t y = (int)(x % 100);
  if (y < 0) y = -y;
  printf("%ld.%02d", x / 100, y);
}

void I2C_readRegister(u8 slave, u8 u8_regAddr, u8 u8_NumByteToRead, u8 *u8_DataBuffer) {

  while (I2C_GetFlagStatus(I2C_FLAG_BUSBUSY) && tout()) {
    I2C_GenerateSTOP(ENABLE);
    while (I2C->CR2 & I2C_CR2_STOP && tout());
  }

  I2C_AcknowledgeConfig(I2C_ACK_CURR);

  I2C_GenerateSTART(ENABLE);
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if(tout()) {
    I2C_Send7bitAddress((u8)(slave << 1), I2C_DIRECTION_TX);
  }

  while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && tout());
  I2C->SR3;

  while(!I2C_GetFlagStatus(I2C_FLAG_TXEMPTY) && tout());
  if (tout()) {
    I2C_SendData(u8_regAddr);
  }
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) && tout());

  I2C_GenerateSTART(ENABLE);
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if(tout()) {
    I2C_Send7bitAddress((u8)(slave << 1), I2C_DIRECTION_RX);
  }
  while (!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && tout());

  if (u8_NumByteToRead > 2) {
    I2C->SR3;                                            // ADDR clearing sequence

    while (u8_NumByteToRead > 3 && tout()) {
      while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
      *u8_DataBuffer++ = I2C_ReceiveData();
      --u8_NumByteToRead;
    }
    while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
    I2C_AcknowledgeConfig(I2C_ACK_NONE);
    disableInterrupts();                               // Errata workaround (Disable interrupt)
    *u8_DataBuffer++ = I2C_ReceiveData();
    I2C_GenerateSTOP(ENABLE);
    *u8_DataBuffer++ = I2C_ReceiveData();
    enableInterrupts();		                             // Errata workaround (Enable interrupt)

    while(I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY) == RESET && tout());
    *u8_DataBuffer++ = I2C_ReceiveData();
  }
  else if(u8_NumByteToRead == 2) {
    I2C_AcknowledgeConfig(I2C_ACK_NEXT);
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2C->SR3;                                       	// Clear ADDR Flag
    I2C_AcknowledgeConfig(I2C_ACK_NONE);
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2C_GenerateSTOP(ENABLE);
    *u8_DataBuffer++ = I2C_ReceiveData();
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    *u8_DataBuffer++ = I2C_ReceiveData();
  } else {
    I2C_AcknowledgeConfig(I2C_ACK_NONE);
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2C->SR3;                                       	// Clear ADDR Flag
    I2C_GenerateSTOP(ENABLE);
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    while(I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY) == RESET && tout());
    *u8_DataBuffer = I2C_ReceiveData();
  }

  while((I2C->CR2 & I2C_CR2_STOP) && tout());
  I2C_AcknowledgeConfig(I2C_ACK_CURR);
}

void main(void) {
  // char ans;
  uint8_t dt[3];
  // uint8_t s[2] = {0xe3, 0xe5};
  // uint8_t err = 0;
  int32_t t;
  int32_t t2;

  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_SLOW);

  // LSI = 128kHz
  // IWDG counter clock = 128 kHz / 256 = 500 Hz = 1 / 2ms
  // Reload value = 0.5s * 500Hz = 250

  //
  /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  // IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  //
  // IWDG_SetPrescaler(IWDG_Prescaler_256);
  // IWDG_SetReload(250);
  //
  // IWDG_ReloadCounter();

  UART1_DeInit();
  UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
              UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);

  TIM4_DeInit();
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  TIM4_Cmd(ENABLE);
  //
  //define SDA, SCL outputs, HiZ, Open drain, Fast
  GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_HIZ_FAST);
  GPIO_Init(GPIOB, GPIO_PIN_4, GPIO_MODE_OUT_OD_HIZ_FAST);

  I2C_Init(300000, 0xA0, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 16);
  I2C_ITConfig(I2C_IT_ERR, ENABLE);

  enableInterrupts();

  while (1) {
    set_tout_ms(100);
    I2C_readRegister(0x40, 0xe3, 3, dt);
    // printf("\r\nrx: %02x %02x %02x, %02x", dt[0], dt[1], dt[2], checkCrc(dt, 2));

    if (dt[2] == checkCrc(dt, 2)) {
      t = (((int32_t)dt[0] << 8) + dt[1]) & 0xfffc;
      t2 = ((t * 17572) >> 16) - 4685;
      printf("\r\nT: ");
      putFloat(t2);
    }

    set_tout_ms(100);
    I2C_readRegister(0x40, 0xe5, 3, dt);

    // printf("\r\nrx: %02x %02x %02x", dt[0], dt[1], dt[2]);

    if (dt[2] == checkCrc(dt, 2)) {
      t = (((int32_t)dt[0] << 8) + dt[1]) & 0xfffc;
      t2 = ((t * 125) >> 16) - 6;
      // printf("\r\n%lx", t);
      printf(", H: ");
      putFloat(t2);
    }

    // 100ms to do the job
    // i2cMasterTransmit(0x40, s+1, 1, dt, 3);
    // printf("\r\ncrc: %02x", checkCrc(dt, 2));
    // getchar();
    GPIO_WriteReverse(GPIOA, GPIO_PIN_3);
    // delay(1000);
    // IWDG_ReloadCounter();
    // putchar(ans);
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  (void) file;
  (void) line;
  // printf("Wrong parameters value: file %s on line %ld\r\n", file, line);

  while (1);
}

#endif
