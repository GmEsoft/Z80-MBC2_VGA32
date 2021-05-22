/*
    Z80-MBC2_VGA32 - CPU Clock throttler
    

    Created by Michel Bernard (michel_bernard@hotmail.com) 
    - <http://www.github.com/GmEsoft/Z80-MBC2_VGA32>
    Copyright (c) 2021 Michel Bernard.
    All rights reserved.
    

    This file is part of Z80-MBC2_VGA32.
    
    Z80-MBC2_VGA32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    Z80-MBC2_VGA32 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Z80-MBC2_VGA32.  If not, see <https://www.gnu.org/licenses/>.

*/

#pragma once

#include "Clock_I.h"

#include "Sleeper_I.h"
#include "IRQ_I.h"
#include "runtime.h"

class SystemClock :
	public Clock_I
{
public:
	SystemClock(void) : 
		clockSpeed_( 0 ), clock100_( -1L ), rtcRate_( 30 ), rtcTest_( 0 ), 
		throttleFreq_( 100 ), tstates_( 0 ), sleeper_( 0 ), irq_( 0 )
	{
	}

	virtual ~SystemClock(void)
	{
	}

	// do CPU cycles
	virtual void runCycles( long cycles );

	// Set clock speed (kHz)
	virtual void setClockSpeed( long speed )
	{
		clockSpeed_ = speed;
		tstates_ = 0;
	}

	// Set RTC rate (kHz)
	virtual void setRtcRate( long rate )
	{
		rtcRate_ = rate;
	}

	// Set sleeper
	void setSleeper( Sleeper_I *sleeper )
	{
		sleeper_ = sleeper;
	}

	// Set IRQ handler
	void setIRQ( IRQ_I *irq )
	{
		irq_ = irq;
	}

	void sleep( ulong millis )
	{
		if ( sleeper_ )
			sleeper_->sleep( millis );
	}

	void trigIrq()
	{
		if ( irq_ )
			irq_->trigger();
	}

	
protected:
	long			clockSpeed_;		// Clock speed in kHz
	long			clock100_;			// Clock as n/100 sec from midnight
	long			rtcRate_;			// RTC rate in Hz
	long			rtcTest_;			// 
	long			throttleFreq_;		//
	long			tstates_;			// cycles to consume
	Sleeper_I		*sleeper_;			// sleep( millis )
	IRQ_I			*irq_;				// IRQ handler
};
