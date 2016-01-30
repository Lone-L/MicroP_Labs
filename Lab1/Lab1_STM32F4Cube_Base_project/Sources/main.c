#include <stdio.h>
#include "arm_math.h"

struct kalman_state{
	float q;
	float r;
	float x;
	float p;
	float k;
};

int Kalmanfilter_C(float* InputArray, float* OutputArray, struct kalman_state* kstate, int Length){
	int i;
	for(i = 0; i < Length; i++) {
		
		kstate -> p = kstate->p + kstate->q; 													// p = p + q
		if ((((*(unsigned int *)&kstate->p) >> 23) & 0xff) == 0xff){  // shift to get exponent part of float and compare to 0xff (INF/-INF/NAN in IEEE 754
			return 1;
		}
		
		kstate->k = kstate->p / (kstate->p + kstate->r);							// p / (p + r) 
		if ((((*(unsigned int *)&kstate->p) >> 23) & 0xff) == 0xff){
			return 1;
		}
		
		kstate->x = kstate->x + kstate->k *(InputArray[i] - kstate->x); // x + k * (measurement - x)
		if ((((*(unsigned int *)&kstate->p) >> 23) & 0xff) == 0xff){
			return 1;
		}
		
		kstate->p = (1 - kstate->k) * kstate->p;												// (1 - k) * p
		if ((((*(unsigned int *)&kstate->p) >> 23) & 0xff) == 0xff){
			return 1;
		}
		OutputArray[i] = kstate->x;
	}
	return 0;
}


int main()
{
	struct kalman_state k_state = {0.1, 0.1, 0, 0.1, 0};
	
	float InputArray[] = {1.0, 1.5, 0.0, 0.78, 1.32, 1.44};
	float OutputArray[sizeof InputArray / sizeof(float)];
	int i = 0;
	
	if (!Kalmanfilter_C(InputArray, OutputArray, &k_state, sizeof InputArray / sizeof(float))) {
		
	}
	return 0;
}
