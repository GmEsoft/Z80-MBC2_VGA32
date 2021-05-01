/*
    Z80-MBC2_VGA32 - Term_I adaptor for "Serial" stream
    

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

#include "Term_I.h"

class SerialTerm : public Term_I
{
public:
    SerialTerm()
    {
    }

    virtual ~SerialTerm()
    {
        LOGW( "Unexpected?" );
    }

    static void begin();

    static void end();

    int input( const char *input );

    virtual int kbhit();

    virtual int getch();

    virtual int ungetch( int ch );

    virtual int putch( int ch );

    virtual int puts( const char *str );

    virtual char *gets( char *str );

    virtual int vprintf( const char *format, va_list args );

    virtual void setScreenSize( int x, int y );

    virtual void setFullScreen( bool fs );

    virtual int textAttr( const int c );

    virtual int curAttr();

    virtual int clrScr();

	virtual int clrEos();//new

    virtual int clrEol();

    virtual int gotoXY( int x, int y );

    virtual int whereX();

    virtual int whereY();

    virtual int getTextInfo( text_info* ti );

    virtual int getText( int left, int top, int right, int bottom, char* buffer );

    virtual int putText( int left, int top, int right, int bottom, const char* buffer );

    virtual int setCursorType( int curstype );

    virtual int setGraphViewport( int x0, int y0, int x1, int y1 );

    virtual void setGraphOverlay( int p_GraphOverlay );

    virtual int selectFontBank( unsigned nPage, unsigned nBank );

    virtual int scanKbd( int row );

    virtual int setKbdScanMode( int mode );

    virtual void setWideMode( bool wide );
};
