#include <avr/io.h>

void T0Delay();

int main(void)
{
	DDRB |= 0x04;		// Set PORTB.2 as output
	PORTB = 0x00;		// Clear PORTB
	while(1)
	{
		T0Delay();		// Call delay
		PORTB ^= 0x04;	// Toggle the LED
	}
	return 0;
}

// Delay for 0.25 second
// Delay subroutine using Timer0 (F = 0.5 MHz)
void T0Delay()
{
	TCNT0 = 0x86;		// Set TCNT0 = 256-122 = 134
	TCCR0A = 0x00;
	TCCR0B = 0x05;		// Start the timer0 with a prescalar of 1024
	while((TIFR0&0x01) == 0);
	TCCR0B = 0x00;		// Stop timer0
	TIFR0 |= (1<<TOV0); // Reset TOV flag
}