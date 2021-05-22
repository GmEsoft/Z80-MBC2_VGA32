/*
    Z80-MBC2_VGA32 - Stream Splitter: allows to send to and receive from
    2 distinct streams at the same time.
    

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

#include <Stream.h>

#include "HardwareSerial.h"

class DualStream : public Stream
{
public:
	DualStream( Stream &stream1, Stream &stream2 )
	: stream1_( stream1 ), stream2_( stream2 )
	{}

	virtual ~DualStream()
	{}

    virtual int available()
    {
        int ret1 = stream1_.available();
        //Serial.printf("available1=%d\n",ret1);
		int ret2 = stream2_.available();
        //Serial.printf("available1=%d\n",ret2);
        return ret1 + ret2;
	}

    virtual int read()
    {
        int c=-1;
        while ( c < 0 )
        {
            if ( stream1_.available() )
                c = stream1_.read();
            else if ( stream2_.available() )
                c = stream2_.read();
            else
                delay( 50 );
        }
		return c;
	}

    virtual int peek()
    {
		int c = stream1_.peek();
		if ( c < 0 )
			c = stream2_.peek();
		return c;
	}

    virtual void flush()
   	{
		stream1_.flush();
		stream2_.flush();
	}

    virtual size_t write(uint8_t c)
    {
		size_t n1 = stream1_.write( c );
		size_t n2 = stream2_.write( c );
		return n1 > n2 ? n1 : n2;
	}

private:
	Stream &stream1_;
	Stream &stream2_;
};
