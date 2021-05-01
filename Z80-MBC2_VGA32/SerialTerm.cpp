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

#include "SerialTerm.h"

#include "DualStream.h"

#include "HardwareSerial.h"

extern Stream &subStream;

static const char *pinput = "";
static int curx = 0;
static int cury = 0;
static int attr = 0x0F;

void SerialTerm::begin()
{
    subStream.println( "Serial Terminal ready" );
}

void SerialTerm::end()
{
    subStream.println( "Serial Terminal closed" );
}

int SerialTerm::input( const char *input )
{
    pinput = input;
    return 0;
}

static int bufch = 0;
static int timer = 0;
static int reload = 5000;

int SerialTerm::kbhit()
{
    if ( bufch )
        return bufch;

    int c = *pinput;

    if ( c )
    {
        ++pinput;
    }
    else if ( timer <= 0 )
    {
        if ( subStream.available() )
        {
            c = subStream.read();
            timer = 1;
        }
        else
        {
            timer = reload;
        }
    }

    --timer;

    if ( c )
    {
        //printf( "[%02X]", c );
        if ( c == 0x7F ) // DEL -> BS
            c = '\b';
    }

    bufch = c;
    return c;
}

int SerialTerm::getch()
{
    int c;
    if ( bufch )
    {
        c = bufch;
    }
    else
    {
        timer = 0;
        while ( !(c = kbhit() ) )
        {
            delay(10);
            timer = 0;
        }
    }
    bufch = 0;
    //putch( c );
    return c;
}

int SerialTerm::ungetch( int ch )
{
    bufch = ch;
    return 0;
}

int SerialTerm::putch( int ch )
{
    return subStream.print( (char)ch );
}

int SerialTerm::puts( const char *str )
{
    //Serial.println("int SerialTerm::puts( const char *str )");
    LOGD( "str=%p", str );
    LOGD( "str='%s'", str );
    LOGD( "after Terminal" );
    return subStream.print( str );
}

char *SerialTerm::gets( char *str )
{
    int len = (int)(*str);
    int p = 0;
    int c = getch();

    while ( c != '\r' )
    {
        if ( c >= ' ' && p < len )
        {
            str[2+p] = (char)c;
            ++p;
            putch( c );
        }
        else if ( ( c == 8 || c == 0x7F ) && p > 0 )
        {
            --p;
            puts( "\x08\x20\x08" );
        }
        else
        {
            printf( "[%04X]\x08\x08\x08\x08\x08\x08", c );
        }
        c = getch();
    }

    putch( c );
    str[2+p] = 0;
    str[1] = p;

    return str + 2;
}

int SerialTerm::vprintf( const char *format, va_list args )
{
    char buffer[1000];
    //vsprintf_s( buffer, sizeof( buffer ), format, args );
    vsprintf( buffer, format, args );
    int ret = subStream.print( buffer );
    return ret;
}

void SerialTerm::setScreenSize( int x, int y )
{
}

void SerialTerm::setFullScreen( bool fs )
{
}

int SerialTerm::textAttr( const int c )
{
    return 0;
}

int SerialTerm::curAttr()
{
    return 0;
}

int SerialTerm::clrScr()
{
    return 0;
}

int SerialTerm::clrEos()//new
{
	return 0;
}

int SerialTerm::clrEol()
{
    return 0;
}

int SerialTerm::gotoXY( int x, int y )
{
    curx = x;
    cury = y;
    return 0;
}

int SerialTerm::whereX()
{
    return curx;
}

int SerialTerm::whereY()
{
    return cury;
}

int SerialTerm::getTextInfo( text_info* ti )
{
    return 0;
}

int SerialTerm::getText( int left, int top, int right, int bottom, char* buffer )
{
    return 0;
}

int SerialTerm::putText( int left, int top, int right, int bottom, const char* buffer )
{
    return 0;
}

int SerialTerm::setCursorType( int curstype )
{
    return 0;
}

int SerialTerm::setGraphViewport( int x0, int y0, int x1, int y1 )
{
    return 0;
}

void SerialTerm::setGraphOverlay( int p_GraphOverlay )
{
}

int SerialTerm::selectFontBank( unsigned nPage, unsigned nBank )
{
    return 0;
}

int SerialTerm::scanKbd( int row )
{
    return 0;
}

int SerialTerm::setKbdScanMode( int mode )
{
    return 0;
}

void SerialTerm::setWideMode( bool wide )
{
}
