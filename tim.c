#include "tim.h"

void init_TIM() {
	TIM_TimeBaseInitTypeDef TIM_INIT;
	
	// Enable Clocks
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	//TIM3 reset impulse LCD
	TIM_TimeBaseStructInit(&TIM_INIT);
	TIM_INIT.TIM_Prescaler = 0; // 64Mhz / (0+1) = 64Mhz
	TIM_INIT.TIM_Period= 63; // 64Mhz / (63+1) = 1 ms > 100ns 
	TIM_TimeBaseInit(TIM3,&TIM_INIT);
	
	//TIM2: ADC and Game Logic
	TIM_TimeBaseStructInit(&TIM_INIT);
	TIM_INIT.TIM_Prescaler=31999; // 64MHz / ((Prescaler_TIM2+1)*2) = 1 kHz
	TIM_INIT.TIM_Period= 199; // 0.2 s
	TIM_TimeBaseInit(TIM2,&TIM_INIT);
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}
