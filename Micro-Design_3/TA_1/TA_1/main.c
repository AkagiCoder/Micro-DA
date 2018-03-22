/*
 * TA_1.c
 *
 * Created: 3/21/2018 6:57:28 PM
 * Author : Bryan
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define FOSC 4000000				// Clock speed
#define BAUD 9600					// Desire baud rate
#define MYUBRR FOSC/16/BAUD-1

int main(void)
{
	// Port declarations
	DDRB = (1<<5);			// Set PORTB.5 as output
	PORTB = 0x00;			// Clear PORTB
	
	// F = 8 MHz
	TCNT1 = 34286;			// 65536-(8 MHz/256)
	TIMSK1 = (1<<TOIE1);	// Enable TIMER1 OVF interrupt
	sei();					// Enable global interrupts
	TCCR1A = 0x00;
	TCCR1B = 0x04;			// Start TIMER1 with prescalar 256
	TCCR1C = 0x00;
    while (1) 
    {
    }
	return 0;
}

// Interrupt subroutine for TIMER1 overflow
ISR(TIMER1_OVF_vect)
{
	TIFR1 = (1<<TOV1);		// Clear TOV1 flag
	PORTB ^= (1<<5);		// Toggle LED
	TCNT1 = 34286;			// Reset TCNT1
}