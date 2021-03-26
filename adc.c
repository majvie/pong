#include "adc.h"

void init_ADC()
{
	ADC_InitTypeDef ADC_INIT_STRUCT;
	ADC_CommonInitTypeDef ADC_COMMON_STRUCT;
	NVIC_InitTypeDef NVIC_INIT_STRUCT;
	
	// Enable Clock
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE); // Enable Clock for ADC
	RCC_ADCCLKConfig(RCC_ADC12PLLCLK_OFF); // Use clock from AHB, not using PLL
	
	// Fill ADC Struct with default values
	ADC_CommonStructInit(&ADC_COMMON_STRUCT);
	ADC_COMMON_STRUCT.ADC_Clock = ADC_Clock_SynClkModeDiv1; // AHB Clock / 1
	ADC_CommonInit(ADC1, &ADC_COMMON_STRUCT); // ADC1 important, function want to know which ADC
	
	ADC_StructInit(&ADC_INIT_STRUCT);
	
	ADC_INIT_STRUCT.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable; //continous meassurement disabled
	ADC_INIT_STRUCT.ADC_OverrunMode= ADC_OverrunMode_Disable; //no Override of Data
	ADC_Init(ADC1, &ADC_INIT_STRUCT);
	
	//ADC Channels 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 1, ADC_SampleTime_61Cycles5); // PB1 is connected to ADC1 Channel 12 datasheet S.38
	
	ADC_RegularChannelSequencerLengthConfig(ADC1, 1);
	ADC_VoltageRegulatorCmd(ADC1, ENABLE);
		
	// Wait for voltage regulator
	TIM_Cmd(TIM2, ENABLE);
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);	//Clear Pending Bit because is automatically set at start
	while(TIM_GetFlagStatus(TIM2,TIM_IT_Update) == 0){};
	TIM_Cmd(TIM2, DISABLE);
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	NVIC_ClearPendingIRQ(TIM2_IRQn);

	ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single); //Ref.Man S.217 
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1)); // Wait for calibration to finish
	ADC_GetCalibrationValue(ADC1); 
	
	// Configure Interrupts 
	NVIC_INIT_STRUCT.NVIC_IRQChannel = TIM2_IRQn; // TIM2 for game updates and conversion of ADC Vals
	NVIC_INIT_STRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INIT_STRUCT.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_INIT_STRUCT.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_INIT_STRUCT);
	
	NVIC_INIT_STRUCT.NVIC_IRQChannel = ADC1_2_IRQn; // For conversion
	NVIC_INIT_STRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INIT_STRUCT.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_INIT_STRUCT.NVIC_IRQChannelSubPriority = 1; // same as ADC 
	NVIC_Init(&NVIC_INIT_STRUCT);
	
	ADC_Cmd(ADC1, ENABLE);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY)); // Wait until ADC is ready
	ADC_ITConfig(ADC1,ADC_IT_EOS,ENABLE); // End of Conversion Flag, Ref.Man 223
	ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE); // End of Sampling Phase Flag, Ref.Man 223
	
	TIM_Cmd(TIM2, ENABLE);
}


