.ORG 0
	; Initialize the stack
	LDI		R16, HIGH(RAMEND)
	OUT		SPH, R16
	LDI		R16, LOW(RAMEND)
	OUT		SPL, R16

	SBI		DDRB, 2		; Set PORTB as output
	LDI		R16, (1<<2)	; Output signal of PORTB
	LDI		R17, 0x0
	OUT		PORTB, R17	; Clear PORTB

	; Toggle PB2 port
BEGIN:
	RCALL	DELAY
	EOR		R17, R16	; Toggle logic
	OUT		PORTB, R17	; Output the toggled signal
	RJMP	BEGIN

	; Delay for 0.25 second
	; Delay subroutine using Timer1 (F = 0.5 MHz)
DELAY:
	; Set the TCNT1 = 65536-15625 = 49911
	LDI		R20, HIGH(-15625)
	STS		TCNT1H, R20
	LDI		R20, LOW(-15625)
	STS		TCNT1L, R20
	; Start the timer with a prescalar of 8
	LDI		R20, 0x00
	STS		TCCR1A, R20
	LDI		R20, 0x02
	STS		TCCR1B, R20
	; Delay for 31250 cycles
LOOP:
	IN		R20, TIFR1
	SBRS	R20, TOV1
	RJMP	LOOP
	; Stop timer and reset TOV flag
	LDI		R20, 0x0
	STS		TCCR1B, R20
	LDI		R20, (1<<TOV1)
	OUT		TIFR1, R20
	RET