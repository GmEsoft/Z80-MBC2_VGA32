#pragma once

#include "runtime.h"

class Latch_I
{
public:
	// put char
	virtual uchar put( uchar data ) = 0;

	// get char
	virtual uchar get() = 0;

	// and char
	virtual uchar and_( uchar data ) = 0;

	// or char
	virtual uchar or_( uchar data ) = 0;

	// xor char
	virtual uchar xor_( uchar data ) = 0;

	// not
	virtual uchar not_() = 0;

	// destructor
	virtual ~Latch_I()
	{
	}
};
