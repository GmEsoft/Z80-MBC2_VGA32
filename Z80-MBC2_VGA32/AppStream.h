#pragma one

#include "WString.h"
#include "Stream.h"

class AppStreamImpl : public Stream
{
public:
    AppStreamImpl() : pStream_( 0 )
    {
    }
    
    void setStream( Stream &pStream )
    {
        pStream_ = &pStream;
    }

    virtual size_t write(uint8_t c)
    {
        return pStream_ ? pStream_->write( c ) : 0;
    }

    virtual int available()
    {
        return pStream_ ? pStream_->available() : 0;
    }

    virtual int read()
    {
        return pStream_ ? pStream_->read() : 0;
    }

    virtual int peek()
    {
        return pStream_ ? pStream_->peek() : 0;
    }
    
    virtual void flush()
    {
        if ( pStream_ )
            pStream_->flush();
    }

    String input();


private:
    Stream *pStream_;
};

extern AppStreamImpl AppStream;
