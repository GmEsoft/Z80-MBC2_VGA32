#pragma once

#include "File_I.h"
#include "Dir_I.h"
#include "FileDescriptor.h"

class FileSystem_I
{
public:

	FileSystem_I(void)
	{
	}

	virtual ~FileSystem_I(void)
	{
	}


	virtual File_I *open( const char *filename, const char *mode ) const = 0;

	virtual Dir_I *openDir( const char *dirname ) const = 0;

	virtual int open( const char *filename, int mode, int access ) const = 0;

	virtual int errno_() const = 0;

	virtual int close( int handle ) const = 0;

	virtual int read( int handle, void *buffer, unsigned length ) = 0;

	virtual int write( int handle, void *buffer, unsigned length ) = 0;

	virtual const char *strerror( int code ) const = 0;

	virtual int lseek( int handle, int offset, int ref ) = 0;
};
