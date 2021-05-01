/*
    Z80-MBC2_VGA32 - Minimal implementation for Single-byte latch
    

    Created by Michel Bernard (michel_bernard@hotmail.com) 
    - <http://www.github.com/GmEsoft/Z80-MBC2_VGA32>
    Copyright (c) 2021 Michel Bernard.
    All rights reserved.
    

    This file is part of Z80-MBC2_VGA32.
    
    Z80-MBC2_VGA32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.

*/

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
