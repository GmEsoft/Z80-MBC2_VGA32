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
