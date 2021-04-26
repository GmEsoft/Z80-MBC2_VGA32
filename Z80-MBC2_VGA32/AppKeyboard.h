#pragma once

#include "fabgl.h"

#include "WString.h"

class AppKeyboard
{
public:
    static void setup();

    static bool configure();

    static String getLayout( int n );

    static int getLayout();

    static void setLayout( int n );
};

extern fabgl::PS2Controller    PS2Controller;
