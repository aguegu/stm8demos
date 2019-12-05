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

const static uint8_t numbers[10] = {
  0x3f, 0x06, 0x5b, 0x4f, 0x66,
  0x6d, 0x7d, 0x07 ,0x7f, 0x6f,
};

typedef struct {
  GPIO_TypeDef* port;
  GPIO_Pin_TypeDef pin;
} Pin;


void Delay(uint16_t t) {
  while (t--);
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

  uint16_t cnt = 0;
  uint8_t patterns[4] = {0};
  uint16_t val = 1800;

  for (uint8_t i=0; i < 8; i++) {
    GPIO_Init(segs[i].port, segs[i].pin, GPIO_MODE_OUT_PP_LOW_SLOW);
    GPIO_WriteLow(segs[i].port, segs[i].pin);
  }

  for (uint8_t i=0; i < 4; i++) {
    GPIO_Init(ranks[i].port, ranks[i].pin, GPIO_MODE_OUT_OD_LOW_SLOW);
    GPIO_WriteHigh(ranks[i].port, ranks[i].pin);
  }

  while (1) {
    if (!(cnt & 0x1ff)) {
      showNumber(patterns, val);
      val ++;
    }

    // uint8_t n = cnt & 0x07;
    // uint8_t r = (cnt >> 3) & 0x03;
    //
    // if (patterns[r] & (1 << n)) {
    //   GPIO_WriteReverse(ranks[r].port, ranks[r].pin);
    //   GPIO_WriteReverse(segs[n].port, segs[n].pin);
    //   Delay(0x1f);
    //   GPIO_WriteReverse(segs[n].port, segs[n].pin);
    //   GPIO_WriteReverse(ranks[r].port, ranks[r].pin);
    // }

    uint8_t r = cnt & 0x03;

    for (uint8_t i=0; i<8; i++) {
      if (patterns[r] & (1 << i)) {
        GPIO_WriteReverse(segs[i].port, segs[i].pin);
      }
    }
    GPIO_WriteReverse(ranks[r].port, ranks[r].pin);

    Delay(0x400);

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
