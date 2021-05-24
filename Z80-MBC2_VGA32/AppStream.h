/*
    Z80-MBC2_VGA32 - Stream decorator with line input
    

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
