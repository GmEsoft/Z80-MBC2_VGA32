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
