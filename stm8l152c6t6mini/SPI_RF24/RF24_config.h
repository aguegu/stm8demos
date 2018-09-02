#ifndef __RF24_CONFIG_H__
#define __RF24_CONFIG_H__

  #define rf24_max(a,b) (a>b?a:b)
  #define rf24_min(a,b) (a<b?a:b)

  const uint32_t RF24_SPI_SPEED = 10000000;

  #include <stdio.h>
  #include <string.h>

  #define _BV(x) (1<<(x))

#endif // __RF24_CONFIG_H__
