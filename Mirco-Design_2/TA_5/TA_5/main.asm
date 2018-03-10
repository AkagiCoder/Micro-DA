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
	SBI		PORTB, 2			; Light up the LED
HOLD:
	SBIS	PIND, 2				; Poll until the user lets go of the switch
	RJMP	HOLD
	CALL	DELAY
	CBI		PORTB, 2
	LDI		R21, (1<<INTF0)
	STS		EIFR, R21
	RETI

	; Delay subroutine (F = 0.5 MHz) [Delay for 1 second]
DELAY:
	LDI		R20, HIGH(-62500)
	STS		TCNT1H, R20
	LDI		R20, LOW(-62500)
	STS		TCNT1L, R20
	; Set timer control register
	LDI		R20, 0x00
	STS		TCCR1A, R20
	LDI		R20, 0x02
	STS		TCCR1B, R20	; Start the timer with a prescalar of 8
DLOOP:
	IN		R20, TIFR1	; Check for the TOV flag
	SBRS	R20, TOV0
	RJMP	DLOOP
	; Stop timer and clear TOV flag
	LDI		R20, 0x0
	STS		TCCR1B, R20
	LDI		R20, (1<<TOV0)
	OUT		TIFR1, R20
	RET