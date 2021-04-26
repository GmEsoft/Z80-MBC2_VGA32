#pragma once

#if defined( ESP_PLATFORM )         // ESP32

#define USE_BUTTON              1   // Use User Button
#define USER_BUTTON             36  // User Button = S_VP
#define USE_TFT                 0   // Use TFT LCD (redundant?)

#else

#error Board not supported

#endif
