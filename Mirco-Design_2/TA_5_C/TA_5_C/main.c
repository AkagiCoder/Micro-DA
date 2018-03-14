#include <avr/io.h>
#include <avr/interrupt.h>

void T1Delay();

int main(void)
{
	// Port initialization
	DDRD = 0x00;	// Set PORTD as inputs
	PORTD |= (1<<2);	// Pull up the resistor of PORTD.2
	DDRB |= (1<<2);	// Set PORTB.2 as output
	PORTB = 0x00;	// Clear PORTB
	// Interrupt register initialization
	EIMSK |= (1<<INT0);	// Enable INT0 interrupt
	EICRA |= 0x02;		// INT0 interrupt occurs on the falling edge
	sei();				// Enable global interrupt
    while (1) 
    {
		
    }
	return 0;
}

ISR(INT0_vect)
{
	PORTB |= (1<<2);	// Light up the LED
	while((PIND&(1<<2))&0x00);
	T1Delay();
	PORTB = 0x00;		// Turn off LED
	EIFR = (1<<INTF0);	// Clear interrupt flag
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