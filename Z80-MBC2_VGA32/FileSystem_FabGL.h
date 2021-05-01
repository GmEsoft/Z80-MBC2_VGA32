/*
    Z80-MBC2_VGA32 - FileSystem_I implementation for FabGL
    

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

#include "FileSystem_I.h"

#include "fabutils.h"

#include "stdio_undefs.h"

class FileSystem_FabGL :
    public FileSystem_I
{
public:
    FileSystem_FabGL();

    FileSystem_FabGL( fabgl::FileBrowser &fs );

    virtual ~FileSystem_FabGL( void );

    virtual File_I *open( const char *filename, const char *mode ) const;

    virtual Dir_I *openDir( const char *direname ) const;

    virtual int open( const char *filename, int mode, int access ) const;

    virtual int errno_() const;

    virtual void set_errno_( int errno_ ) const;

    virtual int close( int handle ) const;

    virtual int read( int handle, void *buffer, unsigned length );

    virtual int write( int handle, void *buffer, unsigned length );

    virtual const char *strerror( int code ) const;

    virtual int lseek( int handle, int offset, int origin );

    virtual int getDisk();

    virtual const char* getDir( int disk );

    virtual int chDir( const char *path );

private:
    friend class Dir_FabGL;
    fabgl::FileBrowser &fs_;
    mutable int errno__;
    mutable const char  *strerror_;
};
