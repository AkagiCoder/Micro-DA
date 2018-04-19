/*
 * Mid-Term_2.c
 *
 * Created: 4/18/2018 1:58:43 PM
 * Author : takemb1
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdint.h>

#define FOSC 16000000			// Clock speed
#define BAUD 115200				// Desire baud rate
#define MYUBRR FOSC/8/BAUD-1	// Formula to set the baud rate

volatile uint8_t ADCvalue;
void UART_TX(char data);

int main(void)
{
	// Port declaration
	DDRB |= (1 << 1);			// Set PB.1 as output
	
	// ADC declaration
	ADMUX = 0;					// Use ADC0
	ADMUX |= (1 << ADLAR);		// Left justified
	ADMUX |= (1 << REFS0);		// AVcc is reference with ARef connected to external capacitor
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);	// 16 MHz with prescalar of 128
	ADCSRA |= (1 << ADATE);		// Enable auto trigger
	ADCSRB = 0;					// Free running settings for auto trigger
	ADCSRA |= (1 << ADEN);		// Enable ADC
	ADCSRA |= (1 << ADSC);		// Start conversion
	
	// USART declaration
	UBRR0H = ((MYUBRR) >> 8);	// Set baud rate for UPPER Register
	UBRR0L = MYUBRR;			// Set baud rate for LOWER Register
	UCSR0A |= (1 << U2X0);		// Double UART transmission speed
	UCSR0B |= (1 << TXEN0);		// Enable transmitter
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);	// Frame: 8-bit Data and 1 Stop bit
	
	// F = 16 MHz
	TCNT1 = 49911;				// 65536 - (16 MHz/1024
	TIMSK1 |= (1 << TOIE1);		// Enable TIMER1 OVF interrupt
	TCCR1A = 0;					// Not used
	TCCR1B |= (1 << CS12) | (1 << CS10);	// Prescalar of 1024
	TCCR1C = 0;					// Not used
	sei();						// Enable gloabl interrupts
	
    while (1) 
    {
		while((ADCSRA & (1 << ADIF)) == 0);		// Wait for ADC conversion
		ADCvalue = (ADCH << 1);					// Assign the temperature
    }
}

ISR(TIMER1_OVF_vect)
{
	uint8_t i = 0;						// Iterative variable
	char temp[3];						// Temperature variable
	
	TIFR1 = (1 << TOV1);				// Clear TOV1 flag
	TCNT1 = 49911;						// Reset TCNT1
	PORTB ^= (1 << 1);					// Toggle LED
	itoa(ADCvalue, temp, 10);			// Convert integer value into ASCII
	
	// Transmit the temperature to terminal
	for(i = 0; i < 3; i++)
		UART_TX(temp[i]);				// Temperature [ASCII]
	UART_TX('\n');						// Line feed
}

// Function to transmit a byte of data via UART
void UART_TX(char data)
{
	while(!(UCSR0A & (1 << UDRE0)));	// Wait for UART to be available
	UDR0 = data;						// Send the data
}