# STM32 OneWire slave


Usage
-----

Example:
```c++
#define F_CPU 72000000UL
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_flash.h>
#include <misc.h>
#include <OneWireSlave.h>
#include <Util.h>
#include <core_cm3.h>

extern "C" {
	volatile void SysTick_Handler(void);
	void EXTI0_IRQHandler(void);
}

OneWireSlave OneWire;

bool InitSysClock()
{
	ErrorStatus HSEStartUpStatus;
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS)
	{
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		FLASH_SetLatency(FLASH_Latency_2);
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		RCC_PCLK2Config(RCC_HCLK_Div1);
		RCC_PCLK1Config(RCC_HCLK_Div2);
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
		RCC_PLLCmd(ENABLE);
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) ;
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		while (RCC_GetSYSCLKSource() != 0x08) ;
	}
	else return false; /* error HSE */
	return true;	
}

void InitSysTickTimer(void) {
	SysTick->LOAD = F_CPU / 1000000 - 1;
	SysTick->VAL = F_CPU / 1000000 - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk   | SysTick_CTRL_ENABLE_Msk;
}

int main()
{
	if (!InitSysClock())__asm("bkpt 255"); //error system clock
	InitSysTickTimer();
	OneWire.init(GPIOB, GPIO_Pin_0, EXTI_Line0);
	OneWire.setROM(new uint8_t[6] { 0x28, 0xB4, 0x99, 0x01, 0x00, 0x00 });
	OneWire.setIsEnable(true);

	while (1)
	{
		OneWire.listener()
	}
}

volatile void SysTick_Handler(void) {
	delayInterrupt(); 
}

void EXTI0_IRQHandler(void) {
	OneWire.interrupt();
}

```

Start Inverted JSON:
```
version: '3.7'

services:
  ijson:
    image: lega911/ijson
    container_name: base-ijson
    ports:
      - 8001:8001
```

Send POST request directly to: http://localhost:8001
```bash
curl http://127.0.0.1:8001/my-microservice -d '{"id": 1, "params":{"test":1}}'
```

**If you run [gateway](https://github.com/kakadu-dev/nodejs-ijson-microservices).** Run POST request to: http://localhost:3000
```json
{
  "id": 1,
  "method": "my-service.test-method",
  "params": {
    "test": 1
  }
}
```

That's all. Check it.
