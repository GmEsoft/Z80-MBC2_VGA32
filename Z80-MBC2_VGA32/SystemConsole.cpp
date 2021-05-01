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
    
    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "SystemConsole.h"

#include "runtime.h"

int SystemConsole::kbhit()
{
	if ( console_ && !console_->kbhit() )
		return ch_;

	ch_ = 0;
	ch_ = getch();
	return ch_;
}

int SystemConsole::getch()
{
	int ret = ch_;
	ch_ = 0;
	if ( ret )
	{
		return ret;
	}
	
	if ( console_ )
	{
		ret = console_->getch();
		if ( ret == -68 ) // F10 == stop
		{
			if ( system_ )
				system_->stop();				
		}
		else if ( ret == -93 ) // Sh-F10 = exit
		{
			if ( system_ )
				system_->exit();				
		}
		else if ( ret == -84 ) // Sh-F1 = reset
		{
			if ( system_ )
				system_->reset();				
		}
		else
			return ret;
	}
	return 0;
}

int SystemConsole::ungetch( int ch )
{
	return ch_ = ch;
}

int SystemConsole::putch( int ch )
{
	if ( console_ )
		return console_->putch( ch );
	return ch;
}

int SystemConsole::puts( const char *str )
{
	if ( console_ )
		return console_->puts( str );;
	return 0;
}

char *SystemConsole::gets( char *str )
{
	if ( console_ )
		return console_->gets( str );;
	return 0;
}

int SystemConsole::vprintf( const char *str, va_list args )
{
	if ( console_ )
		return console_->vprintf( str, args );
	return 0;
}
