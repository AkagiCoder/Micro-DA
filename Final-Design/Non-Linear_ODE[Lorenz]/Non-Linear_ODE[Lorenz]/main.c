/*
 * Non-Linear_ODE[Lorenz].c
 *
 * Created: 3/27/2018 11:53:41 PM
 * Author : Bryan
 */ 

#include <avr/io.h>
#include <stdint.h>

void i2c_init(void);
void i2c_write(unsigned char data);
void i2c_start(void);
void i2c_stop(void);

int main(void)
{
	PORTC = (1 << 5) | (1 << 4);  // Pull-up the resistors on the SDA and SCL line
	i2c_init();
	i2c_start();
	i2c_write(0b11000000);
	i2c_write(0b01000000);
	i2c_write(0b10000000);
	i2c_write(0b00000000);
	i2c_stop();
    while (1) 
    {
    }
}

//--------------------------------------------------------------------------------------
// I2C Functions
//--------------------------------------------------------------------------------------

// Data Packet
// | STA | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 || ACK || 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 || ACK || D11 | D10 | D9 | D8 | D7 | D6 | D5 | D4 || ACK || D3 | D2 | D1 | D0 | 0 | 0 | 0 | 0 || ACK | STO |
//                        A2  A1  A0  R/W		  C2  C1  C0   x   x  PD1 PD0  x
void i2c_init(void)
{
	TWSR = 0x00;										// Set prescalar to 0
	TWBR = 0x48;										// SCL = 50 KHz (Fosc = 8 MHz)
	TWCR = (1 << TWEN);									// Enable TWI
}

void i2c_write(unsigned char data)
{
	TWDR = data;										// Data to be transmitted
	TWCR = (1 << TWINT) | (1 << TWEN);					// Use TWI module and write 
	while((TWCR & (1 << TWINT)) == 0);					// Wait for TWI to complete
}

void i2c_start(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);	// Transmit START condition
	while((TWCR & (1 << TWINT)) == 0);					// Wait for TWI to complete
}

void i2c_stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);	// Transmit STOP condition
}
//--------------------------------------------------------------------------------------