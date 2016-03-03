	AREA text, CODE, READONLY
	EXPORT Kalmanfilter_asm

; function Kalmanfilter_asm
; inputs:
; -------
; R0: pointer to the input (float *)
; R1: pointer to filtered data array (float *)
; R2: length of array (number of floats in array)
; R3: pointer to Kalman filter state (struct kalman_state *)
;
; outputs:
; --------
; R0: 0 if successful, 1 if failed due to overflow (int)
Kalmanfilter_asm
	PUSH 			{R4-R6}
	VLDR.f32	S0, [R3]						; q
	VLDR.f32	S1, [R3, #4]				; r
	VLDR.f32	S2, [R3, #8]				; x
	VLDR.f32	S3, [R3, #12]				; p
	VLDR.f32	S4, [R3, #16]				; k
	MOV 			R4, #0							; index = 0
	MOV				R5, R1							; current address in filtered
	MOV				R6, R0							; current address in input array

	LSL R4, R2, #2
	ADD R4, R4, R5

loop
	CMP 			R4, R5
	BEQ 			end_loop
	VADD.f32 	S3, S3, S0					; p = p + q
	VADD.f32	S5, S3, S1					; p + r
	VDIV.f32	S4, S3, S5					; k = p / (p + r)
	VLDR.f32	S5, [R6]						; read measurement at index
	VSUB.f32	S5, S5, S2					; measurement - x
	VMUL.f32	S5, S5, S4					; k * (measurement - x)
	VADD.f32	S2, S2, S5					; x = x + k * (measurement - x)
	VSTR.f32	S2, [R5]						; store filtered x in array
	VMUL.f32	S3, S4, S1					; p = k * r = (1 - p/(p+r)) * p = (1 - k) * p
	ADD				R5, R5, #4					; output array: the address in bytes increments by 4
	ADD				R6, R6, #4					; input array : the address in bytes increments by 4
	B					loop

end_loop

	VSTR.f32	S0, [R3]						; q
	VSTR.f32	S1, [R3, #4]				; r
	VSTR.f32	S2, [R3, #8]				; x
	VSTR.f32	S3, [R3, #12]				; p
	VSTR.f32	S4, [R3, #16]				; k
	
	VMRS R0, FPSCR
	AND R0, #0x0F 

	POP				{R4-R6}
	BX				LR									; return 0

error
	MOV				R0, #1
	POP				{R4-R6}
	BX				LR									; return 1
	END