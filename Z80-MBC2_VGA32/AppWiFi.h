#pragma once

#include "WString.h"

class AppWiFi
{
public:
    static void setup();

    static bool configure();

    static String getSsid();

    static void setSsid( String ssid );

    static String getPasswd();

    static void setPasswd( String passwd );

    static bool connect();

    static bool disconnect();

private:
    static bool connect( const String &ssid, const String &passwd );
};
