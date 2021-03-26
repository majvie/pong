/*
  ******************************************************************************
  MIKROCOMPUTER LABOR - HOME-STATION

  Maximilian Vieweg, 11806443

  In diesem Projekt gilt:
  *=============================================================================
  *        SYSCLK(Hz)                             | 64000000
  *-----------------------------------------------------------------------------
  *        AHB Prescaler                          | 1
  *-----------------------------------------------------------------------------
  *        APB2 Prescaler                         | 1
  *-----------------------------------------------------------------------------
  *        APB1 Prescaler                         | 2
  *=============================================================================
  ******************************************************************************
*/

#include "stm32f30x_conf.h"

#include "stm32f30x_adc.h"
#include "stm32f30x_usart.h"
#include "stm32f30x_tim.h"

#include "gpio.h"
#include "tim.h"
#include "LCD.h"
#include "adc.h"

field_t field;
const long MAX_ADC_RESOLUTION = 4095;
	
int main(void) {
	init_GPIO();
	init_TIM();
	init_LCD();
	init_ADC();
	
	field = initGame();
	drawGame(field);
	
  while(1) {}
}

void TIM2_IRQHandler(){
	ADC_StartConversion(ADC1);
	
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}


void ADC1_2_IRQHandler(){
		double y = ADC_GetConversionValue(ADC1);
		
		if (y / MAX_ADC_RESOLUTION > 0.53) {
			field = game_logic(field, 1);
			drawGame(field);
		}
		else if (y / MAX_ADC_RESOLUTION < 0.47){ 
			field = game_logic(field, -1);
			drawGame(field);
		}
		else {
			field = game_logic(field, 0);
			drawGame(field);
		}

		ADC_ClearITPendingBit(ADC1, ADC_IT_EOC); // Clear ADC End of Conversion Flag
		ADC_ClearITPendingBit(ADC1, ADC_IT_EOS); // Clear ADC End of Sequence Flag
}
