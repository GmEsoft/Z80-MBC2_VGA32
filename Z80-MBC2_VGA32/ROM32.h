#pragma once

// ROM ACCESSOR

#include "Memory32_I.h"

class ROM32 : public Memory32_I
{
public:
	ROM32()
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
