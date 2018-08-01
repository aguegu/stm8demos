#include "stm8l15x_conf.h"
#include <stdio.h>

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
  uint8_t * const udid = (uint8_t *)((uint16_t)0x4926); // chip unique 12-byte id
  // __IO uint8_t * eeprom = (__IO uint8_t *)(FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS);

  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);

  CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);

  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_2, ENABLE); // RX
  GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_3, ENABLE); // TX

  USART_Init(USART1, (uint32_t)9600, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,
              USART_Mode_Rx | USART_Mode_Tx);

  printf("\r\nUDID: ");
  for (uint8_t i=0; i<12; i++) {
    printf("%02x ", udid[i]);
  }

  FLASH_Unlock(FLASH_MemType_Data);

  for (uint16_t i = 0; i < 0x400; i++) {
    // eeprom[i] = i;
    FLASH_ProgramByte(FLASH_DATA_EEPROM_START_PHYSICAL_ADDRESS + i, ~i);
    while (!(FLASH->IAPSR & FLASH_IAPSR_EOP));
  }

  FLASH_Lock(FLASH_MemType_Data);

  printf("\r\nUART1 Example :retarget the C library printf()/getchar() functions to the UART\r\n");
  printf("Enter Text\r\n");

  while (1) {
    ans = getchar();
    printf("%c %02x \r\n", ans, ans);
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
