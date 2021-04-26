#pragma once

#include <cstring>
#include <string>

class FileTools
{
public:

	FileTools(void)
	{
	}

	~FileTools(void)
	{
	}

	static void addDefaultExt( char *name, const char *ext )
	{
		if	(	strrchr( name, '.' ) == 0
			||	strrchr( name, '.' ) < strrchr( name, '\\' )
			)
		{
			strcat( name, ext );
		}
	}

	static void addDefaultExt( std::string &name, const char *ext )
	{
		const char *cname = name.c_str();

		if	(	strrchr( cname, '.' ) == 0
			||	strrchr( cname, '.' ) < strrchr( cname, '\\' )
			)
		{
			name += ext;
		}
	}

};
