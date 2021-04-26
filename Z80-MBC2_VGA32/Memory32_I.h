#pragma once

// MEMORY API

#include "runtime.h"

class Memory32_I
{
public:
	// write char
	virtual uchar write( uint addr, uchar data ) = 0;

	// read char
	virtual uchar read( uint addr ) = 0;

	// destructor
	virtual ~Memory32_I()
	{
	}
};
