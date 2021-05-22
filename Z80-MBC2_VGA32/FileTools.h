/*
    Z80-MBC2_VGA32 - File Tools
    

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

#include <cstring>
#include <string>

class FileTools
{
public:

	FileTools(void)
	{
	}

	~FileTools(void)
	{
	}

	static void addDefaultExt( char *name, const char *ext )
	{
		if	(	strrchr( name, '.' ) == 0
			||	strrchr( name, '.' ) < strrchr( name, '\\' )
			)
		{
			strcat( name, ext );
		}
	}

	static void addDefaultExt( std::string &name, const char *ext )
	{
		const char *cname = name.c_str();

		if	(	strrchr( cname, '.' ) == 0
			||	strrchr( cname, '.' ) < strrchr( cname, '\\' )
			)
		{
			name += ext;
		}
	}

};
