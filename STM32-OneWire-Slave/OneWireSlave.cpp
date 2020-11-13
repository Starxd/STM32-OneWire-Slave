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

inline void OneWireSlave::SET_PIN_MODE_OUT()
{
	this->GPIO->CRL &= ~GPIO_CRL_CNF0;
	//this->GPIO->CRL |= GPIO_CRL_CNF0_1;
	this->GPIO->CRL |= GPIO_CRL_MODE0;
}

inline void OneWireSlave::SET_PIN_MODE_IN() {
	this->GPIO->CRL &= ~(GPIO_CRL_CNF0 | GPIO_CRL_MODE0);
	this->GPIO->CRL |= GPIO_CRL_CNF0_0;
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
	this->GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	this->GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(this->GPIO, &this->GPIO_InitStructure);
	
	SET_PIN_MODE_IN();
	
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
void OneWireSlave::reset(void) {
	this->currentState = 0;
	this->counterReciveCMD = 0;
	this->reciveCMD = 0;
	this->counterBitWrite = 0;
	this->isMasterRead = false;
	this->isMasterReset = false;
	this->isSearchRom = false;
	currentBitRom = 0;
	reciveCmdIsComplite = false;
}

inline void OneWireSlave::presence() {
	this->EXTI_InitStruct.EXTI_LineCmd = DISABLE;
	EXTI_Init(&EXTI_InitStruct);
	SET_PIN_MODE_OUT();
	SET_PIN_VALUE(Bit_RESET);
	delay_us(65);
	SET_PIN_MODE_IN();
	this->EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);
}

inline void OneWireSlave::write_bit(bool bit)
{
	SET_PIN_VALUE((BitAction)bit);
	SET_PIN_MODE_OUT();
	delay_us(55);
	SET_PIN_MODE_IN();
}
uint8_t OneWireSlave::getByte()
{
	
}

bool bit;
bool readBit;
uint8_t i;
inline volatile void OneWireSlave::searchRom()
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
	for (i = 0; i < 64; i++)
	{
		while (this->GPIO->IDR & GPIO_IDR_IDR0) ;
		this->write_bit(bitROM[i]);

		while(this->GPIO->IDR & GPIO_IDR_IDR0);
		this->write_bit(!bitROM[i]);
		
		__asm("nop");
		
		while (this->GPIO->IDR & GPIO_IDR_IDR0) ;
		delay_us(20);
		readBit = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
		while (!(this->GPIO->IDR & GPIO_IDR_IDR0)) ;
		
		bit = (bool)bitROM[i];
		if (!(readBit == bit))
		{
			this->currentState = 0;
			GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);
			break;
		}
	}
}

void OneWireSlave::interrupt() {
	if (EXTI_GetITStatus(this->EXTI_InitStruct.EXTI_Line) != RESET) {
		this->isMasterReleased = true;
		uint8_t pin = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
		if (!pin) 
		{
			reset_delay();
		}
		else 
		{
			uint32_t delay = get_delay();
			if (delay > 400) this->isMasterReset = true;
			else
			{
				bool reciveBit = ((get_delay() < 20) ? 1 : 0);
				this->reciveCMD |= (reciveBit << this->counterReciveCMD);
				if (this->counterReciveCMD < 7) this->counterReciveCMD++;
				else if (this->reciveCMD == 0xF0) 
				{
					this->isSearchRom = true;
				}
			}
		}

		EXTI_ClearITPendingBit(this->EXTI_InitStruct.EXTI_Line);
	}
}

uint8_t OneWireSlave::listener(void) {
	if (this->isMasterReleased) 
	{
		if (this->isMasterReset)
		{	
			this->reset();
			this->presence();
			this->currentState = 1;
			GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
		}
		
		if (this->isSearchRom)
		{
			GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);
		}
	}
	
	
//	if (this->isMasterReleased)
//	{
//		this->isMasterReleased = false;
//		
//		if (this->isMasterReset)
//		{
//			this->reset();
//			this->presence();
//			GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
//		}
//		else
//		{
//			bool reciveBit = (this->delay < 20) ? 1 : 0;
//			switch (this->currentState)
//			{
//			case 0: //read cmd
//				this->reciveCMD |= (reciveBit << this->counterReciveCMD);
//				if (this->counterReciveCMD == 7)
//				{
//					if (this->reciveCMD == 0xF0) {
//						this->currentState = 1;
//					}
//				}
//				else this->counterReciveCMD++;
//				break;
//			case 1: //write pos
//					this->write_bit(bitROM[currentBitRom]);
//
//				break;
//				
//			case 2: //write neg
//				//GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_RESET);
//				SET_PIN_MODE_OUT();
//				GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
//				SET_PIN_VALUE((BitAction)!bitROM[currentBitRom]);
//				delay_us(1);
//				SET_PIN_MODE_IN();	
//				this->currentState = 3;
//				break;
//				
//			case 3: //read
//				if (reciveBit && bitROM[currentBitRom]) {
//					this->currentState = 1;
//				}
//				else
//				{
//					this->currentState = 90;
//					isMasterRead = false;
//				}
//				break;
//				
//			default:
//				isMasterRead = false;
//				break;
//			}
			
			
			
//			switch (this->currentState)
//			{
//			case 0: //get cmd
//				this->reciveCMD |= (((this->delay < 20) ? 1 : 0) << this->counterReciveCMD);
//				if (this->counterReciveCMD == 7)
//				{
//					if (this->reciveCMD == 0xF0) {
//						//GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
//						this->searchRom();
//					}
//				}
//				else this->counterReciveCMD++;
//				break;
//								
//			case 1:
//				if (this->reciveCMD == 0xF0)
//				{
//					this->reset();
//					this->searchRom();
//				}
//				break;
//			}
//		}
		
		
//		if ((delay > 400)&&(delay < 600)) //this is reset
//		{
//			this->reset();
//			this->presence();
//		}
//		else
//		{
//			switch (this->currentState)
//			{
//			case 0: //get cmd
//				this->reciveCMD |= (((this->delay < 20) ? 1 : 0) << this->counterReciveCMD);
//				if (this->counterReciveCMD == 7)
//				{
//					if (this->reciveCMD == 0xF0) {
//						//GPIO_WriteBit(GPIOA, GPIO_Pin_9, Bit_SET);
//						this->searchRom();
//					}
//				}
//				else this->counterReciveCMD++;
//				break;
//				
//			case 1:
//				if (this->reciveCMD == 0xF0)
//				{
//					this->reset();
//					this->searchRom();
//				}
//				break;
//			}
//		}
//	}
	
//	switch (this->state)
//	{
//	case 0: //wait for reset
//		if (this->isWizardActivity)
//		{
//			this->isWizardActivity = false;
//			if ((delay > 200)&&(delay < 600))
//			{
//				OneWireSlave::presence();
//				this->counterReciveCMD = 7;
//				state = 1;
//			}
//		}
//		break;
//	case 1: // read cmd
//		if (this->isWizardActivity)
//		{
//			this->isWizardActivity = false;
//			this->reciveCMD |= (((this->delay < 20) ? 1 : 0) << this->counterReciveCMD);
//			if (this->counterReciveCMD > 0)this->counterReciveCMD--;
//			else 
//			{
//				this->state = 2;
//				counterReciveCMD = 0;
//			}
//		}
//		break;
//	case 2:
//		__asm("nop");
//		
//		switch (this->reciveCMD)
//		{
//		case 0x0F:
//			this->isWriteBit = true;
//			if (this->isWizardSync)
//			{
//				this->isWizardSync = false;
//				this->write_bit();
//				if (this->counterBitWrite > 63)
//				{
//					this->counterBitWrite = 0;
//					this->state = 0;
//					this->isWriteBit = false;
//				}
//			}
//			
//			break;
//		}
//		break;
//	}
	
	return 0;
}
