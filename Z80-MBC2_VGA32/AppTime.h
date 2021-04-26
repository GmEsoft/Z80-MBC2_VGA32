#pragma once

class AppTime
{
public:
    static void setup();

    static bool configure();

    static bool isEnabled();

    static void setEnabled( bool enabled );

    static long getGmtOffset_sec();
    
    static void setEnabled( long gmtOffset_sec );
    
    static int getDstOffset_sec();
    
    static void setEnabled( int dstOffset_sec );
};
