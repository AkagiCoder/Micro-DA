/*
 * TA_1.c
 *
 * Created: 4/9/2018 6:08:56 PM
 * Author : Bryan
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>


int main(void)
{
	volatile uint16_t ADCvalue;
	DDRD |= (1 << 6);					// PD.6 is output [PWM]
	PORTD |= (1 << 2);					// Pull up the resistor for INT0
	EIMSK |= (1 << INT0);				// Enable interrupt for INT0
	EICRA |= 0x2;						// Trigger on falling edge
	
	// ADC settings
	ADMUX |= (1 << REFS0);									// AVcc with external capacitor at AREF pin
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);	// Set prescalar to 128 for 16 MHz
	ADCSRA |= (1 << ADEN);									// Enable ADC
	
	// PWM Settings
	DDRD = (1 << 6);										// Set OC0A as output (PD6)
	TCCR0A |= (1 << COM0A1) | (1 << COM0A0);				// Inverting mode
	TCCR0A |= (1 << WGM01) | (1 << WGM00);					// Fast PWM mode
	TCCR0B |= (1 << CS02) | (1 << CS00);					// Fosc/1024
	
	sei();
	
    while (1) 
    {
		ADCSRA |= (1 << ADSC);				// Start conversion
		while((ADCSRA & (1 << ADIF)) == 0);	// Wait for conversion
		ADCvalue = ADC >> 2;
		ADCvalue = 0xFF - ADCvalue;
		OCR0A = ADCvalue;
    }
	return 0;
}

ISR(INT0_vect)
{
	EIFR = (1 << INTF0);
}