#include "stm32f4xx_hal.h"
#include "math.h"

#include "seven_segment.h"

/* Private variables ---------------------------------------------------------*/
static float displayed_angle = 0.0;
static int activated = 0;

/**
   * @brief Initialize SevenSegment GPIO pins.
   * @param None
   * @retval None
   */
void SevenSegment_Init(void)
{
	GPIO_InitTypeDef SEGMENT_PINS_E;
  __HAL_RCC_GPIOE_CLK_ENABLE();

	/* Pins E4 to E10 correspond to segments a to g on the 7-segment display.
		 Pins E11 to E14 correspond to turning on of the displays from left to right.
		 Pin E15 corresponds to the decimal point. */
	SEGMENT_PINS_E.Pin   = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 
											 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  SEGMENT_PINS_E.Mode  = GPIO_MODE_OUTPUT_PP;     /* Use output push-pull mode */
  SEGMENT_PINS_E.Pull  = GPIO_NOPULL;             /* Do not use pullup/pulldown resistors */
  SEGMENT_PINS_E.Speed = GPIO_SPEED_FREQ_MEDIUM;  /* 12.5 MHz - 50 MHz */

	/* Initialize GPIO configuration */
  HAL_GPIO_Init(GPIOE, &SEGMENT_PINS_E);
}

/**
   * @brief Set value to be displayed on the seven-segment display.
	 * @param float angle: angle to be displayed
   * @retval None
   */
void SevenSegment_SetDisplayValue(float angle)
{
	displayed_angle = angle;
}

/**
   * @brief Displays the next digit of the displayed_angle on the 7-segment display.
   * @param None
   * @retval None
   */
void SevenSegment_ToggleDisplayedDigit(void)
{
    static uint16_t digit_pins[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, DEGREE};	/* Pins to turn on to display each digit */
    static uint16_t display_pins[] = {GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14};								/* Pins to turn on to turn on each display */
		static uint16_t point_pin = GPIO_PIN_15;	/* Pin for the decimal point */
		static int display = 0; /* Keep track of current display to be turned on */
    int digit = 0;					/* Digit to display on the segment */
		int have_decimal_point = 0;
		float abs_angle = 0.0;
		
		/* If not activated, just turn off the display. */
		if (!activated) {
			GPIOE->ODR &= 0x000F;	/* Clear all bits to turn off the display */
			return;
		}
		
		/* We only care about absolute values. Angles should be between 0 and 180 so it's fine. */
		abs_angle = fabsf(displayed_angle);
		
		if (abs_angle < (float)(10.0)) {
			/* Format is X.YY° decimal point on display 0 */
			switch (display) {
				case 0: digit = (int)(abs_angle); have_decimal_point = 1; break;
				case 1: digit = (int)(abs_angle * 10) % 10; break;
				case 2: digit = (int)(abs_angle * 100) % 10; break;
				case 3: digit = 10;	break;	/* DEGREE sign */
				default: display = 0;	/* Should not happen */
			}
		} else if (fabsf(displayed_angle) < (float)(100.0)) {
			/* Format is XX.Y° decimal point on display 1 */
			switch (display) {
				case 0: digit = (int)(abs_angle) / 10; break;
				case 1: digit = (int)(abs_angle) % 10; have_decimal_point = 1; break;
				case 2: digit = (int)(abs_angle * 10) % 10; break;
				case 3: digit = 10; break;	/* DEGREE sign */
				default: display = 0;	/* Should not happen */
			}
		} else {
			/* Format is XXX° no decimal point */
			switch (display) {
				case 0: digit = (int)(abs_angle) / 100; break;
				case 1: digit = (int)(abs_angle / 10) % 10; break;
				case 2: digit = (int)(abs_angle) % 10; break;
				case 3: digit = 10;	break;/* DEGREE sign */
				default: display = 0;	/* Should not happen */
			}
		}
		
		GPIOE->ODR = GPIOE->ODR & 0x000F;	/* Clear the bits corresponding to GPIO_PIN_4 to GPIO_PIN_15 */
    GPIOE->ODR = GPIOE->ODR | (display_pins[display] | digit_pins[digit]);	/* Set bits corresponding to desired displayed digit */
		
		/* Add decimal point if it's there. */
		if (have_decimal_point)
			GPIOE->ODR |= point_pin;
		
    display = (display + 1) % 4;	/* Increment to next display */
}

/**
   * @brief Activates 7-segment display.
   * @param None
   * @retval None
   */
void SevenSegment_TurnOn(void)
{
	activated = 1;
}

/**
   * @brief Deactivates 7-segment display and turn off the LEDs.
   * @param None
   * @retval None
   */
void SevenSegment_TurnOff(void)
{
	GPIOE->ODR &= 0x000F;	/* Clear the bits corresponding to GPIO_PIN_4 to GPIO_PIN_15 */
	activated = 0;
}
