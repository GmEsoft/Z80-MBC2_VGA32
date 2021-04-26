#pragma once

#include "Sleeper_I.h"

class ArduSleeper :
	public Sleeper_I
{
public:

	ArduSleeper(void)
	{
	}

	virtual ~ArduSleeper(void)
	{
	}

	void sleep( ulong millis )
	{
		delay( millis );
	}
};
