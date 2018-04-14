#include <stdio.h>
#include "stm8s.h"
#include "stm8s_it.h"

#define USE_FULL_ASSERT    (1)

#define FAST_I2C_MODE  (1)
#define SLAVE_ADDRESS (0x68)

// TEST LED between vcc and PB5, no use
// Oscilloscope on PA3
// UART1_TX on PD5
// UART1_RX on PD6
// I2C SDA on PB5
// I2C SCL on PB4

// void ErrProc(void);

#define delay(a)          { TIM4_tout= a; while(TIM4_tout); }
#define tout()            (TIM4_tout)
#define set_tout_ms(a)    { TIM4_tout= a; }

volatile u16 TIM4_tout = 0;
u8 dt[8];

int putchar (int c) {
  UART1_SendData8(c);
  while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
  return (c);
}

// void TIM4_init (void) {
//   TIM4->ARR = 0x80;                // init timer 4 1ms inetrrupts
//   TIM4->PSCR= 7;
//   TIM4->IER = 1;
//   TIM4->CR1 |= 1;
// }

void I2C_init(void) {
  //define SDA, SCL outputs, HiZ, Open drain, Fast
  GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_OUT_OD_HIZ_FAST);
  GPIO_Init(GPIOB, GPIO_PIN_4, GPIO_MODE_OUT_OD_HIZ_FAST);

  I2C_Init(400000, 0xA0, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 16);
  I2C_ITConfig(I2C_IT_ERR, ENABLE);
}

void I2C_readRegister(u8 u8_regAddr, u8 u8_NumByteToRead, u8 *u8_DataBuffer) {
  /*--------------- BUSY? -> STOP request ---------------------*/
	// while(I2C->SR3 & I2C_SR3_BUSY && tout())	 				// Wait while the bus is busy
  // {
	// 	I2C->CR2 |= I2C_CR2_STOP;                   				// Generate stop here (STOP=1)
  //   while(I2C->CR2 & I2C_CR2_STOP && tout()); 				// Wait until stop is performed
	// }
  while (I2C_GetFlagStatus(I2C_FLAG_BUSBUSY) && tout()) {
    I2C_GenerateSTOP(ENABLE);
    while (I2C->CR2 & I2C_CR2_STOP && tout());
  }

  // I2C->CR2 |= I2C_CR2_ACK;                      				// ACK=1, Ack enable
  I2C_AcknowledgeConfig(I2C_ACK_CURR);

  /*--------------- Start communication -----------------------*/
  // I2C->CR2 |= I2C_CR2_START;                    				// START=1, generate start
  // while((I2C->SR1 & I2C_SR1_SB)==0  &&  tout());				// Wait for start bit detection (SB)

  I2C_GenerateSTART(ENABLE);
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && tout());

  /*------------------ Address send ---------------------------*/
  if(tout()) {
    // I2C->DR = (u8)(SLAVE_ADDRESS << 1);   						// Send 7-bit device address & Write (R/W = 0)
    I2C_Send7bitAddress((u8)(SLAVE_ADDRESS << 1), I2C_DIRECTION_TX);
  }
  // while(!(I2C->SR1 & I2C_SR1_ADDR) &&  tout()); 				// test EV6 - wait for address ack (ADDR)
  I2C->SR3;
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && tout());
  /*--------------- Register/Command send ----------------------*/
  // while(!(I2C->SR1 & I2C_SR1_TXE) &&  tout());  				// Wait for TxE
  while(!I2C_GetFlagStatus(I2C_FLAG_TXEMPTY) && tout());
  if (tout()) {
    // I2C->DR = u8_regAddr;                         			// Send register address
    I2C_SendData(u8_regAddr);
  }                                            					// Wait for TxE & BTF
  // while((I2C->SR1 & (I2C_SR1_TXE | I2C_SR1_BTF)) != (I2C_SR1_TXE | I2C_SR1_BTF)  &&  tout());
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) && tout());

  /*--------------- Restart communication ---------------------*/
  // I2C->CR2 |= I2C_CR2_START;                     				// START=1, generate re-start
  // while((I2C->SR1 & I2C_SR1_SB)==0  &&  tout()); 				// Wait for start bit detection (SB)
  I2C_GenerateSTART(ENABLE);
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && tout());
  /*------------------ Address send ---------------------------*/
  if(tout()) {
    // I2C->DR = (u8)(SLAVE_ADDRESS << 1) | 1;         	// Send 7-bit device address & Write (R/W = 1)
    I2C_Send7bitAddress((u8)(SLAVE_ADDRESS << 1), I2C_DIRECTION_RX);
  }
  // while(!(I2C->SR1 & I2C_SR1_ADDR)  &&  tout());  			// Wait for address ack (ADDR)
  while (!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && tout());
  /*------------------- Data Receive --------------------------*/
  if (u8_NumByteToRead > 2)                 						// *** more than 2 bytes are received? ***
  {
    I2C->SR3;                                     			// ADDR clearing sequence
    // while(u8_NumByteToRead > 3  &&  tout())       			// not last three bytes?
    // {
    //   while(!(I2C->SR1 & I2C_SR1_BTF)  &&  tout()); 				// Wait for BTF
		// 	*u8_DataBuffer++ = I2C->DR;                   				// Reading next data byte
    //   --u8_NumByteToRead;																		// Decrease Numbyte to reade by 1
    // }
    while (u8_NumByteToRead > 3 && tout()) {
      while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
      *u8_DataBuffer++ = I2C_ReceiveData();
      --u8_NumByteToRead;																		// Decrease Numbyte to reade by 1
    }
																												//last three bytes should be read
    // while(!(I2C->SR1 & I2C_SR1_BTF)  &&  tout()); 			// Wait for BTF
    while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
    // I2C->CR2 &=~I2C_CR2_ACK;                      			// Clear ACK
    I2C_AcknowledgeConfig(I2C_ACK_NONE);
    disableInterrupts();                          			// Errata workaround (Disable interrupt)
    // *u8_DataBuffer++ = I2C->DR;                     		// Read 1st byte
    *u8_DataBuffer++ = I2C_ReceiveData();
    // I2C->CR2 |= I2C_CR2_STOP;                       		// Generate stop here (STOP=1)
    I2C_GenerateSTOP(ENABLE);
    // *u8_DataBuffer++ = I2C->DR;                     		// Read 2nd byte
    *u8_DataBuffer++ = I2C_ReceiveData();
    enableInterrupts();																	// Errata workaround (Enable interrupt)

    // while(!(I2C->SR1 & I2C_SR1_RXNE)  &&  tout());			// Wait for RXNE
    while(I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY) == RESET && tout());
    // *u8_DataBuffer++ = I2C->DR;                   			// Read 3rd Data byte
    *u8_DataBuffer++ = I2C_ReceiveData();
  }
  else if(u8_NumByteToRead == 2)                						// *** just two bytes are received? ***
  {
    // I2C->CR2 |= I2C_CR2_POS;                      		// Set POS bit (NACK at next received byte)
    I2C_AcknowledgeConfig(I2C_ACK_NEXT);
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2C->SR3;                                       	// Clear ADDR Flag
    // I2C->CR2 &=~I2C_CR2_ACK;                        	// Clear ACK
    I2C_AcknowledgeConfig(I2C_ACK_NONE);
    enableInterrupts();																// Errata workaround (Enable interrupt)
    // while(!(I2C->SR1 & I2C_SR1_BTF)  &&  tout()); 		// Wait for BTF
    while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET && tout());
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    // I2C->CR2 |= I2C_CR2_STOP;
    I2C_GenerateSTOP(ENABLE);                       	// Generate stop here (STOP=1)
    // *u8_DataBuffer++ = I2C->DR;                     	// Read 1st Data byte
    *u8_DataBuffer++ = I2C_ReceiveData();
    enableInterrupts();																// Errata workaround (Enable interrupt)
		// *u8_DataBuffer = I2C->DR;													// Read 2nd Data byte
    *u8_DataBuffer++ = I2C_ReceiveData();
  }
  else                                      					// *** only one byte is received ***
  {
    // I2C->CR2 &=~I2C_CR2_ACK;;                     		// Clear ACK
    I2C_AcknowledgeConfig(I2C_ACK_NONE);
    disableInterrupts();                          		// Errata workaround (Disable interrupt)
    I2C->SR3;                                       	// Clear ADDR Flag
    // I2C->CR2 |= I2C_CR2_STOP;                       	// generate stop here (STOP=1)
    I2C_GenerateSTOP(ENABLE);                       	// Generate stop here (STOP=1)
    enableInterrupts();																// Errata workaround (Enable interrupt)
    // while(!(I2C->SR1 & I2C_SR1_RXNE)  &&  tout()); 		// test EV7, wait for RxNE
    // *u8_DataBuffer = I2C->DR;                     		// Read Data byte
    while(I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY) == RESET && tout());
    *u8_DataBuffer = I2C_ReceiveData();
  }

  /*--------------- All Data Received -----------------------*/
  while((I2C->CR2 & I2C_CR2_STOP) && tout());     		// Wait until stop is performed (STOPF = 1)
  // I2C->CR2 &=~I2C_CR2_POS;                          		// return POS to default state (POS=0)
  I2C_AcknowledgeConfig(I2C_ACK_CURR);
}

void I2C_writeRegister(u8 u8_regAddr, u8 u8_NumByteToWrite, u8 *u8_DataBuffer) {
  // while((I2C->SR3 & 2) && tout())       									// Wait while the bus is busy
  // {
  //   I2C->CR2 |= 2;                        								// STOP=1, generate stop
  //   while((I2C->CR2 & 2) && tout());      								// wait until stop is performed
  // }
  while (I2C_GetFlagStatus(I2C_FLAG_BUSBUSY) && tout()) {
    I2C_GenerateSTOP(ENABLE);
    while (I2C->CR2 & I2C_CR2_STOP && tout());
  }

  // I2C->CR2 |= 1;                        									// START=1, generate start
  // while(((I2C->SR1 & 1)==0) && tout()); 									// Wait for start bit detection (SB)
  I2C_GenerateSTART(ENABLE);
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && tout());

  if (tout()) {
    // I2C->DR = (u8)(SLAVE_ADDRESS << 1);   							// Send 7-bit device address & Write (R/W = 0)
    I2C_Send7bitAddress((u8)(SLAVE_ADDRESS << 1), I2C_DIRECTION_TX);
  }

  // while(!(I2C->SR1 & 2) && tout());     									// Wait for address ack (ADDR)
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && tout());

  I2C->SR3;
  // while(!(I2C->SR1 & 0x80) && tout());  									// Wait for TxE
  while(!I2C_GetFlagStatus(I2C_FLAG_TXEMPTY) && tout());
  if (tout()) {
    // I2C->DR = u8_regAddr;                 								// send Offset command
    I2C_SendData(u8_regAddr);
  }
  if(u8_NumByteToWrite)
  {
    while(u8_NumByteToWrite--)
    {																											// write data loop start
      // while(!(I2C->SR1 & 0x80) && tout());  								// test EV8 - wait for TxE
      // I2C->DR = *u8_DataBuffer++;           								// send next data byte
      while(!I2C_GetFlagStatus(I2C_FLAG_TXEMPTY) && tout());
      I2C_SendData(*u8_DataBuffer++);
    }																											// write data loop end
  }
  // while(((I2C->SR1 & 0x84) != 0x84) && tout()); 					// Wait for TxE & BTF
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) && tout());

  // I2C->CR2 |= 2;                        									// generate stop here (STOP=1)
  I2C_GenerateSTOP(ENABLE);                       	// Generate stop here (STOP=1)
  // while((I2C->CR2 & 2) && tout());      									// wait until stop is performed
  while (I2C->CR2 & I2C_CR2_STOP && tout());
}

void main(void) {
  uint8_t x = 0x80;
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  GPIO_Init(GPIOA, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_SLOW);

  UART1_DeInit();
  UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
              UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);

  // TIM4_init();
  TIM4_DeInit();
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  TIM4_Cmd(ENABLE);

  I2C_init();

  enableInterrupts();

  set_tout_ms(10);
  I2C_writeRegister(0x0e, 1, &x);

  x = 0x00;
  if (tout()) {
    set_tout_ms(10);
    I2C_writeRegister(0x0f, 1, &x);
  }

  while (1) {
    // GPIO_WriteReverse(GPIOA, GPIO_PIN_3);
    set_tout_ms(10);
    I2C_readRegister(0, 1, dt);
    printf("%02x\r\n", dt[0]);

    set_tout_ms(10);
    I2C_readRegister(0, 2, dt);
    printf("%02x:%02x\r\n", dt[1], dt[0]);

    set_tout_ms(10);
    I2C_readRegister(0, 3, dt);
    printf("%02x:%02x:%02x\r\n", dt[2], dt[1], dt[0]);

    set_tout_ms(10);
    I2C_readRegister(0, 7, dt);
    printf("%02x %02x %02x %02x %02x:%02x:%02x\r\n", dt[6], dt[5], dt[4], dt[3], dt[2], dt[1], dt[0]);

    for (u8 i=0; i<7; i++) {
      dt[i] = (dt[0] >> 4) * 10 + (dt[0] & 0x0f);
      dt[i] --;
      if (i == 0) dt[i]--;
      dt[i] = (dt[0] / 10 << 4) + (dt[0] % 10);
    }

    set_tout_ms(10);
    I2C_writeRegister(0x00, 7, dt);

    delay(1000);
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  printf("Wrong parameters value: file %s on line %ld\r\n", file, line);
  while (1);
}

#endif
