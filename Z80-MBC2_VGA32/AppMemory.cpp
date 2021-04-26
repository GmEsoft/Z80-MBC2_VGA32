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
