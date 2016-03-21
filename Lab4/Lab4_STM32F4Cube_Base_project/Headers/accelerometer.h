#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX

typedef enum AngleMode {
	PITCH_MODE,
	ROLL_MODE
} AngleMode;

void Accelerometer_Init(void);
void Accelerometer_ReadAccel(float *ax, float *ay, float *az);
void Accelerometer_Calibrate(float *ax, float *ay, float *az);
void ACCELEROMETER_Callback(void);
void Accelerometer_GetTiltAngle(float ax, float ay, float az, float *pitch, float *roll);

void Accelerometer_SetAngleMode(AngleMode mode);
AngleMode Accelerometer_GetAngleMode(void);

extern int start_Thread_ACCELEROMETER (void);
void Accelerometer_Init               (void);

extern osThreadId tid_Thread_ACCELEROMETER;
extern osMutexId accel_mutex;

/* SIGNAL used to resume thread */
#define ACCELEROMETER_SIGNAL 0x0001

#endif
