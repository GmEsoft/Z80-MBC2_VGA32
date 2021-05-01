/*
    Z80-MBC2_VGA32 - PS/2 Keyboard support
    with international layouts selector.  


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

#include "AppKeyboard.h"

#include "AppStream.h"
#include "log.h"

#include "fabgl.h"

#include "Preferences.h"

extern Preferences preferences;

// globals
fabgl::PS2Controller    PS2Controller;


void AppKeyboard::setup()
{
    // setup Keyboard (default configuration)
    LOGD( "PS2Controller.begin()" );
    PS2Controller.begin(PS2Preset::KeyboardPort0);

    int nLayout = preferences.getInt( "kblayout", -1 );
    int nLayouts = fabgl::SupportedLayouts::count() ;
    const fabgl::KeyboardLayout * * layouts = fabgl::SupportedLayouts::layouts();

    for ( int i=0; nLayout < 0 && i < nLayouts; ++i )
    {
        if ( !strcmp( layouts[i]->name, "US" ) )
            nLayout = i;
    }
    
    if ( nLayout >= 0 && nLayout < nLayouts )
    {
        LOGD( "set keyboard layout %d: %s - %s", nLayout, layouts[nLayout]->name, layouts[nLayout]->desc  );
        PS2Controller.keyboard()->setLayout( layouts[nLayout] );
    }
    else
        LOGW( "layout %d out of bounds (0..%d) !", nLayout, nLayouts );
        
    Serial.printf( "Keyboard Layout    : #%d %s\r\n", nLayout, getLayout( nLayout ).c_str() );
}

bool AppKeyboard::configure()
{
    AppStream.println();
    AppStream.println("Available Layouts");
    AppStream.println("=====================");
    AppStream.println();
    int nLayout = preferences.getInt( "kblayout", -1 );
    const fabgl::KeyboardLayout * * layouts = fabgl::SupportedLayouts::layouts();
    int nLayouts = fabgl::SupportedLayouts::count();
    for ( int i=0; i < nLayouts; ++i )
    {
        if ( nLayout < 0 && !strcmp( layouts[i]->name, "US" ) )
            nLayout = i;
        AppStream.printf( "%2s [%d] %s - %s", i == nLayout ? ">>" : "", i, layouts[i]->name, layouts[i]->desc );
        AppStream.println();
    }
    AppStream.print("==> ");
    while ( AppStream.available() ) 
      AppStream.read();
    int c;
    while ( !isalnum( c = AppStream.read() ) );
    if ( isdigit( c ) && c - '0' < nLayouts)
    {
        AppStream.println("Layout changed");
        nLayout = c - '0';
        preferences.putInt( "kblayout", nLayout );
        PS2Controller.keyboard()->setLayout( layouts[nLayout] );
    }
    return false; // no restart needed
    
}

String AppKeyboard::getLayout( int i )
{
    const fabgl::KeyboardLayout * * layouts = fabgl::SupportedLayouts::layouts();
    return String( i >= 0 && i < fabgl::SupportedLayouts::count() ? layouts[i]->name : "US (default)" );
}

int AppKeyboard::getLayout()
{
    return preferences.getInt( "kblayout", -1 );
}

void AppKeyboard::setLayout( int n )
{
    
}
