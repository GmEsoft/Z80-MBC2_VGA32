#pragma once

#include "CPU.h"

class CPUExtender
{
public:
	CPUExtender( CPUExtender *cpuex )
		: cpuex_( cpuex )
	{
	}

	virtual ~CPUExtender(void)
	{
	}

	int sim( ushort opcode )
	{
		int ret = simex( opcode );
		if ( !ret && cpuex_ )
		{
			ret = cpuex_->sim( opcode );
		}
		return ret;
	}

protected:
	virtual int simex( ushort opcode ) = 0;

private:
	CPUExtender	*cpuex_;
};
