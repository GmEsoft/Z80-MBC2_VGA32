/*
    Z80-MBC2_VGA32 - Directory browser for FabGL filesystem.
    

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

#include "Dir_I.h"

#include "FileSystem_FabGL.h"

#include "fabutils.h"

#include "stdio_undefs.h"

class Dir_FabGL :
    public Dir_I
{
public:
    Dir_FabGL( const FileSystem_FabGL &fileSystem ) 
    : fileSystem_( fileSystem ), count_( fileSystem.fs_.count() ), index_( 1 ) // 0 = is always parent directory
    {        
    }

    virtual DirEntry next();

    virtual int close()
    {
        return 0;
    }

    virtual const char *error() const
    {
        return "";
    }

    virtual operator bool() const
    {
        return true;
    }
private:
    const FileSystem_FabGL &fileSystem_;
    int count_;
    int index_;
};
