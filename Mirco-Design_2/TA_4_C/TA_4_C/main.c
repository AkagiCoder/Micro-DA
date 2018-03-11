#include <avr/io.h>
#include <avr/interrupt.h>


int main(void)
{
	DDRB = (1<<2);			// Set PORTB.2 as output
	TCNT0 = 0x86;			// Set TCNT0 = 134 (DELAY)
	TIMSK0 = (1<<TOIE0);	// Enable TIMER0 interrupt for OVF
	sei();					// Enable global interrupts
	TCCR0A = 0x00;
	TCCR0B = 0x05;			// Start TIMER0 with a prescalar of 1024 (F = 0.5 MHz)
	PORTB = 0x00;			// Clear PORTB
	// Poll until the interrupt occurs
    while(1)
	{
		
	}
	return 0;
}

// Subroutine to handle the TIMER0 OVF interrupt
// Clear TOV, toggle PB.2, and reset TCNT0
ISR(TIMER0_OVF_vect)
{
	TIFR0 = (1<<TOV0);		// Clear TOV flag
	PORTB ^= (1<<2);		// Toggle PORTB.2
	TCNT0 = 0x86;			// Reset TCNT0 count back to 134
}
