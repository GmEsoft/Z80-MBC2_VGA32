/*
    Z80-MBC2_VGA32 - Proxy to File System, for decorators.
    

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

#include "FileSystem_I.h"

class FileSystemProxy :
	public FileSystem_I
{
public:

	FileSystemProxy(void)
		: fileSystem_( 0 )
	{
	}

	virtual ~FileSystemProxy(void)
	{
	}

	void setFileSystem( FileSystem_I *fileSystem )
	{
		fileSystem_ = fileSystem;
	}

    virtual File_I *open( const char *filename, const char *mode ) const
    {
        return fileSystem_ ? fileSystem_->open( filename, mode ) : 0;
    }

    virtual Dir_I *openDir( const char *dirname ) const
    {
        return fileSystem_ ? fileSystem_->openDir( dirname ) : 0;
    }

	virtual int open( const char *filename, int mode, int access ) const
	{
		return fileSystem_ ? fileSystem_->open( filename, mode, access ) : -1;
	}

	virtual int errno_() const
	{
		return fileSystem_ ? fileSystem_->errno_() : -1;
	}

	virtual void set_errno_( int errno_ ) const
	{
		if ( fileSystem_ ) 
		    fileSystem_->set_errno_( errno_ );
	}

	virtual int close( int handle ) const
	{
		return fileSystem_ ? fileSystem_->close( handle ) : -1;
	}

	virtual int read( int handle, void *buffer, unsigned length )
	{
		return fileSystem_ ? fileSystem_->read( handle, buffer, length ) : -1;
	}

	virtual int write( int handle, void *buffer, unsigned length )
	{
		return fileSystem_ ? fileSystem_->write( handle, buffer, length ) : -1;
	}

	virtual const char *strerror( int code ) const
	{
		return fileSystem_ ? fileSystem_->strerror( code ) : "no filesystem";
	}

	virtual int lseek( int handle, int offset, int origin )
	{
		return fileSystem_ ? fileSystem_->lseek( handle, offset, origin ) : -1;
	}

	virtual int getDisk()
	{
		return fileSystem_ ? fileSystem_->getDisk() : -1;
	}

	virtual const char* getDir( int disk )
	{
		return fileSystem_ ? fileSystem_->getDir( disk ) : 0;
	}

	virtual int chDir( const char *path )
	{
		return fileSystem_ ? fileSystem_->chDir( path ) : -1;
	}

protected:
	FileSystem_I *fileSystem_;

};
