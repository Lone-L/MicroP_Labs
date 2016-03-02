/**
  ******************************************************************************
  * @file    HAL/HAL_TimeBase/Inc/main.h 
  * @author  MCD Application Team
  * @version   V1.2.4
  * @date      13-November-2015
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define V_25			((float)0.760)	/* V, for temperature sensor calculation at 25 C */
#define AVG_SLOPE	((float)0.0025)	/* V/C, for temperature sensor calculation */
#define TEMP_REF	((float)25.0)	/* C */

#define V_REF			((float)3.0)	/* Volts */
#define FUDGE_FACTOR	((float)(-12.0))
//#define VTEMP_MAX		((float)3.6)	/* Volts */
//#define VTEMP_MIN		((float)1.8)	/* Volts */
	
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

#define GET_BIT(X, n) (((X) >> (n)) & 0x1)
// SET THE DATA BITS FOR THE LCD SCREEN
#define SET_D_LINE(X, bits)      ((X) = ((GET_BIT((X), 0) << 0) |(GET_BIT((bits), 0) << 1) | (GET_BIT((bits), 1) << 2) | (GET_BIT((X), 3) << 3) | (GET_BIT((bits), 2) << 4) | (GET_BIT((bits), 3) << 5) | (GET_BIT((bits), 4) << 6) | (GET_BIT((X), 7) << 7) | (GET_BIT((bits), 5) << 8) | (GET_BIT((bits), 6) << 9) | (GET_BIT((bits), 7) << 11) | (GET_BIT((X), 12) << 12) | (GET_BIT((X), 13) << 13) | (GET_BIT((X), 14) << 14) | (GET_BIT((X), 15) << 15)))
// SET BITS ENABLE (E), READ/WRITE (RW), RS 
#define SET_B_LINE(X, E, RW, RS) ((X) = ((GET_BIT((X), 15) << 15) | (GET_BIT((X), 14) << 14) | (GET_BIT((X), 13) << 13) | (GET_BIT((X), 12) << 12) | (GET_BIT((X), 11) << 11) | (GET_BIT((X), 10) << 10) | (GET_BIT((X), 9) << 9) | (GET_BIT((X), 8) << 8) | (GET_BIT((X), 7) << 7) | (GET_BIT((X), 6) << 6) | (GET_BIT((X), 5) << 5) | (GET_BIT((X), 4) << 4) | (GET_BIT((X), 3) << 3) | ((E) << 2) | ((RW) << 1) | ((RS) << 0)))
#define DELAY(US) {int i; for(i = 0; i < (int) ((US) * 168); i++){}}
#define CMD_DELAY 37
#define CLEAR_DISP_DELAY 1520

#define ZERO                    (0x03F0)
#define ONE                     (0x0060)
#define TWO                     (0x05B0) //0000 0ABC DEFG 0000 => 0000 0GFE DCBA 0000
#define THREE                   (0x04F0) //0000 0111 1001 0000 => 
#define FOUR                    (0x0660)
#define FIVE                    (0x06D0)
#define SIX                     (0x07D0)
#define SEVEN                   (0x0070)
#define EIGHT                   (0x07F0)
#define NINE                    (0x06F0)

#define SEGMENT_DISPLAY_PERIOD					50				/* How many temperature readings are skipped to display one on the 7-segment display */
#define ALARM_LED_TOGGLE_PERIOD					60				/* How many temperature readings are taken to display one LED during alarm toggle */
#define THRESHOLD_TEMP					((float)27.0)			/* Temperature threshold above which overheating alarm is triggered */
#define LOWER_THRESHOLD_TEMP		((float)26.0)			/* When alarm is activated, wait until temperature goes below this value beform turning off alarm. */
	
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* Global flags ------ */
extern int SYSTICK_READ_TEMP_FLAG;				/* Set by Systick_Handler and unset in main */
extern int SYSTICK_DISPLAY_SEGMENT_FLAG;
#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
