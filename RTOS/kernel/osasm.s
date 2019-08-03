;/*****************************************************************************/
; OSasm.s: low-level OS commands, written in assembly                       */
;


        AREA |.text|, CODE, READONLY, ALIGN=2
        THUMB
        REQUIRE8
        PRESERVE8
        EXTERN  Pt_Task            ; currently running pointer to thread
        EXPORT  __start_os__
        EXPORT  __Sys_Handler__
        IMPORT  SchedulerTask


__Sys_Handler__               		; 1) Saves R0-R3,R12,LR,PC,PSR
    CPSID   I                   	; 2) Prevent interrupt during switch
    PUSH {R4-R11}					; 3) Save remaining regs r4-r11
	LDR	R0,=Pt_Task					; 4) R0 = pointer to Pt_Task, old thread 
	LDR R1, [R0]					; 5) R1 = RunPt
	STR SP, [R1]					; 6) SP = RunPt, Save SP into TCB
	PUSH {R0, LR}					; save R0, LR before branch
	BL SchedulerTask					; 7) Run the subroutine Scheduler
	POP {R0, LR}					; restore R0, LR
	LDR R1, [R0]					; 8) R1= RunPt, new thread
	LDR SP, [R1]					; 10) new thread SP; SP = Pt_Task->sp
	POP {R4-R11}					; 11) restore r4-r11
    CPSIE   I                   	; 12) tasks run with interrupts enabled
    BX      LR                  	; 13) restore R0-R3,R12,LR,PC,PSR

__start_os__
    LDR R0,=Pt_Task			   	; currently running thread
	LDR R2, [R0]				; R2 = value of RunPt
	LDR SP, [R2]				; new thread SP; SP = Pt_Task->stack pointer
	POP {R4-R11}				; restore regs r4-11
	POP {R0-R3}					; restore regs r0-3
	POP {R12}					; restore r12
	ADD SP,SP,#4				; discard LR from initial stack. SP = PC
	POP {LR}					; start location
	ADD SP,SP,#4				; discard PSR
    CPSIE   I                   ; Enable interrupts at processor level
    BX      LR                  ; start first thread

    ALIGN
    END
