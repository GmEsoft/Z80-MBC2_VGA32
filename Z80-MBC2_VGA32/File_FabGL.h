#pragma once

#include "File_I.h"

#include <cstdio>

#include "stdio_undefs.h"

class File_FabGL :
    public File_I
{
public:
    File_FabGL( FILE *file );

    virtual ~File_FabGL(void);

    virtual int close();

    virtual bool eof() const;

    virtual int getc();

    virtual int putc( int c );

    virtual int seek( long offset, int origin );

    virtual int scanf( const char *format, ... );

    virtual int printf( const char *format, ... );

    virtual char* gets( char *buffer, int len );

    virtual int puts( const char *str );

    virtual const char *error() const;

    virtual operator bool() const;

private:
    FILE        *file_;
    const char  *strerror_;
};
