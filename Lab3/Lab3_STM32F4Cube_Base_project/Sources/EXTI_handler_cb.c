#include "stm32f4xx_hal.h"
#include "accelerometer.h"
#include "EXTI_handler_cb.h"
#include "keypad.h"

/**
   * @brief  GPIO EXTI Callback. Calls Accelerometer or Keypad callbacks depending on GPIO pin.
   * @param  uint16_t GPIO_Pin: Which GPIO pin triggered the callback.
   * @retval None
   */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/* GPIO_PIN_0 corresponds to the Accelerometer (PE0 DRY event)
		 GPIO_PIN_1 to GPIO_PIN_4 correspond to Keypad column pins (key pressed event) */
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
		default:
			/* Shouldn't happen */
			break;
	}
}


