	AREA text, CODE, READONLY
	IMPORT Kalmanfilter_asm
	EXPORT workbench_asm

; function workbench_asm
; inputs:
; -------
; none
;
; outputs:
; --------
; none
workbench_asm
	PUSH			{R4-R5, LR}
	LDR				R0, =InputArray
	LDR				R1, =init_InputArray
	MOV				R2, #0

	; initialize input array
loop1
	CMP				R2, #5
	BEQ				end_loop1
	VLDR.f32	S0, [R1]
	VSTR.f32	S0, [R0]
	ADD				R0, R0, #4
	ADD				R1, R1, #4
	ADD				R2, R2, #1
	B					loop1
end_loop1
	LDR				R0, =kstate
	LDR				R1, =init_kstate
	MOV				R2, #0

	; initialize kstate
loop2
	CMP				R2, #5
	BEQ				end_loop2
	VLDR.f32	S0, [R1]
	VSTR.f32	S0, [R0]
	ADD				R0, R0, #4
	ADD				R1, R1, #4
	ADD				R2, R2, #1
	B					loop2
end_loop2

	
	; call Kalmanfilter_asm
	LDR				R0,	=InputArray
	LDR				R1, =OutputArray
	MOV				R2, #5
	LDR				R3, =kstate
	LDR				R4, =Kalmanfilter_asm
	BLX				R4 
	
	; Return
	POP				{R4-R5, LR}
	BX				LR
	
	; Initialized array and struct kstate
	ALIGN
init_InputArray	DCFS 1.0, 1.5, 0.0, 0.78, 1.32
init_kstate			DCFS 0.1, 0.1, 0, 0.1, 0
		
	AREA myData, DATA, READWRITE
	
	ALIGN
InputArray			SPACE 5*4	; 5 floats
OutputArray 		SPACE 5*4
kstate					SPACE 5*4	; kstate[0] = q, kstate[1] = r, kstate[2] = x, kstate[3]= p, kstate[4] = k
	END