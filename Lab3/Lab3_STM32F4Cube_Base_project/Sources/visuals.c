#include "stm32f4xx_hal.h"

static int led = 0;				/* Current LED being turned on */
static int activated = 0;	/* Whether the visuals are currently on */
static int direction = 0;	/* Rotation direction of LEDs: 0 is CCW, 1 is CW */

/**
   * @brief Initialize the visuals module.
   * @param None
   * @retval None
   */
void Visuals_Init(void)
{
	GPIO_InitTypeDef LED_PINS_D;
	
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	LED_PINS_D.Pin	 = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; /* LEDs are LD3:PD13, LD4:PD12, LD5:PD14, LD6:PD15 */
	LED_PINS_D.Mode  = GPIO_MODE_OUTPUT_PP;     /* Push-Pull mode */
  LED_PINS_D.Pull  = GPIO_NOPULL;             /* No pullup/pulldown resistors. */
  LED_PINS_D.Speed = GPIO_SPEED_FREQ_MEDIUM;  /* 12.5 MHz - 50 MHz */
	
	/* Initialize GPIO pins configuration */
	HAL_GPIO_Init(GPIOD, &LED_PINS_D);
}

/**
   * @brief Toggle LEDs to display the direction visuals.
   * @param None
   * @retval None
   */
void Visuals_ToggleLEDs(void)
{
	static int LEDS[] = {GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15, GPIO_PIN_12};
	
	if (activated) {
		GPIOD->ODR = GPIOD->ODR & 0x0FFF;			/* Clear bits 12 to 15 corresponding to the 4 LEDs. */
		GPIOD->ODR = GPIOD->ODR | LEDS[led];
		
		if (direction == 0) {	/* Counterclockwise */
			if (led == 0)
				led = 4;	/* Avoid getting -1 */
			
			led = (led - 1) % 4;
		} else {							/* Clockwise */
			led = (led + 1) % 4;
		}
	}
}

/**
   * @brief Set rotation direction of visual LEDs.
	 * @param int direction: 0 for counterlockwise (increase tilt), 1 for clockwise (decrease tilt)
   * @retval None
   */
void Visuals_SetDirection(int dir)
{
	direction = dir;
}

/**
   * @brief Activates the visual LEDs (does not toggle LEDs).
   * @param None
   * @retval None
   */
void Visuals_TurnOn(void)
{
	activated = 1;
}

/**
   * @brief Turns off all LEDs and deactivates visual LEDs.
   * @param None
   * @retval None
   */
void Visuals_TurnOff(void)
{
	GPIOD->ODR = GPIOD->ODR & 0x0FFF;	/* Clears bits 12 to 15 */
	activated = 0;
}
