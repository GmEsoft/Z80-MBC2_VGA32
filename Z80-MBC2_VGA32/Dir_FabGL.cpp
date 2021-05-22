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
    
    Z80-MBC2_VGA32 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Z80-MBC2_VGA32.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "Dir_FabGL.h"

#include "log.h"

#include <string.h>


DirEntry Dir_FabGL::next()
{
    DirEntry entry;
    if ( index_ < count_ )
    {
        auto *item = fileSystem_.fs_.get( index_++ );
        entry.name = item->name;
        entry.size = fileSystem_.fs_.fileSize( item->name );
        entry.isDir = item->isDir;
    }
    else
    {
        fileSystem_.set_errno_( 0x12 );    // end of directory
    }
    //LOGD( "DirEntry: %s %d %d", entry.name ? entry.name : "<null>", entry.size, entry.isDir );
    return entry;
}
