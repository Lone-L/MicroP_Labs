#include "stm32f4xx_hal.h"
#include "math.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "seven_segment.h"

/* Private variables ---------------------------------------------------------*/
static uint16_t digit_pins[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, DEGREE};				/* Pins to turn on to display each digit */
static uint16_t display_pins[] = {GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14};								/* Pins to turn on to turn on each display */

static float displayed_angle = 0.0;
static float displayed_temp = 0.0;
static int activated = 0;
static int flashing = 0;
static DisplayMode display_mode = ANGLE_MODE;

void Thread_SEGMENT (void const *argument);                 // thread function
osThreadId tid_Thread_SEGMENT;                              // thread id
osThreadDef(Thread_SEGMENT, osPriorityNormal, 1, 0);		// thread definition struct (last argument default stack size)

osMutexId segment_mutex;
osMutexDef(segment_mutex);

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
void SevenSegment_SetDisplayValue_Temp(float temp)
{
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	displayed_temp = temp;
	osMutexRelease(segment_mutex);
}

/**
   * @brief Get value to be displayed on the seven-segment display.
   * @param None
   * @retval float angle: angle to be displayed
   */
float SevenSegment_GetDisplayValue_Temp(void)
{
	float temp;
	
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	temp = displayed_temp;
	osMutexRelease(segment_mutex);
	
	return temp;
}

/**
   * @brief Set value to be displayed on the seven-segment display.
   * @param float angle: angle to be displayed
   * @retval None
   */
void SevenSegment_SetDisplayValue_Angle(float angle)
{
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	displayed_angle = angle;
	osMutexRelease(segment_mutex);
}

/**
   * @brief Get value to be displayed on the seven-segment display.
   * @param None
   * @retval float angle: angle to be displayed
   */
float SevenSegment_GetDisplayValue_Angle(void)
{
	float angle;
	
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	angle = displayed_angle;
	osMutexRelease(segment_mutex);
	
	return angle;
}

/**
   * @brief Displays the next digit of the displayed_angle on the 7-segment display.
   * @param None
   * @retval None
   */
void SevenSegment_ToggleDisplayedDigit_Angle(void)
{
	static uint16_t point_pin = GPIO_PIN_15;	/* Pin for the decimal point */
	static int display = 0;                     /* Keep track of current display to be turned on */
	int digit = 0;					            /* Digit to display on the segment */
	int have_decimal_point = 0;
	float abs_angle = 0.0;
	float copy_displayed_angle;
	
	/* If not activated, just turn off the display. */
	if (!SevenSegment_IsActivated()) {
		GPIOE->ODR &= 0x000F;	/* Clear all bits to turn off the display */
		return;
	}
	
	copy_displayed_angle = SevenSegment_GetDisplayValue_Angle();
	
	/* We only care about absolute values. Angles should be between 0 and 180 so it's fine. */
	abs_angle = fabsf(copy_displayed_angle);
	
	if (abs_angle < (float)(10.0)) {
		/* Format is X.YY° decimal point on display 0 */
		switch (display) {
			case 0: digit = (int)(abs_angle); have_decimal_point = 1; break;
			case 1: digit = (int)(abs_angle * 10) % 10; break;
			case 2: digit = (int)(abs_angle * 100) % 10; break;
			case 3: digit = 10;	break;	/* DEGREE sign */
			default: display = 0;	/* Should not happen */
		}
	} else if (fabsf(copy_displayed_angle) < (float)(100.0)) {
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
   * @brief output global displayed_segment_value float to 7 segment display
   * @param None
   * @retval None
   */
void SevenSegment_ToggleDisplayedDigit_Temp(void)
{
	static int segment = 0; /* Keep track of current segment to be turned on */
	int int_value;		    /* Example: 12.3 would be converted to 123, keeping track of the tenths */
    int digit;				/* Digit to display on the segment */

	/* If not activated, just turn off the display. */
	if (!SevenSegment_IsActivated()) {
		GPIOE->ODR &= 0x000F;	/* Clear all bits to turn off the display */
		return;
	}

	int_value = (int)(SevenSegment_GetDisplayValue_Temp() * 10);
	
	switch (segment) {
		case 2: digit = int_value % 10; break;
		case 1: digit = (int_value / 10) % 10; break;
		case 0: digit = (int_value / 100); break;
	}
	
	GPIOE->ODR = GPIOE->ODR & 0x000F;
    GPIOE->ODR = GPIOE->ODR | (display_pins[segment] | digit_pins[digit]);
	
	/* Add decimal point */
	if (segment == 1)
		GPIOE->ODR |= GPIO_PIN_15;
	
    segment = (segment + 1) % 3;
}

/**
   * @brief Activates 7-segment display.
   * @param None
   * @retval None
   */
void SevenSegment_TurnOn(void)
{
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	activated = 1;
	osMutexRelease(segment_mutex);
}

/**
   * @brief Deactivates 7-segment display and turn off the LEDs.
   * @param None
   * @retval None
   */
void SevenSegment_TurnOff(void)
{
	osMutexWait(segment_mutex, osWaitForever);
	GPIOE->ODR &= 0x000F;	/* Clear the bits corresponding to GPIO_PIN_4 to GPIO_PIN_15 */
	activated = 0;
	osMutexRelease(segment_mutex);
}

/**
   * @brief Is segment activated?
   * @param None
   * @retval int activated flag.
   */
int SevenSegment_IsActivated(void)
{
	int act;
	
	osMutexWait(segment_mutex, osWaitForever);
	act = activated;
	osMutexRelease(segment_mutex);
	
	return act;
}

/**
   * @brief Starts flashing the display.
   * @param None
   * @retval None
   */
void SevenSegment_StartFlashing(void)
{
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	flashing = 1;
	osMutexRelease(segment_mutex);
}

/**
   * @brief Stops flashing the display.
   * @param None
   * @retval None
   */
void SevenSegment_StopFlashing(void)
{
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	flashing = 0;
	activated = 1;    /* set activated to one to avoid edge case where when seven segment stops flashing but activated is still disabled */
	osMutexRelease(segment_mutex);
}

/**
   * @brief Returns flashing flag.
   * @param None
   * @retval int flashing flag.
   */
int SevenSegment_GetFlashing(void)
{
	int flash;
	
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	flash = flashing;
	osMutexRelease(segment_mutex);
	
	return flash;
}

/**
   * @brief Sets seven-segment display mode (ANGLE_MODE, TEMP_MODE).
   * @param None
   * @retval None
   */
void SevenSegment_SetDisplayMode(DisplayMode mode)
{
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	display_mode = mode;
	osMutexRelease(segment_mutex);
}

/**
   * @brief Sets seven-segment display mode (ANGLE_MODE, TEMP_MODE).
   * @param None
   * @retval None
   */
DisplayMode SevenSegment_GetDisplayMode(void)
{
	DisplayMode mode;
	
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	osMutexWait(segment_mutex, osWaitForever);
	mode = display_mode;
	osMutexRelease(segment_mutex);
	
	return mode;
}

/**
   * @brief Toggles seven-segment display mode (ANGLE_MODE, TEMP_MODE).
   * @param None
   * @retval None
   */
void SevenSegment_ToggleDisplayMode(void)
{
	osMutexWait(segment_mutex, osWaitForever);
	
	/* We have race conditions. Need to use mutexes to protect these global flags. */
	if (display_mode == ANGLE_MODE)
		display_mode = TEMP_MODE;
	else
		display_mode = ANGLE_MODE;
	
	osMutexRelease(segment_mutex);
}

/*----------------------------------------------------------------------------
 *      Create the thread within RTOS context
 *---------------------------------------------------------------------------*/
int start_Thread_SEGMENT (void)
{
  tid_Thread_SEGMENT = osThreadCreate(osThread(Thread_SEGMENT), NULL); // Start SEGMENT Thread
  if (!tid_Thread_SEGMENT) return(-1);

  segment_mutex = osMutexCreate(osMutex(segment_mutex));
	
  return(0);
}

 /*----------------------------------------------------------------------------
*      Thread  'SEGMENT': Display values on 7-segment display
 *---------------------------------------------------------------------------*/
void Thread_SEGMENT (void const *argument) 
{
	int counter = 0;
	DisplayMode mode;
	
	while(1)
	{
		osSignalWait(SEGMENT_SIGNAL, osWaitForever);
		
		if (counter % FLASH_PERIOD == 0)
		{
			if (SevenSegment_GetFlashing()) {
				osMutexWait(segment_mutex, osWaitForever);
				activated = !activated;
				osMutexRelease(segment_mutex);
			}
		}
		
		mode = SevenSegment_GetDisplayMode();
		
		if (mode == TEMP_MODE) {
			SevenSegment_ToggleDisplayedDigit_Angle();
		} else if (mode == ANGLE_MODE) {
			SevenSegment_ToggleDisplayedDigit_Temp();
		}

		counter++;
	}
}
	
/*----------------------------------------------------------------------------
 *      
 *---------------------------------------------------------------------------*/
