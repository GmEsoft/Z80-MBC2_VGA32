/*
    Z80-MBC2_VGA32 - Stream decorator with line input
    

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

#include "AppStream.h"

String AppStreamImpl::input()
{
    String str = "";
    while(true) {
        int c = read();
        if ( c == '\r' ) {
            println();
            break;
        } else if ( ( c == '\b' || c == '\x7F' ) && !str.isEmpty() ) {
            str = str.substring( 0, str.length() - 1 );
            print( "\b \b" );
        } else if ( c == 24 ) {
            for ( int i=0; i<str.length(); ++i )
                print( "\b \b" );
            str.clear();
        } else if ( ( c >= ' ' && c < 0x7F ) ) {
            str += (char)c;
            print( (char)c );
        }
    }
    return str;
}

AppStreamImpl AppStream;
