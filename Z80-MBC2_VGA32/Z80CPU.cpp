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
    
    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "log.h"
#include "Z80CPU.h"

#include "CPUExtender.h"

#include <assert.h>
#include <cstdlib>
#include <string.h>

#define simStop(msg)

#pragma GCC optimize("03")

//  Enumerated constants for instructions, also array subscripts
enum
{
	NOP=0, LD, INC, DEC, ADD, SUB, ADC, SBC, AND, OR, XOR, RLCA,
    RRCA, RLA, RRA, EX, EXX, DJNZ, JR, JP, CALL, RET, RST, CPL, NEG, SCF, CCF,
    CP, IN, OUT, PUSH, POP, HALT, DI, EI, DAA, RLD, RRD,
    RLC, RRC, RL, RR, SLA, SRA, SLL, SRL, BIT, RES, SET,
    LDI, LDD, LDIR, LDDR, CPI, CPIR, CPD, CPDR,
    INI, INIR, IND, INDR, OUTI, OTIR, OUTD, OTDR, IM, RETI, RETN,
    DEFB
};

//  Enumerated constants for operands, also array subscripts
enum {
	R=1, RX, BYTE, WORD, OFFSET, ATR, ATRX, AFP,
	Z, C, NZ, NC, PE, PO, P, M, ATBYTE, ATWORD, DIRECT
};

//  Enumerated constants for register identification
enum {
	simA=1, simB, simC, simD, simE, simH, simL, simI, simR,
	simBC, simDE, simHL, simAF, simSP, simIR
};

enum {
	simNULL = 0,
	// 8 BIT LOAD GROUP ///////////////////////////////////////////////////////////
	simLD_R_R,
	simLDS_A_R,
	simLD_R_b,
	simLD_R_ATRX,
	simLD_ATRX_R,
	simLD_ATRX_b,
	simLD_R_ATw,
	simLD_ATw_R,

	// 16 BIT LOAD GROUP //////////////////////////////////////////////////////////
	simLD_RX_w,
	simLD_RX_ATw,
	simLD_ATw_RX,
	simLD_RX_RX,
	simPUSH_RX,
	simPOP_RX,

	// EXCHANGE, BLOCK XFER AND SEARCH GROUP //////////////////////////////////////
	simEX_RX_RX,
	simEX_AF_AFP,
	simEXX,
	simEX_ATRX_RX,
	simLDI,
	simLDIR,
	simLDD,
	simLDDR,
	simCPI,
	simCPIR,
	simCPD,
	simCPDR,

	// 8 BIT ARITH AND LOGICAL GROUP //////////////////////////////////////////////
	simADD_R_R,
	simADD_R_b,
	simADD_R_ATRX,
	simADC_R_R,
	simADC_R_b,
	simADC_R_ATRX,
	simSUB_R,
	simSUB_b,
	simSUB_ATRX,
	simSBC_R_R,
	simSBC_R_b,
	simSBC_R_ATRX,
	simAND_R,
	simAND_b,
	simAND_ATRX,
	simOR_R,
	simOR_b,
	simOR_ATRX,
	simXOR_R,
	simXOR_b,
	simXOR_ATRX,
	simCP_R,
	simCP_b,
	simCP_ATRX,
	simINC_R,
	simINC_ATRX,
	simDEC_R,
	simDEC_ATRX,

	// GENERAL PURPOSE ARITHMETIC AND CPU CONTROL GROUP ///////////////////////////
	simDAA,
	simCPL,
	simNEG,
	simCCF,
	simSCF,
	simNOP,
	simHALT,
	simDI,
	simEI,
	simIM,

	// 16 BIT ARITHMETIC GROUP ////////////////////////////////////////////////////
	simADD_RX_RX,
	simADC_RX_RX,
	simSBC_RX_RX,
	simINC_RX,
	simDEC_RX,

	// ROTATE AND SHIFT GROUP /////////////////////////////////////////////////////
	simRLCA,
	simRLA,
	simRRCA,
	simRRA,
	simRLC_R,
	simRLC_ATRX,
	simRL_R,
	simRL_ATRX,
	simRRC_R,
	simRRC_ATRX,
	simRR_R,
	simRR_ATRX,
	simSLA_R,
	simSLA_ATRX,
	simSRA_R,
	simSRA_ATRX,
	simSLL_R,
	simSLL_ATRX,
	simSRL_R,
	simSRL_ATRX,
	simRLD,
	simRRD,

	// BIT SET RESET AND TEST GROUP ///////////////////////////////////////////////
	simBIT_n_R,
	simBIT_n_ATRX,
	simSET_n_R,
	simSET_n_ATRX,
	simRES_n_R,
	simRES_n_ATRX,

	// JUMP GROUP /////////////////////////////////////////////////////////////////
	simJP,
	simJP_Z,
	simJP_NZ,
	simJP_C,
	simJP_NC,
	simJP_PE,
	simJP_PO,
	simJP_P,
	simJP_M,
	simJR,
	simJR_Z,
	simJR_NZ,
	simJR_C,
	simJR_NC,
	simJP_ATRX,
	simDJNZ,

	// CALL AND RETURN GROUP //////////////////////////////////////////////////////
	simCALL,
	simCALL_Z,
	simCALL_NZ,
	simCALL_C,
	simCALL_NC,
	simCALL_PE,
	simCALL_PO,
	simCALL_P,
	simCALL_M,
	simRET,
	simRET_Z,
	simRET_NZ,
	simRET_C,
	simRET_NC,
	simRET_PE,
	simRET_PO,
	simRET_P,
	simRET_M,
	simRETI,
	simRETN,
	simRST,

	// INPUT AND OUTPUT GROUP /////////////////////////////////////////////////////
	simIN_R_ATb,
	simIN_R_ATR,
	simINI,
	simINIR,
	simIND,
	simOUT_ATb_R,
	simOUT_ATR_R,
	simOUTI,
	simOTIR,
	simOUTD,

	// MULTI-BYTE OPCODES /////////////////////////////////////////////////////////
	simCB,
	simDD,
	simED,
	simFD,

	// EXTENSIONS ///////////////////////////////////////////////////////////////// 
	simZ80EXT,

	// END OF ENUM ////////////////////////////////////////////////////////////////
	simEND
};



//	struct FLAGS {int c:1,n:1,p:1,x:1,h:1,y:1,z:1,s:1;} ;
static const uchar
	SF = B_10000000, SF_OFF = B_01111111,
	ZF = B_01000000, ZF_OFF = B_10111111,
	YF = B_00100000, YF_OFF = B_11011111,
	HF = B_00010000, HF_OFF = B_11101111,
	XF = B_00001000, XF_OFF = B_11110111,
	PF = B_00000100, PF_OFF = B_11111011,
	NF = B_00000010, NF_OFF = B_11111101,
	CF = B_00000001, CF_OFF = B_11111110;

enum {
	CPOS = 0, NPOS, PPOS, XPOS, HPOS, YPOS, ZPOS, SPOS
};

#define SBIT ( ( regs.h.f >> 7 ) & 1 )
#define ZBIT ( ( regs.h.f >> 6 ) & 1 )
#define YBIT ( ( regs.h.f >> 5 ) & 1 )
#define HBIT ( ( regs.h.f >> 4 ) & 1 )
#define XBIT ( ( regs.h.f >> 3 ) & 1 )
#define PBIT ( ( regs.h.f >> 2 ) & 1 )
#define NBIT ( ( regs.h.f >> 1 ) & 1 )
#define CBIT ( ( regs.h.f >> 0 ) & 1 )


Z80CPU::~Z80CPU()
{
}

// Init internal pointers
void Z80CPU::init()
{
	flags = (struct FLAGS*)&regs.h.f;
	Z80h[simA] = &regs.h.a;
	Z80h[simB] = &regs.h.b;
	Z80h[simC] = &regs.h.c;
	Z80h[simD] = &regs.h.d;
	Z80h[simE] = &regs.h.e;
	Z80h[simI] = &regs.h.i;
	Z80h[simR] = &regs.h.r;
	Z80x[simAF] = &regs.x.af;
	Z80x[simBC] = &regs.x.bc;
	Z80x[simDE] = &regs.x.de;
	Z80x[simSP] = &regs.x.sp;
	Z80x[simIR] = &regs.x.ir;
	selectHL();
	breakOnEI = 0;
	breakOnHalt = 1;
	pCPUExtender_ = 0;
}


// CPU Reset
void Z80CPU::reset()
{
	pc_ = 0;
	regs.x.sp = 0xFFFF;
	selectHL();
	iff1 = iff2 = regs.h.im = 0;
	irq = nmi = 0;
	regs.x.af = regs.x.af1 = regs.x.bc = regs.x.bc1 = regs.x.de = regs.x.de1 = regs.x.hl = regs.x.hl1 = regs.x.ir = regs.x.ix = regs.x.iy = 0xFFFF;
	breakOnEI = 0;
	tstates = 0;
}

// Trigger IRQ interrupt
void Z80CPU::trigIRQ( char myirq )
{
	irq = myirq;
}

// Get IRQ status
char Z80CPU::getIRQ( void )
{
	return irq;
}

// Trigger NMI interrupt
void Z80CPU::trigNMI( void )
{
	nmi = 3;
}

// Get NMI status
char Z80CPU::getNMI( void )
{
	return nmi;
}

// Get Parity of ACC (1=even, 0=odd)
static uchar parity(uchar res)
{
	res ^= res >> 4;
	res ^= res >> 2;
	res ^= res >> 1;
	return (~res) & 1;
}



// Instruction Results
__inline uchar Z80CPU::resINC_byte (uchar res)
{
	regs.h.f &= B_00000001;
    regs.h.f |= res & B_10101000;
	if ( res == 0x80 )
		regs.h.f |= PF;
	else if ( !res )
		regs.h.f |= ZF;
	if ( !( res & 0x0F ) )
		regs.h.f |= HF;
    return res;
}

__inline uchar Z80CPU::resDEC_byte (uchar res)
{
	regs.h.f &= B_00000001;
    regs.h.f |= res & B_10101000;
	regs.h.f |= NF;
	if ( res == 0x7F )
		regs.h.f |= PF;
	else if ( !res )
		regs.h.f |= ZF;
	if ( ( res & 0x0F ) == 0x0F )
		regs.h.f |= HF;
    return res;
}

__inline uchar Z80CPU::resOR_byte (uchar res)
{
    regs.h.f = res & B_10101000;
	if ( parity( res ) )
		regs.h.f |= PF;
	if ( !res )
		regs.h.f |= ZF;
	return res;
}

__inline uchar Z80CPU::resAND_byte (uchar res)
{
    regs.h.f = ( res & B_10101000 ) | HF;
	if ( parity( res ) )
		regs.h.f |= PF;
	if ( !res )
		regs.h.f |= ZF;
	return res;
}

/*				-2		-1		0		+1
		-		110		111		000		001
-2		110		000		001		010(CV)	111(CV)
-1		111		11(C)	00		01(C)	10(CV)
0		000		10		11		00		01
1		001		01(V)	10		11(C)	00
*/
__inline uchar Z80CPU::resSUB_byte (schar res, ushort c)
{
	uchar v = c >> ( 7 - PPOS);
	v ^= v >> 1;

	regs.h.f = NF
			 | res & B_10101000
			 | Reg16( c ).byte.b0 & HF
			 | Reg16( c ).byte.b1 & CF
			 | v & PF;

	if ( !res )
		regs.h.f |= ZF;

	return res;
}

/*				-2		-1		0		+1
		+		10		11		00		01
-2		10		00(CV)	01(CV)	10		11
-1		11		01(CV)	10(CV)	11		00(C)
0		00		10		11		00		01
1		01		11		00(C)	01		10
*/
__inline uchar Z80CPU::resADD_byte (schar res, ushort c)
{
	uchar v = c >> ( 7 - PPOS);
	v ^= v >> 1;

	regs.h.f = res & B_10101000
			 | Reg16( c ).byte.b0 & HF
			 | Reg16( c ).byte.b1 & CF
			 | v & PF;

	if ( !res )
		regs.h.f |= ZF;

	return res;
}

__inline uchar Z80CPU::resROT_byte (uchar res, uchar c)
{
    regs.h.f = res & B_10101000;
	regs.h.f |= c & CF;
	if ( parity( res ) )
		regs.h.f |= PF;
	if ( !res )
		regs.h.f |= ZF;
    return res;
}

__inline uchar Z80CPU::resROTA (uchar res, uchar c)
{
	regs.h.f &= B_11000100;
    regs.h.f |= res & B_00101000
			 |  c & CF;
    return res;
}

__inline uchar Z80CPU::resLDI (uchar res, uint bc)
{
	ushort x = res + regs.h.a;
	regs.h.f &= B_11000001;
	if ( x & B_00000010 )
		regs.h.f |= YF;
	if ( x & B_00001000 )
		regs.h.f |= XF;
	if ( regs.x.bc )
		regs.h.f |= PF;
    return res;
}

__inline uchar Z80CPU::resCPI (uchar res, uint bc, uchar h)
{
	ushort x = res - h;
	regs.h.f &= B_00000001;
	regs.h.f |= res & B_10000000;
	regs.h.f |= NF;
	if ( x & B_00000010 )
		regs.h.f |= YF;
	if ( x & B_00001000 )
		regs.h.f |= XF;
	if ( !res )
		regs.h.f |= ZF;
	if ( regs.x.bc )
		regs.h.f |= PF;
	if ( h )
		regs.h.f |= HF;
    return res;
}

__inline uchar Z80CPU::resROTD_byte (uchar res)
{
	regs.h.f &= B_00000001;
	regs.h.f |= res & B_10101000;
	if ( !res )
		regs.h.f |= ZF;
	if ( parity( res ) )
		regs.h.f |= PF;
    return res;
}

void Z80CPU::incR()
{
	ushort x = regs.h.r & 0x80;
	++regs.h.r;
	regs.h.r &= 0x7F;
	regs.h.r |= x;
}

// Init internal pointers for HL
void Z80CPU::selectHL()
{
	useix = useiy = useixiy = 0;
	Z80h[simH] = &regs.h.h;
	Z80h[simL] = &regs.h.l;
	Z80x[simHL] = &regs.x.hl;
}

// Init internal pointers for IX
void Z80CPU::selectIX()
{
	useix = useixiy = 1;
	Z80h[simH] = &regs.h.ixh;
	Z80h[simL] = &regs.h.ixl;
	Z80x[simHL] = &regs.x.ix;
}

// Init internal pointers for IY
void Z80CPU::selectIY()
{
	useiy = useixiy = 1;
	Z80h[simH] = &regs.h.iyh;
	Z80h[simL] = &regs.h.iyl;
	Z80x[simHL] = &regs.x.iy;
}



// Execute 1 Statement
void Z80CPU::sim()
{
	// interrupts
	if ( nmi )
	{
		// Non-maskable interrupt
		if ( --nmi == 0 )
		{
			iff2 = iff1;
			iff1 = 0;
			putdata( --regs.x.sp, pc_ >> 8 );
			putdata( --regs.x.sp, pc_ );
			pc_ = 0x0066;
		}
	}
	else if ( iff1 && iff2 )
	{
		// accept interrupt
		if ( irq )
		{
			iff1 = iff2 = 0;
			switch (regs.h.im)
			{
				case 0:
					simStop("IRQ in mode 0");
					break;
				case 1:
					putdata( --regs.x.sp, pc_ >> 8 );
					putdata( --regs.x.sp, pc_ );
					pc_ = 0x0038;
#ifdef DBG_INT
					simStop("IRQ in mode 1");
#endif
					break;
				case 2:
					simStop("IRQ in mode 2");
					break;
				default:
					simStop("IRQ in unknown mode");
					break;
			}
			return;
		}
	}
	else
	{
		iff2 |= iff1;
	}

	incR();
	opcode = fetch();
	simop( sInstr[opcode] );
	runcycles( tstates );
	tstates = 0;
}

void Z80CPU::simop( const Instr &instr )
{
	switch ( instr.op )
	{
	// NULL ///////////////////////////////////////////////////////////////////////
	
	// NULL => halt
	case simNULL:
		setMode( MODE_STOP );
		break;

	// 8 BIT LOAD GROUP ///////////////////////////////////////////////////////////

	// LD r,r'
	case simLD_R_R:
		{
			assert(Z80h[instr.arg1]);
			assert(Z80h[instr.arg2]);
			ushort x;
			if (useixiy )
			{
				if ( instr.arg2 == simH )
					x = useix ? regs.h.ixh : regs.h.iyh;
				else if ( instr.arg2 == simL )
					x = useix ? regs.h.ixl : regs.h.iyl;
				else
					x = *Z80h[instr.arg2];

				if ( instr.arg1 == simH )
					*( useix ? &regs.h.ixh : &regs.h.iyh ) = x;
				else if ( instr.arg1 == simL )
					*( useix ? &regs.h.ixl : &regs.h.iyl ) = x;
				else
					*Z80h[instr.arg1] = x;
				// TODO: timing for ix/iy
			}
			else
			{
				*Z80h[instr.arg1] = *Z80h[instr.arg2];
			}
			tstates += 4;
		}
		break;

	// LD A,I & LD A,R
	case simLDS_A_R:
		{
			assert( instr.arg1 == simA );
			assert(Z80h[instr.arg2]);
			*Z80h[instr.arg1] = *Z80h[instr.arg2];
			regs.h.f &= B_00000001;
			regs.h.f |= regs.h.a & B_10101000;
			if ( !regs.h.a )
				regs.h.f |= ZF;
			if ( iff2 )
				regs.h.f |= PF;

			tstates += 9;
		}
		break;

	// LD r,n
	case simLD_R_b:
		{
			assert(Z80h[instr.arg1]);
			ushort x = fetch();
			if (useixiy )
			{
				if ( instr.arg1 == simH )
					*( useix ? &regs.h.ixh : &regs.h.iyh ) = x;
				else if ( instr.arg1 == simL )
					*( useix ? &regs.h.ixl : &regs.h.iyl ) = x;
				else
					*Z80h[instr.arg1] = x;
				// TODO: timing for ix/iy
			}
			else
			{
				*Z80h[instr.arg1] = x;
			}
			tstates += 7;
			// TODO: ixh/ixl/iyh/iyl
		}
		break;

	// LD r,(HL)
	// LD r,(IX+d)
	// LD r,(IY+d)
	// LD A,(BC)
	// LD A,(DE)
	case simLD_R_ATRX:
		{
			assert(Z80h[instr.arg1]);
			assert(Z80x[instr.arg2]);
			ushort y = *Z80x[instr.arg2];
			if ( useixiy )
			{
				y += sfetch();
				selectHL();
				tstates += 19;
			}
			*Z80h[instr.arg1] = getdata( y );
		}
		break;

	// LD (HL),r
	// LD (IX+d),r
	// LD (IY+d),r
	// LD (BC),A
	// LD (DE),A
	case simLD_ATRX_R:
		{
			assert(Z80x[instr.arg1]);
			assert(Z80h[instr.arg2]);
			ushort y = *Z80x[instr.arg1];
			if ( useixiy )
			{
				y += sfetch();
				selectHL();
				tstates += 19;
			}
			putdata( y, *Z80h[instr.arg2] );
		}
		break;

	// LD (HL),n
	// LD (IX+d),n
	// LD (IY+d),n
	case simLD_ATRX_b:
		{
			assert(Z80x[instr.arg1]);
			if (useix && instr.arg1 == simHL) {
				offset = sfetch();
				putdata(regs.x.ix+offset,fetch());
				tstates += 19;
			} else if (useiy && instr.arg1 == simHL) {
				offset = sfetch();
				putdata(regs.x.iy+offset,fetch());
				tstates += 19;
			} else {
				putdata(*Z80x[instr.arg1], fetch());
				tstates += 10;
			}
		}
		break;

	// LD A,(nn)
	case simLD_R_ATw:
		{
			assert(Z80h[instr.arg1]);
			*Z80h[instr.arg1] = getdata(laddr()) ;
			tstates += 13;
		}
		break;

	// LD (nn),A
	case simLD_ATw_R:
		{
			assert(Z80h[instr.arg2]);
			putdata (laddr() , *Z80h[instr.arg2] ) ;
			tstates += 13;
		}
		break;

	// LD A,I

	// LD A,R

	// LD I,A

	// LD R,A

	// 16 BIT LOAD GROUP //////////////////////////////////////////////////////////

	// LD dd,nn 	dd=BC,DE,HL,SP
	// LD IX,nn
	// LD IY,nn
	case simLD_RX_w:
		{
			assert(Z80x[instr.arg1] != 0);
			if (useix && instr.arg1 == simHL) {
				regs.x.ix = laddr() ;
				tstates += 14;
			} else if (useiy && instr.arg1 == simHL) {
				regs.x.iy = laddr() ;
				tstates += 14;
			} else {
				*Z80x[instr.arg1] = laddr() ;
				tstates += 10;
			}
		}
		break;

	// LD HL,(nn)
	// LD dd,(nn)	dd=BC,DE,HL,SP
	// LD IX,(nn)
	// LD IY,(nn)
	case simLD_RX_ATw:
		{
			ushort i = laddr() ;
			ushort x = getdata(i)|(getdata(i+1)<<8);
			assert(Z80x[instr.arg1]);
			if (useix && instr.arg1 == simHL) {
				regs.x.ix = x ;
				tstates += 20;
			} else if (useiy && instr.arg1 == simHL) {
				regs.x.iy = x ;
				tstates += 20;
			} else {
				*Z80x[instr.arg1] = x ;
				tstates += 16; // 20 if BC/DE/SP
			}
		}
		break;

	// LD (nn),HL
	// LD (nn),dd	dd=BC,DE,HL,SP
	// LD (nn),IX
	// LD (nn),IY
	case simLD_ATw_RX:
		{
			ushort i = laddr() ;
			ushort x;
			assert(Z80x[instr.arg2]);
			if (useix && instr.arg2 == simHL) {
				x = regs.x.ix ;
				tstates += 20;
			} else if (useiy && instr.arg2 == simHL) {
				x = regs.x.iy ;
				tstates += 20;
			} else {
				x = *Z80x[instr.arg2] ;
				tstates += 16; // 20 if BC/DE/SP
			}
			putdata(i,x);
			putdata(i+1,x>>8);
		}
		break;

	// LD SP,HL
	// LD SP,IX
	// LD SP,IY
	case simLD_RX_RX:
		{
			assert(Z80x[instr.arg1]);
			assert(Z80x[instr.arg2]);
			assert(instr.arg1==simSP);
			assert(instr.arg2==simHL);
			if (useix && instr.arg2 == simHL) {
				*Z80x[instr.arg1] = regs.x.ix ;
				tstates += 10;
			} else if (useiy && instr.arg2 == simHL) {
				*Z80x[instr.arg1] = regs.x.iy;
				tstates += 10;
			} else {
				*Z80x[instr.arg1] = *Z80x[instr.arg2];
				tstates += 6;
			}
		}
		break;

	// PUSH qq		qq=BC,DE,HL,AF
	// PUSH IX
	// PUSH IY
	case simPUSH_RX:
		{
			assert(Z80x[instr.arg1]);
			ushort x;
			if (useix && instr.arg1 == simHL)
			{
				x = regs.x.ix ;
				tstates += 15;
			}
			else if (useiy && instr.arg1 == simHL)
			{
				x = regs.x.iy ;
				tstates += 15;
			}
			else
			{
				x = *Z80x[instr.arg1] ;
				tstates += 11;
			}
			putdata(--regs.x.sp, x >> 8 );
			putdata(--regs.x.sp, x );
		}
		break;

	// POP qq		qq=BC,DE,HL,AF
	// POP IX
	// POP IY
	case simPOP_RX:
		{
			assert(Z80x[instr.arg1]);
			ushort x = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
			if (useix && instr.arg1 == simHL)
			{
				regs.x.ix = x;
				tstates += 14;
			}
			else if (useiy && instr.arg1 == simHL)
			{
				regs.x.iy = x;
				tstates += 14;
			}
			else
			{
				*Z80x[instr.arg1] = x;
				tstates += 10;
			}
		}
		break;

	// EXCHANGE, BLOCK XFER AND SEARCH GROUP //////////////////////////////////////

	// EX DE,HL
	// EX DE,IX
	// EX DE,IY
	case simEX_RX_RX:
		{
			assert(instr.arg1 == simDE);
			assert(instr.arg2 == simHL);
			assert(Z80x[instr.arg1] );
			assert(Z80x[instr.arg2] );
			ushort y;
			if (useix) {
				y = regs.x.de;
				regs.x.de = regs.x.ix;
				regs.x.ix = y;
				tstates += 10;
			} else if (useiy) {
				y = regs.x.de;
				regs.x.de = regs.x.iy;
				regs.x.iy = y;
				tstates += 10;
			} else {
				y = regs.x.de;
				regs.x.de = regs.x.hl;
				regs.x.hl = y;
				tstates += 4;
			}
		}
		break;

	// EX AF,AF'
	case simEX_AF_AFP:
		{
			ushort x = regs.x.af;
			regs.x.af = regs.x.af1;
			regs.x.af1 = x;
			tstates += 4;
		}
		break;

	// EXX
	case simEXX:
		{
			ushort x = regs.x.hl;
			regs.x.hl = regs.x.hl1;
			regs.x.hl1 = x;
			x = regs.x.de;
			regs.x.de = regs.x.de1;
			regs.x.de1 = x;
			x = regs.x.bc;
			regs.x.bc = regs.x.bc1;
			regs.x.bc1 = x;
			tstates += 4;
		}
		break;

	// EX (SP),HL
	// EX (SP),IX
	// EX (SP),IY
	case simEX_ATRX_RX:
		{
			assert(instr.arg1 == simSP);
			assert(instr.arg2 == simHL);
			assert(Z80x[instr.arg1] );
			assert(Z80x[instr.arg2] );
			ushort y;
			if (useix) {
				y = getdata(regs.x.sp);
				putdata(regs.x.sp, regs.h.ixl);
				regs.h.ixl = y;
				y = getdata(regs.x.sp+1);
				putdata(regs.x.sp+1, regs.h.ixh);
				regs.h.ixh = y;
				tstates += 23;
			} else if (useiy) {
				y = getdata(regs.x.sp);
				putdata(regs.x.sp, regs.h.iyl);
				regs.h.iyl = y;
				y = getdata(regs.x.sp+1);
				putdata(regs.x.sp+1, regs.h.iyh);
				regs.h.iyh = y;
				tstates += 23;
			} else {
				y = getdata(regs.x.sp);
				putdata(regs.x.sp, regs.h.l);
				regs.h.l = y;
				y = getdata(regs.x.sp+1);
				putdata(regs.x.sp+1, regs.h.h);
				regs.h.h = y;
				tstates += 19;
			}
		}
		break;

	// LDI
	case simLDI:
		{
			--regs.x.bc;
			resLDI( putdata(regs.x.de++, getdata(regs.x.hl++)), regs.x.bc );
			tstates += 16;
		}
		break;

	// LDIR
	case simLDIR:
		{
			if (--regs.x.bc)
			{
				pc_ -= 2;
				tstates += 5;
			}
			resLDI( putdata(regs.x.de++, getdata(regs.x.hl++)), regs.x.bc );
			tstates += 16;
		}
		break;

	// LDD
	case simLDD:
		{
			--regs.x.bc;
			resLDI( putdata(regs.x.de--, getdata(regs.x.hl--)), regs.x.bc );
			tstates += 16;
		}
		break;

	// LDDR
	case simLDDR:
		{
			if (--regs.x.bc)
			{
				pc_ -= 2;
				tstates += 5;
			}
			resLDI( putdata(regs.x.de--, getdata(regs.x.hl--)), regs.x.bc );
			tstates += 16;
		}
		break;

	// CPI
	case simCPI:
		{
			ushort x = getdata(regs.x.hl++) ;
			ushort y = regs.h.a - x;

			uchar h = ((y ^ regs.h.a ^ x)>>4) & 1;

			--regs.x.bc;
			resCPI( y, regs.x.bc, h );
			tstates += 16;
		}
		break;

	// CPIR
	case simCPIR:
		{
			ushort x = getdata(regs.x.hl++) ;
			ushort y = regs.h.a - x;
			uchar h = ((y ^ regs.h.a ^ x)>>4) & 1;

			if (--regs.x.bc && y)
			{
				pc_ -= 2;
				tstates += 3;
			}
			resCPI( y, regs.x.bc, h );
			tstates += 18;
		}
		break;


	// CPD
	case simCPD:
		{
			ushort x = getdata(regs.x.hl--) ;
			ushort y = regs.h.a - x;
			uchar h = ((y ^ regs.h.a ^ x)>>4) & 1;

			--regs.x.bc;
			resCPI( y, regs.x.bc, h );
			tstates += 16;
		}
		break;

	// CPDR
	case simCPDR:
		{
			ushort x = getdata(regs.x.hl--) ;
			ushort y = regs.h.a - x;
			uchar h = ((y ^ regs.h.a ^ x)>>4) & 1;

			if (--regs.x.bc && y)
			{
				pc_ -= 2;
				tstates += 3;
			}
			resCPI( y, regs.x.bc, h );
			tstates += 18;
		}
		break;

	// 8 BIT ARITH AND LOGICAL GROUP //////////////////////////////////////////////

	// Note: flags->c = 0 or -1

	// Set Overflow flag resulting from ADD/ADC and SUB/SBC/CP
	//#define ovadd(y,x,c) ( (signed int)( (signed char)x + (signed char)y + (signed char)c ) \
	//				   != (signed char)( (signed char)x + (signed char)y + (signed char)c ) )
	#define ovsub(y,x,c) ( (signed int)( (signed char)x - (signed char)y - (signed char)c ) \
					   != (signed char)( (signed char)x - (signed char)y - (signed char)c ) )

	// ADD A,r
	case simADD_R_R:
		{
			// todo: IXH & IXL
			assert(Z80h[instr.arg1]);
			assert(Z80h[instr.arg2]);
			ushort x = *Z80h[instr.arg2];
			ushort z;
			ushort y = ( z = regs.h.a ) + x;
			ushort c = (y ^ z ^ x);
			resADD_byte(regs.h.a = y, c);
			tstates += 4;
			// TODO: IXh/IXl
		}
		break;

	// ADD A,n
	case simADD_R_b:
		{
			// todo: IXH & IXL
			assert(Z80h[instr.arg1]);
			ushort x = fetch();
			ushort z;
			ushort y = ( z = regs.h.a ) + x;
			ushort c = (y ^ z ^ x);
			resADD_byte(regs.h.a = y, c);
			tstates += 7;
		}
		break;

	// ADD A,(HL)
	// ADD A,(IX+d)
	// ADD A,(IY+d)
	case simADD_R_ATRX:
		{
			assert(Z80h[instr.arg1]);
			assert(Z80x[instr.arg2]);
			ushort x;
			if (useix && instr.arg2 == simHL) {
				offset = sfetch();
				x = getdata(regs.x.ix+offset) ;
				tstates += 19;
			} else if (useiy && instr.arg2 == simHL) {
				offset = sfetch();
				x = getdata(regs.x.iy+offset) ;
				tstates += 19;
			} else {
				x = getdata(*Z80x[instr.arg2]);
				tstates += 7;
			}
			ushort z;
			ushort y = ( z = regs.h.a ) + x;
			ushort c = (y ^ z ^ x);
			resADD_byte(regs.h.a = y, c);
		}
		break;

	// ADC A,r
	case simADC_R_R:
		{
			// todo: IXH & IXL
			assert(Z80h[instr.arg1]);
			assert(Z80h[instr.arg2]);
			ushort x = *Z80h[instr.arg2] ;
			ushort z;
			ushort y = x + ( z = regs.h.a ) + CBIT;
			ushort c = (y ^ z ^ x);
			resADD_byte(regs.h.a = y, c);
			tstates += 4; // TODO: ixyhl
		}
		break;

	// ADC A,n
	case simADC_R_b:
		{
			// todo: IXH & IXL
			assert(Z80h[instr.arg1]);
			ushort x = fetch();
			ushort z;
			ushort y = x + ( z = regs.h.a ) + CBIT;
			ushort c = (y ^ z ^ x);
			resADD_byte(regs.h.a = y, c);
			tstates += 7;
		}
		break;

	// ADC A,(HL)
	// ADC A,(IX+d)
	// ADC A,(IY+d)
	case simADC_R_ATRX:
		{
			assert(Z80h[instr.arg1]);
			assert(Z80x[instr.arg2]);
			ushort x;
			if (useix && instr.arg2 == simHL) {
				offset = sfetch();
				x = getdata(regs.x.ix+offset) ;
				tstates += 19;
			} else if (useiy && instr.arg2 == simHL) {
				offset = sfetch();
				x = getdata(regs.x.iy+offset) ;
				tstates += 19;
			} else {
				x = getdata(*Z80x[instr.arg2]);
				tstates += 7;
			}
			ushort z;
			ushort y = x + ( z = regs.h.a ) + CBIT;
			ushort c = (y ^ z ^ x);
			resADD_byte(regs.h.a = y, c);
		}
		break;

	// SUB r
	case simSUB_R:
		{
			// todo: IXH & IXL
			assert(Z80h[instr.arg1]);
			ushort x = *Z80h[instr.arg1] ;
			ushort z;
			ushort y = ( z = regs.h.a ) - x;
			ushort c = (y ^ z ^ x);
			resSUB_byte(regs.h.a = y, c);
			tstates += 4;
		}
		break;

	// SUB n
	case simSUB_b:
		{
			ushort x = fetch() ;
			ushort z;
			ushort y = ( z = regs.h.a ) - x;
			ushort c = (y ^ z ^ x);
			resSUB_byte(regs.h.a = y, c);
			tstates += 7;
		}
		break;

	// SUB (HL)
	// SUB (IX+d)
	// SUB (IY+d)
	case simSUB_ATRX:
		{
			assert(Z80x[instr.arg1]);
			ushort x;
			if (useix && instr.arg1 == simHL) {
				offset = sfetch();
				x = getdata(regs.x.ix+offset) ;
				tstates += 19;
			} else if (useiy && instr.arg1 == simHL) {
				offset = sfetch();
				x = getdata(regs.x.iy+offset) ;
				tstates += 19;
			} else {
				x = getdata(*Z80x[instr.arg1]);
				tstates += 7;
			}
			ushort z;
			ushort y = ( z = regs.h.a ) - x;
			ushort c = (y ^ z ^ x);
			resSUB_byte(regs.h.a = y, c);
		}
		break;

	// SBC A,r
	case simSBC_R_R:
		{
			assert(Z80h[instr.arg1]);
			assert(Z80h[instr.arg2]);
			ushort x = *Z80h[instr.arg2];
			ushort z;
			ushort y = ( z = regs.h.a ) - x - CBIT;
			ushort c = (y ^ z ^ x);
			resSUB_byte(regs.h.a = y, c);
			tstates += 4;
		}
		break;

	// SBC A,n
	case simSBC_R_b:
		{
			ushort x = fetch();
			ushort z;
			ushort y = ( z = regs.h.a ) - x - CBIT;
			ushort c = (y ^ z ^ x);
			resSUB_byte(regs.h.a = y, c);
			tstates += 7;
		}
		break;

	// SBC A,(HL)
	// SBC A,(IX+d)
	// SBC A,(IY+d)
	case simSBC_R_ATRX:
		{
			assert(Z80x[instr.arg2]);
			ushort x;
			if (useix && instr.arg2 == simHL) {
				offset = sfetch();
				x = getdata(regs.x.ix+offset) ;
				tstates += 19;
			} else if (useiy && instr.arg2 == simHL) {
				offset = sfetch();
				x = getdata(regs.x.iy+offset) ;
				tstates += 19;
			} else {
				x = getdata(*Z80x[instr.arg2]);
				tstates += 7;
			}
			ushort z;
			ushort y = ( z = regs.h.a ) - x - CBIT;
			ushort c = (y ^ z ^ x);
			resSUB_byte(regs.h.a = y, c);
		}
		break;

	// AND r
	case simAND_R:
		{
			assert(Z80h[instr.arg1]);
			resAND_byte(regs.h.a &= *Z80h[instr.arg1]);
			tstates += 4;
		}
		break;

	// AND n
	case simAND_b:
		{
			resAND_byte(regs.h.a &= fetch());
			tstates += 7;
		}
		break;

	// AND (HL)
	// AND (IX+d)
	// AND (IY+d)
	case simAND_ATRX:
		{
			assert(Z80x[instr.arg1]);
			if (useix && instr.arg1 == simHL) {
				offset = sfetch();
				resAND_byte(regs.h.a &= getdata(regs.x.ix+offset));
				tstates += 19;
			} else if (useiy && instr.arg1 == simHL) {
				offset = sfetch();
				resAND_byte(regs.h.a &= getdata(regs.x.iy+offset));
				tstates += 19;
			} else {
				resAND_byte(regs.h.a &= getdata(*Z80x[instr.arg1]));
				tstates += 7;
			}
		}
		break;

	// OR r
	case simOR_R:
		{
			assert(Z80h[instr.arg1]);
			resOR_byte(regs.h.a |= *Z80h[instr.arg1]);
			tstates += 4;
		}
		break;

	// OR n
	case simOR_b:
		{
			resOR_byte(regs.h.a |= fetch());
			tstates += 7;
		}
		break;

	// OR (HL)
	// OR (IX+d)
	// OR (IY+d)
	case simOR_ATRX:
		{
			assert(Z80x[instr.arg1]);
			if (useix && instr.arg1 == simHL) {
				offset = sfetch();
				resOR_byte(regs.h.a |= getdata(regs.x.ix+offset));
				tstates += 19;
			} else if (useiy && instr.arg1 == simHL) {
				offset = sfetch();
				resOR_byte(regs.h.a |= getdata(regs.x.iy+offset));
				tstates += 19;
			} else {
				resOR_byte(regs.h.a |= getdata(*Z80x[instr.arg1]));
				tstates += 7;
			}
		}
		break;

	// XOR r
	case simXOR_R:
		{
			assert(Z80h[instr.arg1]);
			resOR_byte(regs.h.a ^= *Z80h[instr.arg1]);
			tstates += 4;
		}
		break;

	// XOR n
	case simXOR_b:
		{
			resOR_byte(regs.h.a ^= fetch());
			tstates += 7;
		}
		break;

	// XOR (HL)
	// XOR (IX+d)
	// XOR (IY+d)
	case simXOR_ATRX:
		{
			assert(Z80x[instr.arg1]);
			if (useix && instr.arg1 == simHL) {
				offset = sfetch();
				resOR_byte(regs.h.a ^= getdata(regs.x.ix+offset)) ;
				tstates += 19;
			} else if (useiy && instr.arg1 == simHL) {
				offset = sfetch();
				resOR_byte(regs.h.a ^= getdata(regs.x.iy+offset)) ;
				tstates += 19;
			} else {
				resOR_byte(regs.h.a ^= getdata(*Z80x[instr.arg1]));
				tstates += 7;
			}
		}
		break;

	// CP r
	case simCP_R:
		{
			assert(Z80h[instr.arg1]);
			ushort x = *Z80h[instr.arg1] ;
			ushort z;
			ushort y = ( z = regs.h.a ) - x;
			ushort c = (y ^ z ^ x);
			resSUB_byte(y, c);
			// !!! X and Y copied from n and not from A_n
			regs.h.f &= ~0x28;
			regs.h.f |= ( x & 0x28 );
			tstates += 4;
		}
		break;

	// CP n
	case simCP_b:
		{
			ushort x = fetch() ;
			ushort z;
			ushort y = ( z = regs.h.a ) - x;
			ushort c = (y ^ z ^ x);
			resSUB_byte(y, c);
			// !!! X and Y copied from n and not from A_n
			regs.h.f &= ~0x28;
			regs.h.f |= ( x & 0x28 );

		tstates += 7;
	}
		break;

	// CP (HL)
	// CP (IX+d)
	// CP (IY+d)
	case simCP_ATRX:
		{
			assert(Z80x[instr.arg1]);
			ushort x;
			if (useix && instr.arg1 == simHL) {
				offset = sfetch();
				x = getdata(regs.x.ix+offset);
				tstates += 19;
			} else if (useiy && instr.arg1 == simHL) {
				offset = sfetch();
				x = getdata(regs.x.iy+offset);
				tstates += 19;
			} else {
				x = getdata(*Z80x[instr.arg1]);
				tstates += 7;
			}
			ushort z;
			ushort y = ( z = regs.h.a ) - x;
			ushort c = (y ^ z ^ x);
			resSUB_byte(y, c);
			// !!! X and Y copied from n and not from A_n
			regs.h.f &= ~0x28;
			regs.h.f |= ( x & 0x28 );
		}
		break;

	// INC r
	case simINC_R:
		{
			assert(Z80h[instr.arg1]);
			if ( useixiy )
			{
				if ( instr.arg1 == simH )
					resINC_byte( ++*( useix ? &regs.h.ixh : &regs.h.iyh ) );
				else if ( instr.arg1 == simL )
					resINC_byte( ++*( useix ? &regs.h.ixl : &regs.h.iyl ) );
				else
					resINC_byte( ++*Z80h[instr.arg1] );
				//TODO: timing
			}
			else
			{
				resINC_byte(++*Z80h[instr.arg1]);
			}
			tstates += 4;
		}
		break;

	// INC (HL)
	// INC (IX+d)
	// INC (IY+d)
	case simINC_ATRX:
		{
			assert(Z80x[instr.arg1]);
			if (useix && instr.arg1 == simHL) {
				offset = sfetch();
				resINC_byte( putdata( regs.x.ix+offset, getdata( regs.x.ix+offset ) + 1 ) );
				tstates += 26;
			} else if (useiy && instr.arg1 == simHL) {
				offset = sfetch();
				resINC_byte( putdata( regs.x.iy+offset, getdata( regs.x.iy+offset ) + 1 ) );
				tstates += 26;
			} else {
				resINC_byte( putdata( *Z80x[instr.arg1], getdata( *Z80x[instr.arg1] ) + 1 ) );
				tstates += 11;
			}
		}
		break;

	// DEC r
	case simDEC_R:
		{
			assert(Z80h[instr.arg1]);
			if ( useixiy )
			{
				if ( instr.arg1 == simH )
					resDEC_byte( --*( useix ? &regs.h.ixh : &regs.h.iyh ) );
				else if ( instr.arg1 == simL )
					resDEC_byte( --*( useix ? &regs.h.ixl : &regs.h.iyl ) );
				else
					resDEC_byte( --*Z80h[instr.arg1] );
				//TODO: timing
			}
			else
			{
				resDEC_byte(--*Z80h[instr.arg1]);
			}
			tstates += 4;
		}
		break;

	// DEC (HL)
	// DEC (IX+d)
	// DEC (IY+d)
	case simDEC_ATRX:
		{
			assert(Z80x[instr.arg1]);
			if (useix && instr.arg1 == simHL) {
				offset = sfetch();
				resDEC_byte( putdata( regs.x.ix+offset, getdata( regs.x.ix+offset ) - 1 ) );
				tstates += 26;
			} else if (useiy && instr.arg1 == simHL) {
				offset = sfetch();
				resDEC_byte( putdata( regs.x.iy+offset, getdata( regs.x.iy+offset ) - 1 ) );
				tstates += 26;
			} else {
				resDEC_byte( putdata( *Z80x[instr.arg1], getdata( *Z80x[instr.arg1] ) - 1 ) );
				tstates += 11;
			}
		}

		break;

	// GENERAL PURPOSE ARITHMETIC AND CPU CONTROL GROUP ///////////////////////////

	// DAA
	case simDAA:
		{
			uchar &a = regs.h.a;
			uchar hi = a >> 4;
			uchar lo = a & 0x0F;
			uchar diff;

			if ( regs.h.f & CF )
			{
				diff = ( lo > 9 || ( regs.h.f & HF ) ) ? 0x66 : 0x60;
			}
			else
			{
				if ( lo >= 10 )
				{
					diff = ( hi > 8 ) ? 0x66 : 0x06;
				}
				else
				{
					if ( hi >= 10 )
					{
						diff = ( regs.h.f & HF ) ? 0x66 : 0x60;
					}
					else
					{
						diff = ( regs.h.f & HF ) ? 0x06 : 0x00;
					}
				}
			}

			a = ( regs.h.f & NF ) ? a - diff : a + diff;							//	......N.

			regs.h.f &= CF | NF | HF;
			regs.h.f |= a & 0xA8;										//	S.Y.X...

			if ( hi >= ( lo <= 9 ? 10 : 9 ) )
				regs.h.f |= CF;

			if ( ( regs.h.f & NF ) ? ( ( regs.h.f & HF ) && lo <= 5 ) : lo >= 10 )
				regs.h.f |= HF;
			else
				regs.h.f &= HF_OFF;

			if ( !a )
				regs.h.f |= ZF;

			if ( parity( a ) )
				regs.h.f |= PF;

			tstates += 4;
		}
		break;

	// CPL
	case simCPL:
		{
			regs.h.a ^= 0xFF;
			regs.h.f &= B_11000101;
			regs.h.f |= HF | NF
					 | regs.h.a & B_00101000;	// ..Y.X...
			tstates += 4;
		}
		break;

	// NEG
	case simNEG:
		{
			ushort x = regs.h.a;
			ushort z;
			ushort y = ( z = 0 ) - x;
			ushort c = (y ^ x);
			resSUB_byte(regs.h.a = y, c);
			tstates += 8;
		}
		break;

	// CCF
	case simCCF:
		{
			regs.h.f &= B_11000101;
			regs.h.f |= regs.h.a & B_00101000;
			if ( regs.h.f & CF )
				regs.h.f |= HF;
			regs.h.f ^= CF;
			tstates += 4;
		}
		break;

	// SCF
	case simSCF:
		{
			regs.h.f &= B_11000100;
			regs.h.f |= regs.h.a & B_00101000
					 | CF;
			tstates += 4;
		}
		break;

	// NOP
	case simNOP:
		{
			tstates += 4;
		}
		break;

	// HALT
	case simHALT:
		{
			if ( breakOnHalt )
			{
				setMode( MODE_STOP );
			}
			else if ( opcode == 0x76 )
			{
				// true HALT => loop
				--pc_;
			}
			tstates += 4;
		}
		break;

	// DI
	case simDI:
		{
			// disable interrupts
			iff1 = iff2 = 0;
			tstates += 4;
		#ifdef DBG_INT
			simStop( "DI" );
		#endif
		}
		break;

	// EI
	case simEI:
		{
			// enable interrupts
			iff1 = 1;
			iff2 = 0; // normally 1 - trick to skip next instruction
			tstates += 4;
			if ( breakOnEI )
				simStop( "EI" );
		}
		break;

	// IM 0
	// IM 1
	// IM 2
	case simIM:
		{
			regs.h.im = instr.arg1;
			tstates += 8;
		}
		break;




	// 16 BIT ARITHMETIC GROUP ////////////////////////////////////////////////////

	#define ovadd16(y,x,c) ( (signed long)( (signed short)x + (signed short)y + (signed short)c ) \
						  != (signed short)( (signed short)x + (signed short)y + (signed short)c ) )
	#define ovsub16(y,x,c) ( (signed long)( (signed short)x - (signed short)y - (signed short)c ) \
						 != (signed short)( (signed short)x - (signed short)y - (signed short)c ) )

	// ADD HL,rr	rr=BC,DE,HL,SP
	// ADD IX,rr	rr=BC,DE,IX,SP
	// ADD IY,rr	rr=BC,DE,IY,SP
	case simADD_RX_RX:
		{
			assert(instr.arg1 == simHL);
			assert(Z80x[instr.arg1] );
			assert(Z80x[instr.arg2] );

			// Op 2
			uint y;
			if (useix && instr.arg2 == simHL) {
				y = regs.x.ix ;
			} else if (useiy && instr.arg2 == simHL) {
				y = regs.x.iy ;
			} else {
				y = *Z80x[instr.arg2] ;
			}

			// Op 1
			ushort *px;
			if (useix && instr.arg1 == simHL) {
				px = &regs.x.ix;
				tstates += 15;
			} else if (useiy && instr.arg1 == simHL) {
				px = &regs.x.iy;
				tstates += 15;
			} else {
				px = Z80x[instr.arg1];
				tstates += 11;
			}

			uint x = *px;
			uint z = x + y;
			*px = z;
			uint c = x ^ y ^ z;

			regs.h.f &= B_11000100;
			regs.h.f |= ( Reg16( z ).byte.b1 & ( YF | XF ) )
					 |  ( Reg32( c ).byte.b2 & CF )
					 |  ( Reg16( c ).byte.b1 & HF );
		}
		break;

	// ADC HL,rr	rr=BC,DE,HL,SP
	// ADC IX,rr	rr=BC,DE,IX,SP
	// ADC IY,rr	rr=BC,DE,IY,SP
	case simADC_RX_RX:
		{
			assert(instr.arg1 == simHL);
			assert(Z80x[instr.arg1] );
			assert(Z80x[instr.arg2] );

			// Op 2
			uint y;
			if (useix && instr.arg2 == simHL) {
				y = regs.x.ix ;
			} else if (useiy && instr.arg2 == simHL) {
				y = regs.x.iy ;
			} else {
				y = *Z80x[instr.arg2] ;
			}

			// Op 1
			ushort *px;
			if (useix && instr.arg1 == simHL) {
				px = &regs.x.ix;
				tstates += 15;
			} else if (useiy && instr.arg1 == simHL) {
				px = &regs.x.iy;
				tstates += 15;
			} else {
				px = Z80x[instr.arg1];
				tstates += 15;
			}
			
			uint x = *px;
			uint z = x + y + CBIT;
			*px = z;
			uint c = x ^ y ^ z;
			uchar v = c >> ( 15 - PPOS );
			v ^= v >> 1;

			regs.h.f = ( Reg16( z ).byte.b1 & ( SF | YF | XF ) )
					 | ( Reg16( v ).byte.b0 & PF )
					 | ( Reg32( c ).byte.b2 & CF )
					 | ( Reg16( c ).byte.b1 & HF );
			if ( !z )
				regs.h.f |= ZF;

		}
		break;

	// SBC HL,rr	rr=BC,DE,HL,SP
	// SBC IX,rr	rr=BC,DE,IX,SP
	// SBC IY,rr	rr=BC,DE,IY,SP
	case simSBC_RX_RX:
		{
			assert(instr.arg1 == simHL);
			assert(Z80x[instr.arg1] );
			assert(Z80x[instr.arg2] );

			// Op 2
			uint y;
			if (useix && instr.arg2 == simHL) {
				y = regs.x.ix ;
			} else if (useiy && instr.arg2 == simHL) {
				y = regs.x.iy ;
			} else {
				y = *Z80x[instr.arg2] ;
			}

			// Op 1
			ushort *px;
			if (useix && instr.arg1 == simHL) {
				px = &regs.x.ix;
				tstates += 15;
			} else if (useiy && instr.arg1 == simHL) {
				px = &regs.x.iy;
				tstates += 15;
			} else {
				px = Z80x[instr.arg1];
				tstates += 15;
			}
			
			uint x = *px;
			uint z = x - y - CBIT;
			*px = z;
			uint c = x ^ y ^ z;
			uchar v = c >> ( 15 - PPOS );
			v ^= v >> 1;

			regs.h.f = ( Reg16( z ).byte.b1 & ( SF | YF | XF ) )
					 | ( Reg16( v ).byte.b0 & PF )
					 | ( Reg32( c ).byte.b2 & CF )
					 | ( Reg16( c ).byte.b1 & HF )
					 | NF;
			if ( !z )
				regs.h.f |= ZF;

		}
		break;

	// INC ss		ss=BC,DE,HL,SP
	// INC IX
	// INC IY
	case simINC_RX:
		{
			assert(Z80x[instr.arg1]);
			if (useix && instr.arg1 == simHL) {
				++regs.x.ix ;
				tstates += 10;
			} else if (useiy && instr.arg1 == simHL) {
				++regs.x.iy ;
				tstates += 10;
			} else {
				++*Z80x[instr.arg1];
				tstates += 6;
			}
		}
		break;

	// DEC ss		ss=BC,DE,HL,SP
	// DEC IX
	// DEC IY
	case simDEC_RX:
		{
			assert(Z80x[instr.arg1]);
			if (useix && instr.arg1 == simHL) {
				--regs.x.ix ;
				tstates += 10;
			} else if (useiy && instr.arg1 == simHL) {
				--regs.x.iy ;
				tstates += 10;
			} else {
				--*Z80x[instr.arg1];
				tstates += 6;
			}
		}
		break;

	// ROTATE AND SHIFT GROUP /////////////////////////////////////////////////////

	// RLCA
	case simRLCA:
		{
			ushort x = regs.h.a<<1;
			x |= x>>8;
			resROTA(regs.h.a=x, x&1);
			tstates += 4;
		}
		break;

	// RLA
	case simRLA:
		{
			ushort x = regs.h.a << 1 | CBIT;
			resROTA( regs.h.a = x, x >> 8 );
			tstates += 4;
		}
		break;

	// RRCA
	case simRRCA:
		{
			ushort x = regs.h.a << 7;
			x |= x >> 8;
			resROTA( regs.h.a = x, x >> 7 );
			tstates += 4;
		}
		break;

	// RRA
	case simRRA:
		{
			uchar c = regs.h.a & 1;
			ushort x = ( regs.h.a >> 1 ) | ( CBIT << 7 ) ;
			resROTA( regs.h.a = x, c );
			tstates += 4;
		}
		break;

	// RLC r
	case simRLC_R:
		{
			assert(Z80h[instr.arg1]);
			ushort x = *Z80h[instr.arg1] << 1;
			x |= x >> 8;
			resROT_byte( *Z80h[instr.arg1] = x, x & 1 );
			tstates += 8;
		}
		break;

	// RLC (HL)
	// RLC (IX+d)
	// RLC (IY+d)
	case simRLC_ATRX:
		{
			assert( instr.arg1==simHL );
			ushort y;
			if ( useix && instr.arg1 == simHL ) {
				y = regs.x.ix + offset;
				tstates += 23;
			} else if ( useiy && instr.arg1 == simHL ) {
				y = regs.x.iy + offset;
				tstates += 23;
			} else {
				y = *Z80x[instr.arg1];
				tstates += 15;
			}
			ushort x = getdata( y ) << 1;
			x |= x >> 8;
			resROT_byte( putdata(y,x), x & 1 );
		}
		break;

	// RL r
	case simRL_R:
		{
			assert( Z80h[instr.arg1] );
			ushort x = *Z80h[instr.arg1] << 1 | CBIT;
			resROT_byte( *Z80h[instr.arg1] = x, x >> 8 );
			tstates += 8;
		}
		break;

	// RL (HL)
	// RL (IX+d)
	// RL (IY+d)
	case simRL_ATRX:
		{
			assert( instr.arg1 == simHL );
			ushort y;
			if ( useix && instr.arg1 == simHL ) {
				y = regs.x.ix + offset;
				tstates += 23;
			} else if ( useiy && instr.arg1 == simHL ) {
				y = regs.x.iy + offset;
				tstates += 23;
			} else {
				y = *Z80x[instr.arg1];
				tstates += 15;
			}
			ushort x = getdata( y ) << 1 | CBIT;
			resROT_byte(putdata(y,x), x>>8);
		}
		break;

	// RRC r
	case simRRC_R:
		{
			assert( Z80h[instr.arg1] );
			ushort x = *Z80h[instr.arg1] << 7;
			x |= x >> 8;
			resROT_byte( *Z80h[instr.arg1] = x, x >> 7 );
			tstates += 8;
		}
		break;

	// RRC (HL)
	// RRC (IX+d)
	// RRC (IY+d)
	case simRRC_ATRX:
		{
			assert( instr.arg1 == simHL );
			ushort y;
			if ( useix && instr.arg1 == simHL ) {
				y = regs.x.ix + offset;
				tstates += 23;
			} else if ( useiy && instr.arg1 == simHL ) {
				y = regs.x.iy + offset;
				tstates += 23;
			} else {
				y = *Z80x[instr.arg1];
				tstates += 15;
			}
			ushort x = getdata( y ) << 7;
			x |= x >> 8;
			resROT_byte( putdata( y, x ), x >> 7 );
		}
		break;

	// RR r
	case simRR_R:
		{
			assert( Z80h[instr.arg1] );
			uchar c = *Z80h[instr.arg1] & 1;
			ushort x = ( *Z80h[instr.arg1] >> 1 ) | ( CBIT << 7 );
			resROT_byte( *Z80h[instr.arg1] = x, c );
			tstates += 8;
		}
		break;

	// RR (HL)
	// RR (IX+d)
	// RR (IY+d)
	case simRR_ATRX:
		{
			assert( instr.arg1 == simHL );
			ushort y;
			if ( useix && instr.arg1 == simHL ) {
				y = regs.x.ix + offset;
				tstates += 23;
			} else if ( useiy && instr.arg1 == simHL ) {
				y = regs.x.iy + offset;
				tstates += 23;
			} else {
				y = *Z80x[instr.arg1];
				tstates += 15;
			}
			ushort x = getdata( y );
			uchar c = x & 1;
			x = ( x >> 1 ) | ( CBIT << 7 ) ;
			resROT_byte( putdata( y, x ), c);
		}
		break;

	// SLA r
	case simSLA_R:
		{
			assert( Z80h[instr.arg1] );
			ushort x = *Z80h[instr.arg1] << 1;
			resROT_byte( *Z80h[instr.arg1]=  x, x >> 8 );
			tstates += 8;
		}
		break;

	// SLA (HL)
	// SLA (IX+d)
	// SLA (IY+d)
	case simSLA_ATRX:
		{
			assert( instr.arg1 == simHL );
			ushort y;
			if ( useix && instr.arg1 == simHL ) {
				y = regs.x.ix + offset;
				tstates += 23;
			} else if ( useiy && instr.arg1 == simHL ) {
				y = regs.x.iy + offset;
				tstates += 23;
			} else {
				y = *Z80x[instr.arg1];
				tstates += 15;
			}
			ushort x = getdata( y );
			uchar c = x >> 7;
			x = ( x << 1 ) /* | 1*/ ;
			resROT_byte( putdata( y, x ), c );
		}
		break;

	// SRA r
	case simSRA_R:
		{
			assert( Z80h[instr.arg1] );
			ushort x = *Z80h[instr.arg1];
			uchar c = x&1;
			x = ( x>> 1 ) | ( x & 0x80 );
			resROT_byte( *Z80h[instr.arg1] = x, c );
			tstates += 8;
		}
		break;

	// SRA (HL)
	// SRA (IX+d)
	// SRA (IY+d)
	case simSRA_ATRX:
		{
			assert( instr.arg1 == simHL );
			ushort y;
			if ( useix && instr.arg1 == simHL ) {
				y = regs.x.ix + offset;
				tstates += 23;
			} else if ( useiy && instr.arg1 == simHL ) {
				y = regs.x.iy + offset;
				tstates += 23;
			} else {
				y = *Z80x[instr.arg1];
				tstates += 15;
			}
			ushort x = getdata(y);
			uchar c = x & 1;
			x = ( x >> 1 ) | ( x & 0x80 ) ;
			resROT_byte( putdata( y, x ), c );
		}
		break;

	// SLL r (undocumented)
	case simSLL_R:
		{
			assert( Z80h[instr.arg1] );
			ushort x = ( *Z80h[instr.arg1] << 1 ) | 1;
			resROT_byte( *Z80h[instr.arg1] = x, x >> 8 );
			tstates += 8;
		}
		break;

	// SLL (HL) (undocumented)
	// SLL (IX+d)
	// SLL (IY+d)
	case simSLL_ATRX:
		{
			assert( instr.arg1==simHL );
			ushort y;
			if ( useix && instr.arg1 == simHL ) {
				y = regs.x.ix + offset;
				tstates += 23;
			} else if ( useiy && instr.arg1 == simHL ) {
				y = regs.x.iy + offset;
				tstates += 23;
			} else {
				y = *Z80x[instr.arg1];
				tstates += 15;
			}
			ushort x = getdata( y );
			uchar c = x >> 7;
			x = ( x << 1 ) | 1;
			resROT_byte( putdata( y, x ), c);
		}
		break;

	// SRL r
	case simSRL_R:
		{
			assert( Z80h[instr.arg1] );
			ushort x = *Z80h[instr.arg1];
			uchar c = x & 1;
			x = x >> 1;
			resROT_byte( *Z80h[instr.arg1] = x, c );
			tstates += 8;
		}
		break;

	// SRL (HL)
	// SRL (IX+d)
	// SRL (IY+d)
	case simSRL_ATRX:
		{
			assert( instr.arg1 == simHL );
			ushort y;
			if ( useix && instr.arg1 == simHL ) {
				y = regs.x.ix + offset;
				tstates += 23;
			} else if ( useiy && instr.arg1 == simHL ) {
				y = regs.x.iy + offset;
				tstates += 23;
			} else {
				y = *Z80x[instr.arg1];
				tstates += 15;
			}
			ushort x = getdata( y );
			uchar c = x & 1;
			x = x >> 1;
			resROT_byte( putdata( y, x ), c );
		}
		break;

	// RLD
	case simRLD:
		{
			ushort x = getdata( regs.x.hl ) << 4;
			x |= regs.h.a & 0x0F;
			regs.h.a &= 0xF0;
			regs.h.a |= x >> 8;
			putdata( regs.x.hl, x );
			resROTD_byte( regs.h.a );
			tstates += 18;
		}
		break;

	// RRD
	case simRRD:
		{
			ushort x = getdata( regs.x.hl ) ;
			x |= ( regs.h.a & 0x0F ) << 8;
			regs.h.a &= 0xF0;
			regs.h.a |= x & 0x0F;
			putdata( regs.x.hl, x >> 4 );
			resROTD_byte( regs.h.a );
			tstates += 18;
		}

		break;

	// BIT SET RESET AND TEST GROUP ///////////////////////////////////////////////

	// BIT b,r
	case simBIT_n_R:
		{
			assert(Z80h[instr.arg2]);
			ushort x = *Z80h[instr.arg2];
			ushort mask = 1 << instr.arg1;
			regs.h.f &= B_00000001;
			regs.h.f |= ( B_10101000
						& x 
						& ( mask | B_00101000 ) 
						)
			         |  HF;
			if ( ! ( x & mask ) )
				regs.h.f |= ZF | PF;
			tstates += 8;
		}
		break;

	// BIT b,(HL)
	// BIT b,(IX+d)
	// BIT b,(IY+d)
	case simBIT_n_ATRX:
		{
			// IX/IY ...
			assert(Z80x[instr.arg2]);

			ushort x;
			if (useix && instr.arg2 == simHL) {
				x = regs.x.ix+offset;
				tstates += 20;
			} else if (useiy && instr.arg2 == simHL) {
				x = regs.x.iy+offset;
				tstates += 20;
			} else {
				x = *Z80x[instr.arg2];
				tstates += 12;
			}
			ushort z = getdata(x);
			ushort mask = 1 << instr.arg1;
			regs.h.f &= B_00000001;
			regs.h.f |= ( ( ( ( x >> 8 ) & B_00101000 )	// X & Y from HIGH(IX+d)
						  | ( z & B_10000000 )			// S from result
						  ) 
					    & mask							// bit mask
						)
					 |  HF;
			if ( !( z & mask ) )
				regs.h.f |= ZF | PF;
		}
		break;

	// SET b,r
	case simSET_n_R:
		{
			assert(Z80h[instr.arg2]);
			ushort x = *Z80h[instr.arg2];
			x |= (1 << instr.arg1);
			*Z80h[instr.arg2] = x;
			tstates += 8;
		}
		break;

	// SET b,(HL)
	// SET b,(IX+d)
	// SET b,(IY+d)
	case simSET_n_ATRX:
		{
			// IX/IY ...
			assert(Z80x[instr.arg2]);
			ushort x;
			if (useix && instr.arg2 == simHL) {
				x = regs.x.ix+offset;
				tstates += 23;
			} else if (useiy && instr.arg2 == simHL) {
				x = regs.x.iy+offset;
				tstates += 23;
			} else {
				x = *Z80x[instr.arg2];
				tstates += 15;
			}
			ushort y = getdata(x);
			y |= (1 << instr.arg1);
			putdata(x,y);
		}
		break;

	// RES b,r
	case simRES_n_R:
		{
			assert(Z80h[instr.arg2]);
			ushort x = *Z80h[instr.arg2];
			x &= ~(1 << instr.arg1);
			*Z80h[instr.arg2] = x;
			tstates += 8;
		}
		break;

	// RES b,(HL)
	// RES b,(IX+d)
	// RES b,(IY+d)
	case simRES_n_ATRX:
		{
			// IX/IY ...
			assert(Z80x[instr.arg2]);
			ushort x;
			if (useix && instr.arg2 == simHL) {
				x = regs.x.ix+offset;
				tstates += 23;
			} else if (useiy && instr.arg2 == simHL) {
				x = regs.x.iy+offset;
				tstates += 23;
			} else {
				x = *Z80x[instr.arg2];
				tstates += 15;
			}
			ushort y = getdata(x);
			y &= ~(1 << instr.arg1);
			putdata(x,y);
		}

		break;

	// JUMP GROUP /////////////////////////////////////////////////////////////////


	// JP nn
	case simJP:
		{
			pc_ = laddr();
			tstates += 10;
		}
		break;

	// JP Z,nn
	case simJP_Z:
		{
			ushort x = laddr();
			if ( regs.h.f & ZF )
				pc_ = x;
			tstates += 10;
		}
		break;

	// JP NZ,nn
	case simJP_NZ:
		{
			ushort x = laddr();
			if ( ! ( regs.h.f & ZF ) )
				pc_ = x;
			tstates += 10;
		}
		break;

	// JP C,nn
	case simJP_C:
		{
			ushort x = laddr();
			if ( regs.h.f & CF )
				pc_ = x;
			tstates += 10;
		}
		break;

	// JP NC,nn
	case simJP_NC:
		{
			ushort x = laddr();
			if ( !( regs.h.f & CF ) )
				pc_ = x;
			tstates += 10;
		}
		break;

	// JP PE,nn
	case simJP_PE:
		{
			ushort x = laddr();
			if ( regs.h.f & PF )
				pc_ = x;
			tstates += 10;
		}
		break;

	// JP PO,nn
	case simJP_PO:
		{
			ushort x = laddr();
			if ( !( regs.h.f & PF ) )
				pc_ = x;
			tstates += 10;
		}
		break;

	// JP P,nn
	case simJP_P:
		{
			ushort x = laddr();
			if ( !( regs.h.f & SF ) ) 
				pc_ = x;
			tstates += 10;
		}
		break;

	// JP M,nn
	case simJP_M:
		{
			ushort x = laddr();
			if ( regs.h.f & SF )
				pc_ = x;
			tstates += 10;
		}
		break;

	// JR e
	case simJR:
		{
			pc_ = saddr();
			tstates += 12;
		}
		break;

	// JR Z,e
	case simJR_Z:
		{
			ushort x = saddr();
			if ( regs.h.f & ZF )
			{
				pc_ = x;
				tstates += 5;
			}
			tstates += 7;
		}
		break;

	// JR NZ,e
	case simJR_NZ:
		{
			ushort x = saddr();
			if ( !( regs.h.f & ZF ) )
			{
				pc_ = x;
				tstates += 5;
			}
			tstates += 7;
		}
		break;

	// JR C,e
	case simJR_C:
		{
			ushort x = saddr();
			if ( regs.h.f & CF )
			{
				pc_ = x;
				tstates += 5;
			}
			tstates += 7;
		}
		break;

	// JR NC,e
	case simJR_NC:
		{
			ushort x = saddr();
			if ( !( regs.h.f & CF ) )
			{
				pc_ = x;
				tstates += 5;
			}
			tstates += 7;
		}
		break;

	// JP (HL)
	// JP (IX)
	// JP (IY)
	case simJP_ATRX:
		{
			assert(Z80x[instr.arg1]);
			ushort x;
			if (useix && instr.arg1 == simHL) {
				x = regs.x.ix;
				tstates += 8;

			} else if (useiy && instr.arg1 == simHL) {
				x = regs.x.iy;
				tstates += 8;
			} else {
				x = *Z80x[instr.arg1];
				tstates += 4;
			}
			pc_ = x;
		}
		break;

	// DJNZ e
	case simDJNZ:
		{
			ushort x = saddr();
			tstates += 10;
			if (--regs.h.b)
			{
				pc_ = x;
				tstates += 3;
			}
		}

		break;

	// CALL AND RETURN GROUP //////////////////////////////////////////////////////

	// CALL nn
	case simCALL:
		{
			ushort x = laddr();
			putdata( --regs.x.sp, pc_ >> 8 );
			putdata( --regs.x.sp, pc_ );
			pc_ = x;
			tstates += 17;
		}
		break;

	// CALL Z,nn
	case simCALL_Z:
		{
			ushort x = laddr();
			if ( regs.h.f & ZF ) {
				putdata(--regs.x.sp, pc_ >> 8);
				putdata(--regs.x.sp, pc_ );
				pc_ = x;
				tstates += 7;
			}
			tstates += 10;
		}
		break;

	// CALL NZ,nn
	case simCALL_NZ:
		{
			ushort x = laddr();
			if ( !( regs.h.f & ZF ) ) {
				putdata(--regs.x.sp, pc_ >> 8);
				putdata(--regs.x.sp, pc_ );
				pc_ = x;
				tstates += 7;
			}
			tstates += 10;
		}
		break;

	// CALL C,nn
	case simCALL_C:
		{
			ushort x = laddr();
			if ( regs.h.f & CF ) {
				putdata(--regs.x.sp, pc_ >> 8);
				putdata(--regs.x.sp, pc_ );
				pc_ = x;
				tstates += 7;
			}
			tstates += 10;
		}
		break;

	// CALL NC,nn
	case simCALL_NC:
		{
			ushort x = laddr();
			if ( !( regs.h.f & CF ) ) {
				putdata(--regs.x.sp, pc_ >> 8);
				putdata(--regs.x.sp, pc_ );
				pc_ = x;
				tstates += 7;
			}
			tstates += 10;
		}
		break;

	// CALL PE,nn
	case simCALL_PE:
		{
			ushort x = laddr();
			if ( regs.h.f & PF ) {
				putdata(--regs.x.sp, pc_ >> 8);
				putdata(--regs.x.sp, pc_ );
				pc_ = x;
				tstates += 7;
			}
			tstates += 10;
		}
		break;

	// CALL PO,nn
	case simCALL_PO:
		{
			ushort x = laddr();
			if ( !( regs.h.f & PF ) ) {
				putdata(--regs.x.sp, pc_ >> 8);
				putdata(--regs.x.sp, pc_ );
				pc_ = x;
				tstates += 7;
			}
			tstates += 10;
		}
		break;

	// CALL P,nn
	case simCALL_P:
		{
			ushort x = laddr();
			if ( !( regs.h.f & SF ) ) {
				putdata(--regs.x.sp, pc_ >> 8);
				putdata(--regs.x.sp, pc_ );
				pc_ = x;
				tstates += 7;
			}
			tstates += 10;
		}
		break;

	// CALL M,nn
	case simCALL_M:
		{
			ushort x = laddr();
			if ( regs.h.f & SF ) {
				putdata(--regs.x.sp, pc_ >> 8);
				putdata(--regs.x.sp, pc_ );
				pc_ = x;
				tstates += 7;
			}
			tstates += 10;
		}
		break;

	// RET
	case simRET:
		{
			pc_ = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
			tstates += 10;
		}
		break;

	// RET Z
	case simRET_Z:
		{
			if ( regs.h.f & ZF ) {
				pc_ = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
				tstates += 12;
			}
			tstates += 5;
		}
		break;

	// RET NZ
	case simRET_NZ:
		{
			if ( !( regs.h.f & ZF ) ) {
				pc_ = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
				tstates += 12;
			}
			tstates += 5;
		}
		break;

	// RET C
	case simRET_C:
		{
			if ( regs.h.f & CF ) {
				pc_ = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
				tstates += 12;
			}
			tstates += 5;
		}
		break;

	// RET NC
	case simRET_NC:
		{
			if ( !( regs.h.f & CF ) ) {
				pc_ = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
				tstates += 12;
			}
			tstates += 5;
		}
		break;

	// RET PE
	case simRET_PE:
		{
			if ( regs.h.f & PF ) {
				pc_ = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
				tstates += 12;
			}
			tstates += 5;
		}
		break;

	// RET PO
	case simRET_PO:
		{
			if ( !( regs.h.f & PF ) ) {
				pc_ = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
				tstates += 12;
			}
			tstates += 5;
		}
		break;

	// RET P
	case simRET_P:
		{
			if ( !( regs.h.f & SF ) ) {
				pc_ = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
				tstates += 12;
			}
			tstates += 5;
		}
		break;

	// RET M
	case simRET_M:
		{
			if ( regs.h.f & SF ) {
				pc_ = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
				tstates += 12;
			}
			tstates += 5;
		}
		break;

	// RETI
	case simRETI:
		{
			pc_ = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
			tstates += 14;
		}
		break;

	// RETN
	case simRETN:
		{
			iff1 = iff2;
			pc_ = (getdata(regs.x.sp++)) | (getdata(regs.x.sp++)<<8);
			tstates += 14;
		}
		break;

	// RST p
	case simRST:
		{
			putdata( --regs.x.sp, pc_ >> 8 );
			putdata( --regs.x.sp, pc_ );
			pc_ = instr.arg1;
			tstates += 11;
		}

		break;

	// INPUT AND OUTPUT GROUP /////////////////////////////////////////////////////

	// IN A,(n)
	case simIN_R_ATb:
		{
			ushort x = fetch();
			*Z80h[instr.arg1] = indata( x );
			tstates += 11;
		}
		break;

	// IN A,(n)
	case simIN_R_ATR:
		{
			ushort x = *Z80h[instr.arg1] = indata( *Z80h[instr.arg2] );
			regs.h.f = x & B_10101000;
			if ( parity( x ) )
				regs.h.f |= PF;
			if ( !x )
				regs.h.f |= ZF;
			tstates += 12;
		}
		break;

	// IN r,(C)

	// INI
	case simINI:
		{
			putdata( regs.x.hl, indata( regs.h.c ) );
			--regs.h.b;
			++regs.x.hl;
			resDEC_byte(regs.h.b); // temp: flags not all ok
			tstates += 16;
		}

		break;

	// INIR
	case simINIR:
		{
			putdata( regs.x.hl, indata( regs.h.c ) );
			++regs.x.hl;
			if ( --regs.h.b )
			{
				pc_ -= 2;
				tstates += 3;
			}

			resDEC_byte(regs.h.b); // temp: flags not all ok
			tstates += 16;
		}

		break;

	// IND
	// INI
	case simIND:
		{
			putdata( regs.x.hl, indata( regs.h.c ) );
			--regs.h.b;
			--regs.x.hl;
			resDEC_byte(regs.h.b); // temp: flags not all ok
			tstates += 16;
		}

		break;

	// INDR

	// OUT (n),a
	case simOUT_ATb_R:
		{
			ushort x = fetch();
			outdata( x, *Z80h[instr.arg2] );
			tstates += 11;
		}
		break;

	// OUT (C),r
	case simOUT_ATR_R:
		{
			// output to port
			outdata( *Z80h[instr.arg1], *Z80h[instr.arg2] );
			tstates += 12;
		}
		break;

	// OUTI
	case simOUTI:
		{
			outdata( regs.h.c, getdata( regs.x.hl ) );
			--regs.h.b;
			++regs.x.hl;
			resDEC_byte(regs.h.b); // temp: flags not all ok
			tstates += 16;
		}
		break;

	// OTIR
	case simOTIR:
		{
			outdata( regs.h.c, getdata( regs.x.hl ) );
			++regs.x.hl;
			if ( --regs.h.b )
			{
				pc_ -= 2;
				tstates += 3;
			}

			resDEC_byte(regs.h.b); // temp: flags not all ok
			tstates += 16;
		}
		break;

	// OUTD
	case simOUTD:
		{
			outdata( regs.h.c, getdata( regs.x.hl ) );
			--regs.h.b;
			--regs.x.hl;
			resDEC_byte(regs.h.b); // temp: flags not all ok
			tstates += 16;
		}
		break;

	// OTDR

	// MULTI-BYTE OPCODES /////////////////////////////////////////////////////////

	// Z-80 extended instructions set with CB prefix
	case simCB:
		{
			if (useixiy)
			{
				// Fetch offset for indexed memory access
				offset = sfetch() ;
			}
			else
			{
				incR();
			}

			opcode = fetch();
			simop( sInstrCB[opcode] );
		}
		break;

	// Z-80 extended instructions set with DD prefix (select IX register instead of HL)
	case simDD:
		{
			incR();
			selectIX();
			opcode = fetch();
			simop( sInstr[opcode] );
			selectHL();
		}
		break;

	// Z-80 extended instructions set with ED prefix
	case simED:
		{
			incR();
			opcode = fetch();
			simop( sInstrED[opcode] );
		}
		break;

	// Z-80 extended instructions set with DD prefix (select IY register instead of HL)
	case simFD:
		{
			incR();
			selectIY();
			opcode = fetch();
			simop( sInstr[opcode] );
			selectHL();
		}
		break;

	// Z-80 emulator pluggable extender
	case simZ80EXT:
		{
			if ( pCPUExtender_ )
				pCPUExtender_->sim( opcode );
		}
		break;

	default:
	// NULL => halt
		setMode( MODE_STOP );
		break;
	}

}

// PROCESSOR INSTRUCTIONS TABLE ///////////////////////////////////////////////

//  Processor's instruction set
Z80CPU::Instr Z80CPU::sInstr[] = {
// 00-0F
        simNOP,         0,              0,
        simLD_RX_w,     simBC,          0,
        simLD_ATRX_R,   simBC,          simA,
        simINC_RX,      simBC,          0,
        simINC_R,       simB,           0,
        simDEC_R,       simB,           0,
        simLD_R_b,      simB,           0,
        simRLCA,        0,              0,
        simEX_AF_AFP,   simAF,          0,
        simADD_RX_RX,   simHL,          simBC,
        simLD_R_ATRX,   simA,           simBC,
        simDEC_RX,      simBC,          0,
        simINC_R,       simC,           0,
        simDEC_R,       simC,           0,
        simLD_R_b,      simC,           0,
        simRRCA,        0,              0,
// 10-1F
        simDJNZ,        0,              0,
        simLD_RX_w,     simDE,          0,
        simLD_ATRX_R,   simDE,          simA,
        simINC_RX,      simDE,          0,
        simINC_R,       simD,           0,
        simDEC_R,       simD,           0,
        simLD_R_b,      simD,           0,
        simRLA,         0,              0,
        simJR,          0,              0,
        simADD_RX_RX,   simHL,          simDE,
        simLD_R_ATRX,   simA,           simDE,
        simDEC_RX,      simDE,          0,
        simINC_R,       simE,           0,
        simDEC_R,       simE,           0,
        simLD_R_b,      simE,           0,
        simRRA,         0,              0,
// 20-2F
        simJR_NZ,       0,              0,
        simLD_RX_w,     simHL,          0,
        simLD_ATw_RX,   0,              simHL,
        simINC_RX,      simHL,          0,
        simINC_R,       simH,           0,
        simDEC_R,       simH,           0,
        simLD_R_b,      simH,           0,
        simDAA,         0,              0,
        simJR_Z,        0,              0,
        simADD_RX_RX,   simHL,          simHL,
        simLD_RX_ATw,   simHL,          0,
        simDEC_RX,      simHL,          0,
        simINC_R,       simL,           0,
        simDEC_R,       simL,           0,
        simLD_R_b,      simL,           0,
        simCPL,         0,              0,
// 30-3F
        simJR_NC,       0,              0,
        simLD_RX_w,     simSP,          0,
        simLD_ATw_R,    0,              simA,
        simINC_RX,      simSP,          0,
        simINC_ATRX,    simHL,          0,
        simDEC_ATRX,    simHL,          0,
        simLD_ATRX_b,   simHL,          0,
        simSCF,         0,              0,
        simJR_C,        0,              0,
        simADD_RX_RX,   simHL,          simSP,
        simLD_R_ATw,    simA,           0,
        simDEC_RX,      simSP,          0,
        simINC_R,       simA,           0,
        simDEC_R,       simA,           0,
        simLD_R_b,      simA,           0,
        simCCF,         0,              0,
// 40-4F
        simLD_R_R,      simB,           simB,
        simLD_R_R,      simB,           simC,
        simLD_R_R,      simB,           simD,
        simLD_R_R,      simB,           simE,
        simLD_R_R,      simB,           simH,
        simLD_R_R,      simB,           simL,
        simLD_R_ATRX,   simB,           simHL,
        simLD_R_R,      simB,           simA,
        simLD_R_R,      simC,           simB,
        simLD_R_R,      simC,           simC,
        simLD_R_R,      simC,           simD,
        simLD_R_R,      simC,           simE,
        simLD_R_R,      simC,           simH,
        simLD_R_R,      simC,           simL,
        simLD_R_ATRX,   simC,           simHL,
        simLD_R_R,      simC,           simA,
// 50-5F
        simLD_R_R,      simD,           simB,
        simLD_R_R,      simD,           simC,
        simLD_R_R,      simD,           simD,
        simLD_R_R,      simD,           simE,
        simLD_R_R,      simD,           simH,
        simLD_R_R,      simD,           simL,
        simLD_R_ATRX,   simD,           simHL,
        simLD_R_R,      simD,           simA,
        simLD_R_R,      simE,           simB,
        simLD_R_R,      simE,           simC,
        simLD_R_R,      simE,           simD,
        simLD_R_R,      simE,           simE,
        simLD_R_R,      simE,           simH,
        simLD_R_R,      simE,           simL,
        simLD_R_ATRX,   simE,           simHL,
        simLD_R_R,      simE,           simA,
// 60-6F
        simLD_R_R,      simH,           simB,
        simLD_R_R,      simH,           simC,
        simLD_R_R,      simH,           simD,
        simLD_R_R,      simH,           simE,
        simLD_R_R,      simH,           simH,
        simLD_R_R,      simH,           simL,
        simLD_R_ATRX,   simH,           simHL,
        simLD_R_R,      simH,           simA,
        simLD_R_R,      simL,           simB,
        simLD_R_R,      simL,           simC,
        simLD_R_R,      simL,           simD,
        simLD_R_R,      simL,           simE,
        simLD_R_R,      simL,           simH,
        simLD_R_R,      simL,           simL,
        simLD_R_ATRX,   simL,           simHL,
        simLD_R_R,      simL,           simA,
// 70-7F
        simLD_ATRX_R,   simHL,          simB,
        simLD_ATRX_R,   simHL,          simC,
        simLD_ATRX_R,   simHL,          simD,
        simLD_ATRX_R,   simHL,          simE,
        simLD_ATRX_R,   simHL,          simH,
        simLD_ATRX_R,   simHL,          simL,
        simHALT,        0,              0,
        simLD_ATRX_R,   simHL,          simA,
        simLD_R_R,      simA,           simB,
        simLD_R_R,      simA,           simC,
        simLD_R_R,      simA,           simD,
        simLD_R_R,      simA,           simE,
        simLD_R_R,      simA,           simH,
        simLD_R_R,      simA,           simL,
        simLD_R_ATRX,   simA,           simHL,
        simLD_R_R,      simA,           simA,
// 80-8F
        simADD_R_R,     simA,           simB,
        simADD_R_R,     simA,           simC,
        simADD_R_R,     simA,           simD,
        simADD_R_R,     simA,           simE,
        simADD_R_R,     simA,           simH,
        simADD_R_R,     simA,           simL,
        simADD_R_ATRX,  simA,           simHL,
        simADD_R_R,     simA,           simA,
        simADC_R_R,     simA,           simB,
        simADC_R_R,     simA,           simC,
        simADC_R_R,     simA,           simD,
        simADC_R_R,     simA,           simE,
        simADC_R_R,     simA,           simH,
        simADC_R_R,     simA,           simL,
        simADC_R_ATRX,  simA,           simHL,
        simADC_R_R,     simA,           simA,
// 90-9F
        simSUB_R,       simB,           0,
        simSUB_R,       simC,           0,
        simSUB_R,       simD,           0,
        simSUB_R,       simE,           0,
        simSUB_R,       simH,           0,
        simSUB_R,       simL,           0,
        simSUB_ATRX,    simHL,          0,
        simSUB_R,       simA,           0,
        simSBC_R_R,     simA,           simB,
        simSBC_R_R,     simA,           simC,
        simSBC_R_R,     simA,           simD,
        simSBC_R_R,     simA,           simE,
        simSBC_R_R,     simA,           simH,
        simSBC_R_R,     simA,           simL,
        simSBC_R_ATRX,  simA,           simHL,
        simSBC_R_R,     simA,           simA,
// A0-AF
        simAND_R,       simB,           0,
        simAND_R,       simC,           0,
        simAND_R,       simD,           0,
        simAND_R,       simE,           0,
        simAND_R,       simH,           0,
        simAND_R,       simL,           0,
        simAND_ATRX,    simHL,          0,
        simAND_R,       simA,           0,
        simXOR_R,       simB,           0,
        simXOR_R,       simC,           0,
        simXOR_R,       simD,           0,
        simXOR_R,       simE,           0,
        simXOR_R,       simH,           0,
        simXOR_R,       simL,           0,
        simXOR_ATRX,    simHL,          0,
        simXOR_R,       simA,           0,
// B0-BF
        simOR_R,        simB,           0,
        simOR_R,        simC,           0,
        simOR_R,        simD,           0,
        simOR_R,        simE,           0,
        simOR_R,        simH,           0,
        simOR_R,        simL,           0,
        simOR_ATRX,     simHL,          0,
        simOR_R,        simA,           0,
        simCP_R,        simB,           0,
        simCP_R,        simC,           0,
        simCP_R,        simD,           0,
        simCP_R,        simE,           0,
        simCP_R,        simH,           0,
        simCP_R,        simL,           0,
        simCP_ATRX,     simHL,          0,
        simCP_R,        simA,           0,
// C0-CF
        simRET_NZ,      0,              0,
        simPOP_RX,      simBC,          0,
        simJP_NZ,       0,              0,
        simJP,          0,              0,
        simCALL_NZ,     0,              0,
        simPUSH_RX,     simBC,          0,
        simADD_R_b,     simA,           0,
        simRST,         0x00,           0,
        simRET_Z,       0,              0,
        simRET,         0,              0,
        simJP_Z,        0,              0,
        simCB,          0xCB,           0,
        simCALL_Z,      0,              0,
        simCALL,        0,              0,
        simADC_R_b,     simA,           0,
        simRST,         0x08,           0,
// D0-DF
        simRET_NC,      0,              0,
        simPOP_RX,      simDE,          0,
        simJP_NC,       0,              0,
        simOUT_ATb_R,   0,              simA,
        simCALL_NC,     0,              0,
        simPUSH_RX,     simDE,          0,
        simSUB_b,       0,              0,
        simRST,         0x10,           0,
        simRET_C,       0,              0,
        simEXX,         0,              0,
        simJP_C,        0,              0,
        simIN_R_ATb,    simA,           0,
        simCALL_C,      0,              0,
        simDD,          0xDD,           0,
        simSBC_R_b,     simA,           0,
        simRST,         0x18,           0,
// E0-EF
        simRET_PO,      0,              0,
        simPOP_RX,      simHL,          0,
        simJP_PO,       0,              0,
        simEX_ATRX_RX,  simSP,          simHL,
        simCALL_PO,     0,              0,
        simPUSH_RX,     simHL,          0,
        simAND_b,       0,              0,
        simRST,         0x20,           0,
        simRET_PE,      0,              0,
        simJP_ATRX,     simHL,          0,
        simJP_PE,       0,              0,
        simEX_RX_RX,    simDE,          simHL,
        simCALL_PE,     0,              0,
        simED,          0xED,           0,
        simXOR_b,       0,              0,
        simRST,         0x28,           0,
// F0-FF
        simRET_P,       0,              0,
        simPOP_RX,      simAF,          0,
        simJP_P,        0,              0,
        simDI,          0,              0,
        simCALL_P,      0,              0,
        simPUSH_RX,     simAF,          0,
        simOR_b,        0,              0,
        simRST,         0x30,           0,
        simRET_M,       0,              0,
        simLD_RX_RX,    simSP,          simHL,
        simJP_M,        0,              0,
        simEI,          0,              0,
        simCALL_M,      0,              0,
        simFD,          0xFD,           0,
        simCP_b,        0,              0,
        simRST,         0x38,           0,
// END
        0,				0,              0
};

Z80CPU::Instr Z80CPU::sInstrCB[] = {
// CB 00-0F
        simRLC_R,       simB,           0,
        simRLC_R,       simC,           0,
        simRLC_R,       simD,           0,
        simRLC_R,       simE,           0,
        simRLC_R,       simH,           0,
        simRLC_R,       simL,           0,
        simRLC_ATRX,    simHL,          0,
        simRLC_R,       simA,           0,
        simRRC_R,       simB,           0,
        simRRC_R,       simC,           0,
        simRRC_R,       simD,           0,
        simRRC_R,       simE,           0,
        simRRC_R,       simH,           0,
        simRRC_R,       simL,           0,
        simRRC_ATRX,    simHL,          0,
        simRRC_R,       simA,           0,
// CB 10-1F
        simRL_R,        simB,           0,
        simRL_R,        simC,           0,
        simRL_R,        simD,           0,
        simRL_R,        simE,           0,
        simRL_R,        simH,           0,
        simRL_R,        simL,           0,
        simRL_ATRX,     simHL,          0,
        simRL_R,        simA,           0,
        simRR_R,        simB,           0,
        simRR_R,        simC,           0,
        simRR_R,        simD,           0,
        simRR_R,        simE,           0,
        simRR_R,        simH,           0,
        simRR_R,        simL,           0,
        simRR_ATRX,     simHL,          0,
        simRR_R,        simA,           0,
// CB 20-2F
        simSLA_R,       simB,           0,
        simSLA_R,       simC,           0,
        simSLA_R,       simD,           0,
        simSLA_R,       simE,           0,
        simSLA_R,       simH,           0,
        simSLA_R,       simL,           0,
        simSLA_ATRX,    simHL,          0,
        simSLA_R,       simA,           0,
        simSRA_R,       simB,           0,
        simSRA_R,       simC,           0,
        simSRA_R,       simD,           0,
        simSRA_R,       simE,           0,
        simSRA_R,       simH,           0,
        simSRA_R,       simL,           0,
        simSRA_ATRX,    simHL,          0,
        simSRA_R,       simA,           0,
// CB 30-3F
        simSLL_R,       simB,           0,
        simSLL_R,       simC,           0,
        simSLL_R,       simD,           0,
        simSLL_R,       simE,           0,
        simSLL_R,       simH,           0,
        simSLL_R,       simL,           0,
        simSLL_ATRX,    simHL,          0,
        simSLL_R,       simA,           0,
        simSRL_R,       simB,           0,
        simSRL_R,       simC,           0,
        simSRL_R,       simD,           0,
        simSRL_R,       simE,           0,
        simSRL_R,       simH,           0,
        simSRL_R,       simL,           0,
        simSRL_ATRX,    simHL,          0,
        simSRL_R,       simA,           0,
// CB 40-4F
        simBIT_n_R,     0,              simB,
        simBIT_n_R,     0,              simC,
        simBIT_n_R,     0,              simD,
        simBIT_n_R,     0,              simE,
        simBIT_n_R,     0,              simH,
        simBIT_n_R,     0,              simL,
        simBIT_n_ATRX,  0,              simHL,
        simBIT_n_R,     0,              simA,
        simBIT_n_R,     1,              simB,
        simBIT_n_R,     1,              simC,
        simBIT_n_R,     1,              simD,
        simBIT_n_R,     1,              simE,
        simBIT_n_R,     1,              simH,
        simBIT_n_R,     1,              simL,
        simBIT_n_ATRX,  1,              simHL,
        simBIT_n_R,     1,              simA,
// CB 50-5F
        simBIT_n_R,     2,              simB,
        simBIT_n_R,     2,              simC,
        simBIT_n_R,     2,              simD,
        simBIT_n_R,     2,              simE,
        simBIT_n_R,     2,              simH,
        simBIT_n_R,     2,              simL,
        simBIT_n_ATRX,  2,              simHL,
        simBIT_n_R,     2,              simA,
        simBIT_n_R,     3,              simB,
        simBIT_n_R,     3,              simC,
        simBIT_n_R,     3,              simD,
        simBIT_n_R,     3,              simE,
        simBIT_n_R,     3,              simH,
        simBIT_n_R,     3,              simL,
        simBIT_n_ATRX,  3,              simHL,
        simBIT_n_R,     3,              simA,
// CB 60-6F
        simBIT_n_R,     4,              simB,
        simBIT_n_R,     4,              simC,
        simBIT_n_R,     4,              simD,
        simBIT_n_R,     4,              simE,
        simBIT_n_R,     4,              simH,
        simBIT_n_R,     4,              simL,
        simBIT_n_ATRX,  4,              simHL,
        simBIT_n_R,     4,              simA,
        simBIT_n_R,     5,              simB,
        simBIT_n_R,     5,              simC,
        simBIT_n_R,     5,              simD,
        simBIT_n_R,     5,              simE,
        simBIT_n_R,     5,              simH,
        simBIT_n_R,     5,              simL,
        simBIT_n_ATRX,  5,              simHL,
        simBIT_n_R,     5,              simA,
// CB 70-7F
        simBIT_n_R,     6,              simB,
        simBIT_n_R,     6,              simC,
        simBIT_n_R,     6,              simD,
        simBIT_n_R,     6,              simE,
        simBIT_n_R,     6,              simH,
        simBIT_n_R,     6,              simL,
        simBIT_n_ATRX,  6,              simHL,
        simBIT_n_R,     6,              simA,
        simBIT_n_R,     7,              simB,
        simBIT_n_R,     7,              simC,
        simBIT_n_R,     7,              simD,
        simBIT_n_R,     7,              simE,
        simBIT_n_R,     7,              simH,
        simBIT_n_R,     7,              simL,
        simBIT_n_ATRX,  7,              simHL,
        simBIT_n_R,     7,              simA,
// CB 80-8F
        simRES_n_R,     0,              simB,
        simRES_n_R,     0,              simC,
        simRES_n_R,     0,              simD,
        simRES_n_R,     0,              simE,
        simRES_n_R,     0,              simH,
        simRES_n_R,     0,              simL,
        simRES_n_ATRX,  0,              simHL,
        simRES_n_R,     0,              simA,
        simRES_n_R,     1,              simB,
        simRES_n_R,     1,              simC,
        simRES_n_R,     1,              simD,
        simRES_n_R,     1,              simE,
        simRES_n_R,     1,              simH,
        simRES_n_R,     1,              simL,
        simRES_n_ATRX,  1,              simHL,
        simRES_n_R,     1,              simA,
// CB 90-9F
        simRES_n_R,     2,              simB,
        simRES_n_R,     2,              simC,
        simRES_n_R,     2,              simD,
        simRES_n_R,     2,              simE,
        simRES_n_R,     2,              simH,
        simRES_n_R,     2,              simL,
        simRES_n_ATRX,  2,              simHL,
        simRES_n_R,     2,              simA,
        simRES_n_R,     3,              simB,
        simRES_n_R,     3,              simC,
        simRES_n_R,     3,              simD,
        simRES_n_R,     3,              simE,
        simRES_n_R,     3,              simH,
        simRES_n_R,     3,              simL,
        simRES_n_ATRX,  3,              simHL,
        simRES_n_R,     3,              simA,
// CB A0-AF
        simRES_n_R,     4,              simB,
        simRES_n_R,     4,              simC,
        simRES_n_R,     4,              simD,
        simRES_n_R,     4,              simE,
        simRES_n_R,     4,              simH,
        simRES_n_R,     4,              simL,
        simRES_n_ATRX,  4,              simHL,
        simRES_n_R,     4,              simA,
        simRES_n_R,     5,              simB,
        simRES_n_R,     5,              simC,
        simRES_n_R,     5,              simD,
        simRES_n_R,     5,              simE,
        simRES_n_R,     5,              simH,
        simRES_n_R,     5,              simL,
        simRES_n_ATRX,  5,              simHL,
        simRES_n_R,     5,              simA,
// CB B0-BF
        simRES_n_R,     6,              simB,
        simRES_n_R,     6,              simC,
        simRES_n_R,     6,              simD,
        simRES_n_R,     6,              simE,
        simRES_n_R,     6,              simH,
        simRES_n_R,     6,              simL,
        simRES_n_ATRX,  6,              simHL,
        simRES_n_R,     6,              simA,
        simRES_n_R,     7,              simB,
        simRES_n_R,     7,              simC,
        simRES_n_R,     7,              simD,
        simRES_n_R,     7,              simE,
        simRES_n_R,     7,              simH,
        simRES_n_R,     7,              simL,
        simRES_n_ATRX,  7,              simHL,
        simRES_n_R,     7,              simA,
// CB C0-CF
        simSET_n_R,     0,              simB,
        simSET_n_R,     0,              simC,
        simSET_n_R,     0,              simD,
        simSET_n_R,     0,              simE,
        simSET_n_R,     0,              simH,
        simSET_n_R,     0,              simL,
        simSET_n_ATRX,  0,              simHL,
        simSET_n_R,     0,              simA,
        simSET_n_R,     1,              simB,
        simSET_n_R,     1,              simC,
        simSET_n_R,     1,              simD,
        simSET_n_R,     1,              simE,
        simSET_n_R,     1,              simH,
        simSET_n_R,     1,              simL,
        simSET_n_ATRX,  1,              simHL,
        simSET_n_R,     1,              simA,
// CB D0-DF
        simSET_n_R,     2,              simB,
        simSET_n_R,     2,              simC,
        simSET_n_R,     2,              simD,
        simSET_n_R,     2,              simE,
        simSET_n_R,     2,              simH,
        simSET_n_R,     2,              simL,
        simSET_n_ATRX,  2,              simHL,
        simSET_n_R,     2,              simA,
        simSET_n_R,     3,              simB,
        simSET_n_R,     3,              simC,
        simSET_n_R,     3,              simD,
        simSET_n_R,     3,              simE,
        simSET_n_R,     3,              simH,
        simSET_n_R,     3,              simL,
        simSET_n_ATRX,  3,              simHL,
        simSET_n_R,     3,              simA,
// CB E0-EF
        simSET_n_R,     4,              simB,
        simSET_n_R,     4,              simC,
        simSET_n_R,     4,              simD,
        simSET_n_R,     4,              simE,
        simSET_n_R,     4,              simH,
        simSET_n_R,     4,              simL,
        simSET_n_ATRX,  4,              simHL,
        simSET_n_R,     4,              simA,
        simSET_n_R,     5,              simB,
        simSET_n_R,     5,              simC,
        simSET_n_R,     5,              simD,
        simSET_n_R,     5,              simE,
        simSET_n_R,     5,              simH,
        simSET_n_R,     5,              simL,
        simSET_n_ATRX,  5,              simHL,
        simSET_n_R,     5,              simA,
// CB F0-FF
        simSET_n_R,     6,              simB,
        simSET_n_R,     6,              simC,
        simSET_n_R,     6,              simD,
        simSET_n_R,     6,              simE,
        simSET_n_R,     6,              simH,
        simSET_n_R,     6,              simL,
        simSET_n_ATRX,  6,              simHL,
        simSET_n_R,     6,              simA,
        simSET_n_R,     7,              simB,
        simSET_n_R,     7,              simC,
        simSET_n_R,     7,              simD,
        simSET_n_R,     7,              simE,
        simSET_n_R,     7,              simH,
        simSET_n_R,     7,              simL,
        simSET_n_ATRX,  7,              simHL,
        simSET_n_R,     7,              simA,
// END
        0,				0,              0
};

Z80CPU::Instr Z80CPU::sInstrED[] = {
// ED 00-0F
        simHALT,        0xED,           0x00,
        simHALT,        0xED,           0x01,
        simHALT,        0xED,           0x02,
        simHALT,        0xED,           0x03,
        simHALT,        0xED,           0x04,
        simHALT,        0xED,           0x05,
        simHALT,        0xED,           0x06,
        simHALT,        0xED,           0x07,
        simHALT,        0xED,           0x08,
        simHALT,        0xED,           0x09,
        simHALT,        0xED,           0x0a,
        simHALT,        0xED,           0x0b,
        simHALT,        0xED,           0x0c,
        simHALT,        0xED,           0x0d,
        simHALT,        0xED,           0x0e,
        simHALT,        0xED,           0x0f,
// ED 10-1F
        simHALT,        0xED,           0x10,
        simHALT,        0xED,           0x11,
        simHALT,        0xED,           0x12,
        simHALT,        0xED,           0x13,
        simHALT,        0xED,           0x14,
        simHALT,        0xED,           0x15,
        simHALT,        0xED,           0x16,
        simHALT,        0xED,           0x17,
        simHALT,        0xED,           0x18,
        simHALT,        0xED,           0x19,
        simHALT,        0xED,           0x1a,
        simHALT,        0xED,           0x1b,
        simHALT,        0xED,           0x1c,
        simHALT,        0xED,           0x1d,
        simHALT,        0xED,           0x1e,
        simHALT,        0xED,           0x1f,
// ED 20-2F
		simZ80EXT,      0xED,           0x20,
		simZ80EXT,      0xED,           0x21,
		simZ80EXT,      0xED,           0x22,
		simZ80EXT,      0xED,           0x23,
		simZ80EXT,      0xED,           0x24,
		simZ80EXT,      0xED,           0x25,
		simZ80EXT,      0xED,           0x26,
		simZ80EXT,      0xED,           0x26,
		simZ80EXT,      0xED,           0x28,
		simZ80EXT,      0xED,           0x29,
		simZ80EXT,      0xED,           0x2A,
		simZ80EXT,      0xED,           0x2B,
		simZ80EXT,      0xED,           0x2C,
		simZ80EXT,      0xED,           0x2D,
		simZ80EXT,      0xED,           0x2E,
		simZ80EXT,      0xED,           0x2F,
// ED 30-3F
		simZ80EXT,      0xED,           0x30,
		simZ80EXT,      0xED,           0x31,
		simZ80EXT,      0xED,           0x32,
		simZ80EXT,      0xED,           0x33,
		simZ80EXT,      0xED,           0x34,
		simZ80EXT,      0xED,           0x35,
		simZ80EXT,      0xED,           0x36,
		simZ80EXT,      0xED,           0x36,
		simZ80EXT,      0xED,           0x38,
		simZ80EXT,      0xED,           0x39,
		simZ80EXT,      0xED,           0x3A,
		simZ80EXT,      0xED,           0x3B,
		simZ80EXT,      0xED,           0x3C,
		simZ80EXT,      0xED,           0x3D,
		simZ80EXT,      0xED,           0x3E,
		simZ80EXT,      0xED,           0x3F,
// ED 40-4F
        simIN_R_ATR,    simB,           simC,
		simOUT_ATR_R,   simC,           simB,
        simSBC_RX_RX,   simHL,          simBC,
        simLD_ATw_RX,   0,              simBC,
        simNEG,         0,              0,
        simRETN,        0,           	0,
        simIM,			0,				0,
        simLD_R_R,      simI,           simA,
        simIN_R_ATR,    simC,           simC,
		simOUT_ATR_R,   simC,           simC,
        simADC_RX_RX,   simHL,          simBC,
        simLD_RX_ATw,   simBC,          0,
        simHALT,        0xED,           0x4c,
		simRETI,        0,				0,
        simHALT,        0xED,           0x4e,
		simLD_R_R,      simR,           simA,	// TODO: special version of LD
// ED 50-5F
		simIN_R_ATR,    simD,           simC,
        simOUT_ATR_R,   simC,           simD,
        simSBC_RX_RX,   simHL,          simDE,
        simLD_ATw_RX,   0,              simDE,
        simHALT,        0xED,           0x54,
        simHALT,        0xED,           0x55,
        simIM,          1,              0,
        simLDS_A_R,     simA,           simI,
        simIN_R_ATR,    simE,           simC,
		simOUT_ATR_R,   simC,           simE,
        simADC_RX_RX,   simHL,          simDE,
        simLD_RX_ATw,   simDE,          0,
        simHALT,        0xED,           0x5c,
        simHALT,        0xED,           0x5d,
        simIM,			2,				0,
        simLDS_A_R,     simA,           simR,
// ED 60-6F
		simIN_R_ATR,    simH,           simC,
        simOUT_ATR_R,   simC,           simH,
        simSBC_RX_RX,   simHL,          simHL,
        simHALT,        0xED,           0x63,
        simHALT,        0xED,           0x64,
        simHALT,        0xED,           0x65,
        simHALT,        0xED,           0x66,
        simRRD,        	0,           	0,
		simIN_R_ATR,	simL,           simC,
        simOUT_ATR_R,   simC,           simL,
        simADC_RX_RX,   simHL,          simHL,
        simHALT,        0xED,           0x6b,
        simHALT,        0xED,           0x6c,
        simHALT,        0xED,           0x6d,
        simHALT,        0xED,           0x6e,
        simRLD,        	0,           	0,
// ED 70-7F
        simHALT,        0xED,           0x70,
        simHALT,        0xED,           0x71,
		simSBC_RX_RX,	simHL,          simSP,
        simLD_ATw_RX,   0,              simSP,
        simHALT,        0xED,           0x74,
        simHALT,        0xED,           0x75,
        simHALT,        0xED,           0x76,
        simHALT,        0xED,           0x77,
		simIN_R_ATR,    simA,           simC,
        simOUT_ATR_R,   simC,           simA,
        simADC_RX_RX,   simHL,          simSP,
        simLD_RX_ATw,   simSP,          0,
        simHALT,        0xED,           0x7c,
        simHALT,        0xED,           0x7d,
        simHALT,        0xED,           0x7e,
        simHALT,        0xED,           0x7f,
// ED 80-8F
        simHALT,        0xED,           0x80,
        simHALT,        0xED,           0x81,
        simHALT,        0xED,           0x82,
        simHALT,        0xED,           0x83,
        simHALT,        0xED,           0x84,
        simHALT,        0xED,           0x85,
        simHALT,        0xED,           0x86,
        simHALT,        0xED,           0x87,
        simHALT,        0xED,           0x88,
        simHALT,        0xED,           0x89,
        simHALT,        0xED,           0x8a,
        simHALT,        0xED,           0x8b,
        simHALT,        0xED,           0x8c,
        simHALT,        0xED,           0x8d,
        simHALT,        0xED,           0x8e,
        simHALT,        0xED,           0x8f,
// ED 90-9F
        simHALT,        0xED,           0x90,
        simHALT,        0xED,           0x91,
        simHALT,        0xED,           0x92,
        simHALT,        0xED,           0x93,
        simHALT,        0xED,           0x94,
        simHALT,        0xED,           0x95,
        simHALT,        0xED,           0x96,
        simHALT,        0xED,           0x97,
        simHALT,        0xED,           0x98,
        simHALT,        0xED,           0x99,
        simHALT,        0xED,           0x9a,
        simHALT,        0xED,           0x9b,
        simHALT,        0xED,           0x9c,
        simHALT,        0xED,           0x9d,
        simHALT,        0xED,           0x9e,
        simHALT,        0xED,           0x9f,
// ED A0-AF
        simLDI,        	0,           	0,
        simCPI,        	0,           	0,
        simINI,        	0,           	0,
        simOUTI,       	0,           	0,
        simHALT,        0xED,           0xa4,
        simHALT,        0xED,           0xa5,
        simHALT,        0xED,           0xa6,
        simHALT,        0xED,           0xa7,
        simLDD,         0,              0,
        simCPD,        	0,           	0,
        simIND,        	0,           	0,
        simOUTD,       	0,           	0,
        simHALT,        0xED,           0xac,
        simHALT,        0xED,           0xad,
        simHALT,        0xED,           0xae,
        simHALT,        0xED,           0xaf,
// ED B0-BF
        simLDIR,        0,              0,
        simCPIR,        0,           	0,
        simINIR,        0,				0,
        simOTIR,        0,				0,
        simHALT,        0xED,           0xb4,
        simHALT,        0xED,           0xb5,
        simHALT,        0xED,           0xb6,
        simHALT,        0xED,           0xb7,
        simLDDR,        0,              0,
        simCPDR,        0,           	0,
        simHALT,        0xED,           0xba,		//INDR
        simHALT,        0xED,           0xbb,		//OTDR
        simHALT,        0xED,           0xbc,
        simHALT,        0xED,           0xbd,
        simHALT,        0xED,           0xbe,
        simHALT,        0xED,           0xbf,
// ED C0-CF
        simHALT,        0xED,           0xc0,
        simHALT,        0xED,           0xc1,
        simHALT,        0xED,           0xc2,
        simHALT,        0xED,           0xc3,
        simHALT,        0xED,           0xc4,
        simHALT,        0xED,           0xc5,
        simHALT,        0xED,           0xc6,
        simHALT,        0xED,           0xc7,
        simHALT,        0xED,           0xc8,
        simHALT,        0xED,           0xc9,
        simHALT,        0xED,           0xca,
        simHALT,        0xED,           0xcb,
        simHALT,        0xED,           0xcc,
        simHALT,        0xED,           0xcd,
        simHALT,        0xED,           0xce,
        simHALT,        0xED,           0xcf,
// ED D0-DF
        simHALT,        0xED,           0xd0,
        simHALT,        0xED,           0xd1,
        simHALT,        0xED,           0xd2,
        simHALT,        0xED,           0xd3,
        simHALT,        0xED,           0xd4,
        simHALT,        0xED,           0xd5,
        simHALT,        0xED,           0xd6,
        simHALT,        0xED,           0xd7,
        simHALT,        0xED,           0xd8,
        simHALT,        0xED,           0xd9,
        simHALT,        0xED,           0xda,
        simHALT,        0xED,           0xdb,
        simHALT,        0xED,           0xdc,
        simHALT,        0xED,           0xdd,
        simHALT,        0xED,           0xde,
        simHALT,        0xED,           0xdf,
// ED E0-EF
        simHALT,        0xED,           0xe0,
        simHALT,        0xED,           0xe1,
        simHALT,        0xED,           0xe2,
        simHALT,        0xED,           0xe3,
        simHALT,        0xED,           0xe4,
        simHALT,        0xED,           0xe5,
        simHALT,        0xED,           0xe6,
        simHALT,        0xED,           0xe7,
        simHALT,        0xED,           0xe8,
        simHALT,        0xED,           0xe9,
        simHALT,        0xED,           0xea,
        simHALT,        0xED,           0xeb,
        simHALT,        0xED,           0xec,
        simHALT,        0xED,           0xed,
        simHALT,        0xED,           0xee,
        simHALT,        0xED,           0xef,
// ED F0-FF
		simZ80EXT,      0xED,           0xF0,
		simZ80EXT,      0xED,           0xF1,
		simZ80EXT,      0xED,           0xF2,
		simZ80EXT,      0xED,           0xF3,
		simZ80EXT,      0xED,           0xF4,
		simZ80EXT,      0xED,           0xF5,
		simZ80EXT,      0xED,           0xF6,
		simZ80EXT,      0xED,           0xF6,
		simZ80EXT,      0xED,           0xF8,
		simZ80EXT,      0xED,           0xF9,
		simZ80EXT,      0xED,           0xFA,
		simZ80EXT,      0xED,           0xFB,
		simZ80EXT,      0xED,           0xFC,
		simZ80EXT,      0xED,           0xFD,
		simZ80EXT,      0xED,           0xFE,
		simZ80EXT,      0xED,           0xFF,
// END
        0,				0,              0
};
