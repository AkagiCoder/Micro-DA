.ORG 0x0000
	JMP		MAIN
.ORG 0x0020
	JMP		TIMER0_OVF_ISR

.ORG 0x0100
MAIN:
	; Initialize the stack
	LDI		R16, HIGH(RAMEND)
	OUT		SPH, R16
	LDI		R16, LOW(RAMEND)
	OUT		SPL, R16

	SBI		DDRB, 2		; Set PORTB.2 as output
	LDI		R16, (1<<2)	; Value for toggling the LED
	LDI		R17, 0x00
	OUT		PORTB, R17	; Clear PORTB register
	LDI		R20, -122	; 256-122 = 134 cycles (DELAY)
	OUT		TCNT0, R20	; Load 134 into TCNT0
	LDI		R20, (1<<TOIE0)
	STS		TIMSK0, R20	; Enable TIMER0 interrupt for OVF
	SEI					; Enable interrupts
	LDI		R20, 0x00
	OUT		TCCR0A, R20
	LDI		R20, 0x05
	OUT		TCCR0B, R20	; Start TIMER0 with a prescalar of 1024 (F = 0.5 MHz)
LOOP:
	RJMP	LOOP		; Infinite loop (Wait until TIMER0_OVF interrupts)

.ORG 0x0200
	; ISR for toggling PORTB.2
TIMER0_OVF_ISR:
	LDI		R20, (1<<TOV0)	; Clear TOV flag
	OUT		TIFR0, R20
	EOR		R17, R16		; Toggle PORTB.2
	OUT		PORTB, R17
	LDI		R20, -122
	OUT		TCNT0, R20		; Reset TCNT0
	RETI