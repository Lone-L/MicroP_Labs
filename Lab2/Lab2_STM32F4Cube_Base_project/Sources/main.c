/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : A program which showcases ADC and TIM3 under the new firmware
	                       The ADC
	* Author						 : Ashraf Suyyagh
	* Version            : 1.0.0
	* Date							 : January 14th, 2016
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "supporting_functions.h"
#include "main.h"

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config	(void);
void initialize_ADC(void);

/* Global variables ----------------------------------------------------------*/
ADC_HandleTypeDef			ADC1_handle;
int										SYSTICK_READ_TEMP_FLAG = 0;	/* Set by Systick_Handler and unset in main */

int main(void)
{
	uint32_t v_sense;
	float temperature;
	
  /* MCU Configuration----------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
	
  /* Configure the system clock */
  SystemClock_Config();
	
	initialize_ADC();
	
	while (1) {
		if (SYSTICK_READ_TEMP_FLAG == 1) {
			v_sense = HAL_ADC_GetValue(&ADC1_handle);
			temperature = ((float)v_sense - V_25) / AVG_SLOPE + TEMP_REF;	/* Formula for temperature from doc_05 p.230 */
			printf("Temperature = %f\n", temperature);
			SYSTICK_READ_TEMP_FLAG = 0;
		}
	}
}

/** 
   * @brief Initialize global ADC typedef struct
   * @param None
   * @retval None
   */
void initialize_ADC(void)
{
	/* Information on configuring the ADC found in p.104 doc 19 + doc 5 p.207 + doc 6 p.137*/
	ADC_ChannelConfTypeDef channel_config;	/* Channel configuration struct */
	
	ADC1_handle.Instance = ADC1;	/* Temperature sensor is in channel 16 of ADC1 */
	
	/* Set up the InitTypeDef sub-struct */
	/* The clock for the ADC is PCKL2,at 168 MHz. */
	ADC1_handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;	/* Select division by 8 for prescaler. */
	ADC1_handle.Init.Resolution = ADC_RESOLUTION_12B;						/* Select 12 bits resolution */
	ADC1_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;						/* This places the padding zeros on the MS byte */
	ADC1_handle.Init.ScanConvMode = DISABLE;										/* Use single-channel mode */
	ADC1_handle.Init.EOCSelection = ADC_EOC_SEQ_CONV;						/* wat */
	ADC1_handle.Init.ContinuousConvMode = ENABLE;								/* Want continuous conversion mode */
	ADC1_handle.Init.DMAContinuousRequests = DISABLE;						/* wat */
	ADC1_handle.Init.NbrOfConversion = 1;												/* 1 channel / 1 conversion. Simple as that. */
	ADC1_handle.Init.DiscontinuousConvMode = DISABLE;						/* why discontinuous? */
	/* don't even bother with NbrOfDiscConversion */
	ADC1_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;			/* Disable external trigger */
	
	/* We don't know what the NbrOfCurrentConversionRank */
	/* Don't know about the rest either... */
	
	/* Initialize global ADC parameters*/
	if (HAL_ADC_Init(&ADC1_handle) != HAL_OK)
		Error_Handler(ADC_INIT_FAIL);
	
	/* Configure the channel */
	channel_config.Channel = ADC_CHANNEL_16;
	channel_config.Rank = 1;
	channel_config.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	channel_config.Offset = 0;
	
	if (HAL_ADC_ConfigChannel(&ADC1_handle, &channel_config) != HAL_OK)
		Error_Handler(ADC_CH_CONFIG_FAIL);
	
	/* then call start */
	if (HAL_ADC_Start(&ADC1_handle) != HAL_OK)
		Error_Handler(ADC_INIT_FAIL);
}

/* Hal MSP_Init */

/** System Clock Configuration */
void SystemClock_Config(void)
{
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
	
	/*Configures SysTick to provide 1ms interval interrupts.*/
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	/* This function sets the source clock for the internal SysTick Timer to be the maximum,
	   in our case, HCLK is now 168MHz*/
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
   * @brief A function used to setup the ADC to sample Channel 16
   * @retval None
   */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
}
#endif
