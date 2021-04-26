#pragma once

class DirEntry
{
public:
	
	DirEntry() : name( 0 ), size( 0 ), isDir( false )
	{
	}

	const char *name;

	int size;

	bool isDir;

	operator bool()
	{
		return !!name;
	}
};
