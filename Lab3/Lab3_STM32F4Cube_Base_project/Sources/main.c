/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program subroutine
	* Author						 : Ashraf Suyyagh
	* Version            : 1.0.0
	* Date							 : January 14th, 2016
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "supporting_functions.h"

#include "accelerometer.h"
#include "kalmanfilter.h"
#include "seven_segment.h"
#include "visuals.h"
#include "hardware_timer.h"
#include "keypad.h"

/* Global variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
typedef enum _StateEnum {
	WAIT_MODE = 0,
	INPUT_MODE,
	ANGLE_MODE
} StateEnum;

static StateEnum state = WAIT_MODE;
static int desired_tilt = 0;

/* Private function prototypes -----------------------------------------------*/
void handle_input_mode(void);
void handle_angle_mode(void);
void SystemClock_Config	(void);

int main(void)
{	
  /* MCU Configuration----------------------------------------------------------*/
  HAL_Init();
	
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
	Accelerometer_Init();
	SevenSegment_Init();
	Visuals_Init();
	HardwareTimer3_Init();
	HardwareTimer4_Init();
	Keypad_Init();

	SevenSegment_TurnOff();
	Visuals_TurnOff();
	printf("peripherals initialized\n");
	
	/* Polling loop */
	while (1) {
		/* Check if it's time to toggle 7-segment digits */
		if (HardwareTimer3_Elapsed()) {
			HardwareTimer3_ClearElapsed();
			SevenSegment_ToggleDisplayedDigit();
		}
		
		/* Check if it's time to toggle visual LEDs */
		if (HardwareTimer4_Elapsed()) {
			HardwareTimer4_ClearElapsed();
			Visuals_ToggleLEDs();
		}
		
		/* Behavior depends on current state. */
		switch (state) {
			case WAIT_MODE:
				/* We are waiting for keypad press to read into desired tilt.
					 The screen is cleared and visuals turned off. */
				if (Keypad_Pressed()) {
					Keypad_ClearPressed();
					desired_tilt = 0;	/* Start at 0. */
					SevenSegment_TurnOff();
					SevenSegment_TurnOn();	/* Used to display current entered value */
					Visuals_TurnOff();
					state = INPUT_MODE;
				}
				
				break;
				
			case INPUT_MODE:
				/* Input mode: read key presses and store into desired tilt.
					 Also displays current entered value on 7-segment display. */
				if (Keypad_Pressed()) {
					Keypad_ClearPressed();
					handle_input_mode();
				}
				
				break;
			
			case ANGLE_MODE:
				/* Desired tilt has been chosen. Now go processing angles from accelerometer and display visuals.
					 If keypad is pressed again, go back to input mode to read new desired tilt value. */
				if (Keypad_Pressed()) {
					Keypad_ClearPressed();
					desired_tilt = 0;
					SevenSegment_TurnOff();
					SevenSegment_TurnOn();
					Visuals_TurnOff();
					state = INPUT_MODE;
					continue;
				}
					
				handle_angle_mode();
				break;
			
			default:
				/* Shouldn't happen. */
				state = WAIT_MODE;
				break;
		}
	}
}

/**
   * @brief  Handles input mode (reads input from keypad).
   * @param  None
   * @retval None
   */
void handle_input_mode(void)
{
	int key_pressed = -1;
	
	key_pressed = Keypad_ScanKey();
	
	printf("%d\n", key_pressed);
	
	/* We're done reading the new value. Go to angle mode. */
	if (key_pressed == ENTER_KEY) {
		state = ANGLE_MODE;
		return;
	}
	
	/* Ignore invalid keys */
	if (key_pressed == -1)
		return;
	
	/* Update desired tilt from next digit. */
	desired_tilt = desired_tilt * 10 + key_pressed;
	
	/* Go back to 0 if tilt value greater than 180. */
	if (desired_tilt > 180)
		desired_tilt = 0;
	
	/* Display current entered value on 7-segment. */
	SevenSegment_SetDisplayValue((float)desired_tilt);
}

/**
   * @brief  Handles angle mode (processes tilt angle and visuals).
   * @param  None
   * @retval None
   */
void handle_angle_mode(void)
{
	/* These are preserved between function calls (static) */
	static KalmanState kstate = {0.001, 0.0032, 0.0, 0.0, 0.0};	/* Filter parameters obtained by experiment and variance calculations */
	static int counter = 0;	/* Keep track of how many accelerometer readings were made */
	
	float ax, ay, az;
	float tilt, filtered_tilt;
	
	/* Check if there is new data available on accelerometer, and process it. */
	if (Accelerometer_HasNewData()) {
		Accelerometer_ClearNewData();
		Accelerometer_ReadAccel(&ax, &ay, &az);
		Accelerometer_Calibrate(&ax, &ay, &az);
			
		tilt = Accelerometer_GetTiltAngle(ax, ay, az);
		Kalmanfilter_asm(&tilt, &filtered_tilt, 1, &kstate);	/* filter the tilt angle */
		
		/* Determine how measured angle relates to desired tilt, and control visuals/7-segment accordingly. */
		if (desired_tilt > filtered_tilt + (float)(5.0)) {
			SevenSegment_TurnOff();
			Visuals_TurnOn();
			Visuals_SetDirection(COUNTERCLOCKWISE);
		} else if (desired_tilt < filtered_tilt - (float)(5.0)) {
			SevenSegment_TurnOff();
			Visuals_TurnOn();
			Visuals_SetDirection(CLOCKWISE);
		} else {
			/* We are within 5 degrees of the desired tilt. Display angle on 7-segment. */
			Visuals_TurnOff();
			SevenSegment_TurnOn();
		}
		
		/* Give enough time to display a value on 7-segment.
			 Display only one out of SEVEN_SEGMENT_DISPLAY_COUNT values on the 7-segment. */
		if (counter % SEVEN_SEGMENT_DISPLAY_COUNT == 0)
			SevenSegment_SetDisplayValue(filtered_tilt);
		
		++counter;
	}
}

/** System Clock Configuration*/
void SystemClock_Config(void){

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType 	= RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState 			 	= RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState 		= RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource 	= RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM 				= 8;
  RCC_OscInitStruct.PLL.PLLN 				= 336;
  RCC_OscInitStruct.PLL.PLLP 				= RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ 				= 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){Error_Handler(RCC_CONFIG_FAIL);};

  RCC_ClkInitStruct.ClockType 			= RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource 		= RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider 	= RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider 	= RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider 	= RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5)!= HAL_OK){Error_Handler(RCC_CONFIG_FAIL);};
	
	/*Configures SysTick to provide 1ms interval interrupts. SysTick is already 
	  configured inside HAL_Init, I don't kow why the CubeMX generates this call again*/
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	/* This function sets the source clock for the internal SysTick Timer to be the maximum,
	   in our case, HCLK is now 168MHz*/
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line){
}
#endif

