#include "AppStream.h"

String AppStreamImpl::input()
{
    String str = "";
    while(true) {
        int c = read();
        if ( c == '\r' ) {
            println();
            break;
        } else if ( ( c == '\b' || c == '\x7F' ) && !str.isEmpty() ) {
            str = str.substring( 0, str.length() - 1 );
            print( "\b \b" );
        } else if ( c == 24 ) {
            for ( int i=0; i<str.length(); ++i )
                print( "\b \b" );
            str.clear();
        } else if ( ( c >= ' ' && c < 0x7F ) ) {
            str += (char)c;
            print( (char)c );
        }
    }
    return str;
}

AppStreamImpl AppStream;
