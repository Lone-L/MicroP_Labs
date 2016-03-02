#ifndef KALMANFILTER_H
#define KALMANFILTER_H

typedef struct Kalman_State {
	float q;
	float r;
	float x;
	float p;
	float k;
} KalmanState;

extern int Kalmanfilter_asm(float *InputArray, float *OutputArray, int length, KalmanState *kstate);

#endif
