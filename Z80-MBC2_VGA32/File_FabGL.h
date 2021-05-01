/*
    Z80-MBC2_VGA32 - File_I implementation for FabFL File System.
    

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

#include "File_I.h"

#include <cstdio>

#include "stdio_undefs.h"

class File_FabGL :
    public File_I
{
public:
    File_FabGL( FILE *file );

    virtual ~File_FabGL(void);

    virtual int close();

    virtual bool eof() const;

    virtual int getc();

    virtual int putc( int c );

    virtual int seek( long offset, int origin );

    virtual int scanf( const char *format, ... );

    virtual int printf( const char *format, ... );

    virtual char* gets( char *buffer, int len );

    virtual int puts( const char *str );

    virtual const char *error() const;

    virtual operator bool() const;

private:
    FILE        *file_;
    const char  *strerror_;
};
