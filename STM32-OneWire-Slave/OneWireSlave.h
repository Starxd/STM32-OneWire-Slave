#pragma once
#include <stdint.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_exti.h>
#include <misc.h>
#include <Util.h>


class OneWireSlave
{
private:
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	GPIO_TypeDef* GPIO;
	bool isMasterReleased;
	bool isMasterReset;
	bool isSearchRom;
	uint8_t statePin;
	uint8_t currentState;
	uint32_t delay;
	uint8_t reciveCMD;
	uint8_t counterReciveCMD;
	BitAction bitROM[64];
	uint8_t counterBitWrite;
	
	uint8_t currentBitRom;
	
	volatile bool isMasterRead;
	volatile bool isMasterWrite;
	
	bool reciveCmdIsComplite;
	uint8_t searchRomState;
	
	
	
	uint8_t ROM[8] = { 0xC3, 0x00, 0x00, 0x01, 0x99, 0xB4, 0x53, 0x01 };
	
	inline void SET_PIN_MODE_IN();
	inline void SET_PIN_MODE_OUT();
	inline void SET_PIN_VALUE(BitAction bitVal);
	inline void presence(void);
	inline uint8_t getByte(void);
	inline void write_bit(bool bit);
	inline void reset(void);
	inline volatile void searchRom(void);

public:
	OneWireSlave();
	~OneWireSlave();
	void init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_x, uint32_t EXTI_Line);
	void interrupt(void);
	uint8_t listener(void);
	void setROM();
};

