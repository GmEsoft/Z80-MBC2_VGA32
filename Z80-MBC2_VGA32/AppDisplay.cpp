/*
    Z80-MBC2_VGA32 - Raster Display Driver
    for TRS-80 Model 4 16x64 or 24x80 text modes
    with support for enhanced color modes.


    Created by Michel Bernard (michel_bernard@hotmail.com)
    - <http://www.github.com/GmEsoft/Z80-MBC2_VGA32>
    Copyright (c) 2021 Michel Bernard.
    All rights reserved.


    This file is part of Z80-MBC2_VGA32.

    Z80-MBC2_VGA32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Z80-MBC2_VGA32 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Z80-MBC2_VGA32.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "AppDisplay.h"

#include "AppStream.h"
#include "log.h"

#include "Preferences.h"

extern Preferences preferences;

fabgl::Terminal         Terminal;

void AppDisplay::setup()
{
    // setup VGA
    int nResolution = preferences.getInt( "resolution", 0 );
    LOGD( "resolution #%d", nResolution );
    Serial.printf( "Display Resolution : #%d %s\r\n", nResolution, getDisplayMode( nResolution ).c_str() );

    // Instantiate sound generator (BEL)
    Terminal.soundGenerator();

    if ( nResolution )
    {
        LOGD( "Loading VGA Controller" );
        fabgl::BaseDisplayController *displayController
            = nResolution > 7 ? (fabgl::BaseDisplayController *)new fabgl::VGA4Controller
            : nResolution > 6 ? (fabgl::BaseDisplayController *)new fabgl::VGA8Controller
            : (fabgl::BaseDisplayController *)new fabgl::VGA16Controller;
        displayController->begin();
        displayController->setResolution( getResolution( nResolution ) );
        Terminal.begin(displayController);
    }
    else
    {
        LOGD( "Loading VGA Text Controller" );
        fabgl::VGATextController *displayController = new fabgl::VGATextController;
        displayController->begin();
        displayController->setResolution();
        Terminal.begin(displayController);
    }

    LOGD( "Terminal.connectLocally()" );
    Terminal.connectLocally();      // to use Terminal.read(), available(), etc..

    LOGD( "Terminal.setBackgroundColor()" );
    Terminal.setBackgroundColor(Color::Black);
    LOGD( "Terminal.setForegroundColor()" );
    Terminal.setForegroundColor(Color::BrightGreen);
    LOGD( "Terminal.clear()" );
    Terminal.clear();

    LOGD( "Terminal.enableCursor()" );
    Terminal.enableCursor(true);

}

bool AppDisplay::configure()
{
    bool restart = false;
    AppStream.println();
    AppStream.println("Available Resolutions");
    AppStream.println("=====================");
    AppStream.println();
    int nResolution = preferences.getInt( "resolution", 0 );
    const char *resolution ;
    for ( int i=0; resolution = getResolution( i ); ++i )
    {
        AppStream.printf( "%2s [%d] %s", i == nResolution ? ">>" : "", i, getDisplayMode( i ).c_str() );
        AppStream.println();
    }
    AppStream.print("==> ");
    while ( AppStream.available() )
      AppStream.read();
    int c;
    while ( !isalnum( c = AppStream.read() ) );
    if ( isdigit( c ) && getResolution( c - '0' ) )
    {
        AppStream.println("Resolution changed -- restart needed");
        nResolution = c - '0';
        preferences.putInt( "resolution", nResolution );
        restart = true;
    }
    return restart;
}

String AppDisplay::getDisplayMode( int i )
{
    const char *resolution = getResolution( i ) + 1 ;
    return String( resolution ).substring( 0, strchr( resolution, '"' ) - resolution );
}

const char * AppDisplay::getResolution( int n )
{
    switch( n )
    {
    case 0:
        return "\"640x480 Text Only\"";
    case 1:
        return VGA_480x300_75Hz;
    case 2:
        return VGA_640x200_70Hz;
    case 3:
        return VGA_640x200_70HzRetro;
    case 4:
        return VGA_640x240_60Hz;
    case 5:
        return VGA_640x350_70Hz;
    case 6:
        return VGA_640x350_70HzAlt1;
    case 7:
        return VGA_640x400_70Hz;
    case 8:
        return VGA_640x480_60Hz;
    case 9:
        return VESA_640x480_75Hz;
    default:
        return 0;
    }
}

int AppDisplay::getDisplayMode()
{
    return preferences.getInt( "resolution", 0 );
}

void AppDisplay::setDisplayMode( int n )
{

}
