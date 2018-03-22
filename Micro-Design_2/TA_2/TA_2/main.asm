.ORG 0
	; Initialize stack
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
HOLD:
	SBIS	PIND, 2		; Poll until the user lets go of the switch
	RJMP	HOLD
	CALL	DELAY		; Delay for 1 second after switch is off
	CBI		PORTB, 2	; Turn off the LED
	RJMP	POLL

	; Delay subroutine (F = 0.5 MHz) [Delay for 1 second]
DELAY:
	; Set the TCNT1 = 65536-62500 = 3036
	LDI		R20, HIGH(-62500)
	STS		TCNT1H, R20
	LDI		R20, LOW(-62500)
	STS		TCNT1L, R20
	; Set timer control register
	LDI		R20, 0x00
	STS		TCCR1A, R20
	LDI		R20, 0x02
	STS		TCCR1B, R20	; Start the timer with a prescalar of 8
LOOP:
	IN		R20, TIFR1	; Check for the TOV flag
	SBRS	R20, TOV0
	RJMP	LOOP
	; Stop timer and clear TOV flag
	LDI		R20, 0x0
	STS		TCCR1B, R20
	LDI		R20, (1<<TOV0)
	OUT		TIFR1, R20
	RET