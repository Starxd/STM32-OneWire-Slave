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
	uint32_t downTime, upTime;
	bool isPresence;
	bool isReset, wasItReset;
	bool statePin;
	bool isDown, isUp;
	bool isRecive;
	uint8_t counterRecive;
	uint32_t buff[255];
	uint32_t delay;
	bool isWizardActivity;
	uint8_t state;
	uint8_t reciveCMD;
	int8_t counterReciveCMD;
	volatile bool isWriteBit;
	bool isWizardSync;
	BitAction bitROM[64];
	uint8_t counterBitWrite;
	
	uint8_t ROM[8] = { 0xC3, 0x00, 0x00, 0x01, 0x99, 0xB4, 0x53, 0x01 };
	
	inline void SET_PIN_MODE(GPIOMode_TypeDef GPIOMode);
	inline void SET_PIN_VALUE(BitAction bitVal);
	inline void presence(void);
	inline void write_bit(void);

public:
	OneWireSlave();
	~OneWireSlave();
	void init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_x, uint32_t EXTI_Line);
	void interrupt(void);
	uint8_t listener(void);
	void setROM();
};

