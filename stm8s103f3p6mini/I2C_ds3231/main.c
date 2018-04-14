#include <stdio.h>
#include "stm8s.h"
#include "stm8s_it.h"

#define USE_FULL_ASSERT    (1)

#define FAST_I2C_MODE  (1)
#define SLAVE_ADDRESS (0x68)

// TEST LED between vcc and PB5
// Oscilloscope on PA3
// UART1_TX on PD5
// UART1_RX on PD6

// void ErrProc(void);

/* flag clearing sequence - uncoment next for peripheral clock under 2MHz */
#define dead_time() { /* _asm("nop"); _asm("nop"); */ }
#define delay(a)          { TIM4_tout= a; while(TIM4_tout); }
#define tout()            (TIM4_tout)
#define set_tout_ms(a)    { TIM4_tout= a; }

volatile u8 err_save;
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
  GPIOE->ODR |= 6;                //define SDA, SCL outputs, HiZ, Open drain, Fast
  GPIOE->DDR |= 6;
  GPIOE->CR2 |= 6;

  I2C->FREQR = 16;               // input clock to I2C - 16MHz
  I2C->CCRL = 15;                // 900/62.5= 15, (SCLhi must be at least 600+300=900ns!)
  I2C->CCRH = 0x80;              // fast mode, duty 2/1 (bus speed 62.5*3*15~356kHz)
  I2C->TRISER = 5;               // 300/62.5 + 1= 5  (maximum 300ns)

  I2C->OARL = 0xA0;              // own address A0;
  I2C->OARH |= 0x40;
  I2C->ITR = 1;                  // enable error interrupts
  I2C->CR2 |= 0x04;              // ACK=1, Ack enable
  I2C->CR1 |= 0x01;              // PE=1
}

void I2C_readRegister(u8 u8_regAddr, u8 u8_NumByteToRead, u8 *u8_DataBuffer)
{
  /*--------------- BUSY? -> STOP request ---------------------*/
	while(I2C->SR3 & I2C_SR3_BUSY  &&  tout())	  				// Wait while the bus is busy
  {
		I2C->CR2 |= I2C_CR2_STOP;                   				// Generate stop here (STOP=1)
    while(I2C->CR2 & I2C_CR2_STOP  &&  tout()); 				// Wait until stop is performed
	}
  I2C->CR2 |= I2C_CR2_ACK;                      				// ACK=1, Ack enable
  /*--------------- Start communication -----------------------*/
  I2C->CR2 |= I2C_CR2_START;                    				// START=1, generate start
  while((I2C->SR1 & I2C_SR1_SB)==0  &&  tout());				// Wait for start bit detection (SB)
  /*------------------ Address send ---------------------------*/
  if(tout())
  {
    #ifdef TEN_BITS_ADDRESS
      I2C->DR = (u8)(((SLAVE_ADDRESS >> 7) & 6) | 0xF0);// Send header of 10-bit device address (R/W = 0)
      while(!(I2C->SR1 & I2C_SR1_ADD10) &&  tout());		// Wait for header ack (ADD10)
      if(tout())
      {
        I2C->DR = (u8)(SLAVE_ADDRESS);                	// Send lower 8-bit device address & Write
      }
    #else
      I2C->DR = (u8)(SLAVE_ADDRESS << 1);   						// Send 7-bit device address & Write (R/W = 0)
    #endif // TEN_BITS_ADDRESS
  }
  while(!(I2C->SR1 & I2C_SR1_ADDR) &&  tout()); 				// test EV6 - wait for address ack (ADDR)
  dead_time();                                  				// ADDR clearing sequence
  I2C->SR3;
  /*--------------- Register/Command send ----------------------*/
  while(!(I2C->SR1 & I2C_SR1_TXE) &&  tout());  				// Wait for TxE
  if(tout())
  {
    I2C->DR = u8_regAddr;                         			// Send register address
  }                                            					// Wait for TxE & BTF
  while((I2C->SR1 & (I2C_SR1_TXE | I2C_SR1_BTF)) != (I2C_SR1_TXE | I2C_SR1_BTF)  &&  tout());
  dead_time();                                  				// clearing sequence
  /*-------------- Stop/Restart communication -------------------*/
  #ifndef TEN_BITS_ADDRESS
    #ifdef NO_RESTART																		// if 7bit address and NO_RESTART setted
      I2C->CR2 |= I2C_CR2_STOP;                     		// STOP=1, generate stop
      while(I2C->CR2 & I2C_CR2_STOP  &&  tout());   		// wait until stop is performed
    #endif // NO_RESTART
  #endif // TEN_BITS_ADDRESS
  /*--------------- Restart communication ---------------------*/
  I2C->CR2 |= I2C_CR2_START;                     				// START=1, generate re-start
  while((I2C->SR1 & I2C_SR1_SB)==0  &&  tout()); 				// Wait for start bit detection (SB)
  /*------------------ Address send ---------------------------*/
  if(tout())
  {
    #ifdef TEN_BITS_ADDRESS
      I2C->DR = (u8)(((SLAVE_ADDRESS >> 7) & 6) | 0xF1);// send header of 10-bit device address (R/W = 1)
      #ifdef NO_RESTART
        while(!(I2C->SR1 & I2C_SR1_ADD10) &&  tout());	// Wait for header ack (ADD10)
        if(tout())
        {
          I2C->DR = (u8)(SLAVE_ADDRESS);                // Send lower 8-bit device address & Write
        }
      #endif // NO_RESTART
    #else
      I2C->DR = (u8)(SLAVE_ADDRESS << 1) | 1;         	// Send 7-bit device address & Write (R/W = 1)
    #endif  // TEN_BITS_ADDRESS
  }
  while(!(I2C->SR1 & I2C_SR1_ADDR)  &&  tout());  			// Wait for address ack (ADDR)
  /*------------------- Data Receive --------------------------*/
  if (u8_NumByteToRead > 2)                 						// *** more than 2 bytes are received? ***
  {
    I2C->SR3;                                     			// ADDR clearing sequence
    while(u8_NumByteToRead > 3  &&  tout())       			// not last three bytes?
    {
      while(!(I2C->SR1 & I2C_SR1_BTF)  &&  tout()); 				// Wait for BTF
			*u8_DataBuffer++ = I2C->DR;                   				// Reading next data byte
      --u8_NumByteToRead;																		// Decrease Numbyte to reade by 1
    }
																												//last three bytes should be read
    while(!(I2C->SR1 & I2C_SR1_BTF)  &&  tout()); 			// Wait for BTF
    I2C->CR2 &=~I2C_CR2_ACK;                      			// Clear ACK
    disableInterrupts();                          			// Errata workaround (Disable interrupt)
    *u8_DataBuffer++ = I2C->DR;                     		// Read 1st byte
    I2C->CR2 |= I2C_CR2_STOP;                       		// Generate stop here (STOP=1)
    *u8_DataBuffer++ = I2C->DR;                     		// Read 2nd byte
    enableInterrupts();																	// Errata workaround (Enable interrupt)
    while(!(I2C->SR1 & I2C_SR1_RXNE)  &&  tout());			// Wait for RXNE
    *u8_DataBuffer++ = I2C->DR;                   			// Read 3rd Data byte
  }
  else
  {
   if(u8_NumByteToRead == 2)                						// *** just two bytes are received? ***
    {
      I2C->CR2 |= I2C_CR2_POS;                      		// Set POS bit (NACK at next received byte)
      disableInterrupts();                          		// Errata workaround (Disable interrupt)
      I2C->SR3;                                       	// Clear ADDR Flag
      I2C->CR2 &=~I2C_CR2_ACK;                        	// Clear ACK
      enableInterrupts();																// Errata workaround (Enable interrupt)
      while(!(I2C->SR1 & I2C_SR1_BTF)  &&  tout()); 		// Wait for BTF
      disableInterrupts();                          		// Errata workaround (Disable interrupt)
      I2C->CR2 |= I2C_CR2_STOP;                       	// Generate stop here (STOP=1)
      *u8_DataBuffer++ = I2C->DR;                     	// Read 1st Data byte
      enableInterrupts();																// Errata workaround (Enable interrupt)
			*u8_DataBuffer = I2C->DR;													// Read 2nd Data byte
    }
    else                                      					// *** only one byte is received ***
    {
      I2C->CR2 &=~I2C_CR2_ACK;;                     		// Clear ACK
      disableInterrupts();                          		// Errata workaround (Disable interrupt)
      I2C->SR3;                                       	// Clear ADDR Flag
      I2C->CR2 |= I2C_CR2_STOP;                       	// generate stop here (STOP=1)
      enableInterrupts();																// Errata workaround (Enable interrupt)
      while(!(I2C->SR1 & I2C_SR1_RXNE)  &&  tout()); 		// test EV7, wait for RxNE
      *u8_DataBuffer = I2C->DR;                     		// Read Data byte
    }
  }
  /*--------------- All Data Received -----------------------*/
  while((I2C->CR2 & I2C_CR2_STOP)  &&  tout());     		// Wait until stop is performed (STOPF = 1)
  I2C->CR2 &=~I2C_CR2_POS;                          		// return POS to default state (POS=0)
}

void I2C_writeRegister(u8 u8_regAddr, u8 u8_NumByteToWrite, u8 *u8_DataBuffer)
{
  while((I2C->SR3 & 2) && tout())       									// Wait while the bus is busy
  {
    I2C->CR2 |= 2;                        								// STOP=1, generate stop
    while((I2C->CR2 & 2) && tout());      								// wait until stop is performed
  }

  I2C->CR2 |= 1;                        									// START=1, generate start
  while(((I2C->SR1 & 1)==0) && tout()); 									// Wait for start bit detection (SB)
  dead_time();                          									// SB clearing sequence
  if(tout())
  {
    #ifdef TEN_BITS_ADDRESS															  // TEN_BIT_ADDRESS decalred in I2c_master_poll.h
      I2C->DR = (u8)(((SLAVE_ADDRESS >> 7) & 6) | 0xF0);  // Send header of 10-bit device address (R/W = 0)
      while(!(I2C->SR1 & 8) &&  tout());    							// Wait for header ack (ADD10)
      if(tout())
      {
        I2C->DR = (u8)(SLAVE_ADDRESS);        						// Send lower 8-bit device address & Write
      }
    #else
      I2C->DR = (u8)(SLAVE_ADDRESS << 1);   							// Send 7-bit device address & Write (R/W = 0)
    #endif
  }
  while(!(I2C->SR1 & 2) && tout());     									// Wait for address ack (ADDR)
  dead_time();                          									// ADDR clearing sequence
  I2C->SR3;
  while(!(I2C->SR1 & 0x80) && tout());  									// Wait for TxE
  if(tout())
  {
    I2C->DR = u8_regAddr;                 								// send Offset command
  }
  if(u8_NumByteToWrite)
  {
    while(u8_NumByteToWrite--)
    {																											// write data loop start
      while(!(I2C->SR1 & 0x80) && tout());  								// test EV8 - wait for TxE
      I2C->DR = *u8_DataBuffer++;           								// send next data byte
    }																											// write data loop end
  }
  while(((I2C->SR1 & 0x84) != 0x84) && tout()); 					// Wait for TxE & BTF
  dead_time();                          									// clearing sequence

  I2C->CR2 |= 2;                        									// generate stop here (STOP=1)
  while((I2C->CR2 & 2) && tout());      									// wait until stop is performed
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

  // if (tout()) {
    set_tout_ms(10);
    I2C_writeRegister(0x0e, 1, &x);
  // }
  x = 0x00;
  // if (tout()) {
    set_tout_ms(10);
    I2C_writeRegister(0x0f, 1, &x);
  // }

  while (1) {
    // GPIO_WriteReverse(GPIOA, GPIO_PIN_3);

    // if(tout()) {
      set_tout_ms(10);
      I2C_readRegister(0, 3, dt);
    // }

    printf("hello, world. %02x:%02x:%02x\r\n", dt[2], dt[1], dt[0]);
    delay(1000);
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  printf("Wrong parameters value: file %s on line %ld\r\n", file, line);
  while (1);
}

#endif
