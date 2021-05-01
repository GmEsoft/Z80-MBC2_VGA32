/*
    Z80-MBC2_VGA32 - Z80-MBC2 Bootstrap payloads stored in firmware
    

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

#include "runtime.h"

typedef uint	word;
typedef uchar	byte;

#if 0
const word  boot_A_StrAddr = 0x0000;// Payload A image starting address
extern const byte  boot_A_[] ;				// Payload A image (S210718 uBIOS + Basic)
extern const word  boot_A_size;
#endif

#if 1
const word  boot_B_StrAddr = 0xfd10;// Payload B image starting address
extern const byte  boot_B_[] ;				// Payload B image (S200718 iLoad)
extern const word  boot_B_size;
#endif

#if 0
const word  boot_C_StrAddr = 0x0100;// Payload C image starting address
extern const byte  boot_C_[] ;				// Payload C image (Forth)
extern const word  boot_C_size;
#endif
