/*
 * Non-Linear_ODE[Lorenz].c
 *
 * Created: 3/27/2018 11:53:41 PM
 * Author : Bryan
 */ 

#include <avr/io.h>
#include <stdint.h>

void dac_0(unsigned int input);

void i2c_init(void);
void i2c_write(unsigned char data);
void i2c_start(void);
void i2c_stop(void);

uint16_t float_add(volatile uint16_t a, volatile uint16_t b);
uint16_t float_mult(volatile uint16_t a, volatile uint16_t b);

int main(void)
{
	PORTC = (1 << 5) | (1 << 4);	// Pull-up the resistors on the SDA and SCL line
	i2c_init();						// Initialize DAC module [Look at function for specific speed]
	
	uint16_t a = 0x4608;
	uint16_t b = 0xC8D8;
	uint16_t c = 0x0000;
	c = float_mult(a, b);
	
	PORTB = c;
	PORTB = 0x00;
	
	/*
	unsigned char x = 0x1D;			// x = 0.1
	unsigned char y = 0x00;			// y = 0.0
	unsigned char z = 0x00;			// z = 0.0
	
	unsigned char dx = 0x00;
	unsigned char dy = 0x00;
	unsigned char dz = 0x00;
	
	unsigned char a = 0x52;			// a = 10.0
	unsigned char b = 0x5E;			// b = 28.0
	unsigned char c = 0x43;			// c = 2.66
	
	unsigned char dt = 0x02;		// dt = 0.01
	*/
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

void dac_0(unsigned int input)
{
	i2c_start();
	i2c_write(0b11000000);
	i2c_write(0b01000000);
	i2c_write(input >> 4);
	i2c_write(input);
	i2c_stop();
}

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
// Floating Point Functions
//--------------------------------------------------------------------------------------

// Adds/Subtracts two floating points
uint16_t float_add(volatile uint16_t a, volatile uint16_t b)
{
	// 6-bit field
	volatile uint8_t exp0 = 0x00;		// Exponent field of a
	volatile uint8_t exp1 = 0x00;		// Exponent field of b
	
	// 9-bit field
	volatile uint16_t mant0 = 0x00;		// Mantissa field of a
	volatile uint16_t mant1 = 0x00;		// Mantissa field of b
	volatile uint8_t shift = 0x00;		// Required shift to normalize
	volatile uint16_t exp = 0x00;		// Result of the exponents
	volatile uint16_t mant = 0x00;		// Result of the mantissas
	volatile uint16_t final = 0x00;		// Result of the addition
	
								// ******* Num 0 ********
	exp0 = a >> 9;				// Extract exponent field
	exp0 = 0x3F & exp0;			// Mask out bit 6 and 7
	mant0 = 0x01FF & a;			// Extract mantissa
	mant0 |= 0x0200;			// Append the implicit 1
	
								// ******* Num 1 ********
	exp1 = b >> 9;				// Extract exponent field
	exp1 = 0x003F & exp1;		// Mask out bit 6 and 7
	mant1 = 0x01FF & b;			// Extract mantissa
	mant1 |= 0x0200;			// Append the implicit 1
	
	// Adjust and compute the exponent
	if(exp0 > exp1)
	{
		shift = exp0 - exp1;
		exp = exp0;
		mant1 = mant1 >> shift;
	}
	else if(exp1 > exp0)
	{
		shift = exp1 - exp0;
		exp = exp1;
		mant0 = mant0 >> shift;
	}
	else
	{
		exp = exp0;
	}
	
	// Same signs [a + b] or [-a + -b]
	if((a & 0x8000) == (b & 0x8000))
	{
		final |= (a & 0x8000);		// Set the sign
		mant = mant0 + mant1;		// Add the two mantissas
		
		// Normalize the mantissa
		if(mant > 0x03FF)
		{
			mant = mant >> 1;		// Shift mantissa to adjust the floating point
			exp += 0x0001;			// Increment the exponent by 1
		}
	}
	// Different signs
	else
	{
		// [a - b]
		if((b & 0x8000) == 0x8000)
		{
			// Two's complement
			mant1 = ~mant1;
			mant1 += 0x0001;
			// Perform the subtraction
			mant = mant0 + mant1;
		}
		// [b - a]
		else
		{
			// Two's complement
			mant0 = ~mant0;
			mant0 += 0x0001;
			// Perform the subtraction
			mant = mant0 + mant1;
		}
		
		// Check if the value is negative, if so, obtain the positive magnitude and set sign bit to 1
		if(mant > 0x7FFF)
		{
			// Two's complement
			mant = ~mant;
			mant += 1;
			// Set sign as negative
			final |= (0x8000);
		}
		
		// Normalize the mantissa
		while(mant < 0x0200)
		{
			mant = mant << 1;
			exp -= 0x0001;
		}
		/*
		if(mant < 0x0002)
		{
			mant = mant << 9;
			exp -= 0x0009;
		}
		else if(mant < 0x004)
		{
			mant = mant << 8;
			exp -= 0x0008;
		}
		else if(mant < 0x008)
		{
			mant = mant << 7;
			exp -= 0x0007;
		}
		else if(mant < 0x0010)
		{
			mant = mant << 6;
			exp -= 0x0006;
		}
		else if(mant < 0x0020)
		{
			mant = mant << 5;
			exp -= 0x0005;
		}
		else if(mant < 0x0040)
		{
			mant = mant << 4;
			exp -= 0x0004;
		}
		else if(mant < 0x0080)
		{
			mant = mant << 3;
			exp -= 0x0003;
		}
		else if(mant < 0x0100)
		{
			mant = mant << 2;
			exp -= 0x0002;
		}
		else if(mant < 0x0200)
		{
			mant = mant << 1;
			exp -= 0x0001;
		}
		*/
	}
	// Check for overflow
	if(exp > 0x003F)
	{
		// Max out the range
		exp = 0x003F;
		mant = 0xFFFF;
	}
	// Check for underflow
	else if(exp < 0x001F)
	{
		// Set to 0
		exp = 0x0000;
		mant = 0x0000;
	}
	mant &= 0x01FF;				// Remove implicit 1
	exp = exp << 9;				// Shift the exponent to correct position
	final |= (exp | mant);		// Result
	return final;
}

// Multiplies two floating points
uint16_t float_mult(volatile uint16_t a, volatile uint16_t b)
{
	// 6-bit field
	volatile uint8_t exp0 = 0x00;		// Exponent field of a
	volatile uint8_t exp1 = 0x00;		// Exponent field of b
	
	// 9-bit field
	volatile uint16_t mant0 = 0x0000;		// Mantissa field of a
	volatile uint16_t mant1 = 0x0000;		// Mantissa field of b
	volatile uint16_t i = 0x0000;			// Loop iterations
	volatile uint16_t exp = 0x0000;			// Result of the exponents
	volatile uint16_t final = 0x0000;		// Result of the addition
	
	volatile uint8_t UA;				// Bits 16-19 of A;
	volatile uint8_t US;				// Bits 16-19 of S;
	volatile uint8_t UP;				// Bits 16-19 of P;
	
	volatile uint16_t A;				// Multiplicand
	volatile uint16_t S;				// Multiplier
	volatile uint16_t P;				// Product
	
								// ******* Num 0 ********
	exp0 = a >> 9;				// Extract exponent field
	exp0 = 0x3F & exp0;			// Mask out bit 6 and 7
	mant0 = 0x01FF & a;			// Extract mantissa
	mant0 |= 0x0200;			// Append the implicit 1
	
								// ******* Num 1 ********
	exp1 = b >> 9;				// Extract exponent field
	exp1 = 0x003F & exp1;		// Mask out bit 6 and 7
	mant1 = 0x01FF & b;			// Extract mantissa
	mant1 |= 0x0200;			// Append the implicit 1
	
	// Compute the initial exponent
	exp = exp0 + exp1;
	
	// Compute the sign
	final |= (a & 0x8000) ^ (b & 0x8000);
	
	// Multiply the mantissas using Booth's algorithm
	// m = mant0		x = 11 bits
	// r = mant1		y = 11 bits
	// A = S = P = x + y + 1 = 23 [UReg + Reg = 24; Ignore bit 23]
	
	// Fill register A with m
	UA = mant0 >> 4;
	A = mant0 << 12;
	
	// Two's complement of m
	mant0 = ~mant0;
	mant0 += 0x0001;
	mant0 &= 0x07FF;
	
	// Fill register S with -m
	US = mant0 >> 4;
	S = mant0 << 12;
	
	// Fill register P
	P = mant1 << 1;
	for(i = 0x0000; i < 11; i++)
	{
		if((P & 0x0003) == 0x0001)
		{
			P += A;
			if(P < A)
				UP += UA + 1;
			else
				UP += UA;
			UP &= 0x7F;
		}
		else if((P & 0x0003) == 0x0002)
		{
			P += S;
			if(P < S)
				UP = UP + US + 1;
			else
				UP += US;
		}
		
		// Adjust for arithmetic shift
		if((UP & 0x40) == 0x40)
			UP |= 0x80;
		else
			UP &= 0x7F;
		
		// Shift register P down to the right by 1
		if((UP & 0x01) == 0x01)
		{
			UP = UP >> 1;
			P = P >> 1;
			P |= 0x8000;
		}
		else
		{
			UP = UP >> 1;
			P = P >> 1;
		}
	}
	
	// Final shift [rid the implicit 1 of P]
	// Note that final answer must be positive, hence, no arithmetic shift
	if((UP & 0x01) == 0x01)
	{
		UP = UP >> 1;
		P = P >> 1;
		P |= 0x8000;
	}
	else
	{
		UP = UP >> 1;
		P = P >> 1;
	}
	
	// Normalize the result
	if(UP > 0x04)				// [Mantissa too large]
	{
		while(UP > 0x04)
		{
			if((UP & 0x01) == 0x01)
			{
				UP = UP >> 1;
				P = P >> 1;
				P |= 0x8000;
			}
			else
			{
				UP = UP >> 1;
				P = P >> 1;
			}
			exp += 0x0001;		// Update exponent
		}
	}
	else if(UP < 0x04)			// [Mantissa too small]
	{
		while(UP < 0x04)
		{
			if((P & 0x8000) == 0x8000)
			{
				P = P << 1;
				UP = UP << 1;
				UP |= 0x01;
			}
			else
			{
				P = P << 1;
				UP = UP << 1;
			}
			exp -= 0x0001;
		}
	}
	
	// Check for overflow
	if(exp > 0x003F)
	{
		// Max out the range
		final |= 0x3FFF;
	}
	// Check for underflow
	else if(exp < 0x001F)
	{
		// Set to 0
		final = 0x0000;
	}
	else
	{
		final |= exp << 9;
		final |= UP << 6;
		final |= P >> 9;
	}
	return final;
}