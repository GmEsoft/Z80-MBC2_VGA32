/*
    Z80-MBC2_VGA32 - System TTY Console
    

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

#include "Mode.h"
#include "System_I.h"


// Console decorator for system control

class SystemConsole :
	public Console_I
{
public:
	SystemConsole(void) : system_( 0 ), ch_( 0 ), console_( 0 )
	{
	}

	virtual ~SystemConsole(void)
	{
	}

	void setConsole( Console_I *console )
	{
		console_ = console;
	}

	void setSystem( System_I *system )
	{
		system_ = system;
	}

	// Console API implementation
	int kbhit();

	int getch();

	int ungetch( int ch );

	int putch( int ch );

	int puts( const char * str );
	
	char *gets( char * str );
	
	int vprintf( const char* format, va_list args );

private:
	int			ch_;
	Console_I	*console_;
	System_I	*system_;
};
