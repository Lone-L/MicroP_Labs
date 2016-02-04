#include <stdio.h>
#include "arm_math.h"
#include "demo.h"

struct kalman_state {
	float q;
	float r;
	float x;
	float p;
	float k;
};

extern void workbench_asm(void);
extern int Kalmanfilter_asm(const float* InputArray, float* OutputArray, int Length, struct kalman_state* kstate);
void vector_subtract(float *vector_result, const float *vector_a, const float *vector_b, int length)
{
	int i;
	
	for (i = 0; i < length; i++) {
		vector_result[i] = vector_a[i] - vector_b[i];
	}
}

float average(const float *vector_a, int length)
{
	float sum = 0.0;
	int i = 0;
	
	for (i = 0; i < length; i++) {
		sum += vector_a[i];
	}
	
	return sum / length;
}

float stddev(const float *vector_a, int length)
{
	float variance = 0.0;
	float mean = average(vector_a, length);
	int i;
	
	for (i = 0; i < length; i++)
		variance += (vector_a[i] - mean) * (vector_a[i] - mean);
	
	return sqrt(variance / (length - 1));
}

/* vector_result must have a size of 2 * length - 1
 * Also, vector_result[i] does not mean value at i, but i - length + 1 */ 
void correlation(float *vector_result, const float *vector_a, const float *vector_b, int length)
{
	float sum;
	int t, i;
	
	for (t = -length + 1; t < 0; t++) {
		sum = 0.0;
		
		for (i = -t; i < length; i++)
			sum += vector_a[i + t] * vector_b[i];

		vector_result[t + length - 1] = sum;
	}
	
	for (t = 0; t < length; t++) {
		sum = 0.0;
		
		for (i = 0; i < length - t; i++)
			sum += vector_a[i + t] * vector_b[i];
		
		vector_result[t + length - 1] = sum;
	}
}

/* vector_result must have a size of 2 * length - 1
   Also, vector_result[i] corresponds to value at i. */
void convolution(float *vector_result, const float *vector_a, const float *vector_b, int length)
{
	float sum;
	int t, i;
	
	for (t = 0; t < length; t++) {
		sum = 0.0;
		
		for (i = 0; i <= t; i++)
			sum += vector_a[i] * vector_b[t - i];
		
		vector_result[t] = sum;
	}
	
	for (t = length; t < 2 * length - 1; t++) {
		sum = 0.0;
		
		for (i = t - length + 1; i < length; i++)
			sum += vector_a[i] * vector_b[t - i];
		
		vector_result[t] = sum;
	}
}

int Kalmanfilter_C(const float* InputArray, float* OutputArray, struct kalman_state* kstate, int Length)
{
	int i;

	for(i = 0; i < Length; i++) {
		
		kstate -> p = kstate->p + kstate->q; 													// p = p + q
		if ((((*(unsigned int *)&kstate->p) >> 23) & 0xff) == 0xff){  // shift to get exponent part of float and compare to 0xff (INF/-INF/NAN in IEEE 754
			return 1;
		}

		kstate->k = kstate->p / (kstate->p + kstate->r);							// p / (p + r) 
		if ((((*(unsigned int *)&kstate->k) >> 23) & 0xff) == 0xff){
			return 1;
		}
		
		kstate->x = kstate->x + kstate->k *(InputArray[i] - kstate->x); // x + k * (measurement - x)
		if ((((*(unsigned int *)&kstate->x) >> 23) & 0xff) == 0xff){
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

struct container_for_arrays {
	float *our_diff;
	float *cmsis_diff;
	float *our_corr;
	float *cmsis_corr;
	float *our_conv;
	float *cmsis_conv;
};

/* diff, corr and conv are preallocated to the length required in main(). */
void compare_dsp(float *original, float *filtered, int length, struct container_for_arrays *arrays)
{
	float our_avg, our_std;
	float cmsis_avg, cmsis_std;
	
	vector_subtract(arrays->our_diff, original, filtered, length);
	our_avg = average(arrays->our_diff, length);
	our_std = stddev(arrays->our_diff, length);
	correlation(arrays->our_corr, original, filtered, length);
	convolution(arrays->our_conv, original, filtered, length);
	
	arm_sub_f32(original, filtered, arrays->cmsis_diff, length);
	arm_mean_f32(arrays->cmsis_diff, length, &cmsis_avg);
	arm_std_f32(arrays->cmsis_diff, length, &cmsis_std);
	arm_correlate_f32(original, length, filtered, length, arrays->cmsis_corr);
	arm_conv_f32(original, length, filtered, length, arrays->cmsis_conv);
}

int main()
{
	struct kalman_state k_state = {0.1, 0.1, 0, 0.1, 0};
	
	float InputArray[] = {1.0, 1.5, 0.0, 0.78, 1.32};
	float OutputArray[sizeof InputArray / sizeof(float)];
	//float OutputArray[length1];
	float difference1[sizeof InputArray / sizeof(float)];
	float correlation1[(sizeof InputArray / sizeof(float)) * 2 - 1];
	float convolution1[(sizeof InputArray / sizeof(float)) * 2 - 1];
	float difference2[sizeof InputArray / sizeof(float)];
	float correlation2[(sizeof InputArray / sizeof(float)) * 2 - 1];
	float convolution2[(sizeof InputArray / sizeof(float)) * 2 - 1];
	struct container_for_arrays arrays;
	arrays.our_diff   = difference1;
	arrays.cmsis_diff = difference2;
	arrays.our_corr   = correlation1;
	arrays.cmsis_corr = correlation2;
	arrays.our_conv   = convolution1;
	arrays.cmsis_conv = convolution2;
	

/*	if (!Kalmanfilter_C(measurements, OutputArray, &k_state, length1)) {
		printf("success\n");
	}*/
	//workbench_asm();
	
	if(!Kalmanfilter_asm(InputArray, OutputArray, sizeof InputArray / sizeof(float), &k_state)) {
		printf("success\n");
	}
	
	
	
	/*if(!Kalmanfilter_C(InputArray, OutputArray, &k_state, sizeof InputArray / sizeof(float))) {
		printf("success\n");
	}*/
	
	compare_dsp(InputArray, OutputArray, sizeof InputArray / sizeof (float), &arrays);
	
	return 0;
}
