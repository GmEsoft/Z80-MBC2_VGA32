#pragma once

#include "fabgl.h"

#include "WString.h"

class AppDisplay
{
public:
    static void setup();

    static bool configure();

    static String getDisplayMode( int n );

    static const char * getResolution( int n );

    static int getDisplayMode();

    static void setDisplayMode( int n );
};

extern fabgl::Terminal         Terminal;
