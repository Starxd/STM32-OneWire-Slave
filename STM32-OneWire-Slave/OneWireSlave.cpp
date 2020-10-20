#include "OneWireSlave.h"

OneWireSlave::OneWireSlave() {

}
OneWireSlave::~OneWireSlave() {
	
}

void OneWireSlave::setROM() {
	uint8_t cnt = 0;
	for (uint8_t i = 0; i < 8; i++)
	{
		for (uint8_t bit = 1; bit; bit <<= 1)
		{
			this->bitROM[cnt] = (bit & this->ROM[i]) ? Bit_SET : Bit_RESET;
			cnt++;
		}
	}
}

inline void OneWireSlave::SET_PIN_MODE(GPIOMode_TypeDef GPIOMode) {
	this->GPIO_InitStructure.GPIO_Mode = GPIOMode;
	GPIO_Init(this->GPIO, &this->GPIO_InitStructure);
}
inline void OneWireSlave::SET_PIN_VALUE(BitAction bitVal) {
	GPIO_WriteBit(this->GPIO, this->GPIO_InitStructure.GPIO_Pin, bitVal);
}

void OneWireSlave::init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin_x, uint32_t EXTI_Line) {
	/* Check the parameters */
	assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
	if (GPIOx == GPIOA)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	if (GPIOx == GPIOB)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	if (GPIOx == GPIOC)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	if (GPIOx == GPIOD)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	if (GPIOx == GPIOE)RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	this->GPIO = GPIOx;
	this->GPIO_InitStructure.GPIO_Pin = GPIO_Pin_x;
	this->GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	this->GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(this->GPIO, &this->GPIO_InitStructure);
	
	OneWireSlave::SET_PIN_MODE(GPIO_Mode_IN_FLOATING);
	
	this->NVIC_InitStruct.NVIC_IRQChannel = EXTI0_IRQn;
	this->NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	this->NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	this->NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
	this->EXTI_InitStruct.EXTI_Line = EXTI_Line;
	this->EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	this->EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	this->EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_Init(&EXTI_InitStruct);
	
	//////
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructureTX;
	GPIO_InitStructureTX.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructureTX.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructureTX.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructureTX);
	GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
	this->setROM();
}
void OneWireSlave::interrupt() {
	if (EXTI_GetITStatus(this->EXTI_InitStruct.EXTI_Line) != RESET) {
		this->statePin = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
		if (this->isWriteBit) {
			if (!this->statePin)this->isWizardSync = true;
			GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
		}
		else
		{
			if (!this->statePin) {
				reset_delay();
				this->delay = 0;
			}
			else {
				this->delay = get_delay();
				this->isWizardActivity = true;
			}
		}
		
		EXTI_ClearITPendingBit(this->EXTI_InitStruct.EXTI_Line);
	}
}

inline void OneWireSlave::presence() {
	this->EXTI_InitStruct.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStruct);
	SET_PIN_MODE(GPIO_Mode_Out_PP);
	SET_PIN_VALUE(Bit_RESET);
	delay_us(65);
	SET_PIN_MODE(GPIO_Mode_IN_FLOATING);
	this->EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
}

inline void OneWireSlave::write_bit()
{
	this->EXTI_InitStruct.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStruct);
	SET_PIN_MODE(GPIO_Mode_Out_PP);
	SET_PIN_VALUE(this->bitROM[this->counterBitWrite]);
	delay_us(65);
	SET_PIN_MODE(GPIO_Mode_IN_FLOATING);
	this->EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
	this->counterBitWrite++;
	GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);
}

uint8_t OneWireSlave::listener(void) {
	switch (this->state)
	{
	case 0: //wait for reset
		if (this->isWizardActivity)
		{
			this->isWizardActivity = false;
			if ((delay > 200)&&(delay < 600))
			{
				OneWireSlave::presence();
				this->counterReciveCMD = 7;
				state = 1;
			}
		}
		break;
	case 1: // read cmd
		if (this->isWizardActivity)
		{
			this->isWizardActivity = false;
			this->reciveCMD |= (((this->delay < 20) ? 1 : 0) << this->counterReciveCMD);
			if (this->counterReciveCMD > 0)this->counterReciveCMD--;
			else 
			{
				this->state = 2;
				counterReciveCMD = 0;
			}
		}
		break;
	case 2:
		__asm("nop");
		
		switch (this->reciveCMD)
		{
		case 0x0F:
			this->isWriteBit = true;
			if (this->isWizardSync)
			{
				this->isWizardSync = false;
				this->write_bit();
				if (this->counterBitWrite > 63)
				{
					this->counterBitWrite = 0;
					this->state = 0;
					this->isWriteBit = false;
				}
			}
			
			break;
		}
		break;
	}
	
	return 0;
}
