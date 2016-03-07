#ifndef KEYPAD_H
#define KEYPAD_H

void Keypad_Init(void);

int Keypad_Pressed(void);
void Keypad_ClearPressed(void);
int Keypad_ScanKey(void);

void Keypad_KeyPressedCallback(void);

/* Number of rows and columns on our keypad */
#define NUM_ROWS  4
#define NUM_COLS  4

/* Bit corresponding to the row in GPIO ODR/IDR registers (Pin 8 --> Bit 8) */
#define ROW_PIN_NUMBER    0x0100

/* Bits corresponding to the columns in GPIO IDR/ODR registers */
#define COL1_PIN_NUMBER   0x0002	/* GPIO_PIN_1 --> bit 1 */
#define COL2_PIN_NUMBER   0x0004	/* GPIO_PIN_2 --> bit 2 */
#define COL3_PIN_NUMBER   0x0010	/* GPIO_PIN_4 --> bit 4 */
#define COL4_PIN_NUMBER   0x0008	/* GPIO_PIN_3	--> bit 3 */

/* Key code for the enter key used to interpret key values */
#define ENTER_KEY 10

/* Values taken by input_col (in Keypad_ScanKey) depending on which column was pressed.
	 These do not directly correspond to the ODR/IDR bits because we reorder them when
	 or'ing into the input_col variable. */
#define COL_ONE 	1		/* Column 1 --> Bit 0 */
#define COL_TWO 	2		/* Column 2 --> Bit 1 */
#define COL_THREE 4		/* Column 3 --> Bit 2 */
#define COL_FOUR	8		/* Column 4 --> Bit 3 */

/* How many key-press events should be accounted for the same key press.
	 Used for keypad debouncing. */
#define KEYPAD_DEBOUNCE_COUNT 50000

#endif
