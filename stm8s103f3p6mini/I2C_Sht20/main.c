#include "stdio.h"
#include "stm8s.h"

#define FLAG_TIMEOUT         ((uint16_t)0x1000)
#define LONG_TIMEOUT         ((uint16_t)(15 * FLAG_TIMEOUT))

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

void Delay(uint16_t t) {
  while (t--);
}

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

uint8_t i2cMasterTransmit(uint8_t addr, uint8_t * txbuf, uint8_t txbytes, uint8_t * rxbuf, uint8_t rxbytes) {
  uint8_t i = 0;
  uint16_t t;
  uint8_t step = 0;

  do {
    step++;
    for (t = LONG_TIMEOUT; I2C_GetFlagStatus(I2C_FLAG_BUSBUSY) && --t;);
    for (t = LONG_TIMEOUT; I2C_GetFlagStatus(I2C_FLAG_BUSBUSY) && --t;);
    // printf("\r\nMASTER BUSBUSY %u", t);
    if (!t) break;

    step++;
    I2C_GenerateSTART(ENABLE);
    for (t = FLAG_TIMEOUT; !I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && --t;);
    if (!t) break;

    step++;
    I2C_Send7bitAddress(addr << 1, I2C_DIRECTION_TX);
    for (t = FLAG_TIMEOUT; !I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && --t;);
    if (!t) break;
    // printf("\r\nMASTER TRANSMITTER %u", t);

    step++;
    if (txbytes == 1) {
      I2C_SendData(*txbuf);
      for (t = FLAG_TIMEOUT; !I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --t;);
      // printf("\r\nMASTER TRANSMITTED %u", t);
      if (!t) break;
    }

    step++;
    I2C_GenerateSTART(ENABLE);  // Restart
    for (t = FLAG_TIMEOUT; !I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) && --t;);
    // printf("\r\nMASTER Re-Start %u", t);
    if (!t) break;

    step++;
    I2C_Send7bitAddress(addr << 1, I2C_DIRECTION_RX);
    for (t = FLAG_TIMEOUT; !I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && --t;);
    // printf("\r\nMASTER RECEIVER %u", t);
    if (!t) break;

    step++;
    if (rxbytes == 3) {
      for (t = LONG_TIMEOUT; I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET && --t;); /* Poll on BTF */
      if (!t) break;
      // printf("\r\nMASTER TRANSFER FINISHED %u", t);
      I2C_AcknowledgeConfig(I2C_ACK_NONE);

      disableInterrupts();

      rxbuf[i++] = I2C_ReceiveData();  // Read Data N-2
      I2C_GenerateSTOP(ENABLE);

      rxbuf[i++] = I2C_ReceiveData();  // Read DataN-1

      enableInterrupts();

      step++;
      for (t = FLAG_TIMEOUT; I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY) == RESET && --t;);
      if (!t) break;

      rxbuf[i] = I2C_ReceiveData();
      // printf("\r\nrx: %02x %02x %02x", rxbuf[0], rxbuf[1], rxbuf[2]);
    }
  } while (0);

  if (step == 8) {
    step = 0;
  } else {
    I2C_GenerateSTOP(ENABLE);
  }
  I2C_AcknowledgeConfig(I2C_ACK_CURR);
  return step;
}

void main(void) {
  // char ans;
  uint8_t buff[3];
  uint8_t s[2] = {0xe3, 0xe5};
  uint8_t err = 0;
  int32_t t;
  int32_t t2;

  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  Delay(0x4000);

  UART1_DeInit();
  UART1_Init((uint32_t)9600, UART1_WORDLENGTH_8D, UART1_STOPBITS_1, UART1_PARITY_NO,
              UART1_SYNCMODE_CLOCK_DISABLE, UART1_MODE_TXRX_ENABLE);

  I2C_DeInit();
  I2C_Init(400000, 0xA0, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 16);

  while (1) {
    err = i2cMasterTransmit(0x40, s, 1, buff, 3);
    if (err) {
      printf("\r\nE: %d", err);
    } else {
      // printf("\r\nrx: %02x %02x %02x", buff[0], buff[1], buff[2]);
      // printf("\r\ncrc: %02x", checkCrc(buff, 2));
      if (buff[2] == checkCrc(buff, 2)) {
        t = (((int32_t)buff[0] << 8) + buff[1]) & 0xfffc;
        // t2 = (float)t * 175.72 / 65536- 46.85;
        // printf("\r\n%lx", t);
        t2 = ((t * 17572) >> 16) - 4685;
        printf("\r\nT: %ld.%02d", t2 / 100, (int)t2 % 100);
      } else {
        printf("\r\nE: 8");
      }
    }

    err = i2cMasterTransmit(0x40, s + 1, 1, buff, 3);
    if (err) {
      printf("\r\nE: %d", err);
    } else {
      // printf("\r\nrx: %02x %02x %02x", buff[0], buff[1], buff[2]);
      // printf("\r\ncrc: %02x", checkCrc(buff, 2));

      if (buff[2] == checkCrc(buff, 2)) {
        t = (((int32_t)buff[0] << 8) + buff[1]) & 0xfffc;
        // t2 = (float)t * 175.72 / 65536- 46.85;
        t2 = t * 125 / 65536 - 6;
        // printf("\r\n%lx", t);
        printf(", H: %ld.%02d", t2 / 100, (int)t2 % 100);
      } else {
        printf("\r\nE: 8");
      }
    }

    // i2cMasterTransmit(0x40, s+1, 1, buff, 3);
    // printf("\r\ncrc: %02x", checkCrc(buff, 2));
    getchar();
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
