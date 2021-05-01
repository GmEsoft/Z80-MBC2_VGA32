/*
    Z80-MBC2_VGA32 - Interface for Files
    

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

#define FS_NO_GLOBALS 1

class File_I
{
public:

	File_I(void)
	{
	}

	virtual ~File_I(void)
	{
	}

	virtual int close() = 0;

	virtual bool eof() const = 0;

	virtual int getc() = 0;

	virtual int putc( int c ) = 0;

	virtual int seek( long offset, int origin ) = 0;

	virtual int scanf( const char *format, ... ) = 0;

	virtual int printf( const char *format, ... ) = 0;

	virtual char* gets( char *buffer, int len ) = 0;

	virtual int puts( const char *str ) = 0;

	virtual const char *error() const = 0;

	virtual operator bool() const = 0;

};
