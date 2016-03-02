#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

void Accelerometer_Init(void);
void Accelerometer_ReadAccel(float *ax, float *ay, float *az);

int Accelerometer_HasNewData(void);
void Accelerometer_ClearNewData(void);

#endif
