#pragma once

#define FS_NO_GLOBALS 1

class File_I
{
public:

	File_I(void)
	{
	}

	virtual ~File_I(void)
	{
	}

	virtual int close() = 0;

	virtual bool eof() const = 0;

	virtual int getc() = 0;

	virtual int putc( int c ) = 0;

	virtual int seek( long offset, int origin ) = 0;

	virtual int scanf( const char *format, ... ) = 0;

	virtual int printf( const char *format, ... ) = 0;

	virtual char* gets( char *buffer, int len ) = 0;

	virtual int puts( const char *str ) = 0;

	virtual const char *error() const = 0;

	virtual operator bool() const = 0;

};
