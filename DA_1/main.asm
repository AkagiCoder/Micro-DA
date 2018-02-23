	; Addresses in SRAM to store an array of numbers
	.EQU	STARTADDS	=	0x0222
	.EQU	MEM_DIV_5	=	0x0400
	.EQU	MEM_NDIV_5	=	0x0600

	.ORG	0
	; Task #1
	; Initializes the pointer 0x0222 of register X
	LDI		XL, LOW(STARTADDS)
	LDI		XH, HIGH(STARTADDS)

	LDI		R17, 3					; Outer loop which counts to 3 (R17)
LP0:
	LDI		R16, 100				; Inner loop which counts to 100 (R16)
LP1:
	MOV		R0, XL					; Use R0 for LOW(STARTADDS)
	MOV		R1, XH					; Use R1 for HIGH(STARTADDS)
	ADD		R0, R1					; LOW(STARTADDS) + HIGH(STARTADDS)
	ST		X+, R0					; Stores the results
	DEC		R16						; Updates inner loop counter
	BRNE	LP1						; Checks inner loop condition (R16 != 0)
	DEC		R17						; Updates outer loop counter
	BRNE	LP0						; Checks outer loop condition (R17 != 0)
	
	; Task #2
	; Initializes the pointer 0x0222 of register X
	LDI		XL, LOW(STARTADDS)
	LDI		XH, HIGH(STARTADDS)
	; Initializes the pointer 0x0400 of register Y
	LDI		YL, LOW(MEM_DIV_5)
	LDI		YH, HIGH(MEM_DIV_5)
	; Initializes the pointer 0x0600 of register Z
	LDI		ZL,	LOW(MEM_NDIV_5)
	LDI		ZH,	HIGH(MEM_NDIV_5)

	LDI		R24, 0x00				; Count the number of numbers divisible by 5
	LDI		R25, 0x00				; Count the number of numbers isn't divisible by 5

	LDI		R17, 3					; Outer loop which counts to 3 (R17)
LP2:
	LDI		R16, 100				; Inner loop which counts to 100 ($16)
LP3:
	LD		R18, X					; Loads the number to be modified (masking) into R18
	LD		R19, X+					; Saves the unmodified number into R19
SUB_LOOP:
	CPI		R18, 0x05				; Checks if the current result is divisible by 5
	BREQ	DIV_5
	SUBI	R18, 0x05				; Subtracts the number being tested by 5
	BRLO	NOT_DIV_5				; If an underflow occurs, the number is not divisible by 5
	RJMP	SUB_LOOP
DIV_5:
	ST		Y+, R19					; Stores the number that was divisible by 5
	INC		R24						; Updates count (Divisible)
	RJMP	END_DIV
NOT_DIV_5:
	ST		Z+,	R19					; Stores the number that was not divisible by 5
	INC		R25						; Updates count (Not Divisible)
END_DIV:
	DEC		R16						; Updates inner loop counter
	BRNE	LP3						; Checks inner loop condition (R16 != 0)
	DEC		R17						; Updates outer loop counter
	BRNE	LP2						; Checks outer loop condition (R17 != 0)

	; Task #3
	; Initializes the pointer registers X and Y
	LDI		XL,	LOW(MEM_DIV_5)		; Pointer to the array of numbers that are divisible by 5
	LDI		XH,	HIGH(MEM_DIV_5)
	LDI		YL, LOW(MEM_NDIV_5)		; Pointer to the array of numbers that aren't divisible by 5
	LDI		YH,	HIGH(MEM_NDIV_5)
	; Clear the registers
	CLR		R16
	CLR		R17
	CLR		R18
	CLR		R19
	CLR		R22						; Zeroed upper 8-bit for the 16-bit operands
SUM_0:
	LD		R20, X+					; Grabs the number from the array and updates the pointer
	ADD		R16, R20				; Adds lower 8-bits
	ADC		R17, R22				; Adds upper 8-bits
	DEC		R24						; Update counter
	BRNE	SUM_0
SUM_1:
	LD		R21, Y+					; Grabs the number form the array and updates the pointer
	ADD		R18, R21				; Adds lower 8-bits
	ADC		R19, R22				; Adds upper 8-bits
	DEC		R25						; Update counter
	BRNE	SUM_1
END:
	RJMP	END