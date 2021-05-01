/*
    Z80-MBC2_VGA32 - Simple Memory Allocator
      

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

#include "log.h"

#include "HardwareSerial.h"

class Allocator
{
private:

    enum Status
    {
        BOOKED = 0xA10CA7ED, // Allocated
        FREE   = 0xF2EEF00D  // Free food
    };
    
    struct Header
    {
        Header  *prev_,
                *next_;
        size_t  size_;
        int     status_;
        //char    ptr_[0];
    };

public:

    Allocator( void *pool, size_t size )
    : pOrigin_( (Header*)pool ), initSize_( size )
    {
    }

    void *malloc( size_t size )
    {
        if ( initSize_ )
        {
            pOrigin_->prev_ = pOrigin_->next_ = pOrigin_;
            pOrigin_->size_ = initSize_ - sizeof( Header );
            pOrigin_->status_ = FREE;
            LOGD( "New allocator for size 0x%X at %p", pOrigin_->size_ , pOrigin_ );
            debug( "Origin", pOrigin_ );
            initSize_ = 0;
        }

        LOGD( "Allocating 0x%x bytes from pool %p", size, pOrigin_ );

        Header *ptr = pOrigin_;
        do {
            ptr = ptr->prev_;

            debug( "Candidate", ptr );

            if ( ptr->status_ == FREE ) {
                debug( "Free", ptr );
                if( ptr->size_ >= size + sizeof( Header ) ) {
                    debug( "Taking", ptr );
                    Header *pNewFree = (Header*)( (char*)ptr + size + sizeof( Header ) );
                    pNewFree->prev_ = ptr;
                    pNewFree->next_ = ptr->next_;
                    pNewFree->size_ = ptr->size_ - size - sizeof( Header );
                    pNewFree->status_ = FREE;
                    ptr->next_ = pNewFree;
                    ptr->size_ = size;
                    ptr->status_ = BOOKED;
                    pOrigin_->prev_ = pNewFree;
                    debug( "New", ptr );
                    debug( "Prev", ptr->prev_ );
                    debug( "Next", ptr->next_ );
                    check();
                    LOGD( "Returning %p", ptr + 1 );
                    return (void*)( ptr + 1 );
                }
            } else if ( ptr->status_ != BOOKED ) {
                LOGE( "ABEND! Memory Pool Corrupt at %p", ptr );
                Serial.printf( "ABEND! Memory Pool Corrupt at %p", ptr );
                for (;;);
            }
        } while( ptr != pOrigin_ );

        return 0;  // malloc failed
    }

    void free( void *pFree )
    {
        Header *ptr = (Header*)pFree - 1;

        if ( ptr->status_ == FREE ) {
            LOGE( "ABEND! Already free pointer: %p", pFree );
            Serial.printf( "ABEND! Already free pointer: %p", pFree );
            for (;;);
        } else if ( ptr->status_ != BOOKED ) {
            LOGE( "ABEND! Memory Pool Corrupt at %p", ptr );
            Serial.printf( "ABEND! Memory Pool Corrupt at %p", ptr );
            for (;;);
        }
        
        ptr->status_ = FREE;
        debug( "free", ptr );

        // merge with next block if free
        if ( ptr->next_ != pOrigin_ && ptr->next_->status_ == FREE ) {
            LOGD( "Merging with next, deleting next" );
            debug( "Delete", ptr->next_ );
            ptr->next_ = ptr->next_->next_;
            ptr->next_->prev_ = ptr;
            ptr->size_ += ptr->next_->size_ + sizeof( Header );
            debug( "Prev", ptr );
            debug( "Next", ptr->next_ );
        }

        // merge with previous block if free
        if ( ptr != pOrigin_ && ptr->prev_->status_ == FREE ) {
            LOGD( "Merging with next, deleting this" );
            debug( "Delete", ptr );
            ptr->prev_->next_ = ptr->next_;
            ptr->next_->prev_ = ptr->prev_;
            ptr->prev_->size_ += ptr->size_ + sizeof( Header );
            debug( "Prev", ptr->prev_ );
            debug( "Next", ptr->next_ );
        }

        check();
    }

    void debug( const char *name, const Header *header )
    {
        LOGD( "%-10s: %p prev=%p next=%p size=0x%x status=%x", name, header, header->prev_, header->next_, header->size_, header->status_ );
    }

    void check()
    {
        LOGD( "Checking block chain" );
        Header *ptr = pOrigin_;
        int i = 0;
        do {
            String block( "Block " );
            block += i;
            debug( block.c_str(), ptr );
            if ( ptr->next_->prev_ != ptr )
                LOGE( "Error: ptr->next_->prev_ (=0x%x) != ptr (=0x%x)", ptr->next_->prev_, ptr );
            if ( ptr->prev_->next_ != ptr )
                LOGE( "Error: ptr->prev_->next_ (=0x%x) != ptr (=0x%x)", ptr->prev_->next_, ptr );
            if ( ptr < ptr->next_ && ptr->status_ == FREE && ptr->next_->status_ == FREE )
                LOGE( "Error: unmerged free blocks %p & %p", ptr, ptr->next_ );
            ptr = ptr->next_;
            ++i;
        } while ( ptr != pOrigin_ );
    }

private:
    Header  *pOrigin_;
    size_t  initSize_;
};
