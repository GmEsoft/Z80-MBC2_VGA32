/*
    Z80-MBC2_VGA32 - 32-bit Ramdom Access Memory
    

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

// RAM ACCESSOR

#include "Memory32_I.h"

class RAM32 : public Memory32_I
{
public:
	RAM32()
		: buffer_( 0 ), size_( 0 )
	{
	}

	void setBuffer( uchar *_buffer, uint _size )
	{
		buffer_= _buffer; 
		size_ = _size;
	}

	uchar *getBuffer()
	{
		return buffer_;
	}
	
	uint getSize()
	{
		return size_;
	}

	// write char
	virtual uchar write( uint addr, uchar data )
	{
		if ( !size_ || addr < size_ )
			buffer_[addr] = data;
		return data;
	}

	// read char
	virtual uchar read( uint addr )
	{
		if ( !size_ || addr < size_ )
			return buffer_[addr];
		return 0;
	}

private:
	uchar *buffer_;
	uint size_;
};
