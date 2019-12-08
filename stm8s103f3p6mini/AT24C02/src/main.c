#include <stdio.h>
#include "stm8s.h"

// TEST LED between vcc and pb5

#define delay(a)          { TIM4_tout = a; while(TIM4_tout); }
#define tout()            (TIM4_tout)
#define set_tout_ms(a)    { TIM4_tout = a; }

int putchar(int c) {
  UART1_SendData8(c);
  while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
  return (c);
}

int getchar(void) {
  int c = 0;
  while (UART1_GetFlagStatus(UART1_FLAG_RXNE) == RESET);
  c = UART1_ReceiveData8();
  return c;
}

volatile uint16_t TIM4_tout = 0;

void I2C_init(void) {
  //define SDA, SCL outputs, HiZ, Open drain, Fast
  GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_HIZ_FAST);
  GPIO_Init(GPIOB, GPIO_PIN_4, GPIO_MODE_OUT_OD_HIZ_FAST);

  I2C_Init(400000, 0xA0, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 16);
  I2C_ITConfig(I2C_IT_ERR, ENABLE);
}

void I2C_readRegister(uint8_t slave, uint8_t u8_regAddr, uint8_t u8_NumByteToRead, uint8_t *u8_DataBuffer) {

  while (I2C_GetFlagStatus(I2C_FLAG_BUSBUSY) && tout()) {
    I2C_GenerateSTOP(ENABLE);
    while (I2C->CR2 & I2C_CR2_STOP && tout());
  }

  I2C_AcknowledgeConfig(I2C_ACK_CURR);

  I2C_GenerateSTART(ENABLE);
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if(tout()) {
    I2C_Send7bitAddress((uint8_t)(slave << 1), I2C_DIRECTION_TX);
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
    I2C_Send7bitAddress((uint8_t)(slave << 1), I2C_DIRECTION_RX);
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

void I2C_writeRegister(uint8_t slave, uint8_t u8_regAddr, uint8_t u8_NumByteToWrite, uint8_t *u8_DataBuffer) {
  while (I2C_GetFlagStatus(I2C_FLAG_BUSBUSY) && tout()) {
    I2C_GenerateSTOP(ENABLE);
    while (I2C->CR2 & I2C_CR2_STOP && tout());
  }

  I2C_GenerateSTART(ENABLE);
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if (tout()) {
    I2C_Send7bitAddress((uint8_t)(slave << 1), I2C_DIRECTION_TX);
  }

  while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && tout());

  I2C->SR3;

  while(!I2C_GetFlagStatus(I2C_FLAG_TXEMPTY) && tout());
  if (tout()) {
    I2C_SendData(u8_regAddr);
  }
  if (u8_NumByteToWrite)   {
    while (u8_NumByteToWrite--) {
      while(!I2C_GetFlagStatus(I2C_FLAG_TXEMPTY) && tout());
      I2C_SendData(*u8_DataBuffer++);
    }
  }
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) && tout());

  I2C_GenerateSTOP(ENABLE);
  while (I2C->CR2 & I2C_CR2_STOP && tout());
}

void main(void) {
  uint8_t i = 0;
  char x[8] =  { 0 };

  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  UART1_DeInit();
  UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
              UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);

  TIM4_DeInit();
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);  // 16MHz / 128 / 125 = 1KHz
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  TIM4_Cmd(ENABLE);

  I2C_init();

  enableInterrupts();

  while (1) {
    set_tout_ms(10);
    I2C_readRegister(0x50, i, 1, &x);

    printf("%02x: %02x \r\n", i, x);

    I2C_writeRegister(0x50, i, 1, &i);
    delay(100);
    i++;
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  (void) file;
  (void) line;

  while (1);
}
#endif
