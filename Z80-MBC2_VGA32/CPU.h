/*
    Z80-MBC2_VGA32 - 16-bit CPU Interface
    

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

// CPU API

#include "FileSystemProxy.h"

#include "runtime.h"
#include "Memory_I.h"
#include "Clock_I.h"
#include "InOut_I.h"
#include "Mode.h"
#include "Console_I.h"

class CPU : public FileSystemProxy
{
public:

	CPU()
		: code_( 0 ), mem_( 0 ), io_( 0 ), mode_( 0 ), clock_( 0 ), console_( 0 ), reader_( 0 ), writer_( 0 ), fetcher_( 0 )
	{
	}

	// set PC
	void setPC( uint pc )
	{
		pc_ = pc;
	}

	// get PC
	uint getPC()
	{
		return pc_;
	}

	// Set Memory Handler
	void setMemory( Memory_I *mem )
	{
		mem_ = mem;
		if ( !code_ )
			setCode( mem );
        reader_ = mem_->getReader();
        writer_ = mem_->getWriter();
        memobj_ = mem_->getObject();
	}

	// Get Memory Handler
	Memory_I *getMemory()
	{
		return mem_;
	}

	// Set Code Handler
	void setCode( Memory_I *mem )
	{
		code_ = mem;
        fetcher_ = mem->getReader();
        codeobj_ = mem->getObject();
	}

	// Set I/O Handler
	void setInOut( InOut_I *io )
	{
		io_ = io;
	}

	// Set Mode Handler
	void setMode( Mode *mode )
	{
		mode_ = mode;
	}

	// Set Clock
	void setClock( Clock_I *clock )
	{
		clock_ = clock;
	}

	// Set Console
	void setConsole( Console_I *console )
	{
		console_ = console;
	}

	uchar getcode( ushort addr )
	{
		return fetcher_ ? fetcher_( codeobj_, addr ) : code_ ? code_->read( addr ) : 0xFF;
	}

	uchar getdata( ushort addr )
	{
		return reader_ ? reader_( memobj_, addr ) : mem_ ? mem_->read( addr ) : 0xFF;
	}

	uchar putdata( ushort addr, uchar data )
	{
		return writer_ ? writer_( memobj_, addr, data ) : mem_ ? mem_->write( addr, data ) : data;
	}

	uchar indata( ushort addr )
	{
		return io_? io_->in( addr ) : 0xFF;
	}

	uchar outdata( ushort addr, uchar data )
	{
		return io_ ? io_->out( addr, data ) : data;
	}

	uchar fetch()
	{
		return getcode( pc_++ );
	}

	// Set mode
	void setMode( ExecMode pMode )
	{
		if ( mode_ )
			mode_->setMode( pMode );
	}

	// get mode
	ExecMode getMode()
	{
		return mode_ ? mode_->getMode() : MODE_RUN;
	}

	// wait
	void wait()
	{
		//...
	}

	// run cycles
	void runcycles( long cycles )
	{
		if ( clock_ )
			clock_->runCycles( cycles );
	}

	// write char to console
	int putch( int ch )
	{
		return console_ ? console_->putch( ch ) : 0;
	}

	// scan char from console
	int kbhit()
	{
		return console_ ? console_->kbhit() : 0;
	}

	// read char from console
	int getch()
	{
		return console_ ? console_->getch() : 0;
	}

	// write char to console
	int ungetch( int ch )
	{
		return console_ ? console_->ungetch( ch ) : 0;
	}

	// Trigger IRQ interrupt
	virtual void trigIRQ( char irq ) = 0;

	// Get IRQ status
	virtual char getIRQ( void ) = 0;

	// Trigger NMI interrupt
	virtual void trigNMI( void ) = 0;

	// Get NMI status
	virtual char getNMI( void ) = 0;

	// CPU Reset
	virtual void reset() = 0;

	// Execute 1 Statement
	virtual void sim() = 0;

	// destructor
	virtual ~CPU()
	{
	}

protected:
	ushort pc_;
	Console_I *console_;

private:
	Memory_I *mem_;
	Memory_I *code_;
    Memory_I::reader_t reader_;
    Memory_I::writer_t writer_;
    Memory_I::reader_t fetcher_;
    void *memobj_;
    void *codeobj_;
	InOut_I *io_;
	Mode *mode_;
	Clock_I *clock_;
};
