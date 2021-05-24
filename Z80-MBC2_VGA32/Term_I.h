/*
    Z80-MBC2_VGA32 - Interface for Screen-Oriented Terminals
    

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

#include "Console_I.h"

#define _NOCURSOR 0
#define _NORMALCURSOR 1

struct text_info 
{
	int winleft;
	int wintop;
	int winright;
	int winbottom;
	int curx;
	int cury;
};


class Term_I :
	public Console_I
{
public:
	Term_I(void)
	{
	}
	
	virtual ~Term_I(void)
	{
        LOGW( "Unexpected?" );
	}

	virtual void setScreenSize( int x, int y ) = 0;

	virtual void setFullScreen( bool fs ) = 0;
	
	virtual int textAttr( const int c ) = 0;
	
	virtual int curAttr() = 0;
	
	virtual int clrScr() = 0;
	
	virtual int clrEos() = 0;
	
	virtual int clrEol() = 0;
	
	virtual int gotoXY( int x, int y ) = 0;
	
	virtual int whereX() = 0;
	
	virtual int whereY() = 0;

	virtual int getTextInfo( text_info* ti ) = 0;

	virtual int getText( int left, int top, int right, int bottom, char* buffer ) = 0;

	virtual int putText( int left, int top, int right, int bottom, const char* buffer ) = 0;
	
	virtual int setCursorType( int curstype ) = 0;

	virtual int setGraphViewport( int x0, int y0, int x1, int y1 ) = 0;

	virtual void setGraphOverlay( int p_GraphOverlay ) = 0;

	virtual int selectFontBank( unsigned nPage, unsigned nBank ) = 0;

	virtual int scanKbd( int row ) = 0;

	virtual int setKbdScanMode( int mode ) = 0;

	virtual void setWideMode( bool fs ) = 0;
	
};
