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
