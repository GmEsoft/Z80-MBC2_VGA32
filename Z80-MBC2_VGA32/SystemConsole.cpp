#include "SystemConsole.h"

#include "runtime.h"

int SystemConsole::kbhit()
{
	if ( console_ && !console_->kbhit() )
		return ch_;

	ch_ = 0;
	ch_ = getch();
	return ch_;
}

int SystemConsole::getch()
{
	int ret = ch_;
	ch_ = 0;
	if ( ret )
	{
		return ret;
	}
	
	if ( console_ )
	{
		ret = console_->getch();
		if ( ret == -68 ) // F10 == stop
		{
			if ( system_ )
				system_->stop();				
		}
		else if ( ret == -93 ) // Sh-F10 = exit
		{
			if ( system_ )
				system_->exit();				
		}
		else if ( ret == -84 ) // Sh-F1 = reset
		{
			if ( system_ )
				system_->reset();				
		}
		else
			return ret;
	}
	return 0;
}

int SystemConsole::ungetch( int ch )
{
	return ch_ = ch;
}

int SystemConsole::putch( int ch )
{
	if ( console_ )
		return console_->putch( ch );
	return ch;
}

int SystemConsole::puts( const char *str )
{
	if ( console_ )
		return console_->puts( str );;
	return 0;
}

char *SystemConsole::gets( char *str )
{
	if ( console_ )
		return console_->gets( str );;
	return 0;
}

int SystemConsole::vprintf( const char *str, va_list args )
{
	if ( console_ )
		return console_->vprintf( str, args );
	return 0;
}
