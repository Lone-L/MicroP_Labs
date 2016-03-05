#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "keypad.h"


const char values[NUM_ROWS][NUM_COLS] = {{'1', '2', '3', 'A'}, 
																				 {'4', '5', '6', 'B'}, 
																				 {'7', '8', '9', 'C'}, 
																				 {'*', '0', '#', 'D'}};

GPIO_TypeDef *row_pins[] = {GPIOF, GPIOG, GPIOH, GPIOI};
GPIO_TypeDef *col_pins[] = {GPIOA, GPIOB, GPIOC, GPIOD};
	

/**
   * @brief  Index into char values array and return integer value or ENTER_CODE 
   * @param  None
   * @retval int: value pressed on keypad (0-9) or (10 == Enter value)
   */
	
int interpret_key(int row, int col)
{
	
	/*Index into char values array and return integer value or ENTER_CODE  */
	switch(values[row][col])
	{
		case ('1'):
		case ('2'):
		case ('3'):
		case ('4'):
		case ('5'):
		case ('6'):
		case ('7'):
		case ('8'):
		case ('9'):
			return values[row][col] - '0';
		case ('#'):
			return ENTER_CODE; // 
		default:
			return -1; 
	}
	

}


/**
   * @brief  Determine the row and col number of the key being pressed.
   * @param  None
   * @retval int: value pressed on keypad (0-9) or (10 == Enter value)
   */
int scan_keypad(void)
{
	int input_col = ((GPIOA->IDR >> 7) | (GPIOB->IDR >> 6) | (GPIOC->IDR >> 5) | (GPIO->IDR >> 5)) & 0x000F;
	int row = 0, col = 0;
	int i;
	/*Select Column*/
	switch(input_col)
	{
		case (COL_ONE):
			col = 0;
			break;
		case (COL_TWO):
			col = 1;
			break;
		case (COL_THREE):
			col = 2;
			break;
		case (COL_FOUR):
			col = 3;
			break;
	}
	

	/*Turn all row pins to LOW */
	row_pins[0]->ODR = row_pins[0]->ODR & ~PIN_NUMBER;
	row_pins[1]->ODR = row_pins[1]->ODR & ~PIN_NUMBER;
	row_pins[2]->ODR = row_pins[2]->ODR & ~PIN_NUMBER;
	row_pins[3]->ODR = row_pins[3]->ODR & ~PIN_NUMBER;
	
	
	/* Iterate through row pins */
	for (i = 0; i < NUM_ROWS; i++)
	{
		row_pins[i]->ODR = row_pins[i]->ODR | PIN_NUMBER;
		
		/* if corresponding row causes the selected col to go high store val in row*/
		if (col_pins[col]->IDR & PIN_NUMBER)
		{
			row = i;
			break;
		}
		
	}
	
	/*Turn all row pins to HIGH */
	row_pins[0]->ODR = row_pins[0]->ODR | PIN_NUMBER;
	row_pins[1]->ODR = row_pins[1]->ODR | PIN_NUMBER;
	row_pins[2]->ODR = row_pins[2]->ODR | PIN_NUMBER;
	row_pins[3]->ODR = row_pins[3]->ODR | PIN_NUMBER;
	
	return interpret_key(row, col);
}

void initialize_keypad(void)
{
	
		GPIO_InitTypeDef KP_PINS_INPUT;
		GPIO_InitTypeDef KP_PINS_OUTPUT;
	
		__HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOD_CLK_ENABLE();
	  __HAL_RCC_GPIOF_CLK_ENABLE();
	  __HAL_RCC_GPIOG_CLK_ENABLE();
	  __HAL_RCC_GPIOH_CLK_ENABLE();
	  __HAL_RCC_GPIOI_CLK_ENABLE();
		
	  /*Column pins set as input*/
		KP_PINS_INPUT.Pin       = GPIO_PIN_8;
		KP_PINS_INPUT.Mode  		= GPIO_MODE_INPUT;     
		KP_PINS_INPUT.Pull  		= GPIO_PULLDOWN;            
    KP_PINS_INPUT.Speed 		= GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 M
	 		
		HAL_GPIO_Init(GPIOA, &KP_PINS_INPUT);
		HAL_GPIO_Init(GPIOB, &KP_PINS_INPUT);
		HAL_GPIO_Init(GPIOC, &KP_PINS_INPUT);
	  HAL_GPIO_Init(GPIOD, &KP_PINS_INPUT);
	
	  /*Row pins set as output*/
		KP_PINS_OUTPUT.Pin   = GPIO_PIN_8;
		KP_PINS_OUTPUT.Mode  = GPIO_MODE_OUTPUT_PP;     
		KP_PINS_OUTPUT.Pull  = GPIO_NOPULL;            
    KP_PINS_OUTPUT.Speed = GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 MHz?

		HAL_GPIO_Init(GPIOF, &KP_PINS_OUTPUT);
	  HAL_GPIO_Init(GPIOG, &KP_PINS_OUTPUT);
		HAL_GPIO_Init(GPIOH, &KP_PINS_OUTPUT);
		HAL_GPIO_Init(GPIOI, &KP_PINS_OUTPUT);
	
	
		/*Turn all row pins to HIGH */
		GPIOF->ODR = GPIOF->ODR | PIN_NUMBER;
		GPIOG->ODR = GPIOG->ODR | PIN_NUMBER;
		GPIOH->ODR = GPIOH->ODR | PIN_NUMBER;
		GPIOI->ODR = GPIOG->ODR | PIN_NUMBER;
		
	/* Set priority for EXTI9_5_IRQHandler */
	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);

	/* Enable IRQ for the KP GPIO_PINS */
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	
}

void KEYPAD_Callback(void)
{
	
	if ((GPIOA->IDR & 0x0080) | (GPIOB->IDR & 0x0080) | (GPIOC->IDR & 0x0080) | (GPIOD->IDR & 0x0080))
	{
		scan_keypad();
	}
	
	
}
