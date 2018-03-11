.ORG 0
	; Initialize the stack
	LDI		R16, HIGH(RAMEND)
	OUT		SPH, R16
	LDI		R16, LOW(RAMEND)
	OUT		SPL, R16

	SBI		DDRB, 2		; Set PORTB as output
	LDI		R16, (1<<2)	; Output signal of PORTB
	LDI		R17, 0x00
	OUT		PORTB, R17	; Clear PORTB

	; Toggle PB2
BEGIN:
	RCALL	DELAY
	EOR		R17, R16	; Toggle logic
	OUT		PORTB, R17	; Output the toggled signal
	RJMP	BEGIN

	; Delay for 0.25 second
	; Delay subroutine using Timer0 (F = 0.5 MHz)
DELAY:
	; Set TCNT0 = 256-122 = 134
	LDI		R20, -122
	OUT		TCNT0, R20
	; Start the timer with a prescalar of 1024
	LDI		R20, 0x00
	OUT		TCCR0A, R20
	LDI		R20, 0x05
	OUT		TCCR0B, R20
	; Delay for 134 cycles by checking the TOV0 flag
LOOP:
	IN		R20, TIFR0
	SBRS	R20, TOV0
	RJMP	LOOP
	; Stop timer and reset TOV flag
	LDI		R20, 0x00
	OUT		TCCR0B, R20
	LDI		R20, (1<<TOV0)
	OUT		TIFR0, R20
	RET