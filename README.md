# Z80-MBC2_VGA32


## Z80-MBC2 Emulator running on TTGO ESP VGA32

This is a Z80-MBC2 emulator running on a TTGO ESP VGA32 board,
an ESP32 device with a VGA connector, PS/2 keyboard and mouse
connectors, an audio output jack and a Micro-SD card connector.

This emulator runs a Z-80 processor at ca. 5.7 MHz (without throttling).
The emulated Z80-MBC2 system is internally connected to the FABGL terminal,
emulating an extended color ANSI terminal.

It has a terminal configuration menu, accessible when the USER button is pressed
on boot. The configuration menu has the following options:
- `[D]isplay mode`: select the VGA resolution and color depth;
- `[K]eyboard layout`: select an international keyboard layout;
- `[T]ime settings`: automatic clock set-up on boot, via an Internet time server;
- `[W]i-Fi settings`: select a Wi-Fi network, used to set the clock.

The Z80-MBC2 configuration menu is also accessible using the USER button.
The menu has been modified from the real Z80-MBC2 system:
- `6: set Z80 clock speed`, changed to `6: Change user debug mode` - to invoke the simple
  debugger by pressing the USER button while the emulated Z80 is running;
- `9: change RTC time/date`, changed to `9: Change auto-boot mode` - to choose between
  always invoking this menu on start-up or only via the USER button.


## Build

To build this sketch, the following items are needed:
- Espressif ESP-32 hardware support tools and libraries for
  the Arduino IDE, either integrated using the "Additional
  Boards Manager URL":
  <https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json>
  or by cloning the GitHub repo or downloading the ZIP from
  <https://github.com/espressif/arduino-esp32> (be sure to 
  select the release branch "release/v1.0";
- FabGL library from Fabrizio Di Vittorio from GitHub:
  <https://github.com/fdivitto/FabGL.git> or my fork, which
  also contains support for the Belgian keyboard layout:
  <https://github.com/GmEsoft/FabGL.git>.

In Arduino IDE v1.8.x, in Tools - Board:, select the "ESP32 Dev Module"
board. In the configuration settings, select the following:
- Partition Scheme: "Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)";
- PSRAM: "Disabled".


## Run

To start this emulator, get a FAT32-formatted Micro-SD card, copy all files from
the original Z80-MBC2 card image onto it, insert the card in the VGA32 module's
SD connector. Connect a VGA monitor to the VGA blue connector, a PS/2 keyboard
to the PS/2 purple connector, and an USB power supply to the Micro-USB connector.


## Credits

Thanks to the Z80-MBC2 Just4Fun project and its author SuperFabius - Fabio Defabis: 
<https://hackaday.io/project/159973-z80-mbc2-a-4-ics-homebrew-z80-computer>
and <https://github.com/SuperFabius/Z80-MBC2>.

Also thanks to the FabGL library from Fabrizio Di Vittorio that turns an ESP32 
embedded system into a smart VGA terminal with keyboard, mouse and audio output, 
and a Micro-SD card slot: <https://github.com/fdivitto/FabGL>.


## GPLv3 License

Created by Michel Bernard (michel_bernard@hotmail.com) - <http://www.github.com/GmEsoft/Z80-MBC2_VGA32>
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
