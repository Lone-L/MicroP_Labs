#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "RTE_Components.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

#define V_25			((float)0.760)	/* V, for temperature sensor calculation at 25 C */
#define AVG_SLOPE		((float)0.0025)	/* V/C, for temperature sensor calculation */
#define TEMP_REF		((float)25.0)	/* C */
#define THRESHOLD_TEMP	((float)34)	/* C */
	
#define V_REF			((float)3.0)	/* Volts */

#define TEMPERATURE_DISPLAY_COUNT 100   /* Number of temperature reads before displaying on 7-segment */

void Temperature_Init(void);
int start_Thread_TEMPERATURE(void);

extern osThreadId tid_Thread_TEMPERATURE;

extern ADC_HandleTypeDef ADC1_handle;



#define TEMPERATURE_SIGNAL 0x0001

#endif
