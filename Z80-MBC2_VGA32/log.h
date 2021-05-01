/*
    Z80-MBC2_VGA32 - Debug logging helper for Arduino ESP-32 toolset
    

    Created by Michel Bernard (michel_bernard@hotmail.com) 
    - <http://www.github.com/GmEsoft/Z80-MBC2_VGA32>
    Copyright (c) 2021 Michel Bernard.
    All rights reserved.
    

    This file is part of Z80-MBC2_VGA32.
    
    Z80-MBC2_VGA32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef LOG_H
#define LOG_H
#include "esp32-hal-log.h"
#define LOGE(args...) ESP_LOGE( "LOG", args )
#define LOGW(args...) ESP_LOGW( "LOG", args )
#define LOGI(args...) ESP_LOGI( "LOG", args )
#define LOGD(args...) ESP_LOGD( "LOG", args )
#define LOGV(args...) ESP_LOGV( "LOG", args )
#endif