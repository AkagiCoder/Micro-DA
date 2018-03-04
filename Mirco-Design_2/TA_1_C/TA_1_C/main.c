#include <avr/io.h>

void T1Delay();

int main(void)
{
	DDRB |= 0x04;		// Set PORTB.2 as output
	PORTB = 0x00;		// Clear PORTB
	while(1)
	{
		T1Delay();
		PORTB ^= 0x04;	// Toggle the LED
	}
	return 0;
}

// Function that uses Timer1
// F = 1 MHz
void T1Delay()
{
	// Set the TCNT1 = 0x85EE (34286)
	TCNT1H = 0x85;
	TCNT1L = 0xEE;
	// Start clock using prescalar 8
	TCCR1A = 0x00;
	TCCR1B = 0x02;
	while((TIFR1&0x01) == 0); // Loop until TOV flag is set
	TCCR1B = 0x00;			  // Stop the timer
	TIFR1 |= 0x01;			  // Reset TOV flag
}