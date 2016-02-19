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
static float					filtered_temp;											/* Needs to be global to be displayed in the LogicAnalyser */
static float					unfiltered_temp;
static float					displayed_segment_value;						/* Value currently being displayed on the 7-segment screen. */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config	(void);
void initialize_ADC(void);
void initialize_GPIO(void);
void initialize_LCD(void);
void display_segment_val(void);
void toggle_LEDs(void);
void turn_off_LEDs(void);
void display_LCD_num(float num);

/* Global variables ----------------------------------------------------------*/
ADC_HandleTypeDef			ADC1_handle;
int										SYSTICK_READ_TEMP_FLAG = 0;					/* Set by Systick_Handler to read temperature from ADC */
int										SYSTICK_DISPLAY_SEGMENT_FLAG = 0;		/* Set by Systick_Handler to display data on the 7-segment display */
int										ALARM_TRIGGERED_FLAG = 0;						/* Set when the temperature is higher than THRESHOLD_TEMP */

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
	
	/* Initialize the ADC and GPIO and LCD display */
	initialize_ADC();
	initialize_GPIO();
	initialize_LCD();
	
	/* then call start */
	if (HAL_ADC_Start(&ADC1_handle) != HAL_OK)
		Error_Handler(ADC_INIT_FAIL);
	
	while (1) {
		if (SYSTICK_READ_TEMP_FLAG == 1) {
			if ((rc = HAL_ADC_PollForConversion(&ADC1_handle, 10000)) != HAL_OK)
				printf("Error: %d\n", rc);
			
			v_sense = HAL_ADC_GetValue(&ADC1_handle);	/* Read register from the ADC */
			tempinvolts = v_sense * (V_REF / 4096);	/* Scale the reading into V (resolution is 12bits with VREF+ = 3.3V) */
			temperature = (tempinvolts - V_25) / AVG_SLOPE + TEMP_REF;// + FUDGE_FACTOR;	/* Formula for temperature from doc_05 p.230 */
			
			/* Grind through the Kalman filter */
			if (Kalmanfilter_asm(&temperature, &filtered_temp, 1, &kstate) != 0)
				printf("Overflow error\n");
			
			unfiltered_temp = temperature;
//			printf("%d %f\n", counter, filtered_temp);
			
			/* Display only once out of this many times the 7-segment display */
			if (counter % SEGMENT_DISPLAY_PERIOD == 0) {
				displayed_segment_value = filtered_temp;
				display_LCD_num(filtered_temp);
			}
			
			if (ALARM_TRIGGERED_FLAG == 1) {
				/* Avoid spurious noise by waiting for a lower threshold before turning off alarm */
				if (filtered_temp < LOWER_THRESHOLD_TEMP) {
					ALARM_TRIGGERED_FLAG = 0;
					turn_off_LEDs();
				}
			} else {
				if (filtered_temp > THRESHOLD_TEMP)
					ALARM_TRIGGERED_FLAG = 1;
			}

			if (ALARM_TRIGGERED_FLAG == 1) {
				if (counter % ALARM_LED_TOGGLE_PERIOD == 0)
					toggle_LEDs();
			}
			
			++counter;
			SYSTICK_READ_TEMP_FLAG = 0;	/* Reset the flag */
		}
		
		/* Systick_Handler triggers the flag once every 50ms */
		if (SYSTICK_DISPLAY_SEGMENT_FLAG == 1) {
			display_segment_val();
			SYSTICK_DISPLAY_SEGMENT_FLAG = 0;
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
	  GPIO_InitTypeDef SEGMENT_PINS_E, LED_PINS_D, LCD_PINS_C, LCD_PINS_B;
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	
		LCD_PINS_B.Pin       = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2;
		// GPIO_PIN_0 <= RS  | GPIO_PIN_1 <= R/W | GPIO_PIN_2 <= E 
		// 
		LCD_PINS_B.Mode  = GPIO_MODE_OUTPUT_PP;     // push pull mode
    LCD_PINS_B.Pull  = GPIO_NOPULL;             // pin is used as output to drive 7-seg display
    LCD_PINS_B.Speed = GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 MHz?
	
	
		LCD_PINS_C.Pin       = GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_4  | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_11;
		// GPIO_PIN_1,2,4-9, 11 <= D0 - D7
		LCD_PINS_C.Mode  = GPIO_MODE_OUTPUT_PP;     // push pull mode
    LCD_PINS_C.Pull  = GPIO_NOPULL;             // pin is used as output to drive 7-seg display
    LCD_PINS_C.Speed = GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 MHz?
	
	
		LED_PINS_D.Pin	     = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; // LEDs are LD3:PD13, LD4:PD12, LD5:PD14, LD6:PD15
	
		LED_PINS_D.Mode  = GPIO_MODE_OUTPUT_PP;     // push pull mode
    LED_PINS_D.Pull  = GPIO_NOPULL;             // pin is used as output to drive 7-seg display
    LED_PINS_D.Speed = GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 MHz?
	


		SEGMENT_PINS_E.Pin   = GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; // use 7 pins for segment display
		// PIN 0 - 6  correspond to A-G 7-seg display ,
    // PIN 7 - 10 from left correspond to the 4 displays
    // PIN 11     the decimal point.
    

    SEGMENT_PINS_E.Mode  = GPIO_MODE_OUTPUT_PP;     // push pull mode
    SEGMENT_PINS_E.Pull  = GPIO_NOPULL;             // pin is used as output to drive 7-seg display
    SEGMENT_PINS_E.Speed = GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 MHz?
    
		HAL_GPIO_Init(GPIOB, &LCD_PINS_B);
		HAL_GPIO_Init(GPIOC, &LCD_PINS_C);
		HAL_GPIO_Init(GPIOD, &LED_PINS_D);
    HAL_GPIO_Init(GPIOE, &SEGMENT_PINS_E);
		

}

/**
   * @brief Initializes the LCD display
	 * @param None
   * @retval None
   */
void initialize_LCD(void)
{
	/* Initialization sequence from
	 * http://www.protostack.com/blog/2010/03/character-lcd-displays-part-1/ */
		
	/* Clear the display */
	SET_B_LINE(GPIOB->ODR, 0, 0, 0);
	SET_D_LINE(GPIOC->ODR, 0x00);
	DELAY(CMD_DELAY)
	SET_D_LINE(GPIOC->ODR, 0x01);
	SET_B_LINE(GPIOB->ODR, 1, 0, 0);
	DELAY(CMD_DELAY)
	SET_B_LINE(GPIOB->ODR, 0, 0, 0);
	HAL_Delay(15);
		
		/* Reset cursor */
//		SET_D_LINE(GPIOC->ODR, 0x02);
//		SET_B_LINE(GPIOB->ODR, 1, 0, 0);
//		DELAY(CMD_DELAY)
//		SET_B_LINE(GPIOB->ODR, 0, 0, 0);
//		HAL_Delay(15);
	
	/* Set function mode 2 line */
	SET_D_LINE(GPIOC->ODR, 0x38);
	SET_B_LINE(GPIOB->ODR, 1, 0, 0);
	DELAY(CMD_DELAY)
	SET_B_LINE(GPIOB->ODR, 0, 0, 0);
	DELAY(CMD_DELAY);
	
	/* Display On */
	SET_D_LINE(GPIOC->ODR, 0x0c);
	SET_B_LINE(GPIOB->ODR, 1, 0, 0);
	DELAY(CMD_DELAY)
	SET_B_LINE(GPIOB->ODR, 0, 0, 0);
	DELAY(CMD_DELAY);

	SET_D_LINE(GPIOC->ODR, 0x06);
	SET_B_LINE(GPIOB->ODR, 1, 0, 0);
	DELAY(CMD_DELAY)
	SET_B_LINE(GPIOB->ODR, 0, 0, 0);
	DELAY(CMD_DELAY);
}

/**
   * @brief Prints float value on the LCD display
	 * @param float num: value to be displayed
   * @retval None
   */
void display_LCD_num(float num) 
{
	char buffer[6];
	int j;
	snprintf(buffer, sizeof buffer, "%.1f", num);
	buffer[4] = 0xdf;
	buffer[5] = 'A';
	
	/* Reset cursor to initial position */
	DELAY(CMD_DELAY)
	SET_D_LINE(GPIOC->ODR, 0x02);
	DELAY(CMD_DELAY)
	SET_B_LINE(GPIOB->ODR, 1, 0, 0);
	DELAY(CMD_DELAY)
	SET_B_LINE(GPIOB->ODR, 0, 0, 0);
	DELAY(CMD_DELAY)
	
	/* Print each character on the screen */
	for (j = 0; j < sizeof(buffer); j++ ){
		SET_D_LINE(GPIOC->ODR, buffer[j]);
		SET_B_LINE(GPIOB->ODR, 1, 0, 1);
		DELAY(CMD_DELAY)
		SET_B_LINE(GPIOB->ODR, 0, 0, 0);
		DELAY(CMD_DELAY)
	}
}

/**
   * @brief alternate toggling of the 4 LEDs used for alarm display
   * @param None
   * @retval None
   */
void toggle_LEDs(void)
{
		static int LEDS[] = {GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15, GPIO_PIN_12};
		static int led = 0;
		GPIOD->ODR = GPIOD->ODR & 0x0FFF;
		GPIOD->ODR = GPIOD->ODR | LEDS[led];	/* Note: we currently use Port D only for the LEDs, but otherwise we should only
																			update the bits that concern us, not to overwrite other pins! */
		
		led = (led + 1) % 4;
}

/**
   * @brief turns off all LEDs when alarm is deactivated
   * @param None
   * @retval None
   */
void turn_off_LEDs(void)
{
		GPIOD->ODR = GPIOD->ODR & 0x0FFF;
}

/**
   * @brief output global displayed_segment_value float to 7 segment display
   * @param None
   * @retval None
   */
void display_segment_val(void)
{
    static uint16_t SEG_CODES[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};
    static uint16_t segment_number[] = {GPIO_PIN_11, GPIO_PIN_12 | GPIO_PIN_15, GPIO_PIN_13, GPIO_PIN_14};
		static int segment = 0; /* Keep track of current segment to be turned on */
		int int_value;					/* Example: 12.3 would be converted to 123, keeping track of the tenths */
    int digit;							/* Digit to display on the segment */

		int_value = (int)(displayed_segment_value * 10);
		
		switch (segment) {
			case 2: digit = int_value % 10; break;
			case 1: digit = (int_value / 10) % 10; break;
			case 0: digit = (int_value / 100); break;
		}
		
//		printf("segment, digit = %d, %d\n", segment, digit);
		GPIOE->ODR = GPIOE->ODR & 0x000F;
    GPIOE->ODR = GPIOE->ODR | (segment_number[segment] | SEG_CODES[digit]);
    segment = (segment + 1) % 3;
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
	
	/*Configures SysTick to provide 200Hz interval interrupts. (Read ADC once in two times)*/
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/500);

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
