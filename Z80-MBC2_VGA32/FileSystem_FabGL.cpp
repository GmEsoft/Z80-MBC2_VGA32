/*
    Z80-MBC2_VGA32 - FileSystem_I implementation for FabGL
    

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

#include "FileSystem_FabGL.h"

#include "File_FabGL.h"
#include "Dir_FabGL.h"

#include "log.h"

#include "HardwareSerial.h"

#include <cstring>

static const char *NO_ERROR = "";
static const char *ERROR_NOT_FOUND = "File Not Found";
static const char *ERROR_INVALID = "Invalid Function";
static const char *ERROR_NOT_DIR = "File is not a directory";
static const char *ERROR_TOO_MANY = "Too many files open";

static const int NUM_FILES = 8;

static FILE *files[NUM_FILES];

#define SDCARD_MOUNT_PATH  "/SD"

#define MAXDIR 260

static fabgl::FileBrowser s_fs;

FileSystem_FabGL::FileSystem_FabGL() 
    : fs_( s_fs ), strerror_( NO_ERROR ), errno__( 0 )
{
    LOGD( "Mounting SD Card ..." );
    
    bool ok = fabgl::FileBrowser::mountSDCard( false, SDCARD_MOUNT_PATH );
    LOGD("%s", ok ? "OK" : "Failed!!" );
    
    LOGD("Cur dir: %s - count: %d", fs_.directory() ? fs_.directory() : "<nullptr>", fs_.count() );

    fs_.setDirectory( SDCARD_MOUNT_PATH );
    LOGD("Cur dir: %s - count: %d", fs_.directory() ? fs_.directory() : "<nullptr>", fs_.count() );
    
    for ( int i=0; i<fs_.count(); ++i )
    {
        auto *item = fs_.get( i );
        LOGV( "%4s %10d %s",
            item->isDir ? "Dir" : "File",
            item->isDir ? 0 : fs_.fileSize( item->name ),
            item->name );
    }
}

FileSystem_FabGL::FileSystem_FabGL( fabgl::FileBrowser &fs ) 
    : fs_( fs ), strerror_( NO_ERROR ), errno__( 0 )
{
}

FileSystem_FabGL::~FileSystem_FabGL(void)
{
//    fs_.close();
}


File_I *FileSystem_FabGL::open( const char *filename, const char *mode ) const
{
    char path[41] = "/";
    strcat( path, filename );
    LOGD( "path='%s'", path );
    FILE * file = fs_.openFile( path, "r+" );
    if ( file )
    {
        strerror_ = NO_ERROR;
        errno__ = 0;
        return new File_FabGL( file );
    }
    strerror_ = ERROR_NOT_FOUND;
    errno__ = 127; //TODO
    LOGE( "failed" );
    Serial.println( path );
    Serial.println( strerror_ );
    return NULL;
}

Dir_I *FileSystem_FabGL::openDir( const char *dirname ) const
{
    char path[41] = SDCARD_MOUNT_PATH"/";
    LOGD( "dirname='%s'", dirname );

    strcat( path, dirname );
    if ( strrchr( strrchr( path, '/' ), '?' ) )
        *( strrchr( path, '/' ) + 1 ) = '\0'; // strip mask
    LOGD( "path='%s'", path );
    
    bool ok = fs_.setDirectory( path );
    if ( !ok )
    {
        strerror_ = ERROR_NOT_FOUND;
        errno__ = 1;
        LOGE( "setDirectory failed" );
        return NULL;
    }

    strerror_ = NO_ERROR;
    errno__ = 0;
    return new Dir_FabGL( *this );
}

int FileSystem_FabGL::open( const char *filename, int mode, int access ) const
{
    char path[41] = "/";
    strcat( path, filename );
    strerror_ = ERROR_TOO_MANY;
    errno__ = 127;//TODO
    for ( int i=0; i<NUM_FILES; ++i )
    {
        if ( !files[i] )
        {
            files[i] = fs_.openFile( path, "r+" );
            if ( files[i] )
            {
                strerror_ = NO_ERROR;
                errno__ = 0;
                return i;
            }
            strerror_ = ERROR_NOT_FOUND;
            break;
        }
    }
    LOGE("%s: %s", path, strerror_ );
    return -1;
}

int FileSystem_FabGL::errno_() const
{
    return errno__;
}

void FileSystem_FabGL::set_errno_( int errno_ ) const
{
    errno__ = errno_;
}

int FileSystem_FabGL::close( int handle ) const
{  
    strerror_ = ERROR_INVALID;
    errno__ = 127;//TODO
    if ( handle>=0 && handle<NUM_FILES && files[handle] )
    {
        fclose( files[handle] );
        files[handle] = 0;
        strerror_ = NO_ERROR;
        errno__ = 0;
        return 0;
    }
    LOGE( "failed" );
    return -1;
}

int FileSystem_FabGL::read( int handle, void *buffer, unsigned length )
{
    strerror_ = ERROR_INVALID;
    errno__ = 127;//TODO
    if ( handle>=0 && handle<NUM_FILES && files[handle] )
    {
        strerror_ = NO_ERROR;
        errno__ = 0;
        return fread( (uint8_t *)buffer, sizeof(char), length, files[handle] );
    }
    LOGE( "failed" );
    return -1;
}

int FileSystem_FabGL::write( int handle, void *buffer, unsigned length )
{
    strerror_ = ERROR_INVALID;
    errno__ = 127;//TODO
    if ( handle>=0 && handle<NUM_FILES && files[handle] )
    {
        strerror_ = NO_ERROR;
        errno__ = 0;
        return fwrite( (const uint8_t *)buffer, sizeof(char), length, files[handle] );
    }
    LOGE( "failed" );
    return -1;
}

int FileSystem_FabGL::lseek( int handle, int offset, int origin )
{
    strerror_ = ERROR_INVALID;
    errno__ = 127;//TODO
    if ( handle>=0 && handle<NUM_FILES && files[handle] )
    {
        strerror_ = NO_ERROR;
        errno__ = 0;
        fseek( files[handle], offset, origin );
        return 0;
    }
    LOGE( "failed" );
    return -1;
}

const char *FileSystem_FabGL::strerror( int code ) const
{
    return strerror_;
}

int FileSystem_FabGL::getDisk() // TODO
{
    //return _getdrive();
    LOGW( "not implemented" );
    return 1;
}

const char* FileSystem_FabGL::getDir( int unit ) // TODO
{
    static char dir[MAXDIR] = "/";

//    char *curdir = _getdcwd( unit, dir, MAXDIR );
//    errno__ = errno;
//    errno = 0;
//
//    if ( !curdir )
//        dir[0] = 0;
//
    LOGW( "not implemented" );
    return dir;
}

int FileSystem_FabGL::chDir( const char *path ) // TODO
{
//    int ret = _chdir( path );
//    errno__ = errno;
//    errno = 0;
//    return ret;
    LOGW( "not implemented" );
    return 0;
}
