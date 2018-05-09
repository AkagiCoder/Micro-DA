#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

// Operations [float_add]
#define SUB		1
#define ADD		0

// Adjust DAC prescalar output
#define SCALE	5 // 64

// Lorenz constants
#define SIGMA	0x41200000		// 10
#define RHO		0x41e00000		// 28
#define BETA	0x402aaaab		// 8/3 [2.666...]

// Lorenz derivatives
volatile uint32_t dt = 0x3c23d70a;	// 0.01
volatile uint32_t dx = 0x00000000;	// Cleared
volatile uint32_t dy = 0x00000000;	// Cleared
volatile uint32_t dz = 0x00000000;	// Cleared

// Coordinates
volatile uint32_t x = 0x3f800000;	// x = 1.0
volatile uint32_t y = 0x3f800000;	// y = 1.0
volatile uint32_t z = 0x3f800000;	// z = 1.0

// Floating Point Operators
uint32_t float_add(uint32_t A, uint32_t B, uint8_t OP);
uint32_t float_mult(uint32_t M, uint32_t R);
uint16_t float_digital(uint32_t Num);

// I2C Functions
void i2c_init(void);
void i2c_write(unsigned char data);
void i2c_start(void);
void i2c_stop(void);

void dac(unsigned int input);

volatile uint32_t j = 0;
volatile uint32_t k = 27;
volatile uint16_t num_out;

int main(void)
{
	uint32_t temp;		// Temporary variable
	uint32_t osc_out;	// Value to output to the DAC
	uint16_t i;			// Iterative variable
	
	DDRB = (1 << 1);	// PB1
	
	i2c_init();			// Initialize the I2C

	_delay_ms(5000);
	while (1)
	{
		for(i = 0; i < 1000; i++) {
			// dx/dt = SIGMA(y-x)
			dx = float_add(y, x, SUB);
			dx = float_mult(SIGMA, dx);
			dx = float_mult(dx, dt);
			
			// dy/dt = x(RHO-z)-y
			dy = float_add(RHO, z, SUB);
			dy = float_mult(dy, x);
			dy = float_add(dy, y, SUB);
			dy = float_mult(dy, dt);
			
			// dz/dt = xy-(BETA)z
			temp = float_mult(x, y);
			dz = float_mult(BETA, z);
			dz = float_add(temp, dz, SUB);
			dz = float_mult(dz, dt);
			
			x = float_add(x, dx, ADD);		// Update the change to X
			y = float_add(y, dy, ADD);		// Update the change to Y
			z = float_add(z, dz, ADD);		// Update the change to Z
			
			osc_out = float_digital(x);		// Output X
			PORTB = (1 << 1);
			while((PINB & 0x4) != 0x4);
			PORTB = 0x0;
			dac(osc_out);
		}
	}
}

//-------------------------------------------------
//------------- FLOATING POINT FUNCTIONS ----------
//-------------------------------------------------

// Floating point addition/subtraction
uint32_t float_add(uint32_t A, uint32_t B, uint8_t OP)
{
	// Save function arguments
	uint32_t a = A;
	uint32_t b = B;
	uint8_t sub = OP;
	
	// Operand a
	uint8_t exp0 = a >> 23;							// Extract the exponent field of a
	uint32_t mant0 = (a & 0x007FFFFF) | 0x00800000;	// Extract the mantissa field of a
	
	// Operand b
	uint8_t exp1 = b >> 23;							// Extract the exponent field of b
	uint32_t mant1 = (b & 0x007FFFFF) | 0x00800000;	// Extract the mantissa field of b
	
	// Final result
	int16_t exp = 0;		// Final exponent
	uint32_t final = 0;	// Result to be returned
	uint32_t mant = 0;		// Final mantissa
	
	// Temporary variables
	uint32_t temp = 0;
	
	// Check if the operation is subtraction
	if(sub > 0)
	b ^= 0x80000000;
	
	// Adjust and compute the exponent
	if(exp0 > exp1)						// exp(a) > exp(b)
	{
		temp = exp0 - exp1;				// shift amount = exp(a) - exp(b)
		exp = exp0;
		mant1 = mant1 >> temp;			// Adjust B
	}
	else if(exp1 > exp0)				// b > a
	{
		temp = exp1 - exp0;				// shift amount = exp(b) - exp(a)
		exp = exp1;
		mant0 = mant0 >> temp;			// Adjust A
	}
	else
	{
		exp = exp0;						// Equal exponents
	}
	
	// Same signs [a + b] or [(-a) + (-b)]
	if((a & 0x80000000) == (b & 0x80000000))
	{
		final |= (a & 0x80000000);		// Set sign
		mant = mant0 + mant1;			// Add the two mantissas
		
		// Normalize the mantissa
		if(mant > 0x00FFFFFF)
		{
			mant = mant >> 1;			// Shift mantissa to adjust the floating point
			exp += 1;					// Increment the exponent
		}
	}
	// Different signs
	else
	{
		// [a - b]
		if((b & 0x80000000) == 0x80000000)
		{
			// Two's complement
			mant1 = ~mant1;
			mant1 += 1;
			// Perform the subtraction
			mant = mant0 + mant1;
		}
		// [b - a]
		else
		{
			// Two's complement
			mant0 = ~mant0;
			mant0 += 1;
			// Perform the subtraction
			mant = mant0 + mant1;
		}
		
		// Check if the value is negative, if so, absolute value the mantissa and set sign bit to 1
		if(mant > 0x00FFFFFF)
		{
			mant = ~mant;
			mant += 1;
			// Set sign as negative
			final |= 0x80000000;
		}
		
		// Normalize the mantissa
		if(mant > 0)
		{
			while(mant < 0x00800000)
			{
				mant = mant << 1;
				exp -= 1;
			}
		}
		else
		exp = 0;
	}
	
	// Overflow case [Largest value]
	// Exponent cannot be larger than 254 [with bias of +127]
	if(exp > 254)
	{
		final |= 0x7FFFFFFF;
		return final;
	}
	// Underflow case [Smallest number]
	// Exponent cannot be smaller than 0 [with bias of +127]
	else if(exp < 0)
	{
		final &= 0x80000000;
		return final;
	}
	
	mant &= 0x007FFFFF;		// Remove implicit 1
	temp = exp;
	temp = temp << 23;		// Shift the exponent into the correct exponent field
	final |= temp;			// Insert exponent into final
	final |= mant;			// Insert mantissa into final
	
	return final;
}

// Floating point multiplication
uint32_t float_mult(uint32_t M, uint32_t R)
{
	// Save function arguments
	uint32_t a = M;
	uint32_t b = R;
	
	// Operand a
	uint8_t exp0 = a >> 23;							// Extract the exponent field of a
	uint32_t mant0 = (a & 0x007FFFFF) | 0x00800000;	// Extract the mantissa field of a
	
	// Operand b
	uint8_t exp1 = b >> 23;							// Extract the exponent field of b
	uint32_t mant1 = (b & 0x007FFFFF) | 0x00800000;	// Extract the mantissa field of b
	
	// Final result
	int16_t exp = 0;		// Final exponent
	uint32_t final = 0;	// Result to be returned
	
	// Booth Multiplier Variables
	uint8_t i = 0;
	uint64_t A = 0;
	uint64_t S = 0;
	uint64_t P = 0;
	
	// Compute the sign
	final |= (a & 0x80000000) ^ (b & 0x80000000);	// Xor the sign bits
	
	// Compute the initial exponent
	exp = (exp0 + exp1) - 127;						// Add the exponents and subtract 127 [rid the redundant bias]
	if(exp > 0x00FF)								// Early check for overflow
	return final |= (0x7FFFFFFF);
	
	// Multiply the mantissas using Booth's algorithm
	// m = mant0		x = 25 bits
	// r = mant1		y = 25 bits
	// A = S = P = x + y + 1 = 51 bits [USE LONG INT]
	
	// Fill register A with m
	A = mant0;
	A = A << 26;
	
	// Two's complement of m
	mant0 = ~mant0;
	mant0 += 1;
	
	// Fill register S with -m
	S = mant0;
	S = S << 26;
	S &= 0x0007FFFFFFFFFFFF;
	
	// Fill register P with r
	P = mant1 << 1;
	
	for(i = 0; i < 25; i++)
	{
		if((P & 3) == 1)
		{
			P += A;
		}
		else if((P & 3) == 2)
		{
			P += S;
		}
		P &= 0x0007FFFFFFFFFFFF;		// Mask out the overflow
		
		// Shift negative number
		if(P >= 0x0004000000000000)
		{
			P = P >> 1;
			P |= 0x0004000000000000;
		}
		// Shift positive number
		else
		P = P >> 1;
	}
	P = P >> 1;		// One last shift to complete the Booth algorithm
	
	// Normalize the result
	while(P > 0x00007FFFFFFFFFFF)
	{
		P = P >> 1;
		exp += 1;
	}
	while(P < 0x00003FFFFFFFFFFF && P > 0)
	{
		P = P << 1;
		exp -= 1;
	}

	// Truncate
	while(P >= 0x0000000001000000)
	P = P >> 1;
	
	// Overflow case [Largest value]
	// Exponent cannot be larger than 254 [with bias of +127]
	if(exp > 254)
	{
		final |= 0x7FFFFFFF;
		return final;
	}
	// Underflow case [Smallest number]
	// Exponent cannot be smaller than 0 [with bias of +127]
	else if(exp < 0)
	{
		final &= 0x80000000;
		return final;
	}
	
	mant0 = exp;
	mant0 = mant0 << 23;		// Extract the final exponent
	final |= mant0;				// Insert the exponent into final
	mant0 = P;
	mant0 &= 0x007FFFFF;		// Extract the final mantissa
	final |= mant0;				// Insert the mantissa into final
	return final;
}

uint16_t float_digital(uint32_t Num)
{
	uint32_t a = Num;
	
	uint16_t final = 0;
	int8_t temp = (a >> 23) - 127;
	uint64_t mant = (a & 0x007FFFFF) | 0x00800000;
	
	// Denormalize the floating point
	while(temp != 0)
	{
		if(temp > 0)
		{
			temp--;
			mant = mant << 1;
		}
		else if(temp < 0)
		{
			temp++;
			mant = mant >> 1;
		}
	}
	mant = mant << SCALE;		// Scale the value

	mant = mant >> 23;
	final |= mant & 0x0000000000000FFF;
	
	if((a & 0x80000000) == 0x80000000)
	final = 0x07FF - final;		// Negative
	else
	final = 0x07FF + final;		// Positive
	
	return final;
}

//-------------------------------------------------
//----------------I2C FUNCTIONS--------------------
//-------------------------------------------------

// Data Packet
// | STA | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 || ACK || 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 || ACK || D11 | D10 | D9 | D8 | D7 | D6 | D5 | D4 || ACK || D3 | D2 | D1 | D0 | 0 | 0 | 0 | 0 || ACK | STO |
//                        A2  A1  A0  R/W		  C2  C1  C0   x   x  PD1 PD0  x

// DAC function
void dac(unsigned int input)
{
	i2c_start();
	i2c_write(0b11000000);
	i2c_write(0b01000000);
	i2c_write(input >> 4);
	i2c_write(input);
	i2c_stop();
}

// I2C initialization
void i2c_init(void)
{
	TWSR = 0x00;										// Set prescalar to 0
	TWBR = 0x48;										// SCL = 50 KHz (Fosc = 8 MHz)
	TWCR = (1 << TWEN);									// Enable TWI
}

// Write to the I2C
void i2c_write(unsigned char data)
{
	TWDR = data;										// Data to be transmitted
	TWCR = (1 << TWINT) | (1 << TWEN);					// Use TWI module and write
	while((TWCR & (1 << TWINT)) == 0);					// Wait for TWI to complete
}

// I2C start condition
void i2c_start(void)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);	// Transmit START condition
	while((TWCR & (1 << TWINT)) == 0);					// Wait for TWI to complete
}

// I2C stop condition
void i2c_stop(void)
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);	// Transmit STOP condition
}
