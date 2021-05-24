/*
    Z80-MBC2_VGA32 - Z-80 CPU Emulator
    

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

#include "CPUExtender.h"

#include "CPU.h"

class Z80CPU :
	public CPU
{
protected:
	union Reg16 {
		Reg16(ushort x):word(x){}
		ushort word;
		struct {
			uchar b0, b1;
		} byte;
	};

	union Reg32 {
		Reg32(uint x):dword(x){}
		uint dword;
		struct {
			uchar b0, b1, b2, b3;
		} byte;
		struct {
			ushort w0, w1;
		} word;
	};

public:
	struct Instr
{
		int op, arg1, arg2; 
};

	union REGS
{
	struct X
	{
		unsigned short	bc ,
                        de ,
                        hl ,
                        af ,
                        sp ,
                        ix ,
                        iy ,
                        bc1,
                        de1,
                        hl1,
                        af1,
                        ir ;
    } x ;
    struct H
	{
		unsigned char	c, b,
                        e, d,
                        l, h,
                        f, a,
                        spl, sph ,
                        ixl, ixh,
                        iyl, iyh,
                        c1, b1,
                        e1, d1,
                        l1, h1,
                        f1, a1,
                        i, r,
                        im;
	} h ;
	};

	struct FLAGS {int c:1,n:1,p:1,x:1,h:1,y:1,z:1,s:1;} ;

	Z80CPU()
	{
		init();
	}

	~Z80CPU();

	void setExtender( CPUExtender *pCPUExtender )
	{
		pCPUExtender_ = pCPUExtender;
	}

	REGS &getRegs()
	{
		return this->regs;
	}

	FLAGS &getFlags()
	{
		return *this->flags;
	}

	// Trigger IRQ interrupt
	virtual void trigIRQ( char irq );

	// Get IRQ status
	virtual char getIRQ( void );

	// Trigger NMI interrupt
	virtual void trigNMI( void );

	// Get NMI status
	virtual char getNMI( void );

	// Get IRQ Enable flag
	char getIntEn( void )
	{
		return iff1;
	}

	// Init internal pointers
	void init();

	// CPU Reset
	virtual void reset();

	// Set Break On Halt
	void setBreakOnHalt( int _breakOnHalt )
	{
		breakOnHalt = _breakOnHalt;
	}

	// Set Break On EI
	void setBreakOnEI( int _breakOnEI )
	{
		breakOnEI = _breakOnEI;
	}

	// Execute 1 Statement
	virtual void sim();

protected:
	// immediate data addr
	uchar dataaddr()
	{
		return fetch();
	}

	// immediate bit addr
	uchar bitaddr()
	{
		return fetch();
	}

	// immediate short (rel) addr
	ushort saddr()
	{
		schar d = fetch();
		return pc_ + d;
	}

	// fetch signed offset
	schar sfetch()
	{
		return fetch();
	}

	uint laddr()
	{
		uint x = fetch ();
		return x | ( fetch () << 8 );
	}

	void simop( const Instr &instr );

public: //TODO private
	// Instruction Result
	uchar resINC_byte (uchar res);
	uchar resDEC_byte (uchar res);
	uchar resOR_byte (uchar res);
	uchar resAND_byte (uchar res);
	uchar resSUB_byte (schar res, ushort cy);
	uchar resADD_byte (schar res, ushort cy);
	uchar resROT_byte (uchar res, uchar c);
	uchar resROTA (uchar res, uchar c);
	uchar resLDI (uchar res, uint bc);
	uchar resCPI (uchar res, uint bc, uchar h);
	uchar resROTD_byte (uchar res);
private:
	void incR();
	void selectHL();
	void selectIX();
	void selectIY();


private:
	long			tstates;
	ushort			opcode;
	uchar			iff1, iff2, irq, nmi;
	union REGS		regs;
	//  bit-mapping of PSW register
	struct FLAGS	*flags;

	ushort			*Z80x[16];
	uchar			*Z80h[32];


	uchar			useix, useiy, useixiy;
	schar			offset;
	int				breakOnEI;
	int				breakOnHalt;
	CPUExtender		*pCPUExtender_;
public:
	static Instr	sInstr[];
	static Instr	sInstrCB[];
	static Instr	sInstrED[];
};
