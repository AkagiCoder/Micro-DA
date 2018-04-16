#include <avr/io.h>
#include <stdint.h>

volatile uint32_t num0 = 0x41820000;	// 16.25
volatile uint32_t num1 = 0x42360000;	// 45.5
volatile uint32_t num2 = 0x00000000;

uint32_t float_add(volatile uint32_t a, volatile uint32_t b);
uint32_t float_mult(volatile uint32_t a, volatile uint32_t b);

int main(void)
{
    float_mult(num0, num1);
    while (1) 
    {
    }
}

uint32_t float_add(volatile uint32_t a, volatile uint32_t b)
{
	// Operand a
	volatile uint8_t exp0 = a >> 23;							// Extract the exponent field of a
	volatile uint32_t mant0 = (a & 0x007FFFFF) | 0x00800000;	// Extract the mantissa field of a
	
	// Operand b
	volatile uint8_t exp1 = b >> 23;							// Extract the exponent field of b
	volatile uint32_t mant1 = (b & 0x007FFFFF) | 0x00800000;	// Extract the mantissa field of b
	
	// Final result
	volatile int16_t exp;		// Final exponent
	volatile uint32_t final;	// Result to be returned
	volatile uint32_t mant;		// Final mantissa
	
	// Temporary variables
	volatile uint32_t temp;
	
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
		while(mant < 0x00800000)
		{
			mant = mant << 1;
			exp -= 1;
		}
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
	final |= temp;			// Insert exponent into the final
	final |= mant;			// Insert mantissa into the final
	
	return final;
}

uint32_t float_mult(volatile uint32_t a, volatile uint32_t b)
{
	// Operand a
	volatile uint8_t exp0 = a >> 23;							// Extract the exponent field of a
	volatile uint32_t mant0 = (a & 0x007FFFFF) | 0x00800000;	// Extract the mantissa field of a
	
	// Operand b
	volatile uint8_t exp1 = b >> 23;							// Extract the exponent field of b
	volatile uint32_t mant1 = (b & 0x007FFFFF) | 0x00800000;	// Extract the mantissa field of b
	
	// Final result
	volatile uint16_t exp;		// Final exponent
	volatile uint32_t final;	// Result to be returned
	
	// Booth Multiplier Variables
	volatile uint8_t i;
	volatile uint64_t A;
	volatile uint64_t S;
	volatile uint64_t P;
	
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
	while(P >= 0x0000000001000000)
	{
		P = P >> 1;
		//exp += 1;
	}
	return final;
}