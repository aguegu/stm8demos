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

u16 I2C_Read(I2C_TypeDef* I2Cx, u8 slave, u8 reg, u8 *rxbuff, u8 rxlen) {
  while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY) && tout()) {
    I2C_GenerateSTOP(I2Cx, ENABLE);
    while (I2Cx->CR2 & I2C_CR2_STOP && tout());
  }

  I2C_AcknowledgeConfig(I2Cx, ENABLE);
  I2C_GenerateSTART(I2Cx, ENABLE);
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if(tout()) {
    I2C_Send7bitAddress(I2Cx, (u8)(slave << 1), I2C_Direction_Transmitter);
  }
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && tout());

  I2Cx->SR3;
  while(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXE) && tout());

  if (tout()) {
    I2C_SendData(I2Cx, reg);
  }
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && tout());

  I2C_GenerateSTART(I2Cx, ENABLE);
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if(tout()) {
    I2C_Send7bitAddress(I2Cx, (u8)(slave << 1), I2C_Direction_Receiver);
  }
  while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && tout());

  if (rxlen > 2) {
    I2Cx->SR3;                                            // ADDR clearing sequence

    while (rxlen > 3 && tout()) {
      while (!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF) && tout());
      *rxbuff++ = I2C_ReceiveData(I2Cx);
      --rxlen;
    }

    while (!I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF) && tout());
    I2C_AcknowledgeConfig(I2Cx, DISABLE);
    disableInterrupts();                               // Errata workaround (Disable interrupt)
    *rxbuff++ = I2C_ReceiveData(I2Cx);
    I2C_GenerateSTOP(I2Cx, ENABLE);
    *rxbuff++ = I2C_ReceiveData(I2Cx);
    enableInterrupts();		                             // Errata workaround (Enable interrupt)

    while(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE) && tout());
    *rxbuff++ = I2C_ReceiveData(I2Cx);

  } else if(rxlen == 2) {
    // I2C_AcknowledgeConfig(I2Cx, I2C_ACK_NEXT);
    I2C_PECPositionConfig(I2Cx, I2C_PECPosition_Next);
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2Cx->SR3;                                       	// Clear ADDR Flag
    I2C_AcknowledgeConfig(I2Cx, DISABLE);
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BTF) == RESET && tout());
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2C_GenerateSTOP(I2Cx, ENABLE);
    *rxbuff++ = I2C_ReceiveData(I2Cx);
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    *rxbuff++ = I2C_ReceiveData(I2Cx);
  } else {
    I2C_AcknowledgeConfig(I2Cx, DISABLE);
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2Cx->SR3;                                       	// Clear ADDR Flag
    I2C_GenerateSTOP(I2Cx, ENABLE);
    enableInterrupts();	                              // Errata workaround (Enable interrupt)
    while(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE) && tout());
    *rxbuff = I2C_ReceiveData(I2Cx);
  }

  while((I2Cx->CR2 & I2C_CR2_STOP) && tout());
  I2C_AckPositionConfig(I2Cx, I2C_AckPosition_Current);

  return tout();
}

u16 I2C_writeRegister(I2C_TypeDef* I2Cx, u8 slave, u8 reg, u8 *txbuff, u8 txlen) {
  while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY) && tout()) {
    I2C_GenerateSTOP(I2Cx, ENABLE);
    while (I2Cx->CR2 & I2C_CR2_STOP && tout());
  }

  I2C_GenerateSTART(I2Cx, ENABLE);
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if (tout()) {
    I2C_Send7bitAddress(I2Cx, (u8)(slave << 1), I2C_Direction_Transmitter);
  }

  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && tout());

  I2Cx->SR3;

  while(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXE) && tout());
  if (tout()) {
    I2C_SendData(I2Cx, reg);
  }
  if (txlen) {
    while (txlen--) {
      while(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_TXE) && tout());
      I2C_SendData(I2Cx, *txbuff++);
    }
  }
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && tout());

  I2C_GenerateSTOP(I2Cx, ENABLE);
  while (I2Cx->CR2 & I2C_CR2_STOP && tout());

  return tout();
}

void main(void) {
  u8 dt[3];
  s32 t = 0, rh = 0;
  u16 y1, y2;

  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);

  CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
  CLK_PeripheralClockConfig(CLK_Peripheral_I2C1, ENABLE);
  I2C_DeInit(I2C1);
  I2C_Init(I2C1, 400000, 0xA0, I2C_Mode_I2C,
           I2C_DutyCycle_2, I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit);

  I2C_ITConfig(I2C1, I2C_IT_ERR, ENABLE);
  I2C_Cmd(I2C1, ENABLE);

  GPIO_Init(GPIOC, GPIO_Pin_0, GPIO_Mode_Out_OD_HiZ_Fast);  // I2C1 SDA
  GPIO_Init(GPIOC, GPIO_Pin_1, GPIO_Mode_Out_OD_HiZ_Fast);  // I2C1 SCL

  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_2, ENABLE); // UART1 RX
  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_3, ENABLE); // UART1 TX

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

  //  (RELOAD_VALUE + 1) * Prescaler / LSI
  //  = 149 * 256 / 38000 = 1.003
  IWDG_Enable();
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  IWDG_SetReload(148);
  IWDG_ReloadCounter();

  while (1) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_0, SET);
    set_tout_ms(100);
    y1 = I2C_Read(I2C1, 0x40, 0xe3, dt, 3);
    // printf("\r\nrx: %3d:   %02x %02x %02x, %02x", tout(), dt[0], dt[1], dt[2], checkCrc(dt, 2));
    if (y1 && dt[2] == checkCrc(dt, 2)) {
      t = (((int32_t)dt[0] << 8) + dt[1]) & 0xfffc;
      t = ((t * 17572) >> 16) - 4685;
    }
    //
    set_tout_ms(100);
    y2 = I2C_Read(I2C1, 0x40, 0xe5, dt, 3);
    // printf("\r\nrx: %3d:   %02x %02x %02x, %02x", tout(), dt[0], dt[1], dt[2], checkCrc(dt, 2));

    if (y2 && dt[2] == checkCrc(dt, 2)) {
      rh = (((int32_t)dt[0] << 8) + dt[1]) & 0xfffc;
      rh = ((rh * 125) >> 16) - 6;
    }
    GPIO_WriteBit(GPIOB, GPIO_Pin_0, RESET);

    if (y1 && y2) {
      // printf("\r\ny1: %u, y2: %u", y1, y2);
      printf("T: ");
      putFloat(t);
      printf(", RH: ");
      putFloat(rh);
      printf("\r\n");
      IWDG_ReloadCounter();
    }

    delay(500 - y1 - y2);
    delay(500);
    if (y1 && y2) {
      IWDG_ReloadCounter();
    }
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
