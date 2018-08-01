#include "stm8l15x_conf.h"
#include <stdio.h>

#define EEPROM_LEN (FLASH_DATA_EEPROM_END_PHYSICAL_ADDRESS - FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS + 1)

int putchar (int c) {
  USART_SendData8(USART1, c);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
  return (c);
}

int getchar (void) {
  int c = 0;
  while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
  c = USART_ReceiveData8(USART1);
  return c;
}

void main(void) {
  char ans;
  uint16_t p = 0;
  uint8_t * const udid = (uint8_t *)((uint16_t)0x4926); // chip unique 12-byte id

  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);

  CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);

  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_2, ENABLE); // RX
  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_3, ENABLE); // TX

  USART_Init(USART1, (uint32_t)9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,
              USART_Mode_Rx | USART_Mode_Tx);

  printf("\r\nUDID: ");
  for (uint8_t i = 0; i < 12; i++) {
    printf("%02x ", udid[i]);
  }

  for (p = 0; p < EEPROM_LEN; p++) {
    if ((p & 0x0f) == 0) {
      printf("\r\n");
    }
    printf("%02x ", FLASH_ReadByte(FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS + p));
  }

  printf("Enter Text to EEPROM: \r\n");

  p = 0;
  while (1) {
    ans = getchar();
    printf("%d %c %02x \r\n", p, ans, ans);

    FLASH_Unlock(FLASH_MemType_Data);
    FLASH_ProgramByte(FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS + p++, ans);
    if (p >= EEPROM_LEN) {
      p = 0;
    }
    while(!FLASH_GetFlagStatus(FLASH_FLAG_EOP));
    FLASH_Lock(FLASH_MemType_Data);
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t* file, uint32_t line) {
  (void) file;
  (void) line;
  printf("Wrong parameters value: file %s on line %ld\r\n", file, line);
  while (1);
}

#endif
