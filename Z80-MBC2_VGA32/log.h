#ifndef LOG_H
#define LOG_H
#include "esp32-hal-log.h"
#define LOGE(args...) ESP_LOGE( "LOG", args )
#define LOGW(args...) ESP_LOGW( "LOG", args )
#define LOGI(args...) ESP_LOGI( "LOG", args )
#define LOGD(args...) ESP_LOGD( "LOG", args )
#define LOGV(args...) ESP_LOGV( "LOG", args )
#endif
