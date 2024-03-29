#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "keypad.h"

/* Keypad states used for debouncing */
typedef enum _KeypadState {
	UNPRESSED = 0,
	POTENTIALLY_PRESSED,
	DEFINITELY_PRESSED
} KeypadState;

static KeypadState KEYPAD_STATE = UNPRESSED;
static int temp_key;	/* Key detected during debouncing (see Keypad_KeyPressedCallback) */

static int KEYPAD_KEY_PRESSED = 0;
static const char values[NUM_ROWS][NUM_COLS] = {{'1', '2', '3', 'A'}, 
																								{'4', '5', '6', 'B'}, 
																								{'7', '8', '9', 'C'}, 
																								{'*', '0', '#', 'D'}};

GPIO_TypeDef *row_pins[] = {GPIOA, GPIOB, GPIOC, GPIOD};
GPIO_TypeDef *col_pins[] = {GPIOA, GPIOB, GPIOC, GPIOD};

/**
   * @brief  Index into char values array and return integer value or ENTER_CODE 
   * @param  None
   * @retval int: value pressed on keypad (0-9) or (10 == Enter value)
   */
static int interpret_key(int row, int col)
{
	
	/*Index into char values array and return integer value or ENTER_CODE  */
	switch(values[row][col])
	{
		case ('0'):
		case ('1'):
		case ('2'):
		case ('3'):
		case ('4'):
		case ('5'):
		case ('6'):
		case ('7'):
		case ('8'):
		case ('9'):
			return values[row][col] - '0';	/* will return int 0 to 9 for keys '0' to '9'. */
		case ('#'):
			return ENTER_KEY; /* Just pick a value different than 0 to 9 */ 
		default:
			/* Indicates invalid key. */
			return -1; 
	}
}

/**
   * @brief  Determine the row and col number of the key being pressed.
   * @param  None
   * @retval int: value pressed on keypad (0-9) or (10 == Enter value)
   */
int Keypad_ScanKey(void)
{
	/* Read the column bits into input_col (reorder in a more natural order as defined in keypad.h) */
	int input_col = (((col_pins[0]->IDR & COL1_PIN_NUMBER) >> 1) | ((col_pins[1]->IDR & COL2_PIN_NUMBER) >> 1) | ((col_pins[2]->IDR & COL3_PIN_NUMBER) >> 2) | ((col_pins[3]->IDR & COL4_PIN_NUMBER) >> 0)) & 0x000F;
	int row = 0, col = 0;	/* Keypad row and column pressed */
	int col_pin_num = 0;	/* ODR/IDR bit corresponding to column. */
	int i;
	
	/* Determine which column was pressed */
	switch(input_col)
	{
		case (COL_ONE):
			col = 0;
			col_pin_num = COL1_PIN_NUMBER;
			break;
		case (COL_TWO):
			col = 1;
			col_pin_num = COL2_PIN_NUMBER;
			break;
		case (COL_THREE):
			col = 2;
			col_pin_num = COL3_PIN_NUMBER;
			break;
		case (COL_FOUR):
			col = 3;
			col_pin_num = COL4_PIN_NUMBER;
			break;
		default:
			/* Shouldn't happen */
			return -1;
	}
	
	/*Turn all row pins to LOW */
	row_pins[0]->ODR &= ~ROW_PIN_NUMBER;
	row_pins[1]->ODR &= ~ROW_PIN_NUMBER;
	row_pins[2]->ODR &= ~ROW_PIN_NUMBER;
	row_pins[3]->ODR &= ~ROW_PIN_NUMBER;
	
	/* Iterate through row pins */
	for (i = 0; i < NUM_ROWS; i++)
	{
		row_pins[i]->ODR |= ROW_PIN_NUMBER;
		
		/* if corresponding row causes the selected col to go high store val in row*/
		if (col_pins[col]->IDR & col_pin_num)
		{
			row = i;
			break;
		}
		
		row_pins[i]->ODR &= ~ROW_PIN_NUMBER;
	}
	
	/*Turn all row pins to HIGH */
	row_pins[0]->ODR |= ROW_PIN_NUMBER;
	row_pins[1]->ODR |= ROW_PIN_NUMBER;
	row_pins[2]->ODR |= ROW_PIN_NUMBER;
	row_pins[3]->ODR |= ROW_PIN_NUMBER;
	
	return interpret_key(row, col);
}

/**
   * @brief  Initialize the keypad and associated GPIO pins.
   * @param  None
   * @retval None
   */
void Keypad_Init(void)
{
	
		GPIO_InitTypeDef KP_PINSA_INPUT, KP_PINSB_INPUT, KP_PINSC_INPUT, KP_PINSD_INPUT;
		GPIO_InitTypeDef KP_PINS_OUTPUT;
	
		__HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOD_CLK_ENABLE();

		
		/* Column pins set as input: PA1, PB2, PC4, PD3 correspond to columns 1, 2, 3, 4 respectively.
			 Order is weird because apparently PC3 or PD4 was used for other functions.
			 Column pins set to pulldown resistors --> default to 0, measure 1 when key pressed.
			 Key pressed causes transition from 0 to 1 ---> need rising-edge interrupts */
		KP_PINSA_INPUT.Pin      = GPIO_PIN_1;
		KP_PINSA_INPUT.Mode  		= GPIO_MODE_IT_RISING;     
		KP_PINSA_INPUT.Pull  		= GPIO_PULLDOWN;            
    KP_PINSA_INPUT.Speed 		= GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 M
	
	 	KP_PINSB_INPUT.Pin      = GPIO_PIN_2;
		KP_PINSB_INPUT.Mode  		= GPIO_MODE_IT_RISING;     
		KP_PINSB_INPUT.Pull  		= GPIO_PULLDOWN;            
    KP_PINSB_INPUT.Speed 		= GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 M
		
		KP_PINSC_INPUT.Pin      = GPIO_PIN_4;
		KP_PINSC_INPUT.Mode  		= GPIO_MODE_IT_RISING;     
		KP_PINSC_INPUT.Pull  		= GPIO_PULLDOWN;            
    KP_PINSC_INPUT.Speed 		= GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 M
		
		KP_PINSD_INPUT.Pin      = GPIO_PIN_3;
		KP_PINSD_INPUT.Mode  		= GPIO_MODE_IT_RISING;     
		KP_PINSD_INPUT.Pull  		= GPIO_PULLDOWN;            
    KP_PINSD_INPUT.Speed 		= GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 M
	
		
		HAL_GPIO_Init(GPIOA, &KP_PINSA_INPUT);
		HAL_GPIO_Init(GPIOB, &KP_PINSB_INPUT);
		HAL_GPIO_Init(GPIOC, &KP_PINSC_INPUT);
	  HAL_GPIO_Init(GPIOD, &KP_PINSD_INPUT);
	
		/* Row pins set as output: PA8, PB8, PC8, PD8. */
		KP_PINS_OUTPUT.Pin   = GPIO_PIN_8;
		KP_PINS_OUTPUT.Mode  = GPIO_MODE_OUTPUT_PP;     
		KP_PINS_OUTPUT.Pull  = GPIO_NOPULL;            
    KP_PINS_OUTPUT.Speed = GPIO_SPEED_FREQ_MEDIUM;  // 12.5 MHz - 50 MHz?

		HAL_GPIO_Init(GPIOA, &KP_PINS_OUTPUT);
	  HAL_GPIO_Init(GPIOB, &KP_PINS_OUTPUT);
		HAL_GPIO_Init(GPIOC, &KP_PINS_OUTPUT);
		HAL_GPIO_Init(GPIOD, &KP_PINS_OUTPUT);
	

	/*Turn all row pins to HIGH. Should be 1 by default to detect key press on pulled-down columns. */
	row_pins[0]->ODR = row_pins[0]->ODR | ROW_PIN_NUMBER;
	row_pins[1]->ODR = row_pins[1]->ODR | ROW_PIN_NUMBER;
	row_pins[2]->ODR = row_pins[2]->ODR | ROW_PIN_NUMBER;
	row_pins[3]->ODR = row_pins[3]->ODR | ROW_PIN_NUMBER;
	
	/* Set priority for EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn Handler.
		 Each EXTIn_IRQ handler corresponds to column n interrupt (n = 1, 2, 3, 4). */
	HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
	HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
	HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
	HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
	
	/* Enable IRQ for the KP GPIO_PINS */
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	
	KEYPAD_STATE = UNPRESSED;
	KEYPAD_KEY_PRESSED = 0;		/* Initially unpressed. */
}

/**
   * @brief  Indicate if keypad key was pressed.
   * @param  None
	 * @retval int: whether keypad key was pressed.
   */
int Keypad_Pressed(void)
{
	return KEYPAD_KEY_PRESSED;
}

/**
   * @brief  Clear the keypad key pressed flag.
   * @param  None
   * @retval None
   */
void Keypad_ClearPressed(void)
{
	KEYPAD_KEY_PRESSED = 0;
}

/**
   * @brief  Callback when IRQhandler caused by key press occured. Handles debouncing.
   * @param  None
   * @retval None
   */
void Keypad_KeyPressedCallback(void)
{
	static int counter = 0;	/* Used for counting consecutive events related to same key press for debouncing */
	
	/* Check if any of the column pins are set to 1 */
	if ((col_pins[0]->IDR & COL1_PIN_NUMBER) | (col_pins[1]->IDR & COL2_PIN_NUMBER) | (col_pins[2]->IDR & COL3_PIN_NUMBER) | (col_pins[3]->IDR & COL4_PIN_NUMBER))
	{
		switch (KEYPAD_STATE) {
			case UNPRESSED:
				/* First time button press detected. Read the scanned value into temp_key to compare later. */
				temp_key = Keypad_ScanKey();
				counter = 0;
				KEYPAD_STATE = POTENTIALLY_PRESSED;
				break;
			
			case POTENTIALLY_PRESSED:
				/* To check if this is a real key press and not just noise, count the number of times the same value is read consecutively.
					 After the counter reaches above the debounce count, assume it is definitely a key press. */
				if (counter > KEYPAD_DEBOUNCE_COUNT) {
					KEYPAD_STATE = DEFINITELY_PRESSED;
				} else if (Keypad_ScanKey() != temp_key) {
					KEYPAD_STATE = UNPRESSED;	/* Normal user press should not change the key value that fast, so assume it's noise. */
				} else {
					++counter;
				}
				
				break;
				
			case DEFINITELY_PRESSED:
				/* We now believe the key is indeed pressed, and will proceed to scan it for real in main. */
				KEYPAD_KEY_PRESSED = 1;
				KEYPAD_STATE = UNPRESSED;
				break;
			
			default:
				/* Should not happen. */
				KEYPAD_STATE = UNPRESSED;
				break;
		}
	}	
}
