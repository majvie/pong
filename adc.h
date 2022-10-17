#include "stm32f30x_conf.h"
#include "tim.h"

typedef struct adcvalues_s
{
	int x;
	int y;
	int z;

} adcvalues_t;

void init_ADC(void);
