#include "stdio.h"
#include "stm8s.h"

#define FLAG_TIMEOUT         ((uint16_t)0x1000)
#define LONG_TIMEOUT         ((uint16_t)(10 * FLAG_TIMEOUT))

// TEST LED between vcc and pb5

int putchar (int c) {
  UART1_SendData8(c);
  while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET);
  return (c);
}

int getchar (void) {
  int c = 0;
  while (UART1_GetFlagStatus(UART1_FLAG_RXNE) == RESET);
  c = UART1_ReceiveData8();
  return c;
}

//////////////////

void i2c_init(void) {
  // CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C, ENABLE);

  // I2C_Cmd(ENABLE);
  I2C_Init(100000, 0xA0, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 16);

}
//
// void i2c_deinit(void) {
//   I2C_Cmd(DISABLE);
//   I2C_DeInit();
//   CLK_PeripheralClockConfig(CLK_PERIPHERAL_I2C, DISABLE);
//
//   GPIO_Init(GPIOB, GPIO_PIN_4, GPIO_MODE_IN_PU_NO_IT);   // SDA
//   GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_IN_PU_NO_IT);   // SCL
// }

///////////

static const uint16_t POLYNOMIAL = 0x131; //P(x)=x^8+x^5+x^4+1 = 100110001

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

void main(void) {
  char ans;
  uint16_t t;
  uint8_t buff[3];
  uint8_t i = 0;
  // char s[] = "hello\r\n";
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  UART1_DeInit();
  UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
              UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);

  i2c_init();

  t = LONG_TIMEOUT;
  while(I2C_GetFlagStatus(I2C_FLAG_BUSBUSY) && --t);
  printf("\r\nBUSBusy %u", t);

  I2C_GenerateSTART(ENABLE);

  t = FLAG_TIMEOUT;
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && --t);
  printf("\r\nMASTER Mode %u", t);

  I2C_Send7bitAddress((uint8_t)0x40 << 1, I2C_DIRECTION_TX);
  t = FLAG_TIMEOUT;
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && --t);
  printf("\r\nMASTER TRANSMITTER %u", t);

  I2C_SendData(0xe3);
  t = FLAG_TIMEOUT;
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --t);
  printf("\r\nMASTER TRANSMITTED %u", t);

  I2C_GenerateSTART(ENABLE);

  t = FLAG_TIMEOUT;
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && --t);
  printf("\r\nMASTER Re-Start %u", t);

  I2C_Send7bitAddress((uint8_t)0x40 << 1, I2C_DIRECTION_RX);
  t = FLAG_TIMEOUT;
  while(!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && --t);
  printf("\r\nMASTER RECEIVER %u", t);

  t = LONG_TIMEOUT;
  while ((I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET) && --t); /* Poll on BTF */
  printf("\r\nMASTER TRANSFER FINISHED %u", t);

  I2C_AcknowledgeConfig(I2C_ACK_NONE);

  disableInterrupts();

  buff[i] = I2C_ReceiveData();  // Read Data N-2

  i++;
  I2C_GenerateSTOP(ENABLE);

  buff[i] = I2C_ReceiveData();  // Read DataN-1

  enableInterrupts();

  i++;
  t = FLAG_TIMEOUT;
  while ((I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY) == RESET) && --t);

  buff[i] = I2C_ReceiveData();

  printf("\r\nrx: %02x %02x %02x", buff[0], buff[1], buff[2]);
  printf("\r\ncrc: %02x", checkCrc(buff, 2));

  while (1) {
    ans = getchar();
    putchar(ans);
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
