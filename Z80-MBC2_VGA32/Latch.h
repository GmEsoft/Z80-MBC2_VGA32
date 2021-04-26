#pragma once

#include "Latch_I.h"

class Latch : public Latch_I
{
public:
	Latch() : latch_( 0xFF )
	{
	}

	// put char
	virtual uchar put( uchar data )
	{
		return latch_ = data;
	}

	// get char
	virtual uchar get()
	{
		return latch_;
	}

	// and char
	virtual uchar and_( uchar data )
	{
		return latch_ &= data;
	}

	// or char
	virtual uchar or_( uchar data )
	{
		return latch_ |= data;
	}

	// xor char
	virtual uchar xor_( uchar data )
	{
		return latch_ ^= data;
	}

	// not
	virtual uchar not_()
	{
		return latch_ ^= 0xFF;
	}

	// destructor
	virtual ~Latch()
	{
	}

private:	
	uchar latch_;
};
