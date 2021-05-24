/*
    Z80-MBC2_VGA32 - Interface for Emulated Systems
    

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

// SYSTEM API

class System_I
{
public:
	virtual void run() = 0;
	virtual void stop() = 0;
	virtual void exit() = 0;
	virtual void reset() = 0;
	virtual void wakeup() = 0;
	virtual void step() = 0;
	virtual void callstep() = 0;

	// destructor
	virtual ~System_I()
	{
	}
};
