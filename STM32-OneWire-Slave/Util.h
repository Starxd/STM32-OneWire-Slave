#pragma once
#include <stdint.h>

volatile void delayInterrupt(void);
volatile void delay_us(uint32_t us);
volatile uint32_t get_delay(void);
volatile void reset_delay(void);