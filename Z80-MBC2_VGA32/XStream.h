/*
    Z80-MBC2_VGA32 - Thread-safe Null-modem for 2 streams
    

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

//#include "Arduino.h"
//#undef word // from Arduino.h:167: #define word(...) makeWord(__VA_ARGS__)


class XStreams
{
    struct Buffer
    {
        Buffer() : i( 0 ) {}

        int available()
        {
            return i;
        }

        int availableForWrite()
        {
            return 1 - i;
        }

        int read()
        {
            while ( !i ) {}            
            int ret = x;
            i = 0;
            return ret;
        }

        int peek()
        {
            return i ? x : -1;
        }
    
        size_t write(uint8_t c)
        {
            while ( i ) {}
            x = c;
            i = 1;
            return 1;            
        }
    private:        
        volatile int x, i;
    };
    
    struct XStream : public Stream
    {
        XStream( Buffer &rx, Buffer &tx )
        : rx_( rx ), tx_( tx )
        {
        }
        
        virtual int available()
        {
            return rx_.available();
        }
    
        virtual int read()
        {
            return rx_.read();
        }
    
        virtual int peek()
        {
            return rx_.peek();
        }
    
        virtual int availableForWrite() override
        {
            return tx_.availableForWrite();
        }
    
        virtual size_t write(uint8_t c)
        {
            return tx_.write( c );      
        }

        virtual void flush()
        {
        }
    
    private:
        Buffer &rx_;
        Buffer &tx_;
    };
public:
	XStreams()
	: stream1_( buffer1_, buffer2_ ), stream2_( buffer2_, buffer1_ )
	{}

	virtual ~XStreams()
	{}

    XStream &stream1()
    {
        return stream1_;
    }

    XStream &stream2()
    {
        return stream2_;
    }

private:
	XStream stream1_;
	XStream stream2_;
    Buffer  buffer1_;
    Buffer  buffer2_;
};
