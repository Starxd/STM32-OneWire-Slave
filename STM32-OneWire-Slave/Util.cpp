#include "Util.h"

volatile uint32_t ticks_delay = 0;
volatile uint32_t ticks_delay1 = 0;

volatile void delayInterrupt(void) { 
	ticks_delay += 1;
	ticks_delay1 += 1;
}

volatile void delay_us(volatile uint32_t us) {
	volatile uint32_t start = ticks_delay;
	while ((ticks_delay - start) < us) __asm("nop");
}

volatile uint32_t get_delay(void) {
	return ticks_delay1;
}

volatile void reset_delay() {
	ticks_delay1 = 0;
}