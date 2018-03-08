#include <avr/io.h>

void T1Delay();

int main(void)
{
	DDRD = 0x00;		// Set PORTD.2 as input
	PORTD |= (1<<2);	// Pull up the resistor of PORTD.2
	DDRB = 0xFF;		// Set PORTB.2 as output
	PORTB = 0x00;		// Clear PORTB
	
	// Poll until switch is pressed
	while(1)
	{
		if((PIND&0x04) == 0x00)
		{
			PORTB = (1<<2);				// Turn on LED
			while((PIND&0x04) == 0x00);	// Poll until switch is turned off
			T1Delay();					// Call the delay after switch is turned off
		}
		PORTB = 0x00;					// Turn off LED
	}
	return 0;
}

// Delay subroutine for Timer1 [Delay for 1 second]
// F = 0.5 MHz
void T1Delay()
{
	// Set the TCNT1 = 65536-62500 = 3036 [0xBDC]
	TCNT1H = 0x0B;
	TCNT1L = 0xDC;
	// Start Timer1 with prescalar 8
	TCCR1A = 0x00;
	TCCR1B = 0x02;
	while((TIFR1&(1<<TOV1)) == 0x00);	// Check TOV flag
	TCCR1B = 0x00;						// Stop Timer1
	TIFR1 |= (1<<TOV1);					// Reset TOV flag in TIFR1 register
}