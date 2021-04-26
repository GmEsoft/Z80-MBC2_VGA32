#pragma once

#include "Term_I.h"

class TermProxy :
	public Term_I
{
public:

	TermProxy(void) : term_( 0 )
	{
	}

	virtual ~TermProxy(void)
	{
        LOGW( "Unexpected?" );
	}

	void setTerm( Term_I *term )
	{
		term_ = term;
	}

	int putch( int ch )
	{
		return term_ ? term_->putch( ch ) : 0;
	}

	int kbhit()
	{
		return term_ ? term_->kbhit() : 0;
	}

	int getch()
	{
		return term_ ? term_->getch() : 0;
	}

	int ungetch( int ch )
	{
		return term_ ? term_->ungetch( ch ) : 0;
	}

	int puts( const char *str )
	{
		if ( term_ )
			return term_->puts( str );;
		return 0;
	}

	char *gets( char *str )
	{
		if ( term_ )
			return term_->gets( str );;
		return 0;
	}

	int vprintf( const char *str, va_list args )
	{
		if ( term_ )
			return term_->vprintf( str, args );
		return 0;
	}

	void setScreenSize( int x, int y )
	{
		if ( term_ )
			term_->setScreenSize( x, y );			
	}
	
	void setFullScreen( bool fs )
	{
		if ( term_ )
			term_->setFullScreen( fs );			
	}
	
	void setWideMode( bool w )
	{
		if ( term_ )
			term_->setWideMode( w );			
	}
	
	int textAttr( const int c )
	{
		return term_ ? term_->textAttr( c ) : 0;
	}
	
	int curAttr()
	{
		return term_ ? term_->curAttr() : 0;
	}
	
	int clrScr()
	{
		return term_ ? term_->clrScr() : 0;
	}
	
	int clrEos()
	{
		return term_ ? term_->clrEos() : 0;
	}
	
	int clrEol()
	{
		return term_ ? term_->clrEol() : 0;
	}
	
	int gotoXY( int x, int y )
	{
		return term_ ? term_->gotoXY( x, y ) : 0;
	}
	
	int whereX()
	{
		return term_ ? term_->whereX() : 0;
	}
	
	int whereY()
	{
		return term_ ? term_->whereY() : 0;
	}

	int getTextInfo( text_info* ti )
	{
		return term_ ? term_->getTextInfo( ti ) : 0;
	}

	int getText( int left, int top, int right, int bottom, char* buffer )
	{
		return term_ ? term_->getText( left, top, right, bottom, buffer ) : 0;
	}

	int putText( int left, int top, int right, int bottom, const char* buffer )
	{
		return term_ ? term_->putText( left, top, right, bottom, buffer ) : 0;
	}
	
	int setCursorType( int curstype )
	{
		return term_ ? term_->setCursorType( curstype ) : 0;
	}

	int setGraphViewport( int x0, int y0, int x1, int y1 )
	{
		return term_ ? term_->setGraphViewport( x0, y0, x1, y1 ) : 0;
	}

	void setGraphOverlay( int p_GraphOverlay )
	{
		if ( term_ )
			term_->setGraphOverlay( p_GraphOverlay );
	}

	int selectFontBank( unsigned nPage, unsigned nBank )
	{
		return term_ ? term_->selectFontBank( nPage, nBank ) : 0;
	}

	int scanKbd( int row )
	{
		return term_ ? term_->scanKbd( row ) : 0;
	}

	int setKbdScanMode( int mode )
	{
		return term_ ? term_->setKbdScanMode( mode ) : 0;
	}

protected:
	Term_I		*term_;
};
