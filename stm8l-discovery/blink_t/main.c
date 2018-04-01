#include <stdint.h>

#define CLK_DIVR	(*(volatile uint8_t *)0x50c6)
#define CLK_PCKENR1	(*(volatile uint8_t *)0x50c7)

#define TIM1_CR1	(*(volatile uint8_t *)0x5250)
#define TIM1_CNTRH	(*(volatile uint8_t *)0x525e)
#define TIM1_CNTRL	(*(volatile uint8_t *)0x525f)
#define TIM1_PSCRH	(*(volatile uint8_t *)0x5260)
#define TIM1_PSCRL	(*(volatile uint8_t *)0x5261)

#define PD_ODR	(*(volatile uint8_t *)0x500f)
#define PD_DDR	(*(volatile uint8_t *)0x5011)
#define PD_CR1	(*(volatile uint8_t *)0x5012)


unsigned int clock(void)
{	
	return((unsigned int)(TIM1_CNTRH) << 8 | TIM1_CNTRL);
}

void main(void)
{
	CLK_DIVR = 0x00; // Set the frequency to 16 MHz

	// Configure timer
	// 1000 ticks per second, 0x3e80 = 16000
	TIM1_PSCRH = 0x3e;
	TIM1_PSCRL = 0x80;
	// Enable timer
	TIM1_CR1 = 0x01;

	PD_DDR = 0x01;
	PD_CR1 = 0x01;

	for(;;)
		PD_ODR = clock() % 100 < 50;
}
