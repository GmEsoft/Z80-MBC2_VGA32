/*
    Z80-MBC2_VGA32 - 16-bit Ramdom Access Memory
    

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

// RAM ACCESSOR

#define USE_RAM_ACCESSORS 1

#include "Memory_I.h"

#include "log.h"

#if USE_RAM_ACCESSORS
static uchar RAM_read( void *object, ushort addr );
static uchar RAM_write( void *object, ushort addr, uchar data );
#endif

class RAM : public Memory_I
{
public:
	RAM()
		: buffer_( 0 ), size_( 0 )
	{
	}

    // get writer
    virtual writer_t getWriter()
    {
#if USE_RAM_ACCESSORS
        LOGD( "getting writer" );        
		return RAM_write;
#else
		return 0;
#endif
	}

    // get reader
    virtual reader_t getReader()
    {
#if USE_RAM_ACCESSORS
        LOGD( "getting reader" );        
        return RAM_read;
#else
		return 0;
#endif
    }

    // get object
    virtual void* getObject()
    {
#if USE_RAM_ACCESSORS
        LOGD( "getting memory object" );        
        return this;
#else
		return 0;
#endif
    }
    
	void setBuffer( uchar *_buffer, ushort _size )
	{
        LOGD( "setting buffer" );        
		buffer_= _buffer; 
		size_ = _size;
	}

	uchar *getBuffer()
	{
		return buffer_;
	}
	
	ushort getSize()
	{
		return size_;
	}

	// write char
	virtual uchar write( ushort addr, uchar data )
	{
		if ( !size_ || addr < size_ )
			buffer_[addr] = data;
		return data;
	}

	// read char
	virtual uchar read( ushort addr )
	{
		if ( !size_ || addr < size_ )
			return buffer_[addr];
		return 0;
	}

#if USE_RAM_ACCESSORS
	friend uchar RAM_read(void*, ushort);
	friend uchar RAM_write(void*, ushort, uchar);
#endif

private:
	uchar *buffer_;
	ushort size_;
};

#if USE_RAM_ACCESSORS
static uchar RAM_read( void *object, ushort addr )
{
	RAM &ram = *(RAM*)object;
	if ( !ram.size_ || addr < ram.size_ )
		return ram.buffer_[addr];
	return 0;
}

static uchar RAM_write( void *object, ushort addr, uchar data )
{
	RAM &ram = *(RAM*)object;
	if ( !ram.size_ || addr < ram.size_ )
		ram.buffer_[addr] = data;
	return data;
}
#endif
