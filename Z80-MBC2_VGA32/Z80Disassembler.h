/*
    Z80-MBC2_VGA32 - Z-80 CPU Code Disassembler
    

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

#ifndef Z80DISASSEMBLER_H
#define Z80DISASSEMBLER_H

// Z80 DISASSEMBLER

#include "Disassembler.h"

class Z80Disassembler :
	public Disassembler
{
	struct Instr
	{ 
		int mnemon, opn1, opn2, arg1, arg2; 
	};

public:
	Z80Disassembler(void)
	{
	}

	~Z80Disassembler(void)
	{
	}

	//  return hex-string or label for double-byte x (dasm)
	const char* getxaddr (uint x);

	//  return hex-string or label for double-byte x (dasm)
	const char* getsvc( uint x );

	// fetch long external address and return it as hex string or as label
	const char* getladdr ();

	// fetch short relative external address and return it as hex string or as label
	const char* getsaddr ();

	// return operand name or value
	const char* getoperand (int opcode, int pos);

	// get 1st operand name or value
	const char* getoperand1 (int opcode)
	{
		return getoperand (opcode, 1);
	}

	// get 2nd operand name or value
	const char* getoperand2 (int opcode)
	{
		return getoperand (opcode, 2);
	}

	// get single instruction source
	virtual const char *source();
private:
	bool useix, useiy;
public:
	static Instr	sInstr[];
};

#endif
