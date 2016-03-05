#include "lis3dsh.h"
#include "math.h"

/* Private macros ------------------------------------------------------------*/
#define PI ((float)(3.141592653589793))
#define RAD_TO_DEG(x) ((x) * ((float)180.0) / PI)

/* Private variables ---------------------------------------------------------*/
static int ACCELEROMETER_NEW_DATA = 0;	/* Flag which indicates if accelerometer has new data available. Set by interrupt. */

/**
  * @brief  Initialize the accelerometer.
  * @param  None
  * @retval None
  */
void Accelerometer_Init(void)
{
	LIS3DSH_InitTypeDef accel_init;
	LIS3DSH_DRYInterruptConfigTypeDef it_config;
	
	accel_init.Power_Mode_Output_DataRate = LIS3DSH_DATARATE_25;					/* Output data 25 times per second (25 Hz) as by requirements */
	accel_init.Axes_Enable = LIS3DSH_XYZ_ENABLE;													/* Enable x, y and z axes */
	accel_init.Continous_Update = LIS3DSH_ContinousUpdate_Disabled;				/* Disable continuous update to be able to read the MSB of each reading */
	accel_init.AA_Filter_BW = LIS3DSH_AA_BW_50;														/* Anti-aliasing filter bandwidth set to 50Hz to remove frequencies above 2 * 25Hz */
	accel_init.Full_Scale = LIS3DSH_FULLSCALE_2;													/* Use full scale of +/- 2g */
	accel_init.Self_Test = LIS3DSH_SELFTEST_NORMAL;												/* Self test normal */
	
	/* Initialize LIS3DSH */
	LIS3DSH_Init(&accel_init);
	
	it_config.Dataready_Interrupt = LIS3DSH_DATA_READY_INTERRUPT_ENABLED;	/* Enable DRY interrupt */
	it_config.Interrupt_signal = LIS3DSH_ACTIVE_HIGH_INTERRUPT_SIGNAL;		/* Set interrupt to be active high as by requirements */
	it_config.Interrupt_type = LIS3DSH_INTERRUPT_REQUEST_PULSED;					/* Set interrupt type to be pulsed as by requirements */
	
	/* Set LIS3DSH interrupt configuration */
	LIS3DSH_DataReadyInterruptConfig(&it_config);
	
	/* Set priority for EXTI0 IRQ */
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	
	/* Enable IRQ for the EXTI Line 0 (LSI3DSH generates INT1 interrupt on GPIO_PIN_0 PE0) */
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/**
  * @brief  Read acceleration values from accelerometer.
	* @param  ax, ay, az: pointers to float data to store result into.
  * @retval None
  */
void Accelerometer_ReadAccel(float *ax, float *ay, float *az)
{
	float acc_components[3];
	
	LIS3DSH_ReadACC(acc_components);
	*ax = acc_components[0];
	*ay = acc_components[1];
	*az = acc_components[2];
}

/**
  * @brief  Calibrate acceleration values from accelerometer.
	* @param  ax, ay, az: pointers to float data to be calibrated.
  * @retval None
  */
void Accelerometer_Calibrate(float *ax, float *ay, float *az)
{
	/* ACC calibration matrix obtained from least-squares method (Doc 15: Tilt angle application notes) */
	const float ACC[4][3] = {{-0.00100376740404743,  -1.33049359765868e-05, -1.36545479938343e-05},
													 { 2.32874913430264e-05,	0.00101941787448905,  -3.78704315888482e-06},
													 {-9.92411997403039e-06,	1.08648230411970e-06,	 0.00100305721715016},
													 {0.0163849855173971,		 -0.00780422421706874,	-0.0367276451297443}};
	float new_x, new_y, new_z;

	new_x = ACC[0][0] * (*ax) + ACC[1][0] * (*ay) + ACC[2][0] * (*az) + ACC[3][0];
	new_y = ACC[0][1] * (*ax) + ACC[1][1] * (*ay) + ACC[2][1] * (*az) + ACC[3][1];
	new_z = ACC[0][2] * (*ax) + ACC[1][2] * (*ay) + ACC[2][2] * (*az) + ACC[3][2];
	
	*ax = new_x;
	*ay = new_y;
	*az = new_z;
}

/**
  * @brief  Get tilt angle from measured acceleration components.
	* @param  ax, ay, az: float components of acceleration vector (in g)
  * @retval float tilt angle
  */
float Accelerometer_GetTiltAngle(float ax, float ay, float az)
{
	float angle;
	
	/* Definition of angle: hold the board face up with the USB port pointing left.
													Then the tilt angle is the angle of the board's surface
													with respect to the horizontal plane.
	
													So zero degrees corresponds to USB port horizontal pointing to the left,
													with the board facing up. 90 degrees would be with the USB port pointing down.
													180 degrees would be with the USB port pointing right.
	
													Note that turning the board accross the USB axis will change the angle because
													the z-component of the accelerometer will vary.
	*/
	
	/* Axes: the positive y-axis points toward the USB port,
					 the positive x-axis points toward the right (PA7 pin),
					 the positive z-axis points toward the bottom of the board.
	
		Our angle is therefore pitch (with respect to the y-axis of symmetry)
		i.e. we turn the board around the x-axis as defined above, and measure
		the angle of y-axis with respect to horizontal plane.
	*/
	angle = RAD_TO_DEG(atan2f(ay, sqrtf(ax * ax + az * az)));
	
	/* When az < 0, the board is past the point where the y-axis points downwards --> angle goes past 90 degrees.
		 But atan2 returns angles between -90 and 90, so we need to get the 180 - value. */
	if (az < 0)
		angle = ((float)180.0) - angle;
	
	if (angle < 0)
		angle += (float)360.0;	/* Return angles in the range 0 to 360 instead of -90 to 270 */
	
	return angle;
}

/**
  * @brief  Indicates whether accelerometer got new data (used for polling).
	* @param  None
	* @retval int: 1 if new data, 0 otherwise
  */
int Accelerometer_HasNewData(void)
{
	return ACCELEROMETER_NEW_DATA;
}

/**
  * @brief  Clears new data indicator flag.
	* @param  None
  * @retval None
  */
void Accelerometer_ClearNewData(void)
{
	ACCELEROMETER_NEW_DATA = 0;
}

/**
  * @brief  Accelerometer new data callback
  * @param  None
  * @retval None
  */
void ACCELEROMETER_Callback(void)
{
	ACCELEROMETER_NEW_DATA = 1;	/* Set flag to indicate new data is available on sensor */
	/* The DRY interrupt flag on the sensor is cleared when the data is read */
}
