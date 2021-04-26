// Configuration settings
#include "TermCfg.h"

// Project headers
#include "AppDisplay.h"
#include "AppKeyboard.h"
#include "AppMemory.h"
#include "AppPreferences.h"
#include "AppStream.h"
#include "AppTime.h"
#include "AppWiFi.h"

#include "SimZ80_MBC2.h"
#include "DualStream.h"
#include "XStream.h"
#include "log.h"

extern "C" uint8_t temprature_sens_read(); // No header file found

//------------------------------------------------------------------

XStreams xStreams;
Stream &xStream1 = xStreams.stream1();      // used by main thread
Stream &xStream2 = xStreams.stream2();      // used by sub thread

//------------------------------------------------------------------

DualStream dStream( Serial, Terminal );     // used by main thread
Stream &mainStream = dStream;               // use main thread stream

Stream &subStream = xStream2;               // use sub thread stream

//------------------------------------------------------------------

void setup()
{
    Serial.begin( 115200 );
    delay( 200 );
    Serial.println( "setup() started" );

    AppStream.setStream( mainStream );

#if USE_BUTTON
    pinMode(USER_BUTTON, INPUT_PULLUP);
#endif

    bool reconfig = !preferences.begin("CpmZ80", false);
    if ( reconfig )
    {
        preferences.putInt( "resolution", 0 );
    }
#if USE_BUTTON
    reconfig = reconfig || !digitalRead( USER_BUTTON );
#endif

    init_Terminal();
    delay( 200 );

    AppStream.println( "** Z80-MBC2 Simulator **" );
    AppStream.println();

#if USE_BUTTON
    reconfig = reconfig || !digitalRead( USER_BUTTON );
#endif

    if ( reconfig )
    {
        reconfigure();
    }

    AppTime::setup();

    Serial.println( "Starting emulator" );
    preferences.end();

    int mainCore = xPortGetCoreID();
    Serial.printf( "Main task core : %d\n", mainCore );

    Serial.printf( "Wi-Fi core     : %d\n", WIFI_TASK_CORE_ID );
    Serial.printf( "Terminal core  : %d\n", FABGLIB_VIDEO_CPUINTENSIVE_TASKS_CORE );
    Serial.printf( "Emulator core  : %d\n", 1-mainCore );
    Serial.println( "Running on multi-core" );
    setRamBuffer( 1 - mainCore );

    Zsetup();

    int priority = 0;
    int subCore = 1-mainCore;
    Serial.printf("Creating subloop task on core %d with priority %d ...\n", subCore, priority);
    xTaskCreatePinnedToCore(
                        subtask,            /* Function to implement the task */
                        "subtask",          /* Name of the task */
                        10000,              /* Stack size in words */
                        NULL,               /* Task input parameter */
                        priority,           /* Priority of the task */
                        NULL,               /* Task handle. */
                        subCore);           /* Core where the task should run */

}


/* Pi Spigot 500 performance:
 * ==========================
 * 
 * Config                   FabGL   MCORE   Resol   Input   1st Line    Full    MHz     Comment
 * ---------------------------------------------------------------------------------------------------------
 * ESP32-Wrover-Kit
 * TTGO VGA32 (COM5)        0       0       -       Serial  1:57        10:24   4.69    system PSRAM
 * TTGO VGA32               0       0       -       Serial                              manual PSRAM
 * TTGO VGA32               0       0       -       Serial                              SRAM (PSRAM disabled)
 * TTGO VGA32               0       1       -       Serial  1:36         8:30   5.74    manual PSRAM
 * TTGO VGA32               0       1       -       Serial  1:34         8:20           SRAM (PSRAM disabled)
 * TTGO VGA32               1       0       2       Serial  
 * TTGO VGA32               1       0       2       PS/2    
 * TTGO VGA32               1       1       0       Serial  2:51        15:00   3.25    with yield(), system PSRAM
 * TTGO VGA32               1       1       2       Serial  2:28        13:08           with yield(), system PSRAM
 * TTGO VGA32               1       1       0       Serial  1:34         8:19           with yield(), manual PSRAM
 * TTGO VGA32               1       1       2       Serial                              with yield(), manual PSRAM
 * TTGO VGA32               1       1       3       Serial  1:34         8:20           with yield(), manual PSRAM
 * TTGO VGA32               1       1       2       PS/2                                with yield(), system PSRAM
 * TTGO VGA32               1       1       2       Serial  2:28        13:12           with yield() prio=2, system PSRAM
 * TTGO VGA32               1       1       2       Serial  2:46                        w/o yield(), system PSRAM
 * 
 */
void reconfigure()
{
    AppStream.printf("Build            : %s %s\r\n",       __DATE__, __TIME__);
    AppStream.printf("GCC Version      : %s\r\n",          __VERSION__);
    AppStream.printf("SDK Version      : %s\r\n",          ESP.getSdkVersion());
    AppStream.printf("ESP32 Chip Model : %s Rev %d\r\n",   ESP.getChipModel(), ESP.getChipRevision());
    AppStream.printf("CPU Cores        : %d\r\n",          ESP.getChipCores());
    AppStream.printf("CPU Freq         : %d MHz\r\n",      ESP.getCpuFreqMHz());
    if ( temprature_sens_read() != 128 )
        AppStream.printf("CPU Temperature  : %4.1f C\r\n",     temperatureRead());
    else
        AppStream.printf("CPU Temperature  : (no sensor)\r\n",     temperatureRead());
    AppStream.printf("Total  SRAM      : %d bytes\r\n",    ESP.getHeapSize());
    AppStream.printf("Free   SRAM      : %d bytes\r\n",    ESP.getFreeHeap());
    AppStream.printf("Total PSRAM      : %d bytes\r\n",    ESP.getPsramSize());
    AppStream.printf("Free  PSRAM      : %d bytes\r\n",    ESP.getFreePsram());
    AppStream.printf("Total Flash      : %d bytes\r\n",    ESP.getFlashChipSize());

    bool restart = false;
    bool reconfig = true;
    int c;
    while( reconfig )
    {
        AppStream.println();
        AppStream.println("Config Menu");
        AppStream.println("===========");
        AppStream.print  ("[D] Display Mode    ["); AppStream.print( AppDisplay::getDisplayMode() ); AppStream.println( "]" );
        AppStream.print  ("[K] Keyboard Layout ["); AppStream.print( AppKeyboard::getLayout() ); AppStream.println( "]" );;
        AppStream.print  ("[T] Time Settings   ["); AppStream.print( AppTime::isEnabled() ? 'Y' : 'N' ); AppStream.println( "]" );
        AppStream.print  ("[W] Wi-Fi Settings  ["); AppStream.print( AppWiFi::getSsid() ); AppStream.println( "]" );
        AppStream.println("[R] Reboot");
        AppStream.println("[C] Clear All Settings");
        AppStream.println("[Q] Quit menu");
        AppStream.print("==> ");
        while ( !mainStream.available() );
        while ( !isalnum( c = mainStream.read() ) );
        if ( isalnum( c ) )
            AppStream.println( (char)c );
        switch( toupper( c ) )
        {
        case 'D':
            if ( restart = AppDisplay::configure() ) {
                AppStream.println("Display settings changed - restart required");
            }
            break;
        case 'K':
            if ( restart = AppKeyboard::configure( ) ) {
                AppStream.println("Keyboard settings changed - restart required");
            };
            break;
        case 'T':
            if ( restart = AppTime::configure() ) {
                AppStream.println("Time settings changed - restart required");
            }
            break;
        case 'W':
            if ( restart = AppWiFi::configure()  ) {
                AppStream.println("Wi-Fi settings changed - restart required");
            }
            break;
        case 'R':
            restart = true;
            reconfig = false;
            break;
        case 'C':
            AppStream.println("Clearing All Settings - Press [Y] to confirm");
            while ( !isalnum( c = mainStream.read() ) );
            if ( toupper( c ) == 'Y' ) {
                AppStream.println("All settings cleared - restart required");
                preferences.clear();
                restart = true;
                reconfig = false;
            }
            break;
        case 'Q':
            reconfig = false;
            break;
        }
        
    }

    if ( restart )
    {
        preferences.end();
        ESP.restart();            
    }
}

void loop(void)
{
    static int first = 1;

    if ( first ) {
        Serial.printf("Terminal task started on core #%d\n", xPortGetCoreID() );
        first = 0;
    }

    if ( xStream1.available()  ) {
        mainStream.write( xStream1.read() );
    }
    
    if ( mainStream.available() && xStream1.availableForWrite() ) {
        xStream1.write( mainStream.read() );
    }

    yield();

}

void setRamBuffer( int core )
{
    void *data;
    const size_t size = 0x20000;
    Serial.printf( "Allocating 0x%x bytes PSRAM for core %d\r\n", size, core );
    
    data = AppMemory::ps_malloc( core, size );
    if ( !data )
    {
        Serial.print( "ABEND! PSRAM allocation failed" );
    }
    ZsetRamBuffer( data, size );
}

void subtask( void * )
{
    Serial.printf("Emulator task started on core #%d\n", xPortGetCoreID() );
    disableCore0WDT();

    for (;;)
    {
        for (int i=0; i<100; ++i)
            Zloop();

        yield();
    }
}

void init_Terminal()
{
    Serial.println("Init FabGL ...");

    AppKeyboard::setup();
    AppDisplay::setup();
    
    Serial.println("FabGL initialized");
}
