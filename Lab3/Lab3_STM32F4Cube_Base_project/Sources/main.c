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
	
	while (1) {
		if (HardwareTimer3_Elapsed()) {
			HardwareTimer3_ClearElapsed();
			SevenSegment_ToggleDisplayedDigit();
		}
		
		if (HardwareTimer4_Elapsed()) {
			HardwareTimer4_ClearElapsed();
			Visuals_ToggleLEDs();
		}
		
		switch (state) {
			case WAIT_MODE:
				if (Keypad_Pressed()) {
					Keypad_ClearPressed();
					state = INPUT_MODE;
					desired_tilt = 0;
					SevenSegment_TurnOff();
					SevenSegment_TurnOn();
					Visuals_TurnOff();
				}
				
				break;
				
			case INPUT_MODE:
				if (Keypad_Pressed()) {
					Keypad_ClearPressed();
					handle_input_mode();
				}
				
				break;
			
			case ANGLE_MODE:
				if (Keypad_Pressed()) {
					Keypad_ClearPressed();
					SevenSegment_TurnOff();
					Visuals_TurnOff();
					state = WAIT_MODE;
					continue;
				}
					
				handle_angle_mode();
				break;
			
			default:
				state = WAIT_MODE;
				break;
		}
	}
}

void handle_input_mode(void)
{
	int key_pressed = -1;
	
	key_pressed = Keypad_ScanKey();
	
	printf("%d\n", key_pressed);
	
	if (key_pressed == ENTER_KEY) {
		state = ANGLE_MODE;
		return;
	}
	
	/* Ignore invalid keys */
	if (key_pressed == -1)
		return;
	
	desired_tilt = desired_tilt * 10 + key_pressed;
	
	if (desired_tilt > 180)
		desired_tilt = 0;
	
	SevenSegment_SetDisplayValue((float)desired_tilt);
}

void handle_angle_mode(void)
{
	/* These are preserved between function calls (static) */
	static KalmanState kstate = {0.001, 0.0032, 0.0, 0.0, 0.0};	/* Filter parameters obtained by experiment and variance calculations */
	static int counter = 0;	/* Keep track of how many accelerometer readings were made */
	
	float ax, ay, az;
	float tilt, filtered_tilt;
	
	if (Accelerometer_HasNewData()) {
		Accelerometer_ClearNewData();
		Accelerometer_ReadAccel(&ax, &ay, &az);
		Accelerometer_Calibrate(&ax, &ay, &az);
			
		tilt = Accelerometer_GetTiltAngle(ax, ay, az);
		Kalmanfilter_asm(&tilt, &filtered_tilt, 1, &kstate);
			
		if (desired_tilt > filtered_tilt + (float)(5.0)) {
			SevenSegment_TurnOff();
			Visuals_TurnOn();
			Visuals_SetDirection(COUNTERCLOCKWISE);
		} else if (desired_tilt < filtered_tilt - (float)(5.0)) {
			SevenSegment_TurnOff();
			Visuals_TurnOn();
			Visuals_SetDirection(CLOCKWISE);
		} else {
			Visuals_TurnOff();
			SevenSegment_TurnOn();
		}
		
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

