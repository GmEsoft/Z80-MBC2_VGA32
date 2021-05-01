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

#include "Z80Disassembler.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <search.h>

//  Enumerated constants for instructions, also array subscripts
enum {  NOP=0, LD, INC, DEC, ADD, SUB, ADC, SBC, AND, OR, XOR, RLCA, 
        RRCA, RLA, RRA, EX, EXX, DJNZ, JR, JP, CALL, RET, RST, CPL, NEG, SCF, CCF, 
        CP, IN, OUT, PUSH, POP, HALT, DI, EI, DAA, RLD, RRD, 
        RLC, RRC, RL, RR, SLA, SRA, SLL, SRL, BIT, RES, SET, 
        LDI, LDD, LDIR, LDDR, CPI, CPIR, CPD, CPDR, 
        INI, INIR, IND, INDR, OUTI, OTIR, OUTD, OTDR, IM, RETI, RETN, 
        BREAK, EXIT, CSINB, CSOUTC, CSEND, KBINC, KBWAIT, PUTCH, EIBRKON, EIBRKOFF, 
        DIV16, DIV32, IDIV32, 
        OPEN, CLOSE, READ, WRITE, SEEK, 
        GSPD, SSPD, ERROR, FINDFIRST, FINDNEXT, LOAD, 
        CHDIR, GETDIR, PATHOPEN, CLOSEALL, 
        DEFB};

//  Mnemonics for disassembler
static char mnemo[][9] = {
        "NOP", "LD", "INC", "DEC", "ADD", "SUB", "ADC", "SBC", "AND", "OR", "XOR", "RLCA", 
        "RRCA", "RLA", "RRA", "EX", "EXX", "DJNZ", "JR", "JP", "CALL", "RET", "RST", "CPL", "NEG", "SCF", "CCF", 
        "CP", "IN", "OUT", "PUSH", "POP", "HALT", "DI", "EI", "DAA", "RLD", "RRD", 
        "RLC", "RRC", "RL", "RR", "SLA", "SRA", "SLL", "SRL", "BIT", "RES", "SET", 
        "LDI", "LDD", "LDIR", "LDDR", "CPI", "CPIR", "CPD", "CPDR", 
        "INI", "INIR", "IND", "INDR", "OUTI", "OTIR", "OUTD", "OTDR", "IM", "RETI", "RETN", 
        "$BREAK", "$EXIT", "$CSINB", "$CSOUTC", "$CSEND", "$KBINC", "$KBWAIT", "$PUTCH", "$EIBON", "$EIBOFF", 
        "#DIV16", "#DIV32", "#IDIV32", 
		"#OPEN", "#CLOSE", "#READ", "#WRITE", "#SEEK C", 
		"#GSPD", "#SSPD", "#ERROR", "#FFST BC", "#FNXT BC", "#LOAD", 
        "#CHDIR", "#GDIR", "#PATHOPN", "#CLOSALL", 
        "DEFB"
        };

//  Enumerated constants for operands, also array subscripts
enum {R=1, RX, BYTE, WORD, OFFSET, ATR, ATRX, AFP, 
      Z, C, NZ, NC, PE, PO, P, M, ATBYTE, ATWORD, DIRECT};


enum {simA=1, simB, simC, simD, simE, simH, simL, simI, simR, simBC, simDE, simHL, simAF, simSP, simIR};

static char regnames[][3] = { "??", 
    "A", "B", "C", "D", "E", "H", "L", "I", "R", "BC", "DE", "HL", "AF", "SP", "IR"
    } ;

//  return hex-string or label for double-byte x( dasm )
const char* Z80Disassembler::getxaddr( uint x )
{
	static char addr[41];

	if ( symbols_ )
	{
		symbol_t *sym = symbols_->getSymbol( 'C', x );
		if ( sym )
		{
			strcpy( addr, sym->name );
			return addr;
		}
	}
	
	if ( x > 0x9FFF )
		sprintf( addr, "%05Xh", x );
	else
		sprintf( addr, "%04Xh", x );

	return addr;
}


//  return hex-string or label for DOS SVC x( dasm )
const char* Z80Disassembler::getsvc( uint x )
{
	static char addr[41];

	if ( symbols_ )
	{
		symbol_t *sym = symbols_->getSymbol( 'S', x );
		if ( sym )
		{
			strcpy( addr, sym->name );
			return addr;
		}
	}

	if ( x > 0x9F )
		sprintf( addr, "%03XH", x );
	else
		sprintf( addr, "%02XH", x );

	return addr;
}


// fetch long external address and return it as hex string or as label
const char* Z80Disassembler::getladdr()
{
	uint x;
	x = fetch();
	x += fetch() << 8;
	return getxaddr( x );
}

// fetch short relative external address and return it as hex string or as label
const char* Z80Disassembler::getsaddr()
{
	uint x;
	signed char d;
	d = (signed char)fetch();
	x = pc_ + d;
	return getxaddr( x );
}

// Get nth opcode( 1st or 2nd )
__inline int getopn( int opcode, int pos )
{ 
	return pos==1? Z80Disassembler::sInstr[opcode].opn1 : Z80Disassembler::sInstr[opcode].opn2 ;
}

// Get nth argument( 1st or 2nd )
__inline int getarg( int opcode, int pos )
{ 
	return pos==1? Z80Disassembler::sInstr[opcode].arg1 : Z80Disassembler::sInstr[opcode].arg2 ;
}

// return operand name or value
const char* Z80Disassembler::getoperand( int opcode, int pos )
{
	static char op[41];
	uint x;

	strcpy( op, "??" );

	switch ( getopn( opcode, pos ) )
	{
	case 0:
		return NULL;
	case R:
	case RX:
		return regnames[getarg( opcode, pos )] ;
	case WORD:
		return getladdr();
	case BYTE:
		x = fetch();
		if ( opcode == 0x3E && code_->read( pc_ ) == 0xEF )
		{
			// if	LD		A, @svc
			//		RST		28H
			// then get SVC label
			return getsvc( x );
		}
		if ( x>0x9F )
			sprintf( op, "%03Xh", x );
		else
			sprintf( op, "%02Xh", x );
		break;
	case ATR:
	case ATRX:
		strcpy( op, "(" );
		strcat( op, regnames[getarg( opcode, pos )] );
		strcat( op, ")" );
		break;
	case ATWORD:
		strcpy( op, "(" );
		strcat( op, getladdr() );
		strcat( op, ")" );
		break;
	case ATBYTE:
		x = fetch();
		if ( x>0x9F )
			sprintf( op, "(%03Xh)", x );
		else
			sprintf( op, "(%02Xh)", x );
		break;
	case OFFSET:
		return getsaddr();
	case DIRECT:
		sprintf( op, "%03Xh", getarg( opcode, pos ) );
		break;
	case Z:
		return "Z";
	case NZ:
		return "NZ";
	case C:
		return "C";
	case NC:
		return "NC";
	case PE:
		return "PE";
	case PO:
		return "PO";
	case P:
		return "P";
	case M:
		return "M";
	case AFP:
		return "AF'";
	}
	return op;
}

// get single instruction source
const char* Z80Disassembler::source()
{
	int opcode;
	static char src[80];
	char substr[41];
	int i;
	const char *op;
	signed char offset;

	useix = useiy = false;

	opcode = fetch();
	if ( opcode == 0xCB )
		opcode = 0x100 | fetch();
	else if ( opcode == 0xED )
		opcode = 0x200 | fetch();
	else if ( opcode == 0xDD )
	{
		useix = true;
		opcode = fetch();
	}
	else if ( opcode == 0xFD )
	{
		useiy = true;
		opcode = fetch();
	}

	if ( useix || useiy )
	{
		if ( opcode==0xE9 )
		{
			offset = 0;
		} 
		else if ( Z80Disassembler::sInstr[opcode].opn1 == ATRX || Z80Disassembler::sInstr[opcode].opn2 == ATRX )
		{
			offset = (signed char)fetch();
		} 
		else if ( opcode == 0xCB )
		{
			offset = (signed char)fetch();
			opcode = 0x100 | fetch();
		}
	}

	strcpy( src, mnemo[Z80Disassembler::sInstr[opcode].mnemon] );

	for ( i = (int)strlen( src ); i < 8; i++ )
	{
		src[i] = ' ';
	} /* endfor */

	src[i] = '\0';

	op = getoperand1( opcode );
	if ( op != NULL )
	{
		if ( ( useix || useiy ) && Z80Disassembler::sInstr[opcode].arg1 == simHL )
		{
			if ( Z80Disassembler::sInstr[opcode].opn1 == RX )
			{
				op = useix ? "IX" : "IY" ;
			} 
			else if ( Z80Disassembler::sInstr[opcode].opn1 == ATRX )
			{
				sprintf( substr, "(%s%+d)", useix ? "IX" : "IY", offset );
				op = substr ;
			}
		}
		
		strcat( src, op );
		op = getoperand2( opcode );
		
		if ( op != NULL )
		{
			strcat( src, "," );
			if ( ( useix || useiy ) && Z80Disassembler::sInstr[opcode].arg2 == simHL )
			{
				if ( Z80Disassembler::sInstr[opcode].opn2 == RX )
				{
					op = useix ? "IX" : "IY" ;
				} 
				else if ( Z80Disassembler::sInstr[opcode].opn2 == ATRX )
				{
					sprintf( substr, "(%s%+d)", useix ? "IX" : "IY", offset );
					op = substr ;
				}
			}
			strcat( src, op );
		}
	}

	for ( i = (int)strlen( src ); i < 32 ; i++ )
	{
		src[i] = ' ';
	}

	src[i] = '\0';

	return src;
}

//  Processor's instruction set
Z80Disassembler::Instr Z80Disassembler::sInstr[] = {
// 00-0F
        NOP,            0,              0,              0,              0,
        LD,             RX,             WORD,           simBC,          0,
        LD,             ATRX,           R,              simBC,          simA,
        INC,            RX,             0,              simBC,          0,
        INC,            R,              0,              simB,           0,
        DEC,            R,              0,              simB,           0,
        LD,             R,              BYTE,           simB,           0,
        RLCA,           0,              0,              0,              0,
        EX,             RX,             AFP,            simAF,          0,
        ADD,            RX,             RX,             simHL,          simBC,
        LD,             R,              ATRX,           simA,           simBC,
        DEC,            RX,             0,              simBC,          0,
        INC,            R,              0,              simC,           0,
        DEC,            R,              0,              simC,           0,
        LD,             R,              BYTE,           simC,           0,
        RRCA,           0,              0,              0,              0,
// 10-1F
        DJNZ,           OFFSET,         0,              0,              0,
        LD,             RX,             WORD,           simDE,          0,
        LD,             ATRX,           R,              simDE,          simA,
        INC,            RX,             0,              simDE,          0,
        INC,            R,              0,              simD,           0,
        DEC,            R,              0,              simD,           0,
        LD,             R,              BYTE,           simD,           0,
        RLA,            0,              0,              0,              0,
        JR,             OFFSET,         0,              0,              0,
        ADD,            RX,             RX,             simHL,          simDE,
        LD,             R,              ATRX,           simA,           simDE,
        DEC,            RX,             0,              simDE,          0,
        INC,            R,              0,              simE,           0,
        DEC,            R,              0,              simE,           0,
        LD,             R,              BYTE,           simE,           0,
        RRA,            0,              0,              0,              0,
// 20-2F
        JR,             NZ,             OFFSET,         0,              0,
        LD,             RX,             WORD,           simHL,          0,
        LD,             ATWORD,         RX,             0,              simHL,
        INC,            RX,             0,              simHL,          0,
        INC,            R,              0,              simH,           0,
        DEC,            R,              0,              simH,           0,
        LD,             R,              BYTE,           simH,           0,
        DAA,            0,              0,              0,              0,
        JR,             Z,              OFFSET,         0,              0,
        ADD,            RX,             RX,             simHL,          simHL,
        LD,             RX,             ATWORD,         simHL,          0,
        DEC,            RX,             0,              simHL,          0,
        INC,            R,              0,              simL,           0,
        DEC,            R,              0,              simL,           0,
        LD,             R,              BYTE,           simL,           0,
        CPL,            0,              0,              0,              0,
// 30-3F
        JR,             NC,             OFFSET,         0,              0,
        LD,             RX,             WORD,           simSP,          0,
        LD,             ATWORD,         R,              0,              simA,
        INC,            RX,             0,              simSP,          0,
        INC,            ATRX,           0,              simHL,          0,
        DEC,            ATRX,           0,              simHL,          0,
        LD,             ATRX,           BYTE,           simHL,          0,
        SCF,            0,              0,              0,              0,
        JR,             C,              OFFSET,         0,              0,
        ADD,            RX,             RX,             simHL,          simSP,
        LD,             R,              ATWORD,         simA,           0,
        DEC,            RX,             0,              simSP,          0,
        INC,            R,              0,              simA,           0,
        DEC,            R,              0,              simA,           0,
        LD,             R,              BYTE,           simA,           0,
        CCF,            0,              0,              0,              0,
// 40-4F
        LD,             R,              R,              simB,           simB,
        LD,             R,              R,              simB,           simC,
        LD,             R,              R,              simB,           simD,
        LD,             R,              R,              simB,           simE,
        LD,             R,              R,              simB,           simH,
        LD,             R,              R,              simB,           simL,
        LD,             R,              ATRX,           simB,           simHL,
        LD,             R,              R,              simB,           simA,
        LD,             R,              R,              simC,           simB,
        LD,             R,              R,              simC,           simC,
        LD,             R,              R,              simC,           simD,
        LD,             R,              R,              simC,           simE,
        LD,             R,              R,              simC,           simH,
        LD,             R,              R,              simC,           simL,
        LD,             R,              ATRX,           simC,           simHL,
        LD,             R,              R,              simC,           simA,
// 50-5F
        LD,             R,              R,              simD,           simB,
        LD,             R,              R,              simD,           simC,
        LD,             R,              R,              simD,           simD,
        LD,             R,              R,              simD,           simE,
        LD,             R,              R,              simD,           simH,
        LD,             R,              R,              simD,           simL,
        LD,             R,              ATRX,           simD,           simHL,
        LD,             R,              R,              simD,           simA,
        LD,             R,              R,              simE,           simB,
        LD,             R,              R,              simE,           simC,
        LD,             R,              R,              simE,           simD,
        LD,             R,              R,              simE,           simE,
        LD,             R,              R,              simE,           simH,
        LD,             R,              R,              simE,           simL,
        LD,             R,              ATRX,           simE,           simHL,
        LD,             R,              R,              simE,           simA,
// 60-6F
        LD,             R,              R,              simH,           simB,
        LD,             R,              R,              simH,           simC,
        LD,             R,              R,              simH,           simD,
        LD,             R,              R,              simH,           simE,
        LD,             R,              R,              simH,           simH,
        LD,             R,              R,              simH,           simL,
        LD,             R,              ATRX,           simH,           simHL,
        LD,             R,              R,              simH,           simA,
        LD,             R,              R,              simL,           simB,
        LD,             R,              R,              simL,           simC,
        LD,             R,              R,              simL,           simD,
        LD,             R,              R,              simL,           simE,
        LD,             R,              R,              simL,           simH,
        LD,             R,              R,              simL,           simL,
        LD,             R,              ATRX,           simL,           simHL,
        LD,             R,              R,              simL,           simA,
// 70-7F
        LD,             ATRX,           R,              simHL,          simB,
        LD,             ATRX,           R,              simHL,          simC,
        LD,             ATRX,           R,              simHL,          simD,
        LD,             ATRX,           R,              simHL,          simE,
        LD,             ATRX,           R,              simHL,          simH,
        LD,             ATRX,           R,              simHL,          simL,
        HALT,           0,              0,              0,              0,
        LD,             ATRX,           R,              simHL,          simA,
        LD,             R,              R,              simA,           simB,
        LD,             R,              R,              simA,           simC,
        LD,             R,              R,              simA,           simD,
        LD,             R,              R,              simA,           simE,
        LD,             R,              R,              simA,           simH,
        LD,             R,              R,              simA,           simL,
        LD,             R,              ATRX,           simA,           simHL,
        LD,             R,              R,              simA,           simA,
// 80-8F
        ADD,            R,              R,              simA,           simB,
        ADD,            R,              R,              simA,           simC,
        ADD,            R,              R,              simA,           simD,
        ADD,            R,              R,              simA,           simE,
        ADD,            R,              R,              simA,           simH,
        ADD,            R,              R,              simA,           simL,
        ADD,            R,              ATRX,           simA,           simHL,
        ADD,            R,              R,              simA,           simA,
        ADC,            R,              R,              simA,           simB,
        ADC,            R,              R,              simA,           simC,
        ADC,            R,              R,              simA,           simD,
        ADC,            R,              R,              simA,           simE,
        ADC,            R,              R,              simA,           simH,
        ADC,            R,              R,              simA,           simL,
        ADC,            R,              ATRX,           simA,           simHL,
        ADC,            R,              R,              simA,           simA,
// 90-9F
        SUB,            R,              0,              simB,           0,
        SUB,            R,              0,              simC,           0,
        SUB,            R,              0,              simD,           0,
        SUB,            R,              0,              simE,           0,
        SUB,            R,              0,              simH,           0,
        SUB,            R,              0,              simL,           0,
        SUB,            ATRX,           0,              simHL,          0,
        SUB,            R,              0,              simA,           0,
        SBC,            R,              R,              simA,           simB,
        SBC,            R,              R,              simA,           simC,
        SBC,            R,              R,              simA,           simD,
        SBC,            R,              R,              simA,           simE,
        SBC,            R,              R,              simA,           simH,
        SBC,            R,              R,              simA,           simL,
        SBC,            R,              ATRX,           simA,           simHL,
        SBC,            R,              R,              simA,           simA,
// A0-AF
        AND,            R,              0,              simB,           0,
        AND,            R,              0,              simC,           0,
        AND,            R,              0,              simD,           0,
        AND,            R,              0,              simE,           0,
        AND,            R,              0,              simH,           0,
        AND,            R,              0,              simL,           0,
        AND,            ATRX,           0,              simHL,          0,
        AND,            R,              0,              simA,           0,
        XOR,            R,              0,              simB,           0,
        XOR,            R,              0,              simC,           0,
        XOR,            R,              0,              simD,           0,
        XOR,            R,              0,              simE,           0,
        XOR,            R,              0,              simH,           0,
        XOR,            R,              0,              simL,           0,
        XOR,            ATRX,           0,              simHL,          0,
        XOR,            R,              0,              simA,           0,
// B0-BF
        OR,             R,              0,              simB,           0,
        OR,             R,              0,              simC,           0,
        OR,             R,              0,              simD,           0,
        OR,             R,              0,              simE,           0,
        OR,             R,              0,              simH,           0,
        OR,             R,              0,              simL,           0,
        OR,             ATRX,           0,              simHL,          0,
        OR,             R,              0,              simA,           0,
        CP,             R,              0,              simB,           0,
        CP,             R,              0,              simC,           0,
        CP,             R,              0,              simD,           0,
        CP,             R,              0,              simE,           0,
        CP,             R,              0,              simH,           0,
        CP,             R,              0,              simL,           0,
        CP,             ATRX,           0,              simHL,          0,
        CP,             R,              0,              simA,           0,
// C0-CF
        RET,            NZ,             0,              0,              0,
        POP,            RX,             0,              simBC,          0,
        JP,             NZ,             WORD,           0,              0,
        JP,             WORD,           0,              0,              0,
        CALL,           NZ,             WORD,           0,              0,
        PUSH,           RX,             0,              simBC,          0,
        ADD,            R,              BYTE,           simA,           0,
        RST,            DIRECT,         0,              0x00,           0,
        RET,            Z,              0,              0,              0,
        RET,            0,              0,              0,              0,
        JP,             Z,              WORD,           0,              0,
        DEFB,           DIRECT,         BYTE,           0xCB,           0,
        CALL,           Z,              WORD,           0,              0,
        CALL,           WORD,           0,              0,              0,
        ADC,            R,              BYTE,           simA,           0,
        RST,            DIRECT,         0,              0x08,           0,
// D0-DF
        RET,            NC,             0,              0,              0,
        POP,            RX,             0,              simDE,          0,
        JP,             NC,             WORD,           0,              0,
        OUT,            ATBYTE,         R,              0,              simA,
        CALL,           NC,             WORD,           0,              0,
        PUSH,           RX,             0,              simDE,          0,
        SUB,            BYTE,           0,              0,              0,
        RST,            DIRECT,         0,              0x10,           0,
        RET,            C,              0,              0,              0,
        EXX,            0,              0,              0,              0,
        JP,             C,              WORD,           0,              0,
        IN,             R,              ATBYTE,         simA,           0,
        CALL,           C,              WORD,           0,              0,
        DEFB,           DIRECT,         0,              0xDD,           0,
        SBC,            R,              BYTE,           simA,           0,
        RST,            DIRECT,         0,              0x18,           0,
// E0-EF
        RET,            PO,             0,              0,              0,
        POP,            RX,             0,              simHL,          0,
        JP,             PO,             WORD,           0,              0,
        EX,             ATRX,           RX,             simSP,          simHL,
        CALL,           PO,             WORD,           0,              0,
        PUSH,           RX,             0,              simHL,          0,
        AND,            BYTE,           0,              0,              0,
        RST,            DIRECT,         0,              0x20,           0,
        RET,            PE,             0,              0,              0,
        JP,             ATRX,           0,              simHL,          0,
        JP,             PE,             WORD,           0,              0,
        EX,             RX,             RX,             simDE,          simHL,
        CALL,           PE,             WORD,           0,              0,
        DEFB,           DIRECT,         BYTE,           0xED,           0,
        XOR,            BYTE,           0,              0,              0,
        RST,            DIRECT,         0,              0x28,           0,
// F0-FF
        RET,            P,              0,              0,              0,
        POP,            RX,             0,              simAF,          0,
        JP,             P,              WORD,           0,              0,
        DI,             0,              0,              0,              0,
        CALL,           P,              WORD,           0,              0,
        PUSH,           RX,             0,              simAF,          0,
        OR,             BYTE,           0,              0,              0,
        RST,            DIRECT,         0,              0x30,           0,
        RET,            M,              0,              0,              0,
        LD,             RX,             RX,             simSP,          simHL,
        JP,             M,              WORD,           0,              0,
        EI,             0,              0,              0,              0,
        CALL,           M,              WORD,           0,              0,
        DEFB,           DIRECT,         0,              0xFD,           0,
        CP,             BYTE,           0,              0,              0,
        RST,            DIRECT,         0,              0x38,           0,
// CB 00-0F
        RLC,            R,              0,              simB,           0,
        RLC,            R,              0,              simC,           0,
        RLC,            R,              0,              simD,           0,
        RLC,            R,              0,              simE,           0,
        RLC,            R,              0,              simH,           0,
        RLC,            R,              0,              simL,           0,
        RLC,            ATRX,           0,              simHL,          0,
        RLC,            R,              0,              simA,           0,
        RRC,            R,              0,              simB,           0,
        RRC,            R,              0,              simC,           0,
        RRC,            R,              0,              simD,           0,
        RRC,            R,              0,              simE,           0,
        RRC,            R,              0,              simH,           0,
        RRC,            R,              0,              simL,           0,
        RRC,            ATRX,           0,              simHL,          0,
        RRC,            R,              0,              simA,           0,
// CB 10-1F
        RL,             R,              0,              simB,           0,
        RL,             R,              0,              simC,           0,
        RL,             R,              0,              simD,           0,
        RL,             R,              0,              simE,           0,
        RL,             R,              0,              simH,           0,
        RL,             R,              0,              simL,           0,
        RL,             ATRX,           0,              simHL,          0,
        RL,             R,              0,              simA,           0,
        RR,             R,              0,              simB,           0,
        RR,             R,              0,              simC,           0,
        RR,             R,              0,              simD,           0,
        RR,             R,              0,              simE,           0,
        RR,             R,              0,              simH,           0,
        RR,             R,              0,              simL,           0,
        RR,             ATRX,           0,              simHL,          0,
        RR,             R,              0,              simA,           0,
// CB 20-2F
        SLA,            R,              0,              simB,           0,
        SLA,            R,              0,              simC,           0,
        SLA,            R,              0,              simD,           0,
        SLA,            R,              0,              simE,           0,
        SLA,            R,              0,              simH,           0,
        SLA,            R,              0,              simL,           0,
        SLA,            ATRX,           0,              simHL,          0,
        SLA,            R,              0,              simA,           0,
        SRA,            R,              0,              simB,           0,
        SRA,            R,              0,              simC,           0,
        SRA,            R,              0,              simD,           0,
        SRA,            R,              0,              simE,           0,
        SRA,            R,              0,              simH,           0,
        SRA,            R,              0,              simL,           0,
        SRA,            ATRX,           0,              simHL,          0,
        SRA,            R,              0,              simA,           0,
// CB 30-3F
        SLL,            R,              0,              simB,           0,
        SLL,            R,              0,              simC,           0,
        SLL,            R,              0,              simD,           0,
        SLL,            R,              0,              simE,           0,
        SLL,            R,              0,              simH,           0,
        SLL,            R,              0,              simL,           0,
        SLL,            ATRX,           0,              simHL,          0,
        SLL,            R,              0,              simA,           0,
        SRL,            R,              0,              simB,           0,
        SRL,            R,              0,              simC,           0,
        SRL,            R,              0,              simD,           0,
        SRL,            R,              0,              simE,           0,
        SRL,            R,              0,              simH,           0,
        SRL,            R,              0,              simL,           0,
        SRL,            ATRX,           0,              simHL,          0,
        SRL,            R,              0,              simA,           0,
// CB 40-4F
        BIT,            DIRECT,         R,              0,              simB,
        BIT,            DIRECT,         R,              0,              simC,
        BIT,            DIRECT,         R,              0,              simD,
        BIT,            DIRECT,         R,              0,              simE,
        BIT,            DIRECT,         R,              0,              simH,
        BIT,            DIRECT,         R,              0,              simL,
        BIT,            DIRECT,         ATRX,           0,              simHL,
        BIT,            DIRECT,         R,              0,              simA,
        BIT,            DIRECT,         R,              1,              simB,
        BIT,            DIRECT,         R,              1,              simC,
        BIT,            DIRECT,         R,              1,              simD,
        BIT,            DIRECT,         R,              1,              simE,
        BIT,            DIRECT,         R,              1,              simH,
        BIT,            DIRECT,         R,              1,              simL,
        BIT,            DIRECT,         ATRX,           1,              simHL,
        BIT,            DIRECT,         R,              1,              simA,
// CB 50-5F
        BIT,            DIRECT,         R,              2,              simB,
        BIT,            DIRECT,         R,              2,              simC,
        BIT,            DIRECT,         R,              2,              simD,
        BIT,            DIRECT,         R,              2,              simE,
        BIT,            DIRECT,         R,              2,              simH,
        BIT,            DIRECT,         R,              2,              simL,
        BIT,            DIRECT,         ATRX,           2,              simHL,
        BIT,            DIRECT,         R,              2,              simA,
        BIT,            DIRECT,         R,              3,              simB,
        BIT,            DIRECT,         R,              3,              simC,
        BIT,            DIRECT,         R,              3,              simD,
        BIT,            DIRECT,         R,              3,              simE,
        BIT,            DIRECT,         R,              3,              simH,
        BIT,            DIRECT,         R,              3,              simL,
        BIT,            DIRECT,         ATRX,           3,              simHL,
        BIT,            DIRECT,         R,              3,              simA,
// CB 60-6F
        BIT,            DIRECT,         R,              4,              simB,
        BIT,            DIRECT,         R,              4,              simC,
        BIT,            DIRECT,         R,              4,              simD,
        BIT,            DIRECT,         R,              4,              simE,
        BIT,            DIRECT,         R,              4,              simH,
        BIT,            DIRECT,         R,              4,              simL,
        BIT,            DIRECT,         ATRX,           4,              simHL,
        BIT,            DIRECT,         R,              4,              simA,
        BIT,            DIRECT,         R,              5,              simB,
        BIT,            DIRECT,         R,              5,              simC,
        BIT,            DIRECT,         R,              5,              simD,
        BIT,            DIRECT,         R,              5,              simE,
        BIT,            DIRECT,         R,              5,              simH,
        BIT,            DIRECT,         R,              5,              simL,
        BIT,            DIRECT,         ATRX,           5,              simHL,
        BIT,            DIRECT,         R,              5,              simA,
// CB 70-7F
        BIT,            DIRECT,         R,              6,              simB,
        BIT,            DIRECT,         R,              6,              simC,
        BIT,            DIRECT,         R,              6,              simD,
        BIT,            DIRECT,         R,              6,              simE,
        BIT,            DIRECT,         R,              6,              simH,
        BIT,            DIRECT,         R,              6,              simL,
        BIT,            DIRECT,         ATRX,           6,              simHL,
        BIT,            DIRECT,         R,              6,              simA,
        BIT,            DIRECT,         R,              7,              simB,
        BIT,            DIRECT,         R,              7,              simC,
        BIT,            DIRECT,         R,              7,              simD,
        BIT,            DIRECT,         R,              7,              simE,
        BIT,            DIRECT,         R,              7,              simH,
        BIT,            DIRECT,         R,              7,              simL,
        BIT,            DIRECT,         ATRX,           7,              simHL,
        BIT,            DIRECT,         R,              7,              simA,
// CB 80-8F
        RES,            DIRECT,         R,              0,              simB,
        RES,            DIRECT,         R,              0,              simC,
        RES,            DIRECT,         R,              0,              simD,
        RES,            DIRECT,         R,              0,              simE,
        RES,            DIRECT,         R,              0,              simH,
        RES,            DIRECT,         R,              0,              simL,
        RES,            DIRECT,         ATRX,           0,              simHL,
        RES,            DIRECT,         R,              0,              simA,
        RES,            DIRECT,         R,              1,              simB,
        RES,            DIRECT,         R,              1,              simC,
        RES,            DIRECT,         R,              1,              simD,
        RES,            DIRECT,         R,              1,              simE,
        RES,            DIRECT,         R,              1,              simH,
        RES,            DIRECT,         R,              1,              simL,
        RES,            DIRECT,         ATRX,           1,              simHL,
        RES,            DIRECT,         R,              1,              simA,
// CB 90-9F
        RES,            DIRECT,         R,              2,              simB,
        RES,            DIRECT,         R,              2,              simC,
        RES,            DIRECT,         R,              2,              simD,
        RES,            DIRECT,         R,              2,              simE,
        RES,            DIRECT,         R,              2,              simH,
        RES,            DIRECT,         R,              2,              simL,
        RES,            DIRECT,         ATRX,           2,              simHL,
        RES,            DIRECT,         R,              2,              simA,
        RES,            DIRECT,         R,              3,              simB,
        RES,            DIRECT,         R,              3,              simC,
        RES,            DIRECT,         R,              3,              simD,
        RES,            DIRECT,         R,              3,              simE,
        RES,            DIRECT,         R,              3,              simH,
        RES,            DIRECT,         R,              3,              simL,
        RES,            DIRECT,         ATRX,           3,              simHL,
        RES,            DIRECT,         R,              3,              simA,
// CB A0-AF
        RES,            DIRECT,         R,              4,              simB,
        RES,            DIRECT,         R,              4,              simC,
        RES,            DIRECT,         R,              4,              simD,
        RES,            DIRECT,         R,              4,              simE,
        RES,            DIRECT,         R,              4,              simH,
        RES,            DIRECT,         R,              4,              simL,
        RES,            DIRECT,         ATRX,           4,              simHL,
        RES,            DIRECT,         R,              4,              simA,
        RES,            DIRECT,         R,              5,              simB,
        RES,            DIRECT,         R,              5,              simC,
        RES,            DIRECT,         R,              5,              simD,
        RES,            DIRECT,         R,              5,              simE,
        RES,            DIRECT,         R,              5,              simH,
        RES,            DIRECT,         R,              5,              simL,
        RES,            DIRECT,         ATRX,           5,              simHL,
        RES,            DIRECT,         R,              5,              simA,
// CB B0-BF
        RES,            DIRECT,         R,              6,              simB,
        RES,            DIRECT,         R,              6,              simC,
        RES,            DIRECT,         R,              6,              simD,
        RES,            DIRECT,         R,              6,              simE,
        RES,            DIRECT,         R,              6,              simH,
        RES,            DIRECT,         R,              6,              simL,
        RES,            DIRECT,         ATRX,           6,              simHL,
        RES,            DIRECT,         R,              6,              simA,
        RES,            DIRECT,         R,              7,              simB,
        RES,            DIRECT,         R,              7,              simC,
        RES,            DIRECT,         R,              7,              simD,
        RES,            DIRECT,         R,              7,              simE,
        RES,            DIRECT,         R,              7,              simH,
        RES,            DIRECT,         R,              7,              simL,
        RES,            DIRECT,         ATRX,           7,              simHL,
        RES,            DIRECT,         R,              7,              simA,
// CB C0-CF
        SET,            DIRECT,         R,              0,              simB,
        SET,            DIRECT,         R,              0,              simC,
        SET,            DIRECT,         R,              0,              simD,
        SET,            DIRECT,         R,              0,              simE,
        SET,            DIRECT,         R,              0,              simH,
        SET,            DIRECT,         R,              0,              simL,
        SET,            DIRECT,         ATRX,           0,              simHL,
        SET,            DIRECT,         R,              0,              simA,
        SET,            DIRECT,         R,              1,              simB,
        SET,            DIRECT,         R,              1,              simC,
        SET,            DIRECT,         R,              1,              simD,
        SET,            DIRECT,         R,              1,              simE,
        SET,            DIRECT,         R,              1,              simH,
        SET,            DIRECT,         R,              1,              simL,
        SET,            DIRECT,         ATRX,           1,              simHL,
        SET,            DIRECT,         R,              1,              simA,
// CB D0-DF
        SET,            DIRECT,         R,              2,              simB,
        SET,            DIRECT,         R,              2,              simC,
        SET,            DIRECT,         R,              2,              simD,
        SET,            DIRECT,         R,              2,              simE,
        SET,            DIRECT,         R,              2,              simH,
        SET,            DIRECT,         R,              2,              simL,
        SET,            DIRECT,         ATRX,           2,              simHL,
        SET,            DIRECT,         R,              2,              simA,
        SET,            DIRECT,         R,              3,              simB,
        SET,            DIRECT,         R,              3,              simC,
        SET,            DIRECT,         R,              3,              simD,
        SET,            DIRECT,         R,              3,              simE,
        SET,            DIRECT,         R,              3,              simH,
        SET,            DIRECT,         R,              3,              simL,
        SET,            DIRECT,         ATRX,           3,              simHL,
        SET,            DIRECT,         R,              3,              simA,
// CB E0-EF
        SET,            DIRECT,         R,              4,              simB,
        SET,            DIRECT,         R,              4,              simC,
        SET,            DIRECT,         R,              4,              simD,
        SET,            DIRECT,         R,              4,              simE,
        SET,            DIRECT,         R,              4,              simH,
        SET,            DIRECT,         R,              4,              simL,
        SET,            DIRECT,         ATRX,           4,              simHL,
        SET,            DIRECT,         R,              4,              simA,
        SET,            DIRECT,         R,              5,              simB,
        SET,            DIRECT,         R,              5,              simC,
        SET,            DIRECT,         R,              5,              simD,
        SET,            DIRECT,         R,              5,              simE,
        SET,            DIRECT,         R,              5,              simH,
        SET,            DIRECT,         R,              5,              simL,
        SET,            DIRECT,         ATRX,           5,              simHL,
        SET,            DIRECT,         R,              5,              simA,
// CB F0-FF
        SET,            DIRECT,         R,              6,              simB,
        SET,            DIRECT,         R,              6,              simC,
        SET,            DIRECT,         R,              6,              simD,
        SET,            DIRECT,         R,              6,              simE,
        SET,            DIRECT,         R,              6,              simH,
        SET,            DIRECT,         R,              6,              simL,
        SET,            DIRECT,         ATRX,           6,              simHL,
        SET,            DIRECT,         R,              6,              simA,
        SET,            DIRECT,         R,              7,              simB,
        SET,            DIRECT,         R,              7,              simC,
        SET,            DIRECT,         R,              7,              simD,
        SET,            DIRECT,         R,              7,              simE,
        SET,            DIRECT,         R,              7,              simH,
        SET,            DIRECT,         R,              7,              simL,
        SET,            DIRECT,         ATRX,           7,              simHL,
        SET,            DIRECT,         R,              7,              simA,
// ED 00-0F
        DEFB,           DIRECT,         DIRECT,         0xED,           0x00,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x01,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x02,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x03,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x04,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x05,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x06,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x07,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x08,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x09,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x0a,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x0b,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x0c,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x0d,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x0e,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x0f,
// ED 10-1F
        DEFB,           DIRECT,         DIRECT,         0xED,           0x10,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x11,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x12,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x13,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x14,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x15,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x16,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x17,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x18,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x19,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x1a,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x1b,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x1c,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x1d,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x1e,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x1f,
// ED 20-2F
		DEFB,           DIRECT,         DIRECT,         0xED,           0x20,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x21,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x22,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x23,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x24,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x25,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x26,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x26,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x28,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x29,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x2A,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x2B,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x2C,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x2D,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x2E,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x2F,
// ED 30-3F
		DEFB,           DIRECT,         DIRECT,         0xED,           0x30,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x31,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x32,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x33,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x34,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x35,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x36,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x36,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x38,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x39,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x3A,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x3B,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x3C,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x3D,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x3E,
		DEFB,           DIRECT,         DIRECT,         0xED,           0x3F,
// ED 40-4F
        IN,				R,				ATR,			simB,           simC,
		OUT,           	ATR,         	R,         		simC,           simB,
        SBC,            RX,             RX,             simHL,          simBC,
        LD,             ATWORD,         RX,             0,              simBC,
        NEG,            0,              0,              0,              0,
        RETN,           0,         		0,         		0,           	0,
        IM,				0,				0,				0,				0,
        LD,           	R,         		R,         		simI,           simA,
        IN,				R,				ATR,			simC,           simC,
		OUT,			ATR,			R,				simC,           simC,
        ADC,           	RX,         	RX,         	simHL,          simBC,
        LD,             RX,             ATWORD,         simBC,          0,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x4c,
		RETI,           0,				0,				0,				0,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x4e,
		LD,				R,				R,				simR,           simA,	// TODO: special version of LD
// ED 50-5F
		IN,				R,				ATR,			simD,           simC,
        OUT,           	ATR,         	R,         		simC,           simD,
        SBC,            RX,             RX,             simHL,          simDE,
        LD,             ATWORD,         RX,             0,              simDE,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x54,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x55,
        IM,             DIRECT,         0,              1,              0,
        LD,           	R,         		R,         		simA,           simI,
        IN,				R,				ATR,			simE,           simC,
		OUT,			ATR,			R,				simC,           simE,
        ADC,            RX,             RX,             simHL,          simDE,
        LD,             RX,             ATWORD,         simDE,          0,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x5c,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x5d,
        IM,				DIRECT,         0,				2,				0,
        LD,             R,              R,              simA,           simR,
// ED 60-6F
		IN,				R,				ATR,			simH,           simC,
        OUT,            ATR,            R,              simC,           simH,
        SBC,            RX,             RX,             simHL,          simHL,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x63,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x64,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x65,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x66,
        RRD,           	0,         		0,         		0,           	0,
		IN,				R,				ATR,			simL,           simC,
        OUT,            ATR,            R,              simC,           simL,
        ADC,           	RX,         	RX,         	simHL,          simHL,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x6b,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x6c,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x6d,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x6e,
        RLD,           	0,         		0,         		0,           	0,
// ED 70-7F
        DEFB,           DIRECT,         DIRECT,         0xED,           0x70,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x71,
		SBC,			RX,				RX,				simHL,          simSP,
        LD,             ATWORD,         RX,             0,              simSP,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x74,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x75,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x76,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x77,
		IN,				R,				ATR,			simA,           simC,
        OUT,           	ATR,         	R,         		simC,           simA,
        ADC,			RX,				RX,				simHL,          simSP,
        LD,             RX,             ATWORD,         simSP,          0,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x7c,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x7d,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x7e,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x7f,
// ED 80-8F
        DEFB,           DIRECT,         DIRECT,         0xED,           0x80,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x81,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x82,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x83,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x84,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x85,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x86,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x87,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x88,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x89,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x8a,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x8b,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x8c,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x8d,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x8e,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x8f,
// ED 90-9F
        DEFB,           DIRECT,         DIRECT,         0xED,           0x90,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x91,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x92,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x93,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x94,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x95,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x96,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x97,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x98,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x99,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x9a,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x9b,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x9c,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x9d,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x9e,
        DEFB,           DIRECT,         DIRECT,         0xED,           0x9f,
// ED A0-AF
        LDI,           	0,         		0,         		0,           	0,
        CPI,           	0,         		0,         		0,           	0,
        INI,           	0,         		0,         		0,           	0,
        OUTI,           0,         		0,         		0,           	0,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xa4,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xa5,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xa6,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xa7,
        LDD,            0,              0,              0,              0,
        CPD,           	0,         		0,         		0,           	0,
        IND,           	0,         		0,         		0,           	0,
        OUTD,           0,         		0,         		0,           	0,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xac,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xad,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xae,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xaf,
// ED B0-BF
        LDIR,           0,              0,              0,              0,
        CPIR,           0,         		0,         		0,           	0,
        INIR,           0,				0,				0,				0,
        OTIR,           0,				0,				0,				0,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xb4,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xb5,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xb6,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xb7,
        LDDR,           0,              0,              0,              0,
        CPDR,           0,         		0,         		0,           	0,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xba,		//INDR
        DEFB,           DIRECT,         DIRECT,         0xED,           0xbb,		//OTDR
        DEFB,           DIRECT,         DIRECT,         0xED,           0xbc,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xbd,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xbe,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xbf,
// ED C0-CF
        DEFB,           DIRECT,         DIRECT,         0xED,           0xc0,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xc1,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xc2,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xc3,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xc4,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xc5,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xc6,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xc7,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xc8,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xc9,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xca,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xcb,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xcc,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xcd,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xce,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xcf,
// ED D0-DF
        DEFB,           DIRECT,         DIRECT,         0xED,           0xd0,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xd1,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xd2,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xd3,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xd4,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xd5,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xd6,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xd7,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xd8,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xd9,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xda,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xdb,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xdc,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xdd,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xde,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xdf,
// ED E0-EF
        DEFB,           DIRECT,         DIRECT,         0xED,           0xe0,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xe1,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xe2,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xe3,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xe4,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xe5,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xe6,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xe7,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xe8,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xe9,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xea,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xeb,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xec,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xed,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xee,
        DEFB,           DIRECT,         DIRECT,         0xED,           0xef,
// ED F0-FF
		DEFB,           DIRECT,         DIRECT,         0xED,           0xF0,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xF1,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xF2,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xF3,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xF4,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xF5,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xF6,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xF6,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xF8,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xF9,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xFA,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xFB,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xFC,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xFD,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xFE,
		DEFB,           DIRECT,         DIRECT,         0xED,           0xFF,
// END
        0,              0,              0,              0,						 0,              0
};
