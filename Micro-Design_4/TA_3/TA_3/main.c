/*
 * TA_3.c
 *
 * Created: 4/14/2018 3:03:50 PM
 * Author : Bryan
 */ 

#include <avr/io.h>
#include <stdint.h>

int main(void)
{
	// ADC settings
	ADMUX |= (1 << REFS0);									// AVcc with external capacitor at AREF pin
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);	// Set prescalar to 128
	ADCSRA |= (1 << ADEN);									// Enable ADC
	
	// PWM Settings
	DDRB = (1 << 1);										// Set OC1A as output (PD1)
	ICR1 = 5000;											// Set top
	TCCR1A |= (1 << COM1A1) | (1 << COM1B1);				// Set OC1A high on compare
	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM12) | (1 << WGM13);					// Fast PWM Mode
	TCCR1B |= (1 << CS11) | (1 << CS10);					// Clocked divided by 64
	
    while (1) 
    {
		ADCSRA |= (1 << ADSC);								// Start ADC conversion
		while((ADCSRA & (1 << ADIF)) == 0);					// Wait for conversion
		OCR1A = (ADC/3) + 200;								// Assign ADC value to PWM
	}
}

