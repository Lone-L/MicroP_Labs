#include "lis3dsh.h"

/* Private variables ---------------------------------------------------------*/
static int ACCELEROMETER_NEW_DATA = 0;

/**
  * @brief  Initialize the accelerometer.
  * @param  None
  * @retval None
  */
void Accelerometer_Init(void)
{
	LIS3DSH_InitTypeDef accel_init;
	LIS3DSH_DRYInterruptConfigTypeDef it_config;
	
	accel_init.Power_Mode_Output_DataRate = LIS3DSH_DATARATE_25;					/* Output data 25 times per second as by requirements */
	accel_init.Axes_Enable = LIS3DSH_XYZ_ENABLE;													/* Enable x, y and z axes */
	accel_init.Continous_Update = LIS3DSH_ContinousUpdate_Enabled;				/* Enable continuous update */
	accel_init.AA_Filter_BW = LIS3DSH_AA_BW_800;													/* Anti-aliasing filter bandwidth (default value is 800Hz) */
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
  * @brief  Indicates whether accelerometer got new data (used for polling).
	* @param  None
  * @retval None
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
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	ACCELEROMETER_NEW_DATA = 1;	/* Set flag to indicate new data is available on sensor */
	/* The DRY interrupt flag on the sensor is cleared when the data is read */
}
