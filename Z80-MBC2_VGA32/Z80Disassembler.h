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
