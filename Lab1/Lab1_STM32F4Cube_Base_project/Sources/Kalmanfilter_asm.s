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

loop
	CMP 			R4, R2
	BEQ 			end_loop
	VADD.f32 	S3, S3, S0					; p = p + q
	VMRS			APSR_nzcv, FPSCR		; load status
	BVS				error								; if overflow, goto error
	VADD.f32	S5, S3, S1					; p + r
	VMRS			APSR_nzcv, FPSCR		; load status
	BVS				error								; if overflow, goto error
	BEQ				error								; if zero, goto error
	VDIV.f32	S4, S3, S5					; k = p / (p + r)
	VMRS			APSR_nzcv, FPSCR		; load status
	BVS				error								; if overflow, goto error
	VLDR.f32	S5, [R6]						; read measurement at index
	VSUB.f32	S5, S5, S2					; measurement - x
	VMRS			APSR_nzcv, FPSCR		; load status
	BVS				error								; if overflow, goto error
	VMUL.f32	S5, S5, S4					; k * (measurement - x)
	VMRS			APSR_nzcv, FPSCR		; load status
	BVS				error								; if overflow, goto error
	VADD.f32	S2, S2, S5					; x = x + k * (measurement - x)
	VMRS			APSR_nzcv, FPSCR		; load status
	BVS				error								; if overflow, goto error
	VSTR.f32	S2, [R5]						; store filtered x in array
	VLDR.f32	S5, =1.0						; 1
	VSUB.f32	S5, S5, S4					; 1 - k
	VMRS			APSR_nzcv, FPSCR		; load status
	BVS				error								; if overflow, goto error
	VMUL.f32	S3, S3, S5					; p = (1 - k)*p
	VMRS			APSR_nzcv, FPSCR		; load status
	BVS				error								; if overflow, goto error
	ADD				R4, R4, #1					; increment index
	ADD				R5, R5, #4					; output array: the address in bytes increments by 4
	ADD				R6, R6, #4					; input array : the address in bytes increments by 4
	B					loop

end_loop
	VSTR.f32	S0, [R3]						; q
	VSTR.f32	S1, [R3, #4]				; r
	VSTR.f32	S2, [R3, #8]				; x
	VSTR.f32	S3, [R3, #12]				; p
	VSTR.f32	S4, [R3, #16]				; k
	MOV				R0, #0
	POP				{R4-R6}
	BX				LR									; return 0

error
	MOV				R0, #1
	POP				{R4-R6}
	BX				LR									; return 1
	END