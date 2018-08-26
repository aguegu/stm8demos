#include <stdio.h>
#include <string.h>
#include "stm8l15x.h"
#include "stm8l15x_it.h"

// SCK: PB5
// MOSI: PB6
// MISO: PB7
// CE: PB1
// CSN: PB2

#define NRF_CONFIG  0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define NRF_STATUS  0x07
#define OBSERVE_TX  0x08
#define CD          0x09
#define RX_ADDR_P0  0x0A
#define RX_ADDR_P1  0x0B
#define RX_ADDR_P2  0x0C
#define RX_ADDR_P3  0x0D
#define RX_ADDR_P4  0x0E
#define RX_ADDR_P5  0x0F
#define TX_ADDR     0x10
#define RX_PW_P0    0x11
#define RX_PW_P1    0x12
#define RX_PW_P2    0x13
#define RX_PW_P3    0x14
#define RX_PW_P4    0x15
#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17
#define DYNPD	    	0x1C
#define FEATURE	    0x1D

#define FLUSH_TX      0xE1
#define FLUSH_RX      0xE2

#define R_RX_PL_WID   0x60
#define R_RX_PAYLOAD  0x61
#define W_TX_PAYLOAD  0xA0
#define W_ACK_PAYLOAD 0xA8

#define MAX_TX  		0x10
#define TX_OK   		0x20
#define RX_OK   		0x40


__IO uint32_t TimingDelay;

void csn(BitAction val) {
	GPIO_WriteBit(GPIOB, GPIO_Pin_2, val);
}

void ce(BitAction val) {
	GPIO_WriteBit(GPIOB, GPIO_Pin_1, val);
}

void Delay(__IO uint32_t nTime) {
  TimingDelay = nTime;
  while (TimingDelay) {
    wfi();
  }
}

void DelayMicroseconds(__IO uint32_t nTime) {
	nTime >>= 1;
	while(nTime--);
}

void blink() {
	GPIO_ToggleBits(GPIOB, GPIO_Pin_0);
}

int putchar (int c) {
  USART_SendData8(USART1, c);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
  return (c);
}


// void nRF24_WriteReg(uint8_t reg, uint8_t value) {
//   csn(LOW);
//   SPI1_SendRecv(nRF24_CMD_WREG | reg); // Select register
//   SPI1_SendRecv(value); // Write value to register
// 	csnHigh();
// }
//
// uint8_t nRF24_ReadReg(uint8_t reg) {
//   uint8_t value;
//
//   CSN_L();
//   SPI1_SendRecv(reg & 0x1f); // Select register to read from
//   value = SPI1_SendRecv(nRF24_CMD_NOP); // Read register value
//   CSN_H();
//
//   return value;
// }

u8 spi_transfer(u8 dt) {
  uint8_t status;

  while (!SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE));
  SPI_SendData(SPI1, dt);

  while (!SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE));
  status = SPI_ReceiveData(SPI1);

  return status;
}

void spi_transfer_n(u8 *out, u8 length, u8 *in) {
	while (length--) {
		u8 val = spi_transfer(*out++);
		if (in)
			*in++ = val;
	}
}

void command(u8 cmd) {
	csn(0);
	spi_transfer(cmd);
	csn(1);
}

void read_register_n(u8 reg, u8 * data, u8 length) {
	csn(0);
	spi_transfer(reg);
	while (length--) {
		*data++ = spi_transfer(0xff);
	}
	csn(1);
}

u8 read_register(u8 reg) {
	u8 data;
	read_register_n(reg, &data, 1);
	return data;
}

void write_register_n(u8 addr, u8 * data, u8 length) {
	csn(0);
	spi_transfer(0x20 | addr);
	spi_transfer_n(data, length, NULL);
	csn(1);
}

void write_register(u8 addr, u8 val) {
	csn(0);
	spi_transfer(0x20 | addr);
	spi_transfer(val);
	csn(1);
}

void read_status() {
	printf("\r\n");
	for (u8 i = 0; i < 0x18; i++) {
		if (i == 0x0a || i == 0x0b || i == 0x10) {
			u8 dt[5];
			read_register_n(i, dt, 5);
			printf("%02x: ", i);
			for (u8 j = 0; j < 5; j++) {
				printf("%02x ", dt[j]);
			}
			printf("\r\n");
		} else {
			u8 dt = read_register(i);
			printf("%02x: %02x\r\n", i, dt);
		}
	}
}

void init(u8 channel) {
	u8 status;
	ce(0);
	write_register(NRF_CONFIG, 0x08); // Power Down
	command(FLUSH_TX);
	command(FLUSH_RX);
	write_register(RF_CH, channel);
	write_register(RF_SETUP, 0x0f);
	write_register(SETUP_AW, 0x03);

	write_register(FEATURE, 0x04); // dynamic payload

	status = read_register(RF_SETUP);
	printf("status: %02x\r\n", status);
}

void setup_tx() {
	ce(0);
	write_register(NRF_CONFIG, 0x0e); // power on and ptx
	Delay(2);
}

void setup_rx() {
	ce(0);
	write_register(NRF_CONFIG, 0x0f); // power on and prx
	ce(1);
}

void initMaster() {
	ce(0);
	write_register(DYNPD, 0x3e);
	write_register(EN_AA, 0x3e);
	write_register(EN_RXADDR, 0x3e);
	write_register(SETUP_RETR, 0x1f);	// 500us * 15tnimes
}

void initSlave() {
	ce(0);
	write_register(DYNPD, 0x01);
	write_register(EN_AA, 0x01);
	write_register(EN_RXADDR, 0x01);

	write_register(SETUP_RETR, 0x3f);

	setup_rx();
}


u8 send(u8 *data, u8 length) {
	u8 status;

	csn(0);
	spi_transfer(W_TX_PAYLOAD);
	spi_transfer_n(data, length, NULL);

	csn(1);

	ce(1);
	DelayMicroseconds(10);
	ce(0);

	while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3));

	status = read_register(NRF_STATUS);
	write_register(NRF_STATUS, status);

 	if (status & MAX_TX) {
 		command(FLUSH_TX);
	}

	setup_rx();

	return status;
}

u8 master_send(u8 * to, u8 * data, u8 length) {
	u8 result;
	ce(0);
	write_register_n(TX_ADDR, to, 5);
	write_register_n(RX_ADDR_P0, to, 5);
	write_register(DYNPD, 0x01);
	write_register(EN_AA, 0x01);
	write_register(EN_RXADDR, 0x01);

	setup_tx();
	result = send(data, length);

	write_register(DYNPD, 0x3e);
	write_register(EN_AA, 0x3e);
	write_register(EN_RXADDR, 0x3e);
	setup_rx();

	return result;
}

u8 recv(u8 *data) {
	u8 status, len;
	if (!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3)) {

		status = read_register(NRF_STATUS);
		write_register(NRF_STATUS, status);

		if (status & RX_OK) {
			read_register_n(R_RX_PL_WID, &len, 1);

			if (len > 32) {
				return 0;
			}
			read_register_n(R_RX_PAYLOAD, data, len);
			command(FLUSH_RX);
			return len;
		}
	}
	return 0;
}


void main(void) {
	u8 address_master[6] = "1Node";
	u8 address_slave[6] = "2Node";
	u8 data[33];
	u8 len = 1;
	u8 status;
	bool role = TRUE;

	CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Slow);

  CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, ENABLE);
  GPIO_ExternalPullUpConfig(GPIOB, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, ENABLE);

	CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_2 | GPIO_Pin_3, ENABLE); // UART RX/TX
	USART_Init(USART1, (uint32_t)115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,
              USART_Mode_Rx | USART_Mode_Tx);

  SPI_Init(SPI1, SPI_FirstBit_MSB, SPI_BaudRatePrescaler_2, SPI_Mode_Master,
           SPI_CPOL_Low, SPI_CPHA_1Edge, SPI_Direction_2Lines_FullDuplex,
           SPI_NSS_Soft, 0x07);

  GPIO_Init(GPIOB, GPIO_Pin_1, GPIO_Mode_Out_PP_Low_Slow);	// CE
  GPIO_Init(GPIOB, GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Slow);	// CSN
	GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_In_FL_No_IT);			// IRQ

  SPI_Cmd(SPI1, ENABLE);

	CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
	TIM4_DeInit();
	TIM4_TimeBaseInit(TIM4_Prescaler_128, 124);
	TIM4_ClearFlag(TIM4_FLAG_Update);
	TIM4_ITConfig(TIM4_IT_Update, ENABLE);

	enableInterrupts();
	TIM4_Cmd(ENABLE);

	printf("\r\nNRF24L01+ demo on stm8l152.\r\n");

	printf("NULL: %x\r\n", NULL);

	read_status();

	init(0x02);

	if (role) {
		write_register_n(RX_ADDR_P1, address_master, 5);
		initMaster();
	} else {
		write_register_n(TX_ADDR, address_slave, 5);
		write_register_n(RX_ADDR_P0, address_slave, 5);
		initSlave();
	}

	read_status();

  while (1) {
		if (role) {
			memset(data, len, len);
			printf("send: ");
			for (u8 i=0; i<len; i++) {
				printf("%02x ", data[i]);
			}
			printf("\r\n");
			status = master_send(address_slave, data, len);
			printf("status: %02x\r\n", status);
			if (len++ >= 32) {
				len = 1;
			}
			Delay(1000);
		} else {
			len = recv(data);
			if (len) {
				printf("recv: ");
				for (u8 i=0; i<len; i++) {
					printf("%02x ", data[i]);
				}
				printf("\r\n");
			}
		}
		blink();
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line) {
  (void) file;
  (void) line;
	printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  while (1);
}

#endif
