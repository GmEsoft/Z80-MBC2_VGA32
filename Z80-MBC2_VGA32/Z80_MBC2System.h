#ifndef Z80_MBC2SYSTEM_H
#define Z80_MBC2SYSTEM_H

#include "System_I.h"
#include "SystemConsole.h"
#include "SystemClock.h"
#include "Ram32.h"
#include "Z80CPU.h"
#include "Z80Disassembler.h"

#include "TermCfg.h"
#include "SerialTerm.h"
#include "FileSystem_I.h"
#include "runtime.h"

#include "FileSystem_FabGL.h"
#include "log.h"

// ESP32 Core Headers
#include "HardwareSerial.h"

#pragma GCC optimize("03")

#define GME_EXT 1

typedef FileSystem_FabGL FATFS;
typedef uint8_t byte;
typedef unsigned int word;

#define RAM_SIZE 0x20000

// ------------------------------------------------------------------------------
//
// Hardware definitions for A040618 (Z80-MBC2) - Base system
//
// ------------------------------------------------------------------------------

#define   BANK1         11    // PD3 pin 17   RAM Memory bank address (High)
#define   BANK0         12    // PD4 pin 18   RAM Memory bank address (Low)

#define DEF_EEPROM 1

#if DEF_EEPROM
struct Eeprom
{
	Eeprom()
	{
		for ( size_t addr=0; addr<256; ++addr )
			eeprom[addr] = 0xFF;
	}

	void update( byte addr, byte data)
	{
		eeprom[addr] = data;
	}

	byte read( byte addr )
	{
		return eeprom[addr];
	}

	int load( FileSystem_I &fs )
	{
		int file = fs.open( "EEPROM.DAT", O_RDONLY | O_BINARY, S_IWRITE );
		int ret = fs.errno_();
		if ( !ret )
		{
			fs.read( file, eeprom, 256 );
			ret = fs.errno_();
			fs.close( file );
		}
		if ( ret )
		{
			Serial.print( "Loading EEPROM.DAT: " );
			Serial.println( fs.strerror( ret ) );
		}
		return ret;
	}

	int save( FileSystem_I &fs )
	{
		int file = fs.open( "EEPROM.DAT", O_CREAT | O_RDWR | O_BINARY, S_IWRITE );
		int ret = fs.errno_();
		if ( !ret )
		{
			fs.write( file, eeprom, 256 );
			ret = fs.errno_();
			fs.close( file );
		}
		if ( ret )
		{
			Serial.print( "Saving EEPROM.DAT: " );
			Serial.println( fs.strerror( ret ) );
		}
		return ret;
	}

	byte eeprom[256];
};

extern Eeprom EEPROM;
#endif

class Z80_MBC2System;

class Z80_MBC2RAM : public Memory_I
{
public:
	Z80_MBC2RAM(
		RAM32 &ram
		)
		: ram_( ram )
	{
	}

    // write byte
    virtual uchar write( ushort addr, uchar data );

    // read byte
    virtual uchar read( ushort addr );

    // get writer
    virtual writer_t getWriter()
    {
        LOGD( "getting writer" );
        return write;
    }

    // get reader
    virtual reader_t getReader()
    {
        LOGD( "getting reader" );
        return read;
    }

    // get object
    virtual void* getObject()
    {
        LOGD( "buffer %p - size: %x", buffer_, size_ );
        return buffer_;
    }

    // static writer
    static uchar write( void *obj, ushort addr, uchar data );

    // static reader
    static uchar read( void *obj, ushort addr );

    void update()
    {
        buffer_ = ram_.getBuffer();
        size_ = ram_.getSize();        
    }

private:
	RAM32	&ram_;
    uchar *buffer_;
    uint size_;

	uint ram_addr( ushort addr );
};

class Z80_MBC2IO : public InOut_I
{
public:
	Z80_MBC2IO
		( Console_I	&console
		, Term_I	&term
		, CPU		&cpu
		, Z80_MBC2System &system
		)
		: console_( console )
		, term_( term )
		, cpu_( cpu )
		, zero_( 0 )
		, system_( system )
	{
	}

	virtual uchar in( ushort addr );

	virtual uchar out( ushort addr, uchar data );

private:
	Console_I	&console_;
	Term_I		&term_;
	CPU			&cpu_;
	Z80_MBC2System &system_;
	uchar		zero_;
};

class Z80_MBC2System
	: public System_I
{
public:
	Z80_MBC2System(void)
		: cpuram_( ram_ )
		, cpuio_( systemConsole_, term_, cpu_, *this )
	{
        //term_.puts("Z80_MBC2System(void)...");
		systemConsole_.setSystem( this );
		systemConsole_.setConsole( &term_);
		cpu_.setMemory( &cpuram_ );
		cpu_.setInOut( &cpuio_ );
		cpu_.setConsole( &systemConsole_ );
		cpu_.setMode( &mode_ );
		//cpu_.setClock( &systemClock_ );
		disass_.setCode( &cpuram_ );
		//systemClock_.setClockSpeed( 8000 );
        //term_.puts("OK"); /!\ I/O not ready yet !!
	}

	~Z80_MBC2System(void)
	{
	}

	virtual void run()
    {
    }

    void setup();

    void loop();

	virtual void stop()
	{
		mode_.setMode( MODE_STOP );
	}

	virtual void reset();

	virtual void exit()
	{
		mode_.setMode( MODE_EXIT );
	}

	virtual void wakeup()
	{
	}

	virtual void step()
	{
	}

	virtual void callstep()
	{
	}

	uchar* data()
	{
		return data_;
	}

	ushort datasize()
	{
		return (ushort)RAM_SIZE;
	}

	Console_I& getConsole()
	{
		return systemConsole_;
	}

	SystemClock& getClock()
	{
		return systemClock_;
	}

    void setRamBuffer( void* data, size_t size );
    

private:
	Z80CPU				cpu_;
	RAM32				ram_;
	Z80_MBC2RAM			cpuram_;
	Z80_MBC2IO			cpuio_;
	uchar				*data_;
	SerialTerm			term_;
	Mode				mode_;
	SystemConsole		systemConsole_;
	Z80Disassembler		disass_;
	SystemClock			systemClock_;
};

#endif
