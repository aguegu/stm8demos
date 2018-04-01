#include "stm8l.h"

int main() {
  unsigned int d, c;
  // Configure pins
  PD_DDR = 0x01;
  PD_CR1 = 0x01;
  // Loop
  while (1) {
    PD_ODR ^= 0x01;
    for (d = 0; d<10000; d++)  {
      for(c = 0; c<10; c++);
    }
  }
}
