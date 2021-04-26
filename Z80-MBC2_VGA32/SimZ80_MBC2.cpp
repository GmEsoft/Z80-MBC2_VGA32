// SimZ80_MBC2.cpp : Defines the entry point for the console application.
//

#include "Z80_MBC2System.h"
#include "S220618_IOS-LITE-Z80-MBC2.h"

// ESP32 Core Headers
#include "HardwareSerial.h"
#include "Stream.h"
#include "Esp.h"

// Standard C Headers
#include <iostream>
#include <time.h>
#include <sys/time.h>

// dummy defines for arduino
#define SERIAL_RX_BUFFER_SIZE 128

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

extern Stream &mainStream;


word	Z80hl = 0;
int		fhandle = -1;
byte	io_BANK0 = 1;
byte	io_BANK1 = 0;

Z80_MBC2System	z80_mbc2;
Console_I		&ZSerial = z80_mbc2.getConsole();
byte			*ram;

// dummy arduino funcs
unsigned long Zmillis()
{
	return 0;
}

void ZpinMode( int addr, byte mode )
{
}

void ZdigitalWrite( int addr, byte data ); // in Z80_MBC2System

byte ZdigitalRead( int addr )
{
	return 0;
}

byte Zpgm_read_byte(const byte *addr)
{
	return *addr;
}


Eeprom EEPROM;

// ------------------------------------------------------------------------------
//  Constants
// ------------------------------------------------------------------------------

extern const byte    diskSetAddr  ;          // Internal EEPROM address for the current Disk Set [0..9]
extern const byte    maxDiskNum   ;          // Max number of virtual disks
extern const byte    maxDiskSet   ;           // Number of configured Disk Sets


// ------------------------------------------------------------------------------
// Generic routines
// ------------------------------------------------------------------------------

void printBinaryByte(byte value);

// Blink led IOS using a timestamp
void blinkIOSled(unsigned long *timestamp);



// ------------------------------------------------------------------------------
// RTC Module routines
// ------------------------------------------------------------------------------


byte decToBcd(byte val);
// Convert a binary byte to a two digits BCD byte

byte bcdToDec(byte val);
// Convert binary coded decimal to normal decimal numbers

void readRTC(byte *second, byte *minute, byte *hour, byte *day, byte *month, byte *year, byte *tempC);
// Read current date/time binary values and the temprerature (2 complement) from the DS3231 RTC
// ------------------------------------------------------------------------------

void writeRTC(byte second, byte minute, byte hour, byte day, byte month, byte year);
// Write given date/time binary values to the DS3231 RTC

byte autoSetRTC();
// Check if the DS3231 RTC is present and set the date/time at compile date/time if
// the RTC "Oscillator Stop Flag" is set (= date/time failure).
// Return value: 0 if RTC not present, 1 if found.

void printDateTime(byte readSourceFlag);
// Print to serial the current date/time from the global variables.
//
// Flag readSourceFlag [0..1] usage:
//    If readSourceFlag = 0 the RTC read is not done
//    If readSourceFlag = 1 the RTC read is done (global variables are updated)

void print2digit(byte data);
// Print a byte [0..99] using 2 digit with leading zeros if needed

byte isLeapYear(byte yearXX);
// Check if the year 2000+XX (where XX is the argument yearXX [00..99]) is a leap year.
// Returns 1 if it is leap, 0 otherwise.
// This function works in the [2000..2099] years range. It should be enough...


// ------------------------------------------------------------------------------
// Z80 bootstrap routines
// ------------------------------------------------------------------------------

// Load a given byte to RAM using a sequence of two Z80 instructions forced on the data bus.
void loadByteToRAM(byte value);

// Load "value" word into the HL registers inside the Z80 CPU, using the "LD HL,nn" instruction.
void loadHL(word value);

// ------------------------------------------------------------------------------
// SD Disk routines (FAT16 and FAT32 filesystems supported) using the PetitFS library.
// For more info about PetitFS see here: http://elm-chan.org/fsw/ff/00index_p.html
// ------------------------------------------------------------------------------


// Mount a volume on SD:
byte mountSD(FATFS* fatFs);

// Open an existing file on SD:
byte openSD(const char* fileName);

// Read one "segment" (32 bytes) starting from the current sector (512 bytes) of the opened file on SD:
byte readSD(void* buffSD, byte* numReadBytes);

// Write one "segment" (32 bytes) starting from the current sector (512 bytes) of the opened file on SD:
byte writeSD(void* buffSD, byte* numWrittenBytes);

// Set the pointer of the current sector for the current opened file on SD:
byte seekSD(word sectNum);

// Print the error occurred during a SD I/O operation:
void printErrSD(byte opType, byte errCode, const char* fileName);

// ------------------------------------------------------------------------------

// Wait a key to continue
void waitKey();

// Print the current Disk Set number and the OS name, if it is defined.
void printOsName(byte currentDiskSet);

// ------------------------------------------------------------------------------
//
// Hardware definitions for A040618 (Z80-MBC2) - Base system
//
// ------------------------------------------------------------------------------

#define   D0            24    // PA0 pin 40   Z80 data bus
#define   D1            25    // PA1 pin 39
#define   D2            26    // PA2 pin 38
#define   D3            27    // PA3 pin 37
#define   D4            28    // PA4 pin 36
#define   D5            29    // PA5 pin 35
#define   D6            30    // PA6 pin 34
#define   D7            31    // PA7 pin 33

#define   AD0           18    // PC2 pin 24   Z80 A0
#define   WR_           19    // PC3 pin 25   Z80 WR
#define   RD_           20    // PC4 pin 26   Z80 RD
#define   MREQ_         21    // PC5 pin 27   Z80 MREQ
#define   RESET_        22    // PC6 pin 28   Z80 RESET
#define   MCU_RTS_      23    // PC7 pin 29   * RESERVED - NOT USED *
#define   MCU_CTS_      10    // PD2 pin 16   * RESERVED - NOT USED *
#define   BANK1         11    // PD3 pin 17   RAM Memory bank address (High)
#define   BANK0         12    // PD4 pin 18   RAM Memory bank address (Low)
#define   INT_           1    // PB1 pin 2    Z80 control bus
#define   RAM_CE2        2    // PB2 pin 3    RAM Chip Enable (CE2). Active HIGH. Used only during boot
#define   WAIT_          3    // PB3 pin 4    Z80 WAIT
#define   SS_            4    // PB4 pin 5    SD SPI
#define   MOSI           5    // PB5 pin 6    SD SPI
#define   MISO           6    // PB6 pin 7    SD SPI
#define   SCK            7    // PB7 pin 8    SD SPI
#define   BUSREQ_       14    // PD6 pin 20   Z80 BUSRQ
#define   CLK           15    // PD7 pin 21   Z80 CLK
#define   SCL_PC0       16    // PC0 pin 22   IOEXP connector (I2C)
#define   SDA_PC1       17    // PC1 pin 23   IOEXP connector (I2C)
#define   LED_IOS        0    // PB0 pin 1    Led LED_IOS is ON if HIGH
#define   WAIT_RES_      0    // PB0 pin 1    Reset the Wait FF
#define   USER          13    // PD5 pin 19   Led USER and key (led USER is ON if LOW)

// ------------------------------------------------------------------------------
//
// Hardware definitions for A040618 GPE Option (Optional GPIO Expander)
//
// ------------------------------------------------------------------------------

#define   GPIOEXP_ADDR  0x20  // I2C module address (see datasheet)
#define   IODIRA_REG    0x00  // MCP23017 internal register IODIRA  (see datasheet)
#define   IODIRB_REG    0x01  // MCP23017 internal register IODIRB  (see datasheet)
#define   GPPUA_REG     0x0C  // MCP23017 internal register GPPUA  (see datasheet)
#define   GPPUB_REG     0x0D  // MCP23017 internal register GPPUB  (see datasheet)
#define   GPIOA_REG     0x12  // MCP23017 internal register GPIOA  (see datasheet)
#define   GPIOB_REG     0x13  // MCP23017 internal register GPIOB  (see datasheet)

// ------------------------------------------------------------------------------
//
// Hardware definitions for A040618 RTC Module Option (see DS3231 datasheet)
//
// ------------------------------------------------------------------------------

#define   DS3231_RTC    0x68  // DS3231 I2C address
#define   DS3231_SECRG  0x00  // DS3231 Seconds Register
#define   DS3231_STATRG 0x0F  // DS3231 Status Register

// ------------------------------------------------------------------------------
//
// File names and starting addresses
//
// ------------------------------------------------------------------------------

#define   BASICFN       "BASIC47.BIN"     // "ROM" Basic
#define   FORTHFN       "FORTH13.BIN"     // "ROM" Forth
#define   CPMFN         "CPM22.BIN"       // CP/M 2.2 loader
#define   QPMFN         "QPMLDR.BIN"      // QP/M 2.71 loader
#define   CPM3FN        "CPMLDR.COM"      // CP/M 3 CPMLDR.COM loader
#define   UCSDFN        "UCSDLDR.BIN"     // UCSD Pascal loader
#define   COSFN         "COS.BIN"         // Collapse Os loader
#define   AUTOFN        "AUTOBOOT.BIN"    // Auotobbot.bin file
#define   Z80DISK       "DSxNyy.DSK"      // Generic Z80 disk name (from DS0N00.DSK to DS9N99.DSK)
#define   DS_OSNAME     "DSxNAM.DAT"      // File with the OS name for Disk Set "x" (from DS0NAM.DAT to DS9NAM.DAT)
#define   BASSTRADDR    0x0000            // Starting address for the stand-alone Basic interptreter
#define   FORSTRADDR    0x0100            // Starting address for the stand-alone Forth interptreter
#define   CPM22CBASE    0xD200            // CBASE value for CP/M 2.2
#define   CPMSTRADDR    (CPM22CBASE - 32) // Starting address for CP/M 2.2
#define   QPMSTRADDR    0x80              // Starting address for the QP/M 2.71 loader
#define   CPM3STRADDR   0x100             // Starting address for the CP/M 3 loader
#define   UCSDSTRADDR   0x0000            // Starting address for the UCSD Pascal loader
#define   COSSTRADDR    0x0000            // Starting address for the Collapse Os loader
#define   AUTSTRADDR    0x0000            // Starting address for the AUTOBOOT.BIN file

// ------------------------------------------------------------------------------
//
// Atmega clock speed check
//
// ------------------------------------------------------------------------------

#if F_CPU == 20000000
  #define CLOCK_LOW   "5"
  #define CLOCK_HIGH  "10"
#else
  #define CLOCK_LOW   "4"
  #define CLOCK_HIGH  "8"
#endif

// ------------------------------------------------------------------------------
//
//  Constants
//
// ------------------------------------------------------------------------------

const byte    LD_HL        =  0x36;       // Opcode of the Z80 instruction: LD(HL), n
const byte    INC_HL       =  0x23;       // Opcode of the Z80 instruction: INC HL
const byte    LD_HLnn      =  0x21;       // Opcode of the Z80 instruction: LD HL, nn
const byte    JP_nn        =  0xC3;       // Opcode of the Z80 instruction: JP nn
const String  compTimeStr  = __TIME__;    // Compile timestamp string
const String  compDateStr  = __DATE__;    // Compile datestamp string
const byte    daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const byte    debug        = 0;           // Debug off = 0, on = 1, on = 2 with interrupt trace
const byte    bootModeAddr = 10;          // Internal EEPROM address for boot mode storage
const byte    autoexecFlagAddr = 12;      // Internal EEPROM address for AUTOEXEC flag storage
const byte    clockModeAddr = 13;         // Internal EEPROM address for the Z80 clock high/low speed switch
                                          //  (1 = low speed, 0 = high speed)
const byte    diskSetAddr  = 14;          // Internal EEPROM address for the current Disk Set [0..9]
const byte    debugModeAddr = 15;          // Enable debug via user button
const byte    autoBootModeAddr = 16;       // Enable auto-boot
const byte    maxDiskNum   = 99;          // Max number of virtual disks
const byte    maxDiskSet   = 5;           // Number of configured Disk Sets

const byte * const flahBootTable[1] PROGMEM = {boot_B_}; // Payload pointers table (flash)

// ------------------------------------------------------------------------------
//
//  Global variables
//
// ------------------------------------------------------------------------------

// General purpose variables
byte          ioAddress;                  // Virtual I/O address. Only two possible addresses are valid (0x00 and 0x01)
byte          ioData;                     // Data byte used for the I/O operation
byte          ioOpcode       = 0xFF;      // I/O operation code or Opcode (0xFF means "No Operation")
word          ioByteCnt;                  // Exchanged bytes counter during an I/O operation
byte          tempByte;                   // Temporary variable (buffer)
byte          moduleGPIO     = 0;         // Set to 1 if the module is found, 0 otherwise
byte          bootMode       = 0;         // Set the program to boot (from flash or SD)
									      // (0: boot_A_[], 1: boot_C_[], 2: boot_B_[])
byte *        BootImage;                  // Pointer to selected flash payload array (image) to boot
word          BootImageSize  = 0;         // Size of the selected flash payload array (image) to boot
word          BootStrAddr;                // Starting address of the selected program to boot (from flash or SD)
byte          Z80IntEnFlag   = 0;         // Z80 INT_ enable flag (0 = No INT_ used, 1 = INT_ used for I/O)
unsigned long timeStamp;                  // Timestamp for led blinking
char          inChar;                     // Input char from serial
byte          iCount;                     // Temporary variable (counter)
byte          clockMode;                  // Z80 clock HI/LO speed selector (0 = 8/10MHz, 1 = 4/5MHz)
byte          LastRxIsEmpty = 0;          // "Last Rx char was empty" flag. Is set when a serial Rx operation was done
                                          // when the Rx buffer was empty
byte          autoBootMode = 0;           // SimZ80-MBC2 Auto Boot Mode
byte          debugMode = debug;          // SimZ80-MBC2 Debug Mode
byte          turboMode = 1;			  // SimZ80-MBC2 Turbo Mode
#if GME_EXT
byte          gmeExtVersion = 0x63;       // GME Ext Version #
byte          eepromOffset;               // EEPROM address offset
byte          userButton = 0;             // user button pressed ?
#endif

// DS3231 RTC variables
byte          foundRTC;                   // Set to 1 if RTC is found, 0 otherwise
byte          seconds, minutes, hours, day, month, year;
byte          tempC;                      // Temperature (Celsius) encoded in two’s complement integer format

// SD disk and CP/M support variables
FATFS         *pfilesysSD;                // Filesystem object (PetitFS library)
byte          bufferSD[32];               // I/O buffer for SD disk operations (store a "segment" of a SD sector).
                                          //  Each SD sector (512 bytes) is divided into 16 segments (32 bytes each)
const char *  fileNameSD;                 // Pointer to the string with the currently used file name
byte          autobootFlag;               // Set to 1 if "autoboot.bin" must be executed at boot, 0 otherwise
byte          autoexecFlag;               // Set to 1 if AUTOEXEC must be executed at CP/M cold boot, 0 otherwise
byte          errCodeSD;                  // Temporary variable to store error codes from the PetitFS
byte          numReadBytes;               // Number of read bytes after a readSD() call

// Disk emulation on SD
char          diskName[11]    = Z80DISK;  // String used for virtual disk file name
char          OsName[11]      = DS_OSNAME;// String used for file holding the OS name
word          trackSel;                   // Store the current track number [0..511]
byte          sectSel;                    // Store the current sector number [0..31]
byte          diskErr         = 19;       // SELDISK, SELSECT, SELTRACK, WRITESECT, READSECT or SDMOUNT resulting
                                          //  error code
byte          numWriBytes;                // Number of written bytes after a writeSD() call
byte          diskSet;                    // Current "Disk Set"

// ------------------------------------------------------------------------------
static void dir( FileSystem_I &fs )
{
    Dir_I *pDir = fs.openDir( "" );
    if ( !pDir )
    {
        Serial.print( "NO DIR: " );
        Serial.println( fs.strerror( fs.errno_() ) );
        return;
    }

    while ( DirEntry entry = pDir->next() )
    {
        Serial.print( " Name: " );
        Serial.print( entry.name );
        if ( entry.isDir )
        {
            Serial.println( " DIR" );
        }
        else
        {
            Serial.print( " size: " );
            Serial.println( entry.size );
        }
    }
    pDir->close();
    delete pDir;
}


void Zsetup()
{
    LOGD( "schar : %d", 8*sizeof( schar  ) );
    LOGD( "uchar : %d", 8*sizeof( uchar  ) );
    LOGD( "ushort: %d", 8*sizeof( ushort ) );
    LOGD( "uint  : %d", 8*sizeof( uint   ) );
    LOGD( "ulong : %d", 8*sizeof( ulong  ) );
    LOGD( "Z80FLAGS : %d", 8*sizeof( Z80CPU::FLAGS ) );
    //z80_mbc2.getConsole().puts("<BEG_SETUP>");
    
    // ------------------------------------------------------------------------------
	//
	//  Local variables
	//
	// ------------------------------------------------------------------------------

	byte          data;                       // External RAM data byte
	word          address;                    // External RAM current address;
	char          minBootChar   = '1';        // Minimum allowed ASCII value selection (boot selection)
	char          maxSelChar    = '8';        // Maximum allowed ASCII value selection (boot selection)
	byte          maxBootMode   = 4;          // Default maximum allowed value for bootMode [0..4]
	byte          bootSelection = 0;          // Flag to enter into the boot mode selection

	// ----------------------------------------
	// INITIALIZATION
	// ----------------------------------------
    z80_mbc2.setup();
	ram = z80_mbc2.data();

    size_t freeBefore;
    size_t freeAfter;
    LOGD("Free SRAM before FATFS  : %d bytes\r\n", freeBefore = ESP.getFreeHeap());
    pfilesysSD = new FATFS();
    LOGD("Free SRAM  after FATFS  : %d bytes\r\n", freeAfter = ESP.getFreeHeap());
    LOGD("FATFS usage             : %d bytes\r\n", freeBefore - freeAfter );
    FATFS &filesysSD = *pfilesysSD;

	bootSelection = EEPROM.load( filesysSD );

#if USE_BUTTON
    bootSelection = bootSelection || !digitalRead(USER_BUTTON);
#else
	bootSelection = 1;
#endif

	// ------------------------------------------------------------------------------
	// Initialize the Logical RAM Bank (32KB) to map into the lower half of the Z80 addressing space
	ZpinMode(BANK0, OUTPUT);                         // Set RAM Logical Bank 1 (Os Bank 0)
	ZdigitalWrite(BANK0, HIGH);
	ZpinMode(BANK1, OUTPUT);
	ZdigitalWrite(BANK1, LOW);

    // Read the Z80 CPU speed mode
    if (EEPROM.read(clockModeAddr) > 1)             // Check if it is a valid value, otherwise set it to low speed
    // Not a valid value. Set it to low speed
    {
        EEPROM.update(clockModeAddr, 1);
    }
    clockMode = EEPROM.read(clockModeAddr);         // Read the previous stored value

    // Read the auto boot mode flag
    if (EEPROM.read(autoBootModeAddr) > 1)             // Check if it is a valid value, otherwise set it to off
    // Not a valid value. Set it to low speed
    {
        EEPROM.update(autoBootModeAddr, 0);
    }
    autoBootMode = EEPROM.read(autoBootModeAddr);         // Read the previous stored value

    // Read the user debug mode
    if (EEPROM.read(debugModeAddr) > 1)             // Check if it is a valid value, otherwise set it to low speed
    // Not a valid value. Set it to off
    {
        EEPROM.update(debugModeAddr, 0);
    }
    debugMode = EEPROM.read(debugModeAddr);         // Read the previous stored value

	// Read the stored Disk Set. If not valid set it to 0
	diskSet = EEPROM.read(diskSetAddr);
	if (diskSet >= maxDiskSet)
	{
		EEPROM.update(diskSetAddr, 0);
		diskSet =0;
	}

	mainStream.println(F("\r\n\nZ80-MBC2 - A040618\r\nIOS - I/O Subsystem - S220718-R280819"));
#if GME_EXT
	mainStream.print(F("IOS - GmEsoft Extensions - V:"));
	mainStream.print( gmeExtVersion, HEX );
	mainStream.println(F(" - build: " __DATE__ " " __TIME__ " **"));
#endif
    mainStream.println();
    
	// Print if the input serial buffer is 128 bytes wide (this is needed for xmodem protocol support)
	if (SERIAL_RX_BUFFER_SIZE >= 128)
		mainStream.println(F("IOS: Found extended serial Rx buffer"));

    // Print the user debug mode
    mainStream.print(F("IOS: auto boot mode set to "));
    if (autoBootMode) mainStream.print("ON");
    else mainStream.print("OFF"), bootSelection = 1;
    mainStream.println();

    // Print the user debug mode
    mainStream.print(F("IOS: user debug mode set to "));
    if (debugMode) mainStream.print("ON");
    else mainStream.print("OFF");
    mainStream.println();

	// Print RTC and GPIO informations if found
	foundRTC = autoSetRTC();                        // Check if RTC is present and initialize it as needed
	if (moduleGPIO) Serial.println(F("IOS: Found GPE Option"));

	// Print CP/M Autoexec on cold boot status
	mainStream.print(F("IOS: CP/M Autoexec is "));
	if (EEPROM.read(autoexecFlagAddr) > 1) EEPROM.update(autoexecFlagAddr, 0); // Reset AUTOEXEC flag to OFF if invalid
	autoexecFlag = EEPROM.read(autoexecFlagAddr);   // Read the previous stored AUTOEXEC flag
	if (autoexecFlag) mainStream.println("ON");
	else mainStream.println("OFF");

	// ----------------------------------------
	// BOOT SELECTION AND SYS PARAMETERS MENU
	// ----------------------------------------

	// Boot selection and system parameters menu if requested
	mountSD(&filesysSD);                            // Try to muont the SD volume
	bootMode = EEPROM.read(bootModeAddr);          // Read the previous stored boot mode

	while ((bootSelection == 1 ) || (bootMode > maxBootMode))
	// Enter in the boot selection menu if USER key was pressed at startup
	//   or an invalid bootMode code was read from internal EEPROM
	{
		bootSelection = 0;
		while (mainStream.available() > 0)                // Flush input serial Rx buffer
		{
			mainStream.read();
		}
		mainStream.println();
		mainStream.println(F("IOS: Select boot mode or system parameters:"));
		mainStream.println();
		if (bootMode <= maxBootMode)
		// Previous valid boot mode read, so enable '0' selection
		{
			minBootChar = '0';
			mainStream.print(F(" 0: No change ("));
			mainStream.print(bootMode + 1);
			mainStream.println(")");
		}
		mainStream.println(F(" 1: Basic"));
		mainStream.println(F(" 2: Forth"));
		mainStream.print(F(" 3: Load OS from "));
		printOsName(diskSet);
		mainStream.println(F("\r\n 4: Autoboot"));
		mainStream.println(F(" 5: iLoad"));
        mainStream.print(F(" 6: Change user debug mode (->"));
        if (debugMode) mainStream.print("OFF");
        else mainStream.print("ON");
        mainStream.println(")");
		mainStream.print(" 7: Toggle CP/M Autoexec (->");
		if (!autoexecFlag) mainStream.print("ON");
		else mainStream.print("OFF");
		mainStream.println(")");
		mainStream.print(" 8: Change ");
		printOsName(diskSet);
		mainStream.println();

        mainStream.print(" 9: Change Auto boot mode (->");
        if (!autoBootMode) mainStream.print("ON");
        else mainStream.print("OFF");
        maxSelChar = '9';
        mainStream.println(")");
        

		// Ask a choice
		mainStream.println();
		timeStamp = millis();
		mainStream.print("Enter your choice >");
		do
		{
			blinkIOSled(&timeStamp);
			inChar = mainStream.read();
		}
		while ( inChar <= 0 || inChar == 255 );

        if ( (inChar < minBootChar) || (inChar > maxSelChar) )
        {
            mainStream.print((char)12);
            bootSelection = 1;
            continue;
        }

		mainStream.print(inChar);
		mainStream.println("  Ok");

		// Make the selected action for the system paramters choice
		switch (inChar)
		{
        case '6':                                   // Change the clock speed of the Z80 CPU
            debugMode = !debugMode;                   // Toggle user debug mode (ON/OFF)
            EEPROM.update(debugModeAddr, debugMode);  // Save it to the internal EEPROM
            break;
		case '7':                                   // Toggle CP/M AUTOEXEC execution on cold boot
			autoexecFlag = !autoexecFlag;             // Toggle AUTOEXEC executiont status
			EEPROM.update(autoexecFlagAddr, autoexecFlag); // Save it to the internal EEPROM
			break;

		case '8':                                   // Change current Disk Set
			mainStream.println(F("\r\nPress CR to accept, ESC to exit or any other key to change"));
			iCount = diskSet;
			do
			{
				// Print the OS name of the next Disk Set
				iCount = (iCount + 1) % maxDiskSet;
				mainStream.print("\r ->");
				printOsName(iCount);
				mainStream.print(F("                 \r"));
				while (mainStream.available() > 0) mainStream.read();   // Flush serial Rx buffer
				while(mainStream.available() < 1) blinkIOSled(&timeStamp);  // Wait a key
				inChar = mainStream.read();
			}
			while ((inChar != 13) && (inChar != 27)); // Continue until a CR or ESC is pressed
			mainStream.println();
			mainStream.println();
			if (inChar == 13)                         // Set and store the new Disk Set if required
			{
				diskSet = iCount;
				EEPROM.update(diskSetAddr, iCount);
			}
            inChar = '8';
			break;

		case '9':                                   // Change RTC Date/Time
            autoBootMode = !autoBootMode;             // Toggle AUTO BOOT mode
            EEPROM.update(autoBootModeAddr, autoBootMode); // Save it to the internal EEPROM
            
			break;
		};

		// Save selectd boot program if changed
		bootMode = inChar - '1';                      // Calculate bootMode from inChar
		if (bootMode <= maxBootMode)
			EEPROM.update(bootModeAddr, bootMode); // Save to the internal EEPROM if required
		else
		{
            if ( bootMode < 128 )
			    bootSelection = 1;
			bootMode = EEPROM.read(bootModeAddr);    // Reload boot mode if '0' or > '5' choice selected
		}
	}

	// Print current Disk Set and OS name (if OS boot is enabled)
	if (bootMode == 2)
	{
		mainStream.print(F("IOS: Current "));
		printOsName(diskSet);
		mainStream.println();
	}

	// ----------------------------------------
	// Z80 PROGRAM LOAD
	// ----------------------------------------

	// Get the starting address of the program to load and boot, and its size if stored in the flash
	switch (bootMode)
	{
	case 0:                                       // Load Basic from SD
		fileNameSD = BASICFN;
		BootStrAddr = BASSTRADDR;
		Z80IntEnFlag = 1;                           // Enable INT_ signal generation (Z80 M1 INT I/O)
		break;

	case 1:                                       // Load Forth from SD
		fileNameSD = FORTHFN;
		BootStrAddr = FORSTRADDR;
		break;

	case 2:                                       // Load an OS from current Disk Set on SD
		switch (diskSet)
		{
		case 0:                                     // CP/M 2.2
			fileNameSD = CPMFN;
			BootStrAddr = CPMSTRADDR;
			break;

		case 1:                                     // QP/M 2.71
			fileNameSD = QPMFN;
			BootStrAddr = QPMSTRADDR;
			break;

		case 2:                                     // CP/M 3.0
			fileNameSD = CPM3FN;
			BootStrAddr = CPM3STRADDR;
			break;

		case 3:                                     // UCSD Pascal
			fileNameSD = UCSDFN;
			BootStrAddr = UCSDSTRADDR;
			break;

      case 4:                                     // Collapse Os
        fileNameSD = COSFN;
        BootStrAddr = COSSTRADDR;
      break;
		}
		break;

	case 3:                                       // Load AUTOBOOT.BIN from SD (load an user executable binary file)
		fileNameSD = AUTOFN;
		BootStrAddr = AUTSTRADDR;
		break;

	case 4:                                       // Load iLoad from flash
		BootImage = (byte *) pgm_read_dword (&flahBootTable[0]);
		BootImageSize = boot_B_size;
		BootStrAddr = boot_B_StrAddr;
		break;
	}
	digitalWrite(WAIT_RES_, HIGH);                  // Set WAIT_RES_ HIGH (Led LED_0 ON)

	// Load a JP instruction if the boot program starting addr is > 0x0000
	if (BootStrAddr > 0x0000)                       // Check if the boot program starting addr > 0x0000
	// Inject a "JP <BootStrAddr>" instruction to jump at boot starting address
	{
		loadHL(0x0000);                               // HL = 0x0000 (used as pointer to RAM)
		loadByteToRAM(JP_nn);                         // Write the JP opcode @ 0x0000;
		loadByteToRAM(lowByte(BootStrAddr));          // Write LSB to jump @ 0x0001
		loadByteToRAM(highByte(BootStrAddr));         // Write MSB to jump @ 0x0002
		//
		// DEBUG ----------------------------------
		if (debug)
		{
			mainStream.print("DEBUG: Injected JP 0x");
			mainStream.println(BootStrAddr, HEX);
		}
		// DEBUG END ------------------------------
		//
	}

	// Execute the load of the selected file on SD or image on flash
	loadHL(BootStrAddr);                            // Set Z80 HL = boot starting address (used as pointer to RAM);
	//
	// DEBUG ----------------------------------
	if (debug)
	{
		mainStream.print("DEBUG: Flash BootImageSize = ");
		mainStream.println(BootImageSize);
		mainStream.print("DEBUG: BootStrAddr = ");
		mainStream.println(BootStrAddr, HEX);
	}
	// DEBUG END ------------------------------
	//

	if (bootMode < maxBootMode)
	// Load from SD
	{
		// Mount a volume on SD
		if (mountSD(&filesysSD))
		// Error mounting. Try again
		{
			errCodeSD = mountSD(&filesysSD);
			if (errCodeSD)
			// Error again. Repeat until error disappears (or the user forces a reset)
			do
			{
				printErrSD(0, errCodeSD, NULL);
				waitKey();                                // Wait a key to repeat
				mountSD(&filesysSD);                      // New double try
				errCodeSD = mountSD(&filesysSD);
			}
			while (errCodeSD);
		}

		// Open the selected file to load
		errCodeSD = openSD(fileNameSD);
		if (errCodeSD)
			// Error opening the required file. Repeat until error disappears (or the user forces a reset)
			do
			{
				printErrSD(1, errCodeSD, fileNameSD);
				waitKey();                                  // Wait a key to repeat
				errCodeSD = openSD(fileNameSD);
				if (errCodeSD != 3)
				// Try to do a two mount operations followed by an open
				{
					mountSD(&filesysSD);
					mountSD(&filesysSD);
					errCodeSD = openSD(fileNameSD);
				}
			}
			while (errCodeSD);

		// Read the selected file from SD and load it into RAM until an EOF is reached
		mainStream.print("IOS: Loading boot program (");
		mainStream.print(fileNameSD);
		mainStream.print(")...");
		do
		// If an error occurs repeat until error disappears (or the user forces a reset)
		{
			do
			// Read a "segment" of a SD sector and load it into RAM
			{
				errCodeSD = readSD(bufferSD, &numReadBytes);  // Read current "segment" (32 bytes) of the current SD serctor
				for (iCount = 0; iCount < numReadBytes; iCount++)
				// Load the read "segment" into RAM
				{
					loadByteToRAM(bufferSD[iCount]);        // Write current data byte into RAM
				}
			}
			while ((numReadBytes == 32) && (!errCodeSD));   // If numReadBytes < 32 -> EOF reached
			if (errCodeSD)
			{
				printErrSD(2, errCodeSD, fileNameSD);
				waitKey();                                // Wait a key to repeat
				seekSD(0);                                // Reset the sector pointer
			}
		}
		while (errCodeSD);
	}
	else
	// Load from flash
	{
		mainStream.print("IOS: Loading boot program...");
		for (word i = 0; i < BootImageSize; i++)
		// Write boot program into external RAM
		{
			loadByteToRAM(Zpgm_read_byte(BootImage + i));  // Write current data byte into RAM
		}
	}
	mainStream.println(" Done");

    mainStream.printf ("IOS: Free SRAM = %d bytes\r\n", ESP.getFreeHeap());

	mainStream.println("IOS: Z80 is running from now");
	mainStream.println();

    if ( debugMode )
        z80_mbc2.stop();

	int ret = EEPROM.save( filesysSD );
}

void Zloop()
{
	z80_mbc2.loop();
}

void ZsetRamBuffer( void * data, size_t size )
{
    z80_mbc2.setRamBuffer( data, size );
}

// ------------------------------------------------------------------------------

// Generic routines

// ------------------------------------------------------------------------------



void printBinaryByte(byte value)
{
  for (byte mask = 0x80; mask; mask >>= 1)
  {
    mainStream.print((mask & value) ? '1' : '0');
  }
}

// ------------------------------------------------------------------------------

void serialEvent()
// Set INT_ to ACTIVE if there are received chars from serial to read and if the interrupt generation is enabled
{
  if ((mainStream.available()) && Z80IntEnFlag) digitalWrite(INT_, LOW);
}

// ------------------------------------------------------------------------------

void blinkIOSled(unsigned long *timestamp)
// Blink led IOS using a timestamp
{
  if ((millis() - *timestamp) > 200)
  {
    digitalWrite(LED_IOS,!digitalRead(LED_IOS));
    *timestamp = millis();
  }
}



// ------------------------------------------------------------------------------

// RTC Module routines

// ------------------------------------------------------------------------------


byte decToBcd(byte val)
// Convert a binary byte to a two digits BCD byte
{
  return( (val/10*16) + (val%10) );
}

// ------------------------------------------------------------------------------

byte bcdToDec(byte val)
// Convert binary coded decimal to normal decimal numbers
{
  return( (val/16*10) + (val%16) );
}

// ------------------------------------------------------------------------------

void readRTC(byte *second, byte *minute, byte *hour, byte *day, byte *month, byte *year, byte *tempC)
// Read current date/time binary values and the temprerature (2 complement) from the DS3231 RTC
{
	struct tm *ltm;
	time_t ltime;
	time( &ltime );
	ltm = localtime( &ltime );
	*hour = ltm->tm_hour;
	*minute  = ltm->tm_min;
	*second  = ltm->tm_sec;
	*day  = ltm->tm_mday;
	*month  = ltm->tm_mon+1;
	*year = ltm->tm_year%100;
	*tempC = (int)temperatureRead();
}

// ------------------------------------------------------------------------------

void writeRTC(byte second, byte minute, byte hour, byte day, byte month, byte year)
// Write given date/time binary values to the DS3231 RTC
{
    struct tm ltm;
    time_t ltime;
    struct timeval tv = {0,0};
    struct timezone tz = {0,0};

    ltm.tm_hour = hour;
    ltm.tm_min = minute;
    ltm.tm_sec = second;
    ltm.tm_mday = day;
    ltm.tm_mon = month-1;
    ltm.tm_year = year+100;
    ltime = mktime( &ltm );
    tv.tv_sec = ltime;
    settimeofday( &tv, &tz );
}

// ------------------------------------------------------------------------------

byte autoSetRTC()
// Check if the DS3231 RTC is present and set the date/time at compile date/time if
// the RTC "Oscillator Stop Flag" is set (= date/time failure).
// Return value: 0 if RTC not present, 1 if found.
{
  mainStream.print("IOS: Found RTC DS3231 Module (");
  printDateTime(1);
  mainStream.println(")");

  // Print the temperaturefrom the RTC sensor
  mainStream.print("IOS: RTC DS3231 temperature sensor: ");
  mainStream.print((int8_t)tempC);
  mainStream.println("C");

  return 1;
}

// ------------------------------------------------------------------------------

void printDateTime(byte readSourceFlag)
// Print to serial the current date/time from the global variables.
//
// Flag readSourceFlag [0..1] usage:
//    If readSourceFlag = 0 the RTC read is not done
//    If readSourceFlag = 1 the RTC read is done (global variables are updated)
{
  if (readSourceFlag) readRTC(&seconds, &minutes, &hours, &day,  &month,  &year, &tempC);
  print2digit(day);
  mainStream.print("/");
  print2digit(month);
  mainStream.print("/");
  print2digit(year);
  mainStream.print(" ");
  print2digit(hours);
  mainStream.print(":");
  print2digit(minutes);
  mainStream.print(":");
  print2digit(seconds);
}

// ------------------------------------------------------------------------------

void print2digit(byte data)
// Print a byte [0..99] using 2 digit with leading zeros if needed
{
  if (data < 10) mainStream.print("0");
  mainStream.print(data);
}

// ------------------------------------------------------------------------------

byte isLeapYear(byte yearXX)
// Check if the year 2000+XX (where XX is the argument yearXX [00..99]) is a leap year.
// Returns 1 if it is leap, 0 otherwise.
// This function works in the [2000..2099] years range. It should be enough...
{
  if (((2000 + yearXX) % 4) == 0) return 1;
  else return 0;
}


// ------------------------------------------------------------------------------

// Z80 bootstrap routines

// ------------------------------------------------------------------------------

void loadByteToRAM(byte value)
// Load a given byte to RAM using a sequence of two Z80 instructions forced on the data bus.
// The RAM_CE2 signal is used to force the RAM in HiZ, so the Atmega can write the needed instruction/data
//  on the data bus. Controlling the clock signal and knowing exactly how many clocks pulse are required it
//  is possible control the whole loading process.
// In the following "T" are the T-cycles of the Z80 (See the Z80 datashet).
// The two instruction are "LD (HL), n" and "INC (HL)".
{
  ram[Z80hl] = value;
  ++Z80hl;
}

// ------------------------------------------------------------------------------

void loadHL(word value)
// Load "value" word into the HL registers inside the Z80 CPU, using the "LD HL,nn" instruction.
// In the following "T" are the T-cycles of the Z80 (See the Z80 datashet).
{
  Z80hl = value;
}

// ------------------------------------------------------------------------------

void singlePulsesResetZ80()
// Reset the Z80 CPU using single pulses clock
{
}


// ------------------------------------------------------------------------------

// SD Disk routines (FAT16 and FAT32 filesystems supported) using the PetitFS library.
// For more info about PetitFS see here: http://elm-chan.org/fsw/ff/00index_p.html

// ------------------------------------------------------------------------------


byte mountSD(FATFS* fatFs)
// Mount a volume on SD:
// *  "fatFs" is a pointer to a FATFS object (PetitFS library)
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
{
  return 0;
}

// ------------------------------------------------------------------------------

byte openSD(const char* fileName)
// Open an existing file on SD:
// *  "fileName" is the pointer to the string holding the file name (8.3 format)
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
{
    //Serial.print( pfilesysSD );
    if ( fhandle != -1 )
        pfilesysSD->close( fhandle );
	fhandle = pfilesysSD->open( fileName, O_RDWR | O_BINARY, S_IWRITE );
	int err = pfilesysSD->errno_();
	return err;
}

// ------------------------------------------------------------------------------

byte readSD(void* buffSD, byte* numReadBytes)
// Read one "segment" (32 bytes) starting from the current sector (512 bytes) of the opened file on SD:
// *  "BuffSD" is the pointer to the segment buffer;
// *  "numReadBytes" is the pointer to the variables that store the number of read bytes;
//     if < 32 (including = 0) an EOF was reached).
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
//
// NOTE1: Each SD sector (512 bytes) is divided into 16 segments (32 bytes each); to read a sector you need to
//        to call readSD() 16 times consecutively
//
// NOTE2: Past current sector boundary, the next sector will be pointed. So to read a whole file it is sufficient
//        call readSD() consecutively until EOF is reached
{
	int n = pfilesysSD->read( fhandle, buffSD, 32 );
	*numReadBytes = n;
	int err = pfilesysSD->errno_();
	return err;
}

// ------------------------------------------------------------------------------

byte writeSD(void* buffSD, byte* numWrittenBytes)
// Write one "segment" (32 bytes) starting from the current sector (512 bytes) of the opened file on SD:
// *  "BuffSD" is the pointer to the segment buffer;
// *  "numWrittenBytes" is the pointer to the variables that store the number of written bytes;
//     if < 32 (including = 0) an EOF was reached.
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
//
// NOTE1: Each SD sector (512 bytes) is divided into 16 segments (32 bytes each); to write a sector you need to
//        to call writeSD() 16 times consecutively
//
// NOTE2: Past current sector boundary, the next sector will be pointed. So to write a whole file it is sufficient
//        call writeSD() consecutively until EOF is reached
//
// NOTE3: To finalize the current write operation a writeSD(NULL, &numWrittenBytes) must be called as last action
{
    if (buffSD != NULL)
	{
		int n = pfilesysSD->write( fhandle, buffSD, 32 );
		*numWrittenBytes = n;
		int err = pfilesysSD->errno_();
		return err;
	}
	return 0;
}

// ------------------------------------------------------------------------------

byte seekSD(word sectNum)
// Set the pointer of the current sector for the current opened file on SD:
// *  "sectNum" is the sector number to set. First sector is 0.
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
//
// NOTE: "secNum" is in the range [0..16383], and the sector addressing is continuous inside a "disk file";
//       16383 = (512 * 32) - 1, where 512 is the number of emulated tracks, 32 is the number of emulated sectors
//
{
    return pfilesysSD->lseek( fhandle, ((unsigned long) sectNum) << 9, SEEK_SET );
}

// ------------------------------------------------------------------------------

void printErrSD(byte opType, byte errCode, const char* fileName)
// Print the error occurred during a SD I/O operation:
//  * "OpType" is the operation that generated the error (0 = mount, 1= open, 2 = read,
//     3 = write, 4 = seek);
//  * "errCode" is the error code from the PetitFS library (0 = no error);
//  * "fileName" is the pointer to the file name or NULL (no file name)
//
// ........................................................................
//
// Errors legend (from PetitFS library) for the implemented operations:
//
// ------------------
// mountSD():
// ------------------
// NOT_READY
//     The storage device could not be initialized due to a hard error or no medium.
// DISK_ERR
//     An error occured in the disk read function.
// NO_FILESYSTEM
//     There is no valid FAT partition on the drive.
//
// ------------------
// openSD():
// ------------------
// NO_FILE
//     Could not find the file.
// DISK_ERR
//     The function failed due to a hard error in the disk function, a wrong FAT structure or an internal error.
// NOT_ENABLED
//     The volume has not been mounted.
//
// ------------------
// readSD() and writeSD():
// ------------------
// DISK_ERR
//     The function failed due to a hard error in the disk function, a wrong FAT structure or an internal error.
// NOT_OPENED
//     The file has not been opened.
// NOT_ENABLED
//     The volume has not been mounted.
//
// ------------------
// seekSD():
// ------------------
// DISK_ERR
//     The function failed due to an error in the disk function, a wrong FAT structure or an internal error.
// NOT_OPENED
//     The file has not been opened.
//
// ........................................................................
{
  if (errCode)
  {
    mainStream.print("\r\nIOS: SD error ");
    mainStream.print(errCode);
    mainStream.print(" (");
    switch (errCode)
    // See PetitFS implementation for the codes
    {
      case 1: mainStream.print("DISK_ERR"); break;
      case 2: mainStream.print("NOT_READY"); break;
      case 3: mainStream.print("NO_FILE"); break;
      case 4: mainStream.print("NOT_OPENED"); break;
      case 5: mainStream.print("NOT_ENABLED"); break;
      case 6: mainStream.print("NO_FILESYSTEM"); break;
      default: mainStream.print("UNKNOWN");
    }
    mainStream.print(" on ");
    switch (opType)
    {
      case 0: mainStream.print("MOUNT"); break;
      case 1: mainStream.print("OPEN"); break;
      case 2: mainStream.print("READ"); break;
      case 3: mainStream.print("WRITE"); break;
      case 4: mainStream.print("SEEK"); break;
      default: mainStream.print("UNKNOWN");
    }
    mainStream.print(" operation");
    if (fileName)
    // Not a NULL pointer, so print file name too
    {
      mainStream.print(" - File: ");
      mainStream.print(fileName);
    }
    mainStream.println(")");
  }
}

// ------------------------------------------------------------------------------

void waitKey()
// Wait a key to continue
{
  while (mainStream.available() > 0) mainStream.read();   // Flush serial Rx buffer
  mainStream.println(F("IOS: Check SD and press a key to repeat\r\n"));
  while(mainStream.available() < 1);
}

// ------------------------------------------------------------------------------

void printOsName(byte currentDiskSet)
// Print the current Disk Set number and the OS name, if it is defined.
// The OS name is inside the file defined in DS_OSNAME
{
  mainStream.print("Disk Set ");
  mainStream.print(currentDiskSet);
  OsName[2] = currentDiskSet + 48;    // Set the Disk Set
  openSD(OsName);                     // Open file with the OS name
  readSD(bufferSD, &numReadBytes);    // Read the OS name
  if (numReadBytes > 0)
  // Print the OS name
  {
    mainStream.print(" (");
    mainStream.print((const char *)bufferSD);
    mainStream.print(")");
  }
}
