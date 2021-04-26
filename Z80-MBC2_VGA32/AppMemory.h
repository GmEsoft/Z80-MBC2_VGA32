#pragma once

#include <stddef.h>

class AppMemory
{
public:
    static void *ps_malloc( int core, size_t size );
};
