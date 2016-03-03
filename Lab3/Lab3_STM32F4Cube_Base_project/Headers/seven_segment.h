#ifndef SEVEN_SEGMENT_H
#define SEVEN_SEGMENT_H

void SevenSegment_Init(void);
void SevenSegment_SetDisplayValue(float angle);
void SevenSegment_ToggleDisplayedDigit(void);

void SevenSegment_TurnOn(void);
void SevenSegment_TurnOff(void);

/* LED CONSTANTS FORMAT IS :

ODR REGISTER = P432 1GFE DCBA 0000

FROM LEFT TO RIGHT:

P (DECIMAL POINT ): BIT 15 => GPIO_PIN_15
4 (4TH SEGMENT   ): BIT 14 => GPIO_PIN_14
3 (3RD SEGMENT   ): BIT 13 => GPIO_PIN_13
2 (2ND SEGMENT   ): BIT 12 => GPIO_PIN_12
1 (1ST SEGMENT   ): BIT 11 => GPIO_PIN_11
G (G   SEGMENT   ): BIT 10 => GPIO_PIN_10
F (F   SEGMENT   ): BIT 9  => GPIO_PIN_9
E (E   SEGMENT   ): BIT 8  => GPIO_PIN_8
D (D   SEGMENT   ): BIT 7  => GPIO_PIN_7
C (C   SEGMENT   ): BIT 6  => GPIO_PIN_6
B (B   SEGMENT   ): BIT 5  => GPIO_PIN_5
A (A   SEGMENT   ): BIT 4  => GPIO_PIN_4 
*/

#define ZERO                    (0x03F0)
#define ONE                     (0x0060)
#define TWO                     (0x05B0)
#define THREE                   (0x04F0)
#define FOUR                    (0x0660)
#define FIVE                    (0x06D0)
#define SIX                     (0x07D0)
#define SEVEN                   (0x0070)
#define EIGHT                   (0x07F0)
#define NINE                    (0x06F0)
#define DEGREE									(0x0630)

/* Wait for 6 accelerometer readings before changing the displayed value.
	 This means we change the displayed value 25 / 6 = 4 times per second.
*/
#define SEVEN_SEGMENT_DISPLAY_COUNT 6

#endif
