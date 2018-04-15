/*
 * TA_2.c
 *
 * Created: 4/12/2018 3:44:46 PM
 * Author : takemb1
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

volatile uint8_t stepperPos;
volatile uint16_t ADCvalue;

int main(void)
{
	// Port initialization
	DDRB |= 0x0F;
	PORTB = 0x00;
	
	// ADC settings
	ADMUX |= (1 << REFS0);
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADCSRA |= (1 << ADEN);
	
	// Timer0 settings
	TCCR1B |= (1 << WGM12);					// CTC mode
	TCCR1B |= (1 << CS11);
	TIMSK1 |= (1 << OCIE1A);				// CTC interrupt
	TCNT1 = 0x00;
	OCR1A = 65535;
	
	// Initialize stepperPos
	stepperPos = 0x06;
	//PORTB = stepperPos;
	
	sei();
    while (1) 
    {
		ADCSRA |= (1 << ADSC);					// Start conversion
		while((ADCSRA & (1 << ADIF)) == 0);		// Wait for conversion
		ADCvalue = ADC << 5;
    }
	return 0;
} 

// CTC ISR
ISR(TIMER1_COMPA_vect)
{
	OCR1A = 65535 - ADCvalue;
	if(ADCvalue > 32)
	{
		switch(stepperPos)
		{
			case 0x06:
				stepperPos = 0x0C;
				break;
			case 0x0C:
				stepperPos = 0x09;
				break;
			case 0x09:
				stepperPos = 0x03;
				break;
			case 0x03:
				stepperPos = 0x06;
				break;
			default:
				break;
		}
		PORTB = stepperPos;
	}
	else
		PORTB = 0x00;
	TIFR1 |= (1 << OCF1A);	// Clear flag
}

