#pragma once

#include "Term_I.h"

class SerialTerm : public Term_I
{
public:
    SerialTerm()
    {
    }

    virtual ~SerialTerm()
    {
        LOGW( "Unexpected?" );
    }

    static void begin();

    static void end();

    int input( const char *input );

    virtual int kbhit();

    virtual int getch();

    virtual int ungetch( int ch );

    virtual int putch( int ch );

    virtual int puts( const char *str );

    virtual char *gets( char *str );

    virtual int vprintf( const char *format, va_list args );

    virtual void setScreenSize( int x, int y );

    virtual void setFullScreen( bool fs );

    virtual int textAttr( const int c );

    virtual int curAttr();

    virtual int clrScr();

	virtual int clrEos();//new

    virtual int clrEol();

    virtual int gotoXY( int x, int y );

    virtual int whereX();

    virtual int whereY();

    virtual int getTextInfo( text_info* ti );

    virtual int getText( int left, int top, int right, int bottom, char* buffer );

    virtual int putText( int left, int top, int right, int bottom, const char* buffer );

    virtual int setCursorType( int curstype );

    virtual int setGraphViewport( int x0, int y0, int x1, int y1 );

    virtual void setGraphOverlay( int p_GraphOverlay );

    virtual int selectFontBank( unsigned nPage, unsigned nBank );

    virtual int scanKbd( int row );

    virtual int setKbdScanMode( int mode );

    virtual void setWideMode( bool wide );
};
