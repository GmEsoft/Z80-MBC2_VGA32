#pragma once

#include "FileSystem_I.h"

#include "fabutils.h"

#include "stdio_undefs.h"

class FileSystem_FabGL :
    public FileSystem_I
{
public:
    FileSystem_FabGL();

    FileSystem_FabGL( fabgl::FileBrowser &fs );

    virtual ~FileSystem_FabGL( void );

    virtual File_I *open( const char *filename, const char *mode ) const;

    virtual Dir_I *openDir( const char *direname ) const;

    virtual int open( const char *filename, int mode, int access ) const;

    virtual int errno_() const;

    virtual int close( int handle ) const;

    virtual int read( int handle, void *buffer, unsigned length );

    virtual int write( int handle, void *buffer, unsigned length );

    virtual const char *strerror( int code ) const;

    virtual int lseek( int handle, int offset, int origin );
private:
    mutable const char  *strerror_;
    fabgl::FileBrowser &fs_;
};
