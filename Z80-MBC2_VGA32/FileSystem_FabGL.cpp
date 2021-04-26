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

static fabgl::FileBrowser s_fs;

FileSystem_FabGL::FileSystem_FabGL() 
    : fs_( s_fs ), strerror_( NO_ERROR )
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
    : fs_( fs ), strerror_( NO_ERROR )
{
}

FileSystem_FabGL::~FileSystem_FabGL(void)
{
//    fs_.close();
}


File_I *FileSystem_FabGL::open( const char *filename, const char *mode ) const
{
    char path[41] = "/";
    strcpy( path+1, filename );
    LOGD( "path='%s'", path );
    FILE * file = fs_.openFile( path, "r+" );
    if ( file )
    {
        strerror_ = NO_ERROR;
        return new File_FabGL( file );
    }
    strerror_ = ERROR_NOT_FOUND;
    Serial.println( path );
    Serial.println( strerror_ );
    return NULL;
}

Dir_I *FileSystem_FabGL::openDir( const char *dirname ) const
{
    char path[41] = "/";
    strcpy( path+1, dirname );
    LOGD( "path='%s'", path );
    bool ok = fs_.setDirectory( path );
    if ( !ok )
    {
        strerror_ = ERROR_NOT_FOUND;
        return NULL;
    }

    strerror_ = NO_ERROR;
    return new Dir_FabGL( fs_ );
}

int FileSystem_FabGL::open( const char *filename, int mode, int access ) const
{
    char path[41] = "/";
    strcpy( path+1, filename );
    strerror_ = ERROR_TOO_MANY;
    for ( int i=0; i<NUM_FILES; ++i )
    {
        if ( !files[i] )
        {
            files[i] = fs_.openFile( path, "r+" );
            if ( files[i] )
            {
                strerror_ = NO_ERROR;
                return i;
            }
            strerror_ = ERROR_NOT_FOUND;
            break;
        }
    }
    LOGW("%s: %s", path, strerror_ );
    return -1;
}

int FileSystem_FabGL::errno_() const
{
    return *strerror_ ? 1 : 0;
}

int FileSystem_FabGL::close( int handle ) const
{  
    strerror_ = ERROR_INVALID;
    if ( handle>=0 && handle<NUM_FILES && files[handle] )
    {
        fclose( files[handle] );
        files[handle] = 0;
        strerror_ = NO_ERROR;
        return 0;
    }
    return -1;
}

int FileSystem_FabGL::read( int handle, void *buffer, unsigned length )
{
    strerror_ = ERROR_INVALID;
    if ( handle>=0 && handle<NUM_FILES && files[handle] )
    {
        strerror_ = NO_ERROR;
        return fread( (uint8_t *)buffer, sizeof(char), length, files[handle] );
    }
    return -1;
}

int FileSystem_FabGL::write( int handle, void *buffer, unsigned length )
{
    strerror_ = ERROR_INVALID;
    if ( handle>=0 && handle<NUM_FILES && files[handle] )
    {
        strerror_ = NO_ERROR;
        return fwrite( (const uint8_t *)buffer, sizeof(char), length, files[handle] );
    }
    return -1;
}

const char *FileSystem_FabGL::strerror( int code ) const
{
    return strerror_;
}

int FileSystem_FabGL::lseek( int handle, int offset, int origin )
{
    strerror_ = ERROR_INVALID;
    if ( handle>=0 && handle<NUM_FILES && files[handle] )
    {
        strerror_ = NO_ERROR;
        fseek( files[handle], offset, origin );
        return 0;
    }
    return -1;
}
