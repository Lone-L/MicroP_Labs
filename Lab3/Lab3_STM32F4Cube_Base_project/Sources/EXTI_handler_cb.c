#include "stm32f4xx_hal.h"
#include "accelerometer.h"
#include "EXTI_handler_cb.h"
#include "keypad.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch (GPIO_Pin)
	{
		case GPIO_PIN_0:
			ACCELEROMETER_Callback();// call accelerometer CB
			break;
		case GPIO_PIN_1:
		case GPIO_PIN_2:
		case GPIO_PIN_3:
		case GPIO_PIN_4:
			//call keypad CB
		  Keypad_KeyPressedCallback();
			break;						
	}
}


