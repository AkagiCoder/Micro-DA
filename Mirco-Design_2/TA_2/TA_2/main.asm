.ORG 0
	LDI		R16, HIGH(RAMEND)
	OUT		SPH, R16
	LDI		R16, LOW(RAMEND)
	OUT		SPL, R16

	; Port initialization
	CBI		DDRD, 2		; Set PORTD.2 as an input
	SBI		PORTD, 2	; Pull up the resistor of PORTD.2
	SBI		DDRB, 2		; Set PORTB.2 as an output

POLL:
	SBIC	PIND, 2		; Poll until PIND.2 is low
	RJMP	POLL
	SBI		PORTB, 2	; Light up the LED
	CALL	DELAY		; Delay for 1 second
	CBI		PORTB, 2	; Turn off the LED
	RJMP	POLL

	; Delay subroutine (F = 1 MHz)
DELAY:
	; Set the TCNT1 = 0xC2F7 (49911) [65546-15625 = 49911]
	LDI		R20, HIGH(-15625)
	STS		TCNT1H, R20
	LDI		R20, LOW(-15625)
	STS		TCNT1L, R20
	; Set timer control register
	LDI		R20, 0x00
	STS		TCCR1A, R20
	LDI		R20, 0x03
	STS		TCCR1B, R20	; Start the timer with a prescalar of 64
LOOP:
	IN		R20, TIFR1
	SBRS	R20, TOV0
	RJMP	LOOP
	; Stop timer and reset TOV flag
	LDI		R20, 0x0
	STS		TCCR1B, R20
	LDI		R20, (1<<TOV0)
	OUT		TIFR1, R20
	RET