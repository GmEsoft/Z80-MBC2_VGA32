/*
    Z80-MBC2_VGA32 - PSRAM memory allocator for TRS-80 emulation

    For performance reasons, it is preferable to disable the PSRAM
    in the ESP-32 board configuration settings in the Tools menu
    of Arduino IDE, and let the software itself manage this memory.
    

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

#include "AppMemory.h"

#if !BOARD_HAS_PSRAM // Should be disabled for better performance accessing PSRAM

extern "C" {
#   include "esp_spiram.h"
}

#include "Allocator.h"

#include "HardwareSerial.h"

// to avoid PSRAM bug without -mfix-esp32-psram-cache-issue
// core 0 can only work reliably with the lower 2 MB and core 1 only with the higher 2 MB.
Allocator PSRAM0( (void*)0x3F800000, 0x200000 );
Allocator PSRAM1( (void*)0x3FA00000, 0x200000 );

void * AppMemory::ps_malloc( int core, size_t size )
{
    static bool first = true;
    
    if ( first )
    {
        first = false;

        // note: we use just 2MB of PSRAM so the infamous PSRAM bug should not happen. But to avoid gcc compiler hack (-mfix-esp32-psram-cache-issue)
        // we enable PSRAM at runtime, otherwise the hack slows down CPU too much (PSRAM_HACK is no more required).
        if (esp_spiram_init() != ESP_OK) {
            Serial.println("This app requires PSRAM!!");
            while (1);
        }

        esp_spiram_init_cache();
        Serial.println("Manual PSRAM initialized");
    }

    switch( core )
    {
        case 0:
            return PSRAM0.malloc( size );
        case 1:
            return PSRAM1.malloc( size );
        default:
            return 0;
    }
}

#else

#include "esp32-hal-psram.h"

void * AppMemory::ps_malloc( int core, size_t size )
{
    return ::ps_malloc( size );
}

#endif
