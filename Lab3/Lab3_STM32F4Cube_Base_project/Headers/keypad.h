#ifndef KEYPAD_H
#define KEYPAD_H

void Keypad_Init(void);

int Keypad_Pressed(void);
void Keypad_ClearPressed(void);
int Keypad_ScanKey(void);

void Keypad_KeyPressedCallback(void);

#define NUM_ROWS  4
#define NUM_COLS  4


#define ROW_PIN_NUMBER    0x0100

#define COL1_PIN_NUMBER   0x0002
#define COL2_PIN_NUMBER   0x0004
#define COL3_PIN_NUMBER   0x0010
#define COL4_PIN_NUMBER   0x0008


#define ENTER_KEY 10

#define COL_ONE 	1
#define COL_TWO 	2
#define COL_THREE 4
#define COL_FOUR	8

#define KEYPAD_DEBOUNCE_COUNT 50000

#endif
