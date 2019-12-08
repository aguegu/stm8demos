#include "stm8s_conf.h"

// a: C6
// b: D3
// c: D6
// d: A1
// e: A2
// f: C7
// g: D5
// p: A3

// 0: D4
// 1: D2
// 2: D1
// 3: C5

// TEST LED between vcc and PB5

#define SLAVE_ADDRESS 0x6c
// 7-bit address: 0x36

const static uint8_t numbers[10] = {
  0x3f, 0x06, 0x5b, 0x4f, 0x66, // 0-4
  0x6d, 0x7d, 0x07 ,0x7f, 0x6f, // 5-9
  0x5f, 0x7c, 0x58, 0x5e, 0x79, 0x71, // a-f
};

typedef struct {
  GPIO_TypeDef* port;
  GPIO_Pin_TypeDef pin;
} Pin;

extern volatile uint16_t millis;
volatile uint8_t patterns[4] = {0};

void delay(uint16_t ms) {
  uint16_t start = millis;

  while (ms) {
    if (millis - start) {
      ms--;
      start++;
    }
    wfi();
  }
}

void showNumber(uint8_t *p, uint16_t n) {
  for (uint8_t i=0; i<4; i++) {
    uint8_t j = n % 10;
    p[i] = numbers[j];
    n /= 10;
    if (!j && !n) {
      p[i] = 0x00;
    }
  }
}


void main(void) {
  Pin ranks[4] = {
    { .port = GPIOD, .pin = GPIO_PIN_4 },
    { .port = GPIOD, .pin = GPIO_PIN_2 },
    { .port = GPIOD, .pin = GPIO_PIN_1 },
    { .port = GPIOC, .pin = GPIO_PIN_5 },
  };

  Pin segs[8] = {
    { .port = GPIOC, .pin = GPIO_PIN_6 },
    { .port = GPIOD, .pin = GPIO_PIN_3 },
    { .port = GPIOD, .pin = GPIO_PIN_6 },
    { .port = GPIOA, .pin = GPIO_PIN_1 },
    { .port = GPIOA, .pin = GPIO_PIN_2 },
    { .port = GPIOC, .pin = GPIO_PIN_7 },
    { .port = GPIOD, .pin = GPIO_PIN_5 },
    { .port = GPIOA, .pin = GPIO_PIN_3 },
  };

  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);

  I2C_DeInit();
  I2C_Init(400000, SLAVE_ADDRESS, I2C_DUTYCYCLE_2, I2C_ACK_CURR, I2C_ADDMODE_7BIT, 16);
  I2C_ITConfig((I2C_IT_TypeDef)(I2C_IT_ERR | I2C_IT_EVT | I2C_IT_BUF), ENABLE);

  enableInterrupts();

  TIM4_Cmd(ENABLE);

  uint16_t cnt = 0;
  uint16_t val = 1800;

  for (uint8_t i=0; i < 8; i++) {
    GPIO_Init(segs[i].port, segs[i].pin, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_WriteLow(segs[i].port, segs[i].pin);
  }

  for (uint8_t i=0; i < 4; i++) {
    GPIO_Init(ranks[i].port, ranks[i].pin, GPIO_MODE_OUT_OD_LOW_SLOW);
    GPIO_WriteHigh(ranks[i].port, ranks[i].pin);
  }

  patterns[0] = numbers[(SLAVE_ADDRESS >> 1) & 0x0f];
  patterns[1] = numbers[(SLAVE_ADDRESS >> 1) >> 4];
  patterns[2] = 0;
  patterns[3] = 0;


  while (1) {
    // if (!(cnt & 0x1ff)) {
    //   showNumber(patterns, val);
    //   val ++;
    // }
    uint8_t r = cnt & 0x03;

    for (uint8_t i=0; i<8; i++) {
      if (patterns[r] & (1 << i)) {
        GPIO_WriteReverse(segs[i].port, segs[i].pin);
      }
    }
    GPIO_WriteReverse(ranks[r].port, ranks[r].pin);

    delay(4);

    for (uint8_t i=0; i<8; i++) {
      if (patterns[r] & (1 << i)) {
        GPIO_WriteReverse(segs[i].port, segs[i].pin);
      }
    }
    GPIO_WriteReverse(ranks[r].port, ranks[r].pin);
    cnt ++;
  }
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line) {
  (void) file;
  (void) line;
  while (1);
}
#endif
