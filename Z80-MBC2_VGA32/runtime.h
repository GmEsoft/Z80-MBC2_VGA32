/*
    Z80-MBC2_VGA32 - Runtime types definition
    

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

// RUNTIME TYPES
/*  #pragma GCC optimize("O...")
    -O0:    optimize for compile time and debug-ability. Most performance optimizations are disabled. This is the default for GCC.
    -O1:    pick the low hanging fruit in terms of performance, without impacting compilation time too much.
    -O2:    optimize for performance more aggressively, but not at the cost of larger code size.
    -O3:    optimize for performance at all cost, no matter the code size or compilation time impact.
    -Ofast  a.k.a. ludicrous mode: take off the guard rails, and disregard strict C standard compliance in the name of speed. This may lead to invalid programs.
    -Os     optimize for code size. This uses O2 as the baseline, but disables some optimizations. For example, it will not inline code if that leads to a size increase.
*/
#pragma GCC optimize("03")
//#pragma GCC target("force-no-pic")
//#pragma GCC target("target-align")

#if 1
#include <cstdint>

typedef int8_t          schar;
typedef uint8_t         uchar;
typedef int16_t         sshort;
typedef uint16_t        ushort;
typedef int             sint;   // otherwise conflicts with sys/types.h
typedef unsigned int    uint;   // otherwise conflicts with sys/types.h
typedef unsigned long   ulong;

#else

typedef   signed char   schar;
typedef unsigned char   uchar;
typedef   signed short  sshort;
typedef unsigned short  ushort;
typedef   signed int    sint;
typedef unsigned int    uint;
typedef unsigned long   ulong;

#endif


enum {
  B_00000000=0, B_00000001, B_00000010, B_00000011, B_00000100, B_00000101, B_00000110, B_00000111,
	B_00001000, B_00001001, B_00001010, B_00001011, B_00001100, B_00001101, B_00001110, B_00001111,
	B_00010000, B_00010001, B_00010010, B_00010011, B_00010100, B_00010101, B_00010110, B_00010111,
	B_00011000, B_00011001, B_00011010, B_00011011, B_00011100, B_00011101, B_00011110, B_00011111,
	B_00100000, B_00100001, B_00100010, B_00100011, B_00100100, B_00100101, B_00100110, B_00100111,
	B_00101000, B_00101001, B_00101010, B_00101011, B_00101100, B_00101101, B_00101110, B_00101111,
	B_00110000, B_00110001, B_00110010, B_00110011, B_00110100, B_00110101, B_00110110, B_00110111,
	B_00111000, B_00111001, B_00111010, B_00111011, B_00111100, B_00111101, B_00111110, B_00111111,
	B_01000000, B_01000001, B_01000010, B_01000011, B_01000100, B_01000101, B_01000110, B_01000111,
	B_01001000, B_01001001, B_01001010, B_01001011, B_01001100, B_01001101, B_01001110, B_01001111,
	B_01010000, B_01010001, B_01010010, B_01010011, B_01010100, B_01010101, B_01010110, B_01010111,
	B_01011000, B_01011001, B_01011010, B_01011011, B_01011100, B_01011101, B_01011110, B_01011111,
	B_01100000, B_01100001, B_01100010, B_01100011, B_01100100, B_01100101, B_01100110, B_01100111,
	B_01101000, B_01101001, B_01101010, B_01101011, B_01101100, B_01101101, B_01101110, B_01101111,
	B_01110000, B_01110001, B_01110010, B_01110011, B_01110100, B_01110101, B_01110110, B_01110111,
	B_01111000, B_01111001, B_01111010, B_01111011, B_01111100, B_01111101, B_01111110, B_01111111,
	B_10000000, B_10000001, B_10000010, B_10000011, B_10000100, B_10000101, B_10000110, B_10000111,
	B_10001000, B_10001001, B_10001010, B_10001011, B_10001100, B_10001101, B_10001110, B_10001111,
	B_10010000, B_10010001, B_10010010, B_10010011, B_10010100, B_10010101, B_10010110, B_10010111,
	B_10011000, B_10011001, B_10011010, B_10011011, B_10011100, B_10011101, B_10011110, B_10011111,
	B_10100000, B_10100001, B_10100010, B_10100011, B_10100100, B_10100101, B_10100110, B_10100111,
	B_10101000, B_10101001, B_10101010, B_10101011, B_10101100, B_10101101, B_10101110, B_10101111,
	B_10110000, B_10110001, B_10110010, B_10110011, B_10110100, B_10110101, B_10110110, B_10110111,
	B_10111000, B_10111001, B_10111010, B_10111011, B_10111100, B_10111101, B_10111110, B_10111111,
	B_11000000, B_11000001, B_11000010, B_11000011, B_11000100, B_11000101, B_11000110, B_11000111,
	B_11001000, B_11001001, B_11001010, B_11001011, B_11001100, B_11001101, B_11001110, B_11001111,
	B_11010000, B_11010001, B_11010010, B_11010011, B_11010100, B_11010101, B_11010110, B_11010111,
	B_11011000, B_11011001, B_11011010, B_11011011, B_11011100, B_11011101, B_11011110, B_11011111,
	B_11100000, B_11100001, B_11100010, B_11100011, B_11100100, B_11100101, B_11100110, B_11100111,
	B_11101000, B_11101001, B_11101010, B_11101011, B_11101100, B_11101101, B_11101110, B_11101111,
	B_11110000, B_11110001, B_11110010, B_11110011, B_11110100, B_11110101, B_11110110, B_11110111,
	B_11111000, B_11111001, B_11111010, B_11111011, B_11111100, B_11111101, B_11111110, B_11111111
};

enum {
	BIT_0 = 0x00000001,
	BIT_1 = 0x00000002,
	BIT_2 = 0x00000004,
	BIT_3 = 0x00000008,
	BIT_4 = 0x00000010,
	BIT_5 = 0x00000020,
	BIT_6 = 0x00000040,
	BIT_7 = 0x00000080,
	BIT_8 = 0x00000100,
	BIT_9 = 0x00000200,
	BIT_10 = 0x00000400,
	BIT_11 = 0x00000800,
	BIT_12 = 0x00001000,
	BIT_13 = 0x00002000,
	BIT_14 = 0x00004000,
	BIT_15 = 0x00008000,
	BIT_16 = 0x00010000,
	BIT_17 = 0x00020000,
	BIT_18 = 0x00040000,
	BIT_19 = 0x00080000,
	BIT_20 = 0x00100000,
	BIT_21 = 0x00200000,
	BIT_22 = 0x00400000,
	BIT_23 = 0x00800000,
	BIT_24 = 0x01000000,
	BIT_25 = 0x02000000,
	BIT_26 = 0x04000000,
	BIT_27 = 0x08000000,
	BIT_28 = 0x10000000,
	BIT_29 = 0x20000000,
	BIT_30 = 0x40000000,
	BIT_31 = 0x80000000
};
