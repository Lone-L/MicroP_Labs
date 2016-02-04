#include <measurements.h>
#include <stdint.h>
#include <demo.h>


typedef struct {
	float x;
	float r;
	float q;
	float p;
	float k;
} kalman_t;

extern int Kalmanfilter_asm(float *InputArray, float *OutputArray, int length, kalman_t *kal);

int testfunc() {
	
	float OutputArray[length];
	int i;
	
	kalman_t kal = {
		.q = DEF_q, // Process noise variance
		.r = DEF_r, // Measurement noise variance
		.x = DEF_x, // Initial estimate
		.p = DEF_p, // Estimate error covariance
		.k = DEF_k  // Initial Kalman gain
	};
	
	if(Kalmanfilter_asm(measurements, OutputArray, length, &kal))
		return 1;
	
	for(i=0 ; i<length ; i++) {
		if((OutputArray[i]-estimates[i]>ERR_MARGIN) || (OutputArray[i]-estimates[i]<(-1*ERR_MARGIN)))
			return 2;
	}
	
	*((uint32_t *)&kal.p) = 0x7F7FFFFF;
	*((uint32_t *)&kal.q) = 0x7F7FFFFF;
	kal.x = DEF_x;
	kal.k = DEF_k;
	kal.r = DEF_r;
	
	if(!Kalmanfilter_asm(measurements, OutputArray, length, &kal))
		return 3;
	
	
	kal.q = 0;
	kal.x = 0;
	kal.k = 0;
	kal.r = 0;
	kal.p = 0;
	
	if(!Kalmanfilter_asm(measurements, OutputArray, length, &kal))
		return 4;
	
	return 0;
}
