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
	LDR		

	ALIGN
init_InputArray	DCFS 1.0, 1.5, 0.0, 0.78, 1.32, 1.44
init_kstate			DCFS 0.1, 0.1, 0.0, 0.1, 0.0
		
	AREA myData, DATA, READWRITE
	
	ALIGN
InputArray			SPACE 6*4	; 6 floats
OutputArray 		SPACE 6*4
kstate					SPACE 5*4
	END