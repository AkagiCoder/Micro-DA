#include <avr/io.h>
#include <stdint.h>

volatile uint32_t num0 = 0x41820000;	// 16.25
volatile uint32_t num1 = 0xC2360000;	// 45.5
volatile uint32_t num2 = 0x00000000;

uint32_t float_add(volatile uint32_t a, volatile uint32_t b);

int main(void)
{
    float_add(num1, num0);
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