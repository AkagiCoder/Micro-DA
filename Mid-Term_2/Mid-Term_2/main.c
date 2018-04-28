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
#include <stdbool.h>
#include <string.h>
#include "nrf24l01-mnemonics.h"
#include "nrf24l01.h"

#define F_CPU 16000000UL		// CPU Speed for delay
#define FOSC 16000000			// Clock speed
#define BAUD 115200				// Desire baud rate
#define MYUBRR FOSC/8/BAUD-1	// Formula to set the baud rate

volatile uint8_t ADCvalue;
volatile bool trans_msg = false;
volatile uint8_t strLen = 4;	// Iterative variable
volatile char TEMPERATURE[4];	// Temperature variable

void UART_TX(char *data);

nRF24L01 *setup_rf(void);

int main(void)
{	
	// Temporary variables
	uint8_t i = 0;		// Iterative variables
	char temp[4];		// Temporary temperature
	
	// nRF
	nRF24L01 *rf;				// RF struct
	nRF24L01Message msg;		// Data to send to the RF
	uint8_t to_address[5] = { 0x02, 0x04, 0x06, 0x08, 0x0A };

	// Port declaration
	DDRC |= (1 << 5);			// Set PB.1 as output
	
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
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);				// Frame: 8-bit Data and 1 Stop bit
	
	// F = 16 MHz
	TCNT1 = 49911;				// 65536 - (16 MHz/1024)
	TIMSK1 |= (1 << TOIE1);		// Enable TIMER1 OVF interrupt
	TCCR1A = 0;					// Not used
	TCCR1B |= (1 << CS12) | (1 << CS10);					// Prescalar of 1024
	TCCR1C = 0;					// Not used
	sei();						// Enable global interrupts
	
	rf = setup_rf();			// Initialize the rf
	
    while (1) 
    {
		while((ADCSRA & (1 << ADIF)) == 0);		// Wait for ADC conversion
		ADCvalue = (ADCH << 1);					// Assign the temperature
		
		// Verify if the temperature is ready to be transmitted
		if(trans_msg == true)
		{
			trans_msg = false;							// Signal RF module
			for(i = 0; i < strLen; i++);
			{
				temp[i] = TEMPERATURE[i];
			}
			i = strLen;									// Assign the strLen
			memcpy(msg.data, temp, i);					// Copy temperature to nRF message struct
			msg.length = strlen((char *)msg.data) + 1;	// Obtain the length of the string
			nRF24L01_transmit(rf, to_address, &msg);	// Transmit to the RF
		}
    }
}

ISR(TIMER1_OVF_vect)
{
	char LF = '\n';						// Line Feed
	char temp[4];						// Temporary temperature
	uint8_t i = 0;						// Iterative variables
	strLen = 0;							// Reset strLen
	
	TIFR1 = (1 << TOV1);				// Clear TOV1 flag
	TCNT1 = 49911;						// Reset TCNT1
	PORTC ^= (1 << 5);					// Toggle LED
	itoa(ADCvalue, temp, 10);			// Convert integer value into ASCII
	
	// Send temperature via UART to terminal
	while(temp[i] != 0)					// Loop terminates on NULL
	{
		UART_TX(&temp[i]);				// Output the temperature to terminal
		i++;							// Count the number of characters
		strLen++;						// Size of the string
	}
	UART_TX(&LF);						// Line feed
	
	// Save the temperature
	for(i = 0; i < strLen; i++)
		TEMPERATURE[i] = temp[i];
	trans_msg = true;					// Signal the RF to send
}

// Function to transmit a byte of data via UART
void UART_TX(char *data)
{
	while(!(UCSR0A & (1 << UDRE0)));	// Wait for UART to be available
	UDR0 = *data;						// Send the data
}

// Set up nRF24L01
nRF24L01 *setup_rf(void)
{
    nRF24L01 *rf = nRF24L01_init();
    rf->ss.port = &PORTB;
    rf->ss.pin = PB2;
    rf->ce.port = &PORTB;
    rf->ce.pin = PB1;
    rf->sck.port = &PORTB;
    rf->sck.pin = PB5;
    rf->mosi.port = &PORTB;
    rf->mosi.pin = PB3;
    rf->miso.port = &PORTB;
    rf->miso.pin = PB4;
    nRF24L01_begin(rf);
    return rf;
}