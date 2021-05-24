/*
    Z80-MBC2_VGA32 - Interface for File Systems
    

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

#include "File_I.h"
#include "Dir_I.h"
#include "FileDescriptor.h"

class FileSystem_I
{
public:

	FileSystem_I(void)
	{
	}

	virtual ~FileSystem_I(void)
	{
	}


	virtual File_I *open( const char *filename, const char *mode ) const = 0;

	virtual Dir_I *openDir( const char *dirname ) const = 0;

	virtual int open( const char *filename, int mode, int access ) const = 0;

	virtual int errno_() const = 0;

	virtual void set_errno_( int errno_ ) const = 0;

	virtual int close( int handle ) const = 0;

	virtual int read( int handle, void *buffer, unsigned length ) = 0;

	virtual int write( int handle, void *buffer, unsigned length ) = 0;

	virtual const char *strerror( int code ) const = 0;

	virtual int lseek( int handle, int offset, int ref ) = 0;

	virtual int getDisk() = 0;

	virtual const char* getDir( int disk ) = 0;

	virtual int chDir( const char *path ) = 0;
};
