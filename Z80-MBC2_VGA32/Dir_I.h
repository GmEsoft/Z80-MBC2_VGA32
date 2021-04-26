#pragma once

#include "DirEntry.h"

#define FS_NO_GLOBALS 1

class Dir_I
{
public:

	Dir_I(void)
	{
	}

	virtual ~Dir_I(void)
	{
	}

	virtual DirEntry next() = 0;

	virtual int close() = 0;

	// Deprecated ?
	virtual const char *error() const = 0;

	virtual operator bool() const = 0;

};
