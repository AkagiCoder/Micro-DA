.ORG 0
	LDI		R16, HIGH(RAMEND)
	OUT		SPH, R16
	LDI		R16, LOW(RAMEND)
	OUT		SPL, R16

	SBI		DDRB, 2		; Set PORTB as output
	LDI		R16, (1<<2)	; Output signal of PORTB
	LDI		R17, 0x0
	OUT		PORTB, R17	; Clear PORTB

	; Toggle PB2
BEGIN:
	RCALL	DELAY
	EOR		R17, R16	; Toggle logic
	OUT		PORTB, R17	; Output the toggled signal
	RJMP	BEGIN

	; Delay subroutine using Timer0 (F = 1 MHz)
DELAY:
	; Set TCNT0 = 244
	LDI		R20, -244
	OUT		TCNT0, R20
	; Start the timer with a prescalar of 1024
	LDI		R20, 0x00
	STS		TCCR0A, R20
	LDI		R20, 0x05
	STS		TCCR0B, R20
	; Delay for 244 cycles
LOOP:
	IN		R20, TIFR0
	SBRS	R20, TOV0
	RJMP	LOOP
	; Stop timer and reset TOV flag
	LDI		R20, 0x00
	STS		TCCR0B, R20
	LDI		R20, (1<<TOV0)
	OUT		TIFR0, R20
	RET