#include <stdio.h>
#include "stm8l15x_conf.h"
#include "stm8l15x_it.h"  // important

#define delay(a)          { TIM4_tout = a; while(TIM4_tout) wfi(); }
#define tout()            ( TIM4_tout )
#define set_tout_ms(a)    { TIM4_tout = a; }

__IO u16 TIM4_tout = 0;

int putchar (int c) {
  USART_SendData8(USART1, c);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
  return (c);
}

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

u16 I2C_readRegister(I2C_TypeDef* I2Cx, u8 slave, u8 reg, u8 *rxbuff, u8 rxlen) {
  while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY) && tout()) {
    I2C_GenerateSTOP(I2Cx, ENABLE);
    while (I2Cx->CR2 & I2C_CR2_STOP && tout());
  }

  I2C_AcknowledgeConfig(I2Cx, I2C_ACK_CURR);
  I2C_GenerateSTART(I2Cx, ENABLE);
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if(tout()) {
    I2C_Send7bitAddress(I2Cx, (u8)(slave << 1), I2C_DIRECTION_TX);
  }
  // while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && tout());
  while(!(I2Cx->SR1 & I2C_SR1_ADDR) && tout()); 				// test EV6 - wait for address ack (ADDR)

  I2Cx->SR3;
  while(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXEMPTY) && tout());

  if (tout()) {
    I2C_SendData(I2Cx, reg);
  }
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && tout());

  I2C_GenerateSTART(I2Cx, ENABLE);
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if(tout()) {
    I2C_Send7bitAddress(I2Cx, (u8)(slave << 1), I2C_DIRECTION_RX);
  }
  while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && tout());

  if (rxlen > 2) {
    I2Cx->SR3;                                            // ADDR clearing sequence

    while (rxlen > 3 && tout()) {
      while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
      *rxbuff++ = I2C_ReceiveData(I2Cx);
      --rxlen;
    }

    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
    I2C_AcknowledgeConfig(I2Cx, I2C_ACK_NONE);
    disableInterrupts();                               // Errata workaround (Disable interrupt)
    *rxbuff++ = I2C_ReceiveData(I2Cx);
    I2C_GenerateSTOP(I2Cx, ENABLE);
    *rxbuff++ = I2C_ReceiveData(I2Cx);
    enableInterrupts();		                             // Errata workaround (Enable interrupt)

    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNOTEMPTY) == RESET && tout());
    *rxbuff++ = I2C_ReceiveData(I2Cx);

  } else if(rxlen == 2) {
    I2C_AcknowledgeConfig(I2Cx, I2C_ACK_NEXT);
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2Cx->SR3;                                       	// Clear ADDR Flag
    I2C_AcknowledgeConfig(I2Cx, I2C_ACK_NONE);
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2C_GenerateSTOP(I2Cx, ENABLE);
    *rxbuff++ = I2C_ReceiveData(I2Cx);
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    *rxbuff++ = I2C_ReceiveData(I2Cx);
  } else {
    I2C_AcknowledgeConfig(I2Cx, I2C_ACK_NONE);
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2Cx->SR3;                                       	// Clear ADDR Flag
    I2C_GenerateSTOP(I2Cx, ENABLE);
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNOTEMPTY) == RESET && tout());
    *rxbuff = I2C_ReceiveData(I2Cx);
  }

  while((I2Cx->CR2 & I2C_CR2_STOP) && tout());
  I2C_AcknowledgeConfig(I2Cx, I2C_ACK_CURR);

  return tout();
}

u16 I2C_writeRegister(I2C_TypeDef* I2Cx, u8 slave, u8 reg, u8 *txbuff, u8 txlen) {
  while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSBUSY) && tout()) {
    I2C_GenerateSTOP(I2Cx, ENABLE);
    while (I2Cx->CR2 & I2C_CR2_STOP && tout());
  }

  I2C_GenerateSTART(I2Cx, ENABLE);
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if (tout()) {
    I2C_Send7bitAddress(I2Cx, (u8)(slave << 1), I2C_DIRECTION_TX);
  }

  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && tout());

  I2Cx->SR3;

  while(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXEMPTY) && tout());
  if (tout()) {
    I2C_SendData(I2Cx, reg);
  }
  if (txlen) {
    while (txlen--) {
      while(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXEMPTY) && tout());
      I2C_SendData(I2Cx, *txbuff++);
    }
  }
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && tout());

  I2C_GenerateSTOP(I2Cx, ENABLE);
  while (I2Cx->CR2 & I2C_CR2_STOP && tout());

  return tout();
}

void main(void) {
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);

  CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);

  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_2, ENABLE); // RX
  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_3, ENABLE); // TX

  USART_Init(USART1, (uint32_t)9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,
              USART_Mode_Rx | USART_Mode_Tx);

  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);

  TIM4_DeInit();
  TIM4_TimeBaseInit(TIM4_Prescaler_128, 124);
  TIM4_ClearFlag(TIM4_FLAG_Update);
  TIM4_ITConfig(TIM4_IT_Update, ENABLE);

  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);

  enableInterrupts();
  TIM4_Cmd(ENABLE);

  while (1) {
    GPIO_ToggleBits(GPIOB, GPIO_Pin_0);
    delay(50);
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line) {
  (void) file;
  (void) line;
  printf("Wrong parameters value: file %s on line %ld\r\n", file, line);
  while (1);
}

#endif
