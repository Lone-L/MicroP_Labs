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
#include "Kalmanfilter.h"

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config	(void);
void initialize_ADC(void);
void initialize_GPIO(void);
void display_segment_val(float val);

/* Global variables ----------------------------------------------------------*/
ADC_HandleTypeDef			ADC1_handle;

int										SYSTICK_READ_TEMP_FLAG = 0;	/* Set by Systick_Handler and unset in main */

static float					filtered_temp;							/* Needs to be global to be displayed in the LogicAnalyser */
static float					unfiltered_temp;

int main(void)
{
	uint32_t v_sense;
	float tempinvolts, temperature;
	int counter = 0;
	HAL_StatusTypeDef rc;
	KalmanState kstate = {0.01, 0.3, 0.0, 0.1, 0.0};
		
  /* MCU Configuration----------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
	
  /* Configure the system clock */
  SystemClock_Config();
	
	/* Initialize the ADC and GPIO */
	initialize_ADC();
	initialize_GPIO();
	
	/* then call start */
	if (HAL_ADC_Start(&ADC1_handle) != HAL_OK)
		Error_Handler(ADC_INIT_FAIL);
	
	while (1) {
		if (SYSTICK_READ_TEMP_FLAG == 1) {
			if ((rc = HAL_ADC_PollForConversion(&ADC1_handle, 10000)) != HAL_OK)
				printf("Error: %d\n", rc);
			
			v_sense = HAL_ADC_GetValue(&ADC1_handle);	/* Read register from the ADC */
			tempinvolts = v_sense * (V_REF/4096);	/* Scale the reading into V (resolution is 12bits with VREF+ = 3.3V) */
			temperature = (tempinvolts - V_25) / AVG_SLOPE + TEMP_REF;// + FUDGE_FACTOR;	/* Formula for temperature from doc_05 p.230 */
			
			/* Grind through the Kalman filter */
			if (Kalmanfilter_asm(&temperature, &filtered_temp, 1, &kstate) != 0)
				printf("Overflow error\n");
			
			unfiltered_temp = temperature;
			printf("%d %f\n", counter, filtered_temp);
			//display_segment_val(filtered_temp);
			++counter;
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
}

/* Hal MSP_Init */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
	__ADC1_CLK_ENABLE();
}

/**
   * @brief Initialize GPIO typedef struct
   * @param None
   * @retval None
   */
void initialize_GPIO(void)
{
	  GPIO_InitTypeDef SEGMENT_PINS;
    __HAL_RCC_GPIOE_CLK_ENABLE();
    
    SEGMENT_PINS.Pin   = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; // use 7 pins for segment display
    // PIN 0 - 6  correspond to A-G 7-seg display ,
    // PIN 7 - 10 from left correspond to the 4 displays
    // PIN 11     the decimal point.
    
    SEGMENT_PINS.Mode  = GPIO_MODE_OUTPUT_PP;     // push pull mode
    SEGMENT_PINS.Pull  = GPIO_NOPULL;             // pin is used as output to drive 7-seg display
    SEGMENT_PINS.Speed = GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 MHz?
    
    HAL_GPIO_Init(GPIOE, &SEGMENT_PINS);
    GPIOE->ODR = 0xFFFF;
}

/**
   * @brief output float to 7 segment display
   * @param float value - the value to display on segment display
   * @retval None
   */
void display_segment_val(float val)
{
    static uint16_t SEG_CODES[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};
    static uint16_t segment_number[] = {GPIO_PIN_7, GPIO_PIN_8, GPIO_PIN_9 | GPIO_PIN_11, GPIO_PIN_10};
    int i;
    int temp_val = (int) (val * 10); // get 4 digits from temp reading

    for (i = 2; i >= 0; i--) {
        GPIOE->ODR = segment_number[i] | SEG_CODES[temp_val % 10];
        temp_val = (int) temp_val / 10;
        HAL_Delay(100);
    }
}

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
	
	/*Configures SysTick to provide 100Hz interval interrupts.*/
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/100);

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
