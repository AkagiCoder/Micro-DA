/*
 * TA_1.c
 *
 * Created: 3/21/2018 6:57:28 PM
 * Author : Bryan
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define FOSC 16000000				// Clock speed
#define BAUD 115200					// Desire baud rate
#define MYUBRR FOSC/8/BAUD-1		// Formula to set the baud rate
volatile uint8_t ADCvalue;			// Storage for the temperature in F

// AT Commands
volatile unsigned char WIFI[] = "AT+CWJAP=\"NETGEAR63\",\"freepiano573\"\r\n";
volatile unsigned char AT[] = "AT\r\n";
volatile unsigned char CIPMUX[] = "AT+CIPMUX=1\r\n";
volatile unsigned char CIPSTART[] = "AT+CIPSTART=0,\"TCP\",\"api.thingspeak.com\",80\r\n";
volatile unsigned char CIPSEND[] = "AT+CIPSEND=0,110\r\n";
volatile unsigned char GET_DATA[] = "GET https://api.thingspeak.com/apps/thinghttp/send_request?api_key=VUH8L3BHKFOR587J\r\n";
volatile unsigned char SEND_DATA[] = "GET https://api.thingspeak.com/update?api_key=VUH8L3BHKFOR587J=50\r\n";

void send_AT(volatile unsigned char AT[]);

int main(void)
{
	
	// Port declarations
	DDRB = (1<<5);			// Set PORTB.5 as output
	PORTB = 0x00;			// Clear PORTB
	
	// ADC declaration
	ADMUX = 0;											// Use ADC0
	ADMUX |= (1<<ADLAR);								// Left justified
	ADMUX |= (1<<REFS0);								// AVcc is reference
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);		// 16 MHz with prescalar of 128
	ADCSRA |= (1<<ADATE);								// Enable auto trigger
	ADCSRB = 0;											// Free running settings for auto trigger
	ADCSRA |= (1<<ADEN);								// Enable ADC
	ADCSRA |= (1<<ADIE);								// Enable ADC interrupt
	ADCSRA |= (1<<ADSC);								// Start conversion
	
	// USART declaration
	UBRR0H = ((MYUBRR)>>8);					// Set baud rate for UPPER Register
	UBRR0L = MYUBRR;						// Set baud rate for LOWER Register
	UCSR0A |= (1<<U2X0);					// Double UART transmission speed
	UCSR0B |= (1<<TXEN0);					// Enable transmitter
	UCSR0C |= (1<<UCSZ01) | (1<<UCSZ00);	// Frame: 8-bit Data and 1 Stop bit
	
	// F = 8 MHz
	TCNT1 = 49911;						// 65536-(16 MHz/1024)
	TIMSK1 = (1<<TOIE1);				// Enable TIMER1 OVF interrupt	
	TCCR1A = 0x00;
	TCCR1B = (1<<CS12) | (1<<CS10);		// Start TIMER1 with prescalar 256
	TCCR1C = 0x00;
	
	// ESP8266 settings
	_delay_ms(500);
	sei();								// Enable global interrupts
	
	_delay_ms(200);
	send_AT(AT);
	
	_delay_ms(2000);
	send_AT(WIFI);
	
	_delay_ms(2000);
	send_AT(CIPMUX);
	
	_delay_ms(2000);
	send_AT(CIPSTART);
	
    while (1) 
    {
    }
	return 0;
}

// Interrupt subroutine for TIMER1 overflow (1 second)
ISR(TIMER1_OVF_vect)
{
	TIFR1 = (1<<TOV1);				// Clear TOV1 flag
	PORTB ^= (1<<5);				// Toggle LED
	TCNT1 = 49911;					// Reset TCNT1
}

// Interrupt subroutine for ADC value
ISR(ADC_vect)
{
	ADCvalue = (ADCH<<1);		// Store the decimal value of the converted signal
}								// Shift left by one to multiply by 2 and adjust the value

void send_AT(volatile unsigned char AT[])
{
	volatile unsigned char len = 0;
	volatile unsigned char i;
	while(AT[len] != 0)
	{
		len++;
	}
	for(i = 0x00; i < len; i++){
		while(!(UCSR0A & (1<<UDRE0)));	// Wait for the transmitter to finish
		UDR0 = AT[i];					// Transmit the the new value
	}
}