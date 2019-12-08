#include "stm8s.h"

#define SLAVE_ADDRESS 0x60

void main(void) {
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  I2C_DeInit();

  I2C_Init(400000, SLAVE_ADDRESS, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 16);
  I2C_ITConfig((I2C_IT_TypeDef)(I2C_IT_ERR | I2C_IT_EVT | I2C_IT_BUF), ENABLE);

  enableInterrupts();

  while (1) {

  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line) {
  while (1);
}
#endif
