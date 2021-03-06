/*
    Z80-MBC2_VGA32 - File_I implementation for FabFL File System.
    

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

#include "File_FabGL.h"

#include "HardwareSerial.h"

#include "log.h"

#include <cstdarg>

static const char *NO_ERROR = "";
static const char *ERROR_INVALID = "Invalid Function";
static const char *ERROR_NOTOPEN = "Failed to open file";

File_FabGL::File_FabGL( FILE *file ) :
    file_( file ), strerror_( NO_ERROR )
{
    if ( !file_ )
    {
        strerror_ = ERROR_NOTOPEN;
        LOGE( "File not open" );
    }
}

File_FabGL::~File_FabGL(void)
{
    close();
}

int File_FabGL::close()
{
    if ( file_ )
    {
        fclose( file_ );
        file_ = 0;
        return 0;
    }
    LOGE( "failed" );
    return -1;
}

bool File_FabGL::eof() const
{
    return feof( file_ );
}

int File_FabGL::getc()
{
    return fgetc( file_ );
}

int File_FabGL::putc( int c )
{
//    Serial.println("File_FabGL::putc( int c )");
    return fputc( c, file_ );
}

int File_FabGL::seek( long offset, int origin )
{
//    Serial.println("File_FabGL::seek( long offset, int origin )");
    int seek = fseek( file_, offset, origin );
    return seek;
}


int File_FabGL::scanf( const char *format, ... )
{
/*
    va_list args;
    va_start( args, format );
    void *a[20];
    for ( int i = 0; i < sizeof( a ) / sizeof( a[0] ); ++i ) 
        a[i] = va_arg(args, void *);
    // no vfscanf() in VS before VS2013 :-(
    int ret = fscanf( file, format, 
        a[0], a[1], a[2], a[3], a[4], 
        a[5], a[6], a[7], a[8], a[9], 
        a[10], a[11], a[12], a[13], a[14], 
        a[15], a[16], a[17], a[18], a[19]
    );
    va_end( args );
    return ret;
*/
    strerror_ = ERROR_INVALID;
    LOGE( "not implemented" );
    return -1;
}

int File_FabGL::printf( const char *format, ... )
{
    strerror_ = ERROR_INVALID;
    LOGE( "not implemented" );
    return -1;
}

char* File_FabGL::gets( char *buffer, int len )
{
    int i;
    for ( i=0; i<len-1 && !eof(); ++i ) {
        char c = getc();
        if ( c == '\0' || c == '\r' || c == '\n' ) {
            buffer[i] = 0;
            break;
        }
        buffer[i] = c;
    }
    buffer[i] = 0;
    return buffer;
}

int File_FabGL::puts( const char *str )
{
    strerror_ = ERROR_INVALID;
    LOGE( "not implemented" );
    return -1;
}

const char *File_FabGL::error() const
{
    return strerror_;
}

File_FabGL::operator bool() const
{
    return file_;
}
