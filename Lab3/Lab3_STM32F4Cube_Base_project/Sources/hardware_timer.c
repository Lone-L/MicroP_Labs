#include "stm32f4xx_hal.h"

/* Hardware Timer 3 is TIM3 for the 7-segment display
	 Hardware Timer 4 is TIM4 for the Visuals LEDs */
	 
/* Global variables ----------------------------------------------------------*/
TIM_HandleTypeDef timmy3;	/* Needs to be global to be used by TIM3_IRQHandler */
TIM_HandleTypeDef timmy4;	/* Needs to be global to be used by TIM4_IRQHandler */

/* Private variables ---------------------------------------------------------*/
static int HARDWARE_TIMER3_ELAPSED = 0;	/* Indicates when the timer 3 counter has reached zero */
static int HARDWARE_TIMER4_ELAPSED = 0;	/* Indicates when the timer 4 counter has reached zero */

/**
  * @brief  Initialize the hardware timer 3.
  * @param  None
  * @retval None
  */
void HardwareTimer3_Init(void)
{
	__TIM3_CLK_ENABLE();	/* Enable TIM3 interface clock */
	
	/* Doc 3 - Technical and Electrical 2.2.21 Timers and Watchdog Table 4
		 Says Max Timer Clock is 84 MHz for TIM3 */
	/* Doc 5 - Reference Manual 2.3 Memory Map Table 1
		 Says TIM3 is on APB1 (also in the tutorial slides). The SystemClock_Config sets the APB1 divider to 4
		 so 168 MHz / 4 is 42 MHz. But from the tutorial 3 slides, the APB1 Timer clocks are multiplied by 2
		 so the clock frequency is 84 MHz. */
	
	/* Desired_timer_frequency = Timer_input_frequency / (prescaler * period)
		 Timer_input_frequency = 84 MHz
		 Desired_timer_frequency = 800 Hz (switch 7-segment digits at this frequency to reduce flickering)
		 ---> prescaler * period = 84 MHz / 800 Hz = 105000 = 21 * 5000
		 ---> prescaler = 21, period = 5000 (our choice, man) */
	
	timmy3.Instance = TIM3;																/* Use TIM3 */
	timmy3.Init.Prescaler = 21;														/* Set prescaler to 21 */
	timmy3.Init.CounterMode = TIM_COUNTERMODE_DOWN;				/* Count down */
	timmy3.Init.Period = 5000;														/* Set period count register to 5000 */
	timmy3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;		/* Set clock division to 1 */
	timmy3.Init.RepetitionCounter = 0;										/* Not valid for TIM3 */
	timmy3.Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;			/* Not using channels */
	
	/* Initialize the TIM Time Base Unit */
	HAL_TIM_Base_Init(&timmy3);
	
	/* Start TIM3 in interrupt mode */
	HAL_TIM_Base_Start_IT(&timmy3);
	
	/* Set priority for TIM3 IRQ */
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	
	/* Enable IRQ for the TIM3 Timer*/
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

/**
  * @brief  Tells whether the hardware timer has elapsed.
  * @param  None
	* @retval int: 1 if elapsed, 0 otherwise
  */
int HardwareTimer3_Elapsed(void)
{
	return HARDWARE_TIMER3_ELAPSED;
}

/**
  * @brief  Clear the elapsed flag for timer 3.
  * @param  None
  * @retval None
  */
void HardwareTimer3_ClearElapsed(void)
{
	HARDWARE_TIMER3_ELAPSED = 0;
}

/**
  * @brief  Initialize the hardware timer 4.
  * @param  None
  * @retval None
  */
void HardwareTimer4_Init(void)
{
	__TIM4_CLK_ENABLE();	/* Enable TIM4 interface clock */
		
	/* Desired_timer_frequency = Timer_input_frequency / (prescaler * period)
		 Timer_input_frequency = 84 MHz (same as TIM3, TIM4 is also on APB1)
		 Desired_timer_frequency = 8 Hz (Rotate the LEDs at that frequency for the visuals)
		 ---> prescaler * period = 84 MHz / 8 Hz = 10500000 = 2100 * 5000
		 ---> prescaler = 5000, period = 2100 */
	
	timmy4.Instance = TIM4;																/* Use TIM4 */
	timmy4.Init.Prescaler = 2100;													/* Set prescaler to 2100 */
	timmy4.Init.CounterMode = TIM_COUNTERMODE_DOWN;				/* Count down */
	timmy4.Init.Period = 5000;														/* Set period count register to 5000 */
	timmy4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;		/* Set clock division to 1 */
	timmy4.Init.RepetitionCounter = 0;										/* Not valid for TIM4 */
	timmy4.Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;			/* Not using channels */
	
	/* Initialize the TIM Time Base Unit */
	HAL_TIM_Base_Init(&timmy4);
	
	/* Start TIM3 in interrupt mode */
	HAL_TIM_Base_Start_IT(&timmy4);
	
	/* Set priority for TIM3 IRQ */
	HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
	
	/* Enable IRQ for the TIM3 Timer*/
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

/**
  * @brief  Tells whether the hardware timer 4 has elapsed.
  * @param  None
	* @retval int: 1 if elapsed, 0 otherwise
  */
int HardwareTimer4_Elapsed(void)
{
	return HARDWARE_TIMER4_ELAPSED;
}

/**
  * @brief  Clear the elapsed flag for timer 4.
  * @param  None
  * @retval None
  */
void HardwareTimer4_ClearElapsed(void)
{
	HARDWARE_TIMER4_ELAPSED = 0;
}

/**
  * @brief  The period elapsed callback is called when the counter underflows (UEV Update event).
	* @param  TIM_HandleTypeDef *htim: handle to TIM configuration struct
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3) {
		HARDWARE_TIMER3_ELAPSED = 1;
	} else if (htim->Instance == TIM4) {
		HARDWARE_TIMER4_ELAPSED = 1;
	}
}
