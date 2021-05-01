/*
    Z80-MBC2_VGA32 - Raster Display Driver
    for TRS-80 Model 4 16x64 or 24x80 text modes
    with support for enhanced color modes.
      

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

#include "fabgl.h"

#include "WString.h"

class AppDisplay
{
public:
    static void setup();

    static bool configure();

    static String getDisplayMode( int n );

    static const char * getResolution( int n );

    static int getDisplayMode();

    static void setDisplayMode( int n );
};

extern fabgl::Terminal         Terminal;
