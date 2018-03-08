.ORG 0x0000
	JMP MAIN
.ORG 0x0006
	JMP ISR_INT0

.ORG 0x0100
MAIN:
	; Initialize stack
	LDI		R16, HIGH(RAMEND)
	OUT		SPH, R16
	LDI		R16, LOW(RAMEND)
	OUT		SPL, R16
	; PORT initialization
	CBI		DDRD, 2				; Set PORTD.2 as input
	SBI		PORTD, 2			; Pull the resistor of PORTD.2
	SBI		DDRB, 2				; Set PORTB.2 as output
	CBI		PORTB, 2			; Clear PORTB.2
	; Interrupt register initialization
	LDI		R16, (1<<INT0)
	STS		EIMSK, R16			; Enable INT0 interrupt
	LDI		R16, 0x02
	STS		EICRA, R16			; INT0 interrupt occurs on the falling edge
	SEI
LOOP:
	RJMP	LOOP				; Infinite loop until INT0 interrupt occurs
.ORG 0x0200
ISR_INT0:
	