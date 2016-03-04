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

/* Global variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config	(void);

int main(void)
{	
	float ax, ay, az;
	float tilt, filtered_tilt;
	float desired_tilt = 60.0;
	KalmanState kstate = {0.001, 0.0032, 0.0, 0.0, 0.0};	/* Filter parameters obtained by experiment and variance calculations */
	int counter = 0;	/* Keep track of how many accelerometer readings were made */
	
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
	
	SevenSegment_TurnOff();
	Visuals_TurnOn();
	
	while (1) {
		if (Accelerometer_HasNewData()) {
			Accelerometer_ClearNewData();
			Accelerometer_ReadAccel(&ax, &ay, &az);
			Accelerometer_Calibrate(&ax, &ay, &az);
			
			tilt = Accelerometer_GetTiltAngle(ax, ay, az);
			Kalmanfilter_asm(&tilt, &filtered_tilt, 1, &kstate);
			printf("%f\n", filtered_tilt);
			
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
		
		if (HardwareTimer3_Elapsed()) {
			HardwareTimer3_ClearElapsed();
			SevenSegment_ToggleDisplayedDigit();
		}
		
		if (HardwareTimer4_Elapsed()) {
			HardwareTimer4_ClearElapsed();
			Visuals_ToggleLEDs();
		}
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

