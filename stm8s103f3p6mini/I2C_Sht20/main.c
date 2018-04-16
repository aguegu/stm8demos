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

__IO u16 TIM4_tout = 0;

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

__I uint16_t POLYNOMIAL = 0x131; //P(x)=x^8+x^5+x^4+1 = 100110001

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

u16 I2C_readRegister(u8 slave, u8 reg, u8 *rxbuff, u8 rxlen) {
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
  // while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && tout());
  while(!(I2C->SR1 & I2C_SR1_ADDR) && tout()); 				// test EV6 - wait for address ack (ADDR)

  I2C->SR3;
  while(!I2C_GetFlagStatus(I2C_FLAG_TXEMPTY) && tout());

  if (tout()) {
    I2C_SendData(reg);
  }
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) && tout());

  I2C_GenerateSTART(ENABLE);
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if(tout()) {
    I2C_Send7bitAddress((u8)(slave << 1), I2C_DIRECTION_RX);
  }
  while (!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && tout());

  if (rxlen > 2) {
    I2C->SR3;                                            // ADDR clearing sequence

    while (rxlen > 3 && tout()) {
      while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
      *rxbuff++ = I2C_ReceiveData();
      --rxlen;
    }

    while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
    I2C_AcknowledgeConfig(I2C_ACK_NONE);
    disableInterrupts();                               // Errata workaround (Disable interrupt)
    *rxbuff++ = I2C_ReceiveData();
    I2C_GenerateSTOP(ENABLE);
    *rxbuff++ = I2C_ReceiveData();
    enableInterrupts();		                             // Errata workaround (Enable interrupt)

    while(I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY) == RESET && tout());
    *rxbuff++ = I2C_ReceiveData();

  } else if(rxlen == 2) {
    I2C_AcknowledgeConfig(I2C_ACK_NEXT);
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2C->SR3;                                       	// Clear ADDR Flag
    I2C_AcknowledgeConfig(I2C_ACK_NONE);
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2C_GenerateSTOP(ENABLE);
    *rxbuff++ = I2C_ReceiveData();
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    *rxbuff++ = I2C_ReceiveData();
  } else {
    I2C_AcknowledgeConfig(I2C_ACK_NONE);
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2C->SR3;                                       	// Clear ADDR Flag
    I2C_GenerateSTOP(ENABLE);
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    while(I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY) == RESET && tout());
    *rxbuff = I2C_ReceiveData();
  }

  while((I2C->CR2 & I2C_CR2_STOP) && tout());
  I2C_AcknowledgeConfig(I2C_ACK_CURR);

  return tout();
}

u16 I2C_writeRegister(u8 slave, u8 reg, u8 *txbuff, u8 txlen) {
  while (I2C_GetFlagStatus(I2C_FLAG_BUSBUSY) && tout()) {
    I2C_GenerateSTOP(ENABLE);
    while (I2C->CR2 & I2C_CR2_STOP && tout());
  }

  I2C_GenerateSTART(ENABLE);
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if (tout()) {
    I2C_Send7bitAddress((u8)(slave << 1), I2C_DIRECTION_TX);
  }

  while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && tout());

  I2C->SR3;

  while(!I2C_GetFlagStatus(I2C_FLAG_TXEMPTY) && tout());
  if (tout()) {
    I2C_SendData(reg);
  }
  if (txlen) {
    while (txlen--) {
      while(!I2C_GetFlagStatus(I2C_FLAG_TXEMPTY) && tout());
      I2C_SendData(*txbuff++);
    }
  }
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) && tout());

  I2C_GenerateSTOP(ENABLE);
  while (I2C->CR2 & I2C_CR2_STOP && tout());

  return tout();
}


void main(void) {
  u8 dt[3];
  s32 t = 0, rh = 0;
  u16 y1, y2;

  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_SLOW);

  UART1_DeInit();
  UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
              UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);

  TIM4_DeInit();
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  TIM4_Cmd(ENABLE);

  // define SDA, SCL outputs, HiZ, Open drain, Fast
  GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_HIZ_FAST);
  GPIO_Init(GPIOB, GPIO_PIN_4, GPIO_MODE_OUT_OD_HIZ_FAST);

  I2C_DeInit();
  I2C_Init(100000, 0xA0, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 16);
  I2C_ITConfig(I2C_IT_ERR, ENABLE);

  enableInterrupts();

  delay(20);  // wait for sht20 to get ready

  // LSI = 128kHz / 2 = 64kHz
  // IWDG counter clock = 64 kHz / 256 = 250 Hz = 1 / 4ms
  // Reload value = 1s * 250Hz = 1s / (4ms) = 250
  IWDG_Enable();
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  IWDG_SetReload(250);
  IWDG_ReloadCounter();

  while (1) {
    GPIO_WriteHigh(GPIOA, GPIO_PIN_3);
    set_tout_ms(200);
    y1 = I2C_readRegister(0x40, 0xe3, dt, 3);
    // printf("\r\nrx: %d:   %02x %02x %02x, %02x", tout(), dt[0], dt[1], dt[2], checkCrc(dt, 2));
    if (y1 && dt[2] == checkCrc(dt, 2)) {
      t = (((int32_t)dt[0] << 8) + dt[1]) & 0xfffc;
      t = ((t * 17572) >> 16) - 4685;
    }

    set_tout_ms(100);
    y2 = I2C_readRegister(0x40, 0xe5, dt, 3);
    // printf("\r\nrx: %02x %02x %02x", dt[0], dt[1], dt[2]);

    if (y2 && dt[2] == checkCrc(dt, 2)) {
      rh = (((int32_t)dt[0] << 8) + dt[1]) & 0xfffc;
      rh = ((rh * 125) >> 16) - 6;
    }
    GPIO_WriteLow(GPIOA, GPIO_PIN_3);

    if (y1 && y2) {
      // printf("y1: %u, y2: %u, ", y1, y2);
      printf("T: ");
      putFloat(t);
      printf(", RH: ");
      putFloat(rh);
      printf("\r\n");
      IWDG_ReloadCounter();
    }

    delay(600 - y1 - y2);

    delay(500);
    if (y1 && y2) {
      IWDG_ReloadCounter();
    }
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
