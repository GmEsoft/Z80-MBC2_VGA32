/*
    Z80-MBC2_VGA32 - Directory browser interface
    

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

#include "DirEntry.h"

#define FS_NO_GLOBALS 1

class Dir_I
{
public:

	Dir_I(void)
	{
	}

	virtual ~Dir_I(void)
	{
	}

	virtual DirEntry next() = 0;

	virtual int close() = 0;

	// Deprecated ?
	virtual const char *error() const = 0;

	virtual operator bool() const = 0;

};
