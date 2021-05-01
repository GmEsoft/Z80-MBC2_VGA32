/*
    Z80-MBC2_VGA32 - Real-Time Clock Support, with automatic setup
    via Wi-Fi and time server <pool.ntp.org>

    Configurable to:
    - set enabled;
    - GMT time offset in seconds;
    - DST offset in seconfs.
    Settings are stored in Preferences.
    

    Created by Michel Bernard (michel_bernard@hotmail.com) 
    - <http://www.github.com/GmEsoft/Z80-MBC2_VGA32>
    Copyright (c) 2021 Michel Bernard.
    All rights reserved.
    

    This file is part of Z80-MBC2_VGA32.
    
    Z80-MBC2_VGA32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "AppTime.h"

#include "AppWiFi.h"
#include "AppStream.h"
#include "log.h"

#include "Preferences.h"

extern Preferences preferences;

const char* ntpServer = "pool.ntp.org";

static uint32_t s_timeset  __NOINIT_ATTR;
static char s_timezone[33] __NOINIT_ATTR;

void AppTime::setup()
{

    LOGD( "s_timeset  == %x", s_timeset );

    if ( s_timeset == 0xa11c1ea2 || !isEnabled() )
    {
        if (s_timeset == 0xa11c1ea2 )
            setenv( "TZ", s_timezone, 1 );
        else
            strcpy( s_timezone, "" );
        AppStream.println( "Getting time" );
        struct tm timeinfo;
        if( getLocalTime( &timeinfo ) ) {
            AppStream.print(&timeinfo, "%A, %B %d %Y %H:%M:%S %Z ");
            AppStream.println(s_timezone);
        } else
          AppStream.println( "FAILED" );
        return;
    }

    s_timeset = 0xa11c1ea2;

    bool ok = AppWiFi::connect();
        
    if ( ok ) {
        long  gmtOffset_sec = getGmtOffset_sec();
        int   daylightOffset_sec = getDstOffset_sec();

        //init and get the time
        struct tm timeinfo;
        AppStream.print( "Obtaining time " );
        ok = false;
        for (int i=0; i<5; ++i) {
            configTime( gmtOffset_sec, daylightOffset_sec, ntpServer );
            strncpy( s_timezone, getenv( "TZ" ), sizeof(s_timezone) );
            if( getLocalTime( &timeinfo ) ) {
                AppStream.print(&timeinfo, "%A, %B %d %Y %H:%M:%S ");
                AppStream.println(s_timezone);
                ok = true;
                break;
            }
            delay( 1000 );
            AppStream.print(".");
        }
        
        AppStream.println( ok ? " OK" : " FAILED" );
    }

    AppWiFi::disconnect();

    preferences.end();
    ESP.restart();            
    
}

bool AppTime::configure()
{
    bool  setTime = preferences.getBool("time", false);
    long  gmtOffset_sec = preferences.getLong("gmtOffset_sec", 0);
    int   daylightOffset_sec = preferences.getInt("dstOffset_sec", 0);

    String str;

    AppStream.printf("auto set time      [%c]: ", setTime ? 'Y' : 'N' );
    while ( AppStream.available() ) 
      AppStream.read();
    str = AppStream.input();
    if ( !str.isEmpty() )
        setTime = toupper( str.c_str()[0] ) == 'Y';
    preferences.putBool("time",setTime);

    if ( setTime ) {
        AppStream.printf("gmtOffset_sec      [%ld]: ", gmtOffset_sec );
        while ( AppStream.available() ) 
          AppStream.read();
        str = AppStream.input();
        if ( !str.isEmpty() )
            gmtOffset_sec = str.toInt();
        preferences.putLong("gmtOffset_sec",gmtOffset_sec);

        AppStream.printf("daylightOffset_sec [%ld]: ", daylightOffset_sec );
        while ( AppStream.available() ) 
          AppStream.read();
        str = AppStream.input();
        if ( !str.isEmpty() )
            daylightOffset_sec = str.toInt();
        preferences.putInt("dstOffset_sec",daylightOffset_sec);
    }

    return false;
    
}

bool AppTime::isEnabled()
{
    return preferences.getBool( "time", false );
}

void AppTime::setEnabled( bool enabled )
{
    preferences.putBool( "time", enabled );
}


long AppTime::getGmtOffset_sec()
{
    return preferences.getLong("gmtOffset_sec", 0);
}

void AppTime::setEnabled( long gmtOffset_sec )
{
    preferences.putLong( "gmtOffset_sec", gmtOffset_sec );
}

int AppTime::getDstOffset_sec()
{
    return preferences.getInt("dstOffset_sec", 0);
}

void AppTime::setEnabled( int dstOffset_sec )
{
    preferences.putInt( "dstOffset_sec", dstOffset_sec );
}
