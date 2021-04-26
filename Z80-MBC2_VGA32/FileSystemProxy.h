#pragma once

#include "FileSystem_I.h"

class FileSystemProxy :
	public FileSystem_I
{
public:

	FileSystemProxy(void)
		: fileSystem_( 0 )
	{
	}

	virtual ~FileSystemProxy(void)
	{
	}

	void setFileSystem( FileSystem_I *fileSystem )
	{
		fileSystem_ = fileSystem;
	}

    virtual File_I *open( const char *filename, const char *mode ) const
    {
        return fileSystem_ ? fileSystem_->open( filename, mode ) : 0;
    }

    virtual Dir_I *openDir( const char *dirname ) const
    {
        return fileSystem_ ? fileSystem_->openDir( dirname ) : 0;
    }

	virtual int open( const char *filename, int mode, int access ) const
	{
		return fileSystem_ ? fileSystem_->open( filename, mode, access ) : -1;
	}

	virtual int errno_() const
	{
		return fileSystem_ ? fileSystem_->errno_() : -1;
	}

	virtual int close( int handle ) const
	{
		return fileSystem_ ? fileSystem_->close( handle ) : -1;
	}

	virtual int read( int handle, void *buffer, unsigned length )
	{
		return fileSystem_ ? fileSystem_->read( handle, buffer, length ) : -1;
	}

	virtual int write( int handle, void *buffer, unsigned length )
	{
		return fileSystem_ ? fileSystem_->write( handle, buffer, length ) : -1;
	}

	virtual const char *strerror( int code ) const
	{
		return fileSystem_ ? fileSystem_->strerror( code ) : "no filesystem";
	}

	virtual int lseek( int handle, int offset, int origin )
	{
		return fileSystem_ ? fileSystem_->lseek( handle, offset, origin ) : -1;
	}

protected:
	FileSystem_I *fileSystem_;

};
