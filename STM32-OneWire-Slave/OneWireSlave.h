#pragma once
#include <stdint.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_exti.h>
#include <misc.h>
#include <Util.h>


class OneWireSlave
{
	struct GPIO_REG
	{
		uint32_t _GPIO_CNF;
		uint32_t _GPIO_MODE;
		uint32_t _GPIO_CNF_0;
	};
private:
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	GPIO_TypeDef* GPIO;
	GPIO_REG regGPIO;
	volatile bool isCmdReceive;
	volatile bool isMasterRead;
	volatile bool isWriteBit;
	volatile bool isMasterReset;
	volatile bool isSearchRom;
	volatile bool isEnable = false;
	uint8_t reciveCMD;
	uint8_t counterReciveCMD;
	uint8_t counterBitWrite;
	uint8_t searchRomState;
	uint8_t ROM[8];
	uint8_t portSource;
	uint8_t pinSource;

	inline uint8_t getPinNumber(uint16_t pin);
	inline void SET_PIN_MODE_IN();
	inline void SET_PIN_MODE_OUT();
	inline void SET_PIN_VALUE(bool bitVal);
	inline void presence(void);
	inline void reset(void);
	inline void write_bit(bool bit);
	inline uint8_t calculateCRC8(uint8_t seed, uint8_t inData);

public:
	OneWireSlave();
	~OneWireSlave();
	void init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_x, uint32_t EXTI_Line);
	void interrupt(void);
	uint8_t listener(void);
	void setROM(uint8_t rom[6]);
	void setIsEnable(bool isEnable);
};

