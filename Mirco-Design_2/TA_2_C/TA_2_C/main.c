#include <avr/io.h>

void T0Delay();

int main(void)
{
	// Port initialization
	DDRD = 0x00;		// Set PORTD as an input
	PORTD = 0x04;		// Pull up the resistor of PORTD.2
	DDRB = 0x04;		// Set PORTB.2 as an output
	while(1)
	{
		if((PIND&0x04) == 0x04)
		{
			PORTB |= 0x04;
			T0Delay();
			PORTB &= 0xFB;
		}
	}
}

// Delay subroutine for Timer0
// F = 1 Mhz
void T0Delay()
{
	// Set the TCNT1 = 0xC2F7
	TCNT1H = C2;
	TCNT1L = F7;
	// Start Timer1 with prescalar 64
	TCCR1A = 0x00;
	TCCR1B = 0x03;
	while((TIFR1&0x01)== 0);	// Loop until TOV flag is set
	TCCR1B = 0x00;				// Stop the timer
	TIFR1 |= 0x01;				// Reset TOV flag
}