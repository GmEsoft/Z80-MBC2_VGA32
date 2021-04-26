#include "Z80_MBC2System.h"

#include <stdio.h>
#include <ctype.h>

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

extern Stream& mainStream;

extern Console_I &ZSerial;

// ------------------------------------------------------------------------------
// RTC Module routines
// ------------------------------------------------------------------------------

// Read current date/time binary values and the temprerature (2 complement) from the DS3231 RTC
void readRTC(byte *second, byte *minute, byte *hour, byte *day, byte *month, byte *year, byte *tempC);


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
//
// File names and starting addresses
//
// ------------------------------------------------------------------------------

#define   Z80DISK       "DSxNyy.DSK"      // Generic Z80 disk name (from DS0N00.DSK to DS9N99.DSK)
#define   DS_OSNAME     "DSxNAM.DAT"      // File with the OS name for Disk Set "x" (from DS0NAM.DAT to DS9NAM.DAT)

// ------------------------------------------------------------------------------
//
//  Constants
//
// ------------------------------------------------------------------------------

extern const byte    diskSetAddr  ;          // Internal EEPROM address for the current Disk Set [0..9]
extern const byte    maxDiskNum   ;          // Max number of virtual disks
extern const byte    maxDiskSet   ;           // Number of configured Disk Sets

// ------------------------------------------------------------------------------
//
//  Global variables
//
// ------------------------------------------------------------------------------

// General purpose variables
extern byte          ioAddress;                  // Virtual I/O address. Only two possible addresses are valid (0x00 and 0x01)
extern byte          ioData;                     // Data byte used for the I/O operation
extern byte          ioOpcode;					 // I/O operation code or Opcode (0xFF means "No Operation")
extern word          ioByteCnt;                  // Exchanged bytes counter during an I/O operation
extern byte          tempByte;
extern byte          Z80IntEnFlag;				 // Z80 INT_ enable flag (0 = No INT_ used, 1 = INT_ used for I/O)
extern byte          clockMode;                  // Z80 clock HI/LO speed selector (0 = 8/10MHz, 1 = 4/5MHz)
extern byte          LastRxIsEmpty;              // "Last Rx char was empty" flag. Is set when a serial Rx operation was done
												 // when the Rx buffer was empty
extern byte          debugMode;	                 // SimZ80-MBC2 Debug Mode
extern byte          turboMode;	                 // SimZ80-MBC2 Turbo Mode
#if GME_EXT
extern byte          gmeExtVersion;              // GME Ext Version #
extern byte          eepromOffset;               // EEPROM address offset
extern byte          userButton;                 // user button pressed ?
#endif

// DS3231 RTC variables
extern byte          foundRTC;                   // Set to 1 if RTC is found, 0 otherwise
extern byte          seconds, minutes, hours, day, month, year;
extern byte          tempC;                      // Temperature (Celsius) encoded in two’s complement integer format

// SD disk and CP/M support variables
extern FATFS         *pfilesysSD;                // Filesystem object (PetitFS library)
extern byte          bufferSD[32];               // I/O buffer for SD disk operations (store a "segment" of a SD sector).
                                                 //  Each SD sector (512 bytes) is divided into 16 segments (32 bytes each)
extern const char *  fileNameSD;                 // Pointer to the string with the currently used file name
extern byte          autobootFlag;               // Set to 1 if "autoboot.bin" must be executed at boot, 0 otherwise
extern byte          autoexecFlag;               // Set to 1 if AUTOEXEC must be executed at CP/M cold boot, 0 otherwise
extern byte          errCodeSD;                  // Temporary variable to store error codes from the PetitFS
extern byte          numReadBytes;               // Number of read bytes after a readSD() call

// Disk emulation on SD
extern char          diskName[11];				 // String used for virtual disk file name
extern char          OsName[11];                 // String used for file holding the OS name
extern word          trackSel;                   // Store the current track number [0..511]
extern byte          sectSel;                    // Store the current sector number [0..31]
extern byte          diskErr;					 // SELDISK, SELSECT, SELTRACK, WRITESECT, READSECT or SDMOUNT resulting
                                                 //  error code
extern byte          numWriBytes;                // Number of written bytes after a writeSD() call
extern byte          diskSet;                    // Current "Disk Set"

extern byte			 io_BANK0;
extern byte			 io_BANK1;

static uint bankoff = 0;

void ZdigitalWrite( int addr, byte data )
{
    switch ( addr )
    {
    case BANK0:
        io_BANK0 = data;
        bankoff = ( io_BANK0 ? 0x00000 : 0x08000 ) | ( io_BANK1 ? 0x10000 : 0x00000 );
        break;
    case BANK1:
        io_BANK1 = data;
        bankoff = ( io_BANK0 ? 0x00000 : 0x08000 ) | ( io_BANK1 ? 0x10000 : 0x00000 );
        break;
    }
}


uint Z80_MBC2RAM::ram_addr( ushort addr )
{
    if ( addr & 0x8000 )
        return addr;
    return addr | bankoff;
}

// write byte
uchar Z80_MBC2RAM::write( ushort addr, uchar data )
{
    if ( addr & 0x8000 )
        return buffer_[addr] = data;
    return buffer_[addr|bankoff] = data;
}

// read byte
uchar Z80_MBC2RAM::read( ushort addr )
{
    if ( addr & 0x8000 )
        return buffer_[addr];
    return buffer_[addr|bankoff];
}


// write byte
uchar Z80_MBC2RAM::write( void *obj, ushort addr, uchar data )
{
    if ( addr & 0x8000 )
        return ((uchar*)obj)[addr] = data;
    return ((uchar*)obj)[addr|bankoff] = data;
}

// read byte
uchar Z80_MBC2RAM::read( void *obj, ushort addr )
{
    if ( addr & 0x8000 )
        return ((uchar*)obj)[addr];
    return ((uchar*)obj)[addr|bankoff];
}


uchar Z80_MBC2IO::out( ushort addr, uchar data )
{
	// I/O WRITE operation requested

	// ----------------------------------------
	// VIRTUAL I/O WRTE OPERATIONS ENGINE
	// ----------------------------------------
	ioData = data;
	if ( addr & 1 )
	{
	// .........................................................................................................
	//
	// AD0 = 1 (I/O write address = 0x01). STORE OPCODE.
	//
	// Store (write) an "I/O operation code" (Opcode) and reset the exchanged bytes counter.
	//
	// NOTE 1: An Opcode can be a write or read Opcode, if the I/O operation is read or write.
	// NOTE 2: the STORE OPCODE operation must always precede an EXECUTE WRITE OPCODE or EXECUTE READ OPCODE
	//         operation.
	// NOTE 3: For multi-byte read opcode (as DATETIME) read sequentially all the data bytes without to send
	//         a STORE OPCODE operation before each data byte after the first one.
	// .........................................................................................................
	//
	// Currently defined Opcodes for I/O write operations:
	//
	//   Opcode     Name            Exchanged bytes
	// -------------------------------------------------
	// Opcode 0x00  USER LED        1
	// Opcode 0x01  SERIAL TX       1
	// Opcode 0x03  GPIOA Write     1
	// Opcode 0x04  GPIOB Write     1
	// Opcode 0x05  IODIRA Write    1
	// Opcode 0x06  IODIRB Write    1
	// Opcode 0x07  GPPUA Write     1
	// Opcode 0x08  GPPUB Write     1
	// Opcode 0x09  SELDISK         1
	// Opcode 0x0A  SELTRACK        2
	// Opcode 0x0B  SELSECT         1
	// Opcode 0x0C  WRITESECT       512
	// Opcode 0x0D  SETBANK         1
#if GME_EXT
	// Opcode 0x61  SELDISKSET      1 (0-2,...)
	// Opcode 0x62  SETINTENABLE    1 (0=no,1=yes)
	// Opcode 0x63  SETCPUSPEED     1 (0=4MHz,1=8MHz)
	// OpCode 0x64  RDWREEPROM		2
	// OpCode 0x65  SETKBDMAP		1
#endif
	// Opcode 0xFF  No operation    1
	//
	//
	// Currently defined Opcodes for I/O read operations:
	//
	//   Opcode     Name            Exchanged bytes
	// -------------------------------------------------
	// Opcode 0x80  USER KEY        1
	// Opcode 0x81  GPIOA Read      1
	// Opcode 0x82  GPIOB Read      1
	// Opcode 0x83  SYSFLAGS        1
	// Opcode 0x84  DATETIME        7
	// Opcode 0x85  ERRDISK         1
	// Opcode 0x86  READSECT        512
	// Opcode 0x87  SDMOUNT         1
#if GME_EXT
	// Opcode 0xE0  GETGMEEXTVER    1 (0-2,...)
	// Opcode 0xE1  GETDISKSET      1 (0-2,...)
	// Opcode 0xE2  GETINTENABLE    1 (0=no,1=yes)
	// Opcode 0xE3  GETCPUSPEED     1 (0=8MHz,1=4MHz)
	// OpCode 0x64  RDWREEPROM		2
	// OpCode 0xE5  GETKBDMAP		1
#endif
	// Opcode 0xFF  No operation    1
	//
	// See the following lines for the Opcodes details.
	//
	// .........................................................................................................
/*		if ( ioOpcode != 0xFF )
			console_.printf( "%02X: Busy OP %02X, bytes=%04X \r\n", ioData, ioOpcode, ioByteCnt );
*/
		ioOpcode = ioData;
		ioByteCnt = 0;
	}
	else
	{
	// .........................................................................................................
	//
	// AD0 = 0 (I/O write address = 0x00). EXECUTE WRITE OPCODE.
	//
	// Execute the previously stored I/O write opcode with the current data.
	// The code of the I/O write operation (Opcode) must be previously stored with a STORE OPCODE operation.
	// .........................................................................................................
	//
		switch( ioOpcode )
        // Execute the requested I/O WRITE Opcode. The 0xFF value is reserved as "No operation".
		{
		case 0x00: // user led
			// USER LED:
			//                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                              x  x  x  x  x  x  x  0    USER Led off
			//                              x  x  x  x  x  x  x  1    USER Led on
			break;
		case 0x01: // display char
			// SERIAL TX:
			//                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    ASCII char to be sent to serial
			if ( !data )
			{
				zero_ = 1;
				console_.putch( 0x08 ); //BS
			}
			else
			{
				if ( data == 0x18 )
					data = 0xFE;
				console_.putch( data );
				if ( zero_ )
				{
					console_.putch( 0x08 ); //BS
					zero_ = 0;
				}
			}
			if ( debugMode
					&& data < ' ' && data && data != 0x07 && data != 0x08 && data != 0x0A
					&& data != 0x0C && data != 0x0D && data != 0x12 && data != 0x1B
					&& data != 0x18
				/*|| data > 0x7E*/ ) console_.printf( "<%02X>", data );
			break;
		case  0x09:
			// DISK EMULATION
			// SELDISK - select the emulated disk number (binary). 100 disks are supported [0..99]:
			//
			//                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    DISK number (binary) [0..99]
			//
			//
			// Opens the "disk file" correspondig to the selected disk number, doing some checks.
			// A "disk file" is a binary file that emulates a disk using a LBA-like logical sector number.
			// Every "disk file" must have a dimension of 8388608 bytes, corresponding to 16384 LBA-like logical sectors
			//  (each sector is 512 bytes long), correspinding to 512 tracks of 32 sectors each (see SELTRACK and
			//  SELSECT opcodes).
			// Errors are stored into "errDisk" (see ERRDISK opcode).
			//
			//
			// ...........................................................................................
			//
			// "Disk file" filename convention:
			//
			// Every "disk file" must follow the sintax "DSsNnn.DSK" where
			//
			//    "s" is the "disk set" and must be in the [0..9] range (always one numeric ASCII character)
			//    "nn" is the "disk number" and must be in the [00..99] range (always two numeric ASCII characters)
			//
			// ...........................................................................................
			//
			//
			// NOTE 1: The maximum disks number may be lower due the limitations of the used OS (e.g. CP/M 2.2 supports
			//         a maximum of 16 disks)
			// NOTE 2: Because SELDISK opens the "disk file" used for disk emulation, before using WRITESECT or READSECT
			//         a SELDISK must be performed at first.

			if (ioData <= maxDiskNum)               // Valid disk number
			// Set the name of the file to open as virtual disk, and open it
			{
				diskName[2] = diskSet + 48;           // Set the current Disk Set
				diskName[4] = (ioData / 10) + 48;     // Set the disk number
				diskName[5] = ioData - ((ioData / 10) * 10) + 48;
				diskErr = openSD(diskName);           // Open the "disk file" corresponding to the given disk number
			}
			else diskErr = 16;                      // Illegal disk number
			if ( debugMode )
				this->console_.printf( "DBG: SELDISK num:%2d name:%s => err:%2d\r\n", ioData, diskName, diskErr );
			break;

		case  0x0A:
			// DISK EMULATION
			// SELTRACK - select the emulated track number (word splitted in 2 bytes in sequence: DATA 0 and DATA 1):
			//
			//                I/O DATA 0:  D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    Track number (binary) LSB [0..255]
			//
			//                I/O DATA 1:  D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    Track number (binary) MSB [0..1]
			//
			//
			// Stores the selected track number into "trackSel" for "disk file" access.
			// A "disk file" is a binary file that emulates a disk using a LBA-like logical sector number.
			// The SELTRACK and SELSECT operations convert the legacy track/sector address into a LBA-like logical
			//  sector number used to set the logical sector address inside the "disk file".
			// A control is performed on both current sector and track number for valid values.
			// Errors are stored into "diskErr" (see ERRDISK opcode).
			//
			//
			// NOTE 1: Allowed track numbers are in the range [0..511] (512 tracks)
			// NOTE 2: Before a WRITESECT or READSECT operation at least a SELSECT or a SELTRAK operation
			//         must be performed

			if (!ioByteCnt)
			// LSB
			{
				trackSel = ioData;
			}
			else
			// MSB
			{
				trackSel = (((word) ioData) << 8) | lowByte(trackSel);
#if GME_EXT
				if ((trackSel < 1024) && (sectSel < 32))
#else
				if ((trackSel < 512) && (sectSel < 32))
#endif
				// Sector and track numbers valid
				{
					diskErr = 0;                      // No errors
				}
				else
				// Sector or track invalid number
				{
					if (sectSel < 32) diskErr = 17;     // Illegal track number
					else diskErr = 18;                  // Illegal sector number
				}
				ioOpcode = 0xFF;                      // All done. Set ioOpcode = "No operation"
			}
			ioByteCnt++;
			break;

		case  0x0B:
			// DISK EMULATION
			// SELSECT - select the emulated sector number (binary):
			//
			//                  I/O DATA:  D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    Sector number (binary) [0..31]
			//
			//
			// Stores the selected sector number into "sectSel" for "disk file" access.
			// A "disk file" is a binary file that emulates a disk using a LBA-like logical sector number.
			// The SELTRACK and SELSECT operations convert the legacy track/sector address into a LBA-like logical
			//  sector number used to set the logical sector address inside the "disk file".
			// A control is performed on both current sector and track number for valid values.
			// Errors are stored into "diskErr" (see ERRDISK opcode).
			//
			//
			// NOTE 1: Allowed sector numbers are in the range [0..31] (32 sectors)
			// NOTE 2: Before a WRITESECT or READSECT operation at least a SELSECT or a SELTRAK operation
			//         must be performed

			sectSel = ioData;
#if GME_EXT
			if ((trackSel < 1024) && (sectSel < 32))
#else
			if ((trackSel < 512) && (sectSel < 32))
#endif
			// Sector and track numbers valid
			{
				diskErr = 0;                        // No errors
			}
			else
			// Sector or track invalid number
			{
				if (sectSel < 32) diskErr = 17;     // Illegal track number
				else diskErr = 18;                  // Illegal sector number
			}
			break;

		case  0x0C:
			// DISK EMULATION
			// WRITESECT - write 512 data bytes sequentially into the current emulated disk/track/sector:
			//
			//                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    First Data byte
			//
			//                      |               |
			//                      |               |
			//                      |               |                 <510 Data Bytes>
			//                      |               |
			//
			//               I/O DATA 511: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    512th Data byte (Last byte)
			//
			//
			// Writes the current sector (512 bytes) of the current track/sector, one data byte each call.
			// All the 512 calls must be always performed sequentially to have a WRITESECT operation correctly done.
			// If an error occurs during the WRITESECT operation, all subsequent write data will be ignored and
			//  the write finalization will not be done.
			// If an error occurs calling any DISK EMULATION opcode (SDMOUNT excluded) immediately before the WRITESECT
			//  opcode call, all the write data will be ignored and the WRITESECT operation will not be performed.
			// Errors are stored into "diskErr" (see ERRDISK opcode).
			//
			// NOTE 1: Before a WRITESECT operation at least a SELTRACK or a SELSECT must be always performed
			// NOTE 2: Remember to open the right "disk file" at first using the SELDISK opcode
			// NOTE 3: The write finalization on SD "disk file" is executed only on the 512th data byte exchange, so be
			//         sure that exactly 512 data bytes are exchanged.

			if (!ioByteCnt)
			// First byte of 512, so set the right file pointer to the current emulated track/sector first
			{
#if GME_EXT
				if ((trackSel < 1024) && (sectSel < 32) && (!diskErr))
#else
				if ((trackSel < 512) && (sectSel < 32) && (!diskErr))
#endif
				// Sector and track numbers valid and no previous error; set the LBA-like logical sector
				{
					diskErr = seekSD((trackSel << 5) | sectSel);  // Set the starting point inside the "disk file"
													  //  generating a 14 bit "disk file" LBA-like
													  //  logical sector address created as TTTTTTTTTSSSSS
				}
				if ( debugMode )
					this->console_.printf( "DBG: WRITESECT trk:%3d sct:%3d => seek err:%2d\r\n", trackSel, sectSel, diskErr );
			}


			if (!diskErr)
			// No previous error (e.g. selecting disk, track or sector)
			{
				tempByte = ioByteCnt % 32;            // [0..31]
				bufferSD[tempByte] = ioData;          // Store current exchanged data byte in the buffer array
				if (tempByte == 31)
				// Buffer full. Write all the buffer content (32 bytes) into the "disk file"
				{
					diskErr = writeSD(bufferSD, &numWriBytes);
					if (numWriBytes < 32) diskErr = 19; // Reached an unexpected EOF
					if (ioByteCnt >= 511)
					// Finalize write operation and check result (if no previous error occurred)
					{
						if (!diskErr) diskErr = writeSD(NULL, &numWriBytes);
						ioOpcode = 0xFF;                  // All done. Set ioOpcode = "No operation"
					}
				}
			}
			ioByteCnt++;                            // Increment the counter of the exchanged data bytes
			break;

		case  0x0D:
			// BANKED RAM
			// SETBANK - select the Os RAM Bank (binary):
			//
			//                  I/O DATA:  D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    Os Bank number (binary) [0..2]
			//
			//
			// Set a 32kB RAM bank for the lower half of the Z80 address space (from 0x0000 to 0x7FFF).
			// The upper half (from 0x8000 to 0xFFFF) is the common fixed bank.
			// Allowed Os Bank numbers are from 0 to 2.
			//
			// Please note that there are three kinds of Bank numbers (see the A040618 schematic):
			//
			// * the "Os Bank" number is the bank number managed (known) by the Os;
			// * the "Logical Bank" number is the bank seen by the Atmega32a (through BANK1 and BANK0 address lines);
			// * the "Physical Bank" number is the real bank addressed inside the RAM chip (RAM_A16 and RAM_A15 RAM
			//   address lines).
			//
			// The following tables shows the relations:
			//
			//
			//  Os Bank | Logical Bank |  Z80 Address Bus    |   Physical Bank   |            Notes
			//  number  | BANK1 BANK0  |        A15          |  RAM_A16 RAM_A15  |
			// ------------------------------------------------------------------------------------------------
			//     X    |   X     X    |         1           |     0       1     |  Phy Bank 1 (common fixed)
			//     -    |   0     0    |         0           |     0       1     |  Phy Bank 1 (common fixed)
			//     0    |   0     1    |         0           |     0       0     |  Phy Bank 0 (Logical Bank 1)
			//     2    |   1     0    |         0           |     1       1     |  Phy Bank 3 (Logical Bank 2)
			//     1    |   1     1    |         0           |     1       0     |  Phy Bank 2 (Logical Bank 3)
			//
			//
			//
			//      Physical Bank      |    Logical Bank     |   Physical Bank   |   Physical RAM Addresses
			//          number         |       number        |  RAM_A16 RAM_A15  |
			// ------------------------------------------------------------------------------------------------
			//            0            |         1           |     0       0     |   From 0x00000 to 0x07FFF
			//            1            |         0           |     0       1     |   From 0x08000 to 0x0FFFF
			//            2            |         3           |     1       0     |   From 0x01000 to 0x17FFF
			//            3            |         2           |     1       1     |   From 0x18000 to 0x1FFFF
			//
			//
			// Note that the Logical Bank 0 can't be used as switchable Os Bank bacause it is the common
			//  fixed bank mapped in the upper half of the Z80 address space (from 0x8000 to 0xFFFF).
			//
			//
			// NOTE: If the Os Bank number is greater than 2 no selection is done.

			switch (ioData)
			{
			case 0:                               // Os bank 0
				// Set physical bank 0 (logical bank 1)
				ZdigitalWrite(BANK0, HIGH);
				ZdigitalWrite(BANK1, LOW);
				break;

			case 1:                               // Os bank 1
				// Set physical bank 2 (logical bank 3)
				ZdigitalWrite(BANK0, HIGH);
				ZdigitalWrite(BANK1, HIGH);
				break;

			case 2:                               // Os bank 2
				// Set physical bank 3 (logical bank 2)
				ZdigitalWrite(BANK0, LOW);
				ZdigitalWrite(BANK1, HIGH);
				break;
			}
			break;
#if GME_EXT
		case  0x61:
			// DISK EMULATION
			// SELDISKSET - set the active virtual disk set (0-2 or more)
			//
			//                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             00 00 00 00 00 00 D1 D0    active disk set (0-2 or more)
			//
			//
			//

			diskSet = ioData;
			break;

		case  0x62:
			// SETINTENABLE - Get the interrupt enable flag (0=no,1=yes)
			//
			//                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             00 00 00 00 00 00 00 D0    interrupt enable (0=no,1=yes)
			//
			//
			//

			Z80IntEnFlag = ioData ? 1 : 0;
			break;

		case  0x63:
			// SETCPUSPEED - Set the CPU speed
			//
			//                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             00 00 00 00 00 00 00 D0    CPU speed (0=8/10MHz,1=4/5MHz)
			//
			//
			//

			clockMode = ioData;
			system_.getClock().setClockSpeed( 8000 * turboMode / ( clockMode + 1) );
			//OCR2 = clockMode;   // Set the compare value to toggle OC2 (0 = low or 1 = high)
			break;

		case 0x64:
			// EEPROM ACCESS
			// RWEEPROM - read/write 1 data byte into the EEPROM:
			//
			//                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    EPROM address (output)
			//
			//               I/O DATA 1: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    EEPROM data (input/output)
			//
			//
			// Reads/Writes 1 bytes into the EEPROM.
			switch( ioByteCnt )
			{
			case 0:
				eepromOffset = ioData;
				break;
			case 1:
				EEPROM.update( eepromOffset, ioData );
				EEPROM.save( *pfilesysSD );
				break;
			}
			break;
#endif
		case  0xFF:
			break;
		default:
			if ( debugMode )
				console_.printf( "{OUT FN%02X(%02X)}", ioOpcode, data );
			cpu_.setMode( MODE_STOP );
			break;
		}
#if GME_EXT
		if ((ioOpcode != 0x0A) && (ioOpcode != 0x0C) && (ioOpcode != 0x64)) ioOpcode = 0xFF;  // All done for the single byte opcodes.
																		  //  Set ioOpcode = "No operation"
#else
		if ((ioOpcode != 0x0A) && (ioOpcode != 0x0C)) ioOpcode = 0xFF;    // All done for the single byte opcodes.
																		  //  Set ioOpcode = "No operation"
#endif
	}
	return data;
}

uchar Z80_MBC2IO::in( ushort addr )
{
	if ( addr & 1 )
	{
		ioData = 0xFF;
		if ( console_.available() )
		{
			ioData = console_.getch();
			LastRxIsEmpty = 0;                // Reset the "Last Rx char was empty" flag
		}
		else LastRxIsEmpty = 1;             // Set the "Last Rx char was empty" flag
		cpu_.trigIRQ( 0 );
		return ioData;
	}
	else
	{
		switch( ioOpcode )
		{
		case  0x80:
	        // USER KEY:
            //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
            //                            ---------------------------------------------------------
            //                              0  0  0  0  0  0  0  0    USER Key not pressed
            //                              0  0  0  0  0  0  0  1    USER Key pressed
			ioData = userButton;
			break;
		case  0x83:
			// SYSFLAGS (Various system flags for the OS):
			//                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                              X  X  X  X  X  X  X  0    AUTOEXEC not enabled
			//                              X  X  X  X  X  X  X  1    AUTOEXEC enabled
			//                              X  X  X  X  X  X  0  X    DS3231 RTC not found
			//                              X  X  X  X  X  X  1  X    DS3231 RTC found
			//                              X  X  X  X  X  0  X  X    Serial RX buffer empty
			//                              X  X  X  X  X  1  X  X    Serial RX char available
			//                              X  X  X  X  0  X  X  X    Previous RX char valid
			//                              X  X  X  X  1  X  X  X    Previous RX char was a "buffer empty" flag
			//
			// NOTE: Currently only D0-D3 are used

			ioData = autoexecFlag | (foundRTC << 1) | ((console_.available()) << 2) | ((LastRxIsEmpty > 0) << 3);
			break;
		case  0x84:
			// DATETIME (Read date/time and temperature from the RTC. Binary values):
			//                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                I/O DATA 0   D7 D6 D5 D4 D3 D2 D1 D0    seconds [0..59]     (1st data byte)
			//                I/O DATA 1   D7 D6 D5 D4 D3 D2 D1 D0    minutes [0..59]
			//                I/O DATA 2   D7 D6 D5 D4 D3 D2 D1 D0    hours   [0..23]
			//                I/O DATA 3   D7 D6 D5 D4 D3 D2 D1 D0    day     [1..31]
			//                I/O DATA 4   D7 D6 D5 D4 D3 D2 D1 D0    month   [1..12]
			//                I/O DATA 5   D7 D6 D5 D4 D3 D2 D1 D0    year    [0..99]
			//                I/O DATA 6   D7 D6 D5 D4 D3 D2 D1 D0    tempC   [-128..127] (7th data byte)
			//
			// NOTE 1: If RTC is not found all read values wil be = 0
			// NOTE 2: Overread data (more then 7 bytes read) will be = 0
			// NOTE 3: The temperature (Celsius) is a byte with two complement binary format [-128..127]

			if (foundRTC)
			{
				if (ioByteCnt == 0)
					readRTC(&seconds, &minutes, &hours, &day, &month, &year, &tempC); // Read from RTC
				if (ioByteCnt < 7)
				// Send date/time (binary values) to Z80 bus
				{
					switch (ioByteCnt)
					{
						case 0: ioData = seconds; break;
						case 1: ioData = minutes; break;
						case 2: ioData = hours; break;
						case 3: ioData = day; break;
						case 4: ioData = month; break;
						case 5: ioData = year; break;
						case 6: ioData = tempC; break;
					}
					ioByteCnt++;
				}
				else ioOpcode = 0xFF;              // All done. Set ioOpcode = "No operation"
			}
			else ioOpcode = 0xFF;                 // Nothing to do. Set ioOpcode = "No operation"
			break;

		case  0x85:
			// DISK EMULATION
			// ERRDISK - read the error code after a SELDISK, SELSECT, SELTRACK, WRITESECT, READSECT
			//           or SDMOUNT operation
			//
			//                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    DISK error code (binary)
			//
			//
			// Error codes table:
			//
			//    error code    | description
			// ---------------------------------------------------------------------------------------------------
			//        0         |  No error
			//        1         |  DISK_ERR: the function failed due to a hard error in the disk function,
			//                  |   a wrong FAT structure or an internal error
			//        2         |  NOT_READY: the storage device could not be initialized due to a hard error or
			//                  |   no medium
			//        3         |  NO_FILE: could not find the file
			//        4         |  NOT_OPENED: the file has not been opened
			//        5         |  NOT_ENABLED: the volume has not been mounted
			//        6         |  NO_FILESYSTEM: there is no valid FAT partition on the drive
			//       16         |  Illegal disk number
			//       17         |  Illegal track number
			//       18         |  Illegal sector number
			//       19         |  Reached an unexpected EOF
			//
			//
			//
			//
			// NOTE 1: ERRDISK code is referred to the previous SELDISK, SELSECT, SELTRACK, WRITESECT or READSECT
			//         operation
			// NOTE 2: Error codes from 0 to 6 come from the PetitFS library implementation
			// NOTE 3: ERRDISK must not be used to read the resulting error code after a SDMOUNT operation
			//         (see the SDMOUNT opcode)

			ioData = diskErr;
			break;

		case  0x86:
			// DISK EMULATION
			// READSECT - read 512 data bytes sequentially from the current emulated disk/track/sector:
			//
			//                 I/O DATA:   D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                 I/O DATA 0  D7 D6 D5 D4 D3 D2 D1 D0    First Data byte
			//
			//                      |               |
			//                      |               |
			//                      |               |                 <510 Data Bytes>
			//                      |               |
			//
			//               I/O DATA 127  D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    512th Data byte (Last byte)
			//
			//
			// Reads the current sector (512 bytes) of the current track/sector, one data byte each call.
			// All the 512 calls must be always performed sequentially to have a READSECT operation correctly done.
			// If an error occurs during the READSECT operation, all subsequent read data will be = 0.
			// If an error occurs calling any DISK EMULATION opcode (SDMOUNT excluded) immediately before the READSECT
			//  opcode call, all the read data will be will be = 0 and the READSECT operation will not be performed.
			// Errors are stored into "diskErr" (see ERRDISK opcode).
			//
			// NOTE 1: Before a READSECT operation at least a SELTRACK or a SELSECT must be always performed
			// NOTE 2: Remember to open the right "disk file" at first using the SELDISK opcode

			if (!ioByteCnt)
			// First byte of 512, so set the right file pointer to the current emulated track/sector first
			{
#if GME_EXT
				if ((trackSel < 1024) && (sectSel < 32) && (!diskErr))
#else
				if ((trackSel < 512) && (sectSel < 32) && (!diskErr))
#endif
				// Sector and track numbers valid and no previous error; set the LBA-like logical sector
				{
					diskErr = seekSD((trackSel << 5) | sectSel);  // Set the starting point inside the "disk file"
														//  generating a 14 bit "disk file" LBA-like
														//  logical sector address created as TTTTTTTTTSSSSS
				}
				if ( debugMode )
					this->console_.printf( "DBG: READSECT trk:%3d sct:%3d => seek err:%2d\r\n", trackSel, sectSel, diskErr );
			}


			if (!diskErr)
			// No previous error (e.g. selecting disk, track or sector)
			{
				tempByte = ioByteCnt % 32;          // [0..31]
				if (!tempByte)
				// Read 32 bytes of the current sector on SD in the buffer (every 32 calls, starting with the first)
				{
					diskErr = readSD(bufferSD, &numReadBytes);
					if (numReadBytes < 32) diskErr = 19;    // Reached an unexpected EOF
				}
				if (!diskErr) ioData = bufferSD[tempByte];// If no errors, exchange current data byte with the CPU
			}
			if (ioByteCnt >= 511)
			{
				ioOpcode = 0xFF;                    // All done. Set ioOpcode = "No operation"
			}
			ioByteCnt++;                          // Increment the counter of the exchanged data bytes
			break;

		case  0x87:
			// DISK EMULATION
			// SDMOUNT - mount a volume on SD, returning an error code (binary):
			//
			//                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    error code (binary)
			//
			//
			//
			// NOTE 1: This opcode is "normally" not used. Only needed if using a virtual disk from a custom program
			//         loaded with iLoad or with the Autoboot mode (e.g. ViDiT). Can be used to handle SD hot-swapping
			// NOTE 2: For error codes explanation see ERRDISK opcode
			// NOTE 3: Only for this disk opcode, the resulting error is read as a data byte without using the
			//         ERRDISK opcode

			ioData = mountSD(pfilesysSD);
			break;
#if GME_EXT
		case  0xE0:
			// Get GME_EXT version
			ioData = gmeExtVersion; // Highest IOS OpCode for GME_ext
			break;

		case  0xE1:
			// DISK EMULATION
			// GETDISKSET - get the active virtual disk set (0-2 or more)
			//
			//                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             00 00 00 00 00 00 D1 D0    active disk set (0-2 or more)
			//
			//
			//

			ioData = diskSet;
			break;

		case  0xE2:
			// DISK EMULATION
			// GETINTENABLE - Get the interrupt enable flag (0=no,1=yes)
			//
			//                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             00 00 00 00 00 00 00 D0    interrupt enable (0=no,1=yes)
			//
			//
			//

			ioData = Z80IntEnFlag;
			break;

		case  0xE3:
			// GETCPUSPEED - Get the CPU speed
			//
			//                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             00 00 00 00 00 00 00 D0    CPU speed (0=8/10MHz,1=4/5MHz)
			//
			//
			//

			ioData = clockMode;
			break;
		case  0x64:
			// EEPROM ACCESS
			// RWEEPROM - read/write 1 data byte into the EEPROM:
			//
			//                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    EPROM address (output)
			//
			//               I/O DATA 1: D7 D6 D5 D4 D3 D2 D1 D0
			//                            ---------------------------------------------------------
			//                             D7 D6 D5 D4 D3 D2 D1 D0    EEPROM data (input/output)
			//
			//
			// Reads 1 bytes into the EEPROM.
			if ( ioByteCnt == 1 )
				ioData = EEPROM.read( eepromOffset );
			++ioByteCnt;
			break;
#endif
		case  0xFF:
			break;
		default:
			console_.printf( "{IN FN%02X()}", ioOpcode );
			break;
		}
#if GME_EXT
		if ((ioOpcode != 0x84) && (ioOpcode != 0x86) && (ioOpcode != 0x64)) ioOpcode = 0xFF;  // All done for the single byte opcodes.
#else
		if ((ioOpcode != 0x84) && (ioOpcode != 0x86)) ioOpcode = 0xFF;  // All done for the single byte opcodes.
#endif
		return ioData;
	}
	return 0;
}

void Z80_MBC2System::reset()
{
	io_BANK0 = 1;
	io_BANK1 = 0;
	cpu_.reset();
}

static void hexDumpLine( Console_I &con, Memory_I &mem, ushort p )
{
	con.printf( "%04X :", p );
	for ( int j=0; j<16; ++j )
	{
		if ( !(j&7) )
			con.putch( ' ' );
		con.printf( "%02X ", mem.read( p++ ) );
	}
	p -= 16;
	for ( int j=0; j<16; ++j )
	{
		if ( !(j&7) )
			con.putch( ' ' );
		uchar c=mem.read( p++ );
		con.putch( ( c<' ' || c>0x7F ) ? '.' : c );
	}
	con.println();
}

static void hexDump( Console_I &con, Memory_I &mem, ushort p )
{
	for ( int i=0; i<16; ++i )
	{
		hexDumpLine( con, mem, p );
		p +=16 ;
	}
}

static void showHelp( Console_I &con )
{
	con.puts(
		"\r\n I = Instruction Step"
		"\r\n C = CALL Step"
		"\r\n G = Go"
		"\r\n E = Exec until $BREAK"
		"\r\n B = Exec until breakpoint"
		"\r\n X = Exec until RET"
		"\r\n R = Show reg names"
		"\r\n M = Registers indirect dump"
		"\r\n H = Hex dump current page"
		"\r\n ; = Hex dump next page"
		"\r\n - = Hex dump prev page"
		"\r\n . = Hex dump 16 pages forward"
		"\r\n _ = Hex dump 16 pages backward"
		"\r\n F = Font set"
		);
}

static void showCharSet( Console_I &con )
{
	con.puts( "\r\n   " );
	for (int i=0; i<16; ++i)
		con.printf( "%02X ", i );
	con.println();
	for (int i=0; i<16; ++i )
	{
		con.printf( "%02X  ", i<<4 );
		for (int j=0; j<16; ++j )
		{
			int c=(i<<4)+j;
			if ( !c || c>=0x07 && c<=0x0A || c==0x0C || c==0x0D || c==0x1B )
				c=' ';
			con.putch( c );
			con.puts( "  " );
		}
		con.println();
	}
}

static uint lastpc = 0, pc = 0, nextpc = 0;
static unsigned ttyTimer = 0;
static unsigned buttonTimer = 0;
static int regLines = 0;
static uint bkpt = 0xFFFF;
static uint lastbkpt = 0xFFFF;
static uint retsp = 0;
static bool breakon = true;
static ushort hexptr = 0;

void Z80_MBC2System::setRamBuffer( void* data, size_t size )
{
    data_ = (uchar*)data;
    ram_.setBuffer( data_, size );
    cpuram_.update();
}

void Z80_MBC2System::setup()
{

    if ( !data_ )
    {
        mainStream.println( "ps_malloc failed ==> HALT" );
        for (;;){}
    }

    LOGD( "** PSRAM OK **" );

    ram_.setBuffer( data_, RAM_SIZE );
    cpuram_.update();
    cpu_.setMemory( &cpuram_ );
    cpu_.setCode( &cpuram_ );
    LOGD( "** resetting CPU **" );
	cpu_.reset();
	mode_.setMode( debugMode ? MODE_STOP : MODE_RUN );
    LOGW( "** Mode: %s **", debugMode ? "MODE_STOP" : "MODE_RUN"  );
}

void Z80_MBC2System::loop()
{
    for ( int loop=0; loop<1000; ++loop )
    {
    	lastpc = pc;
    	pc = cpu_.getPC();
    
    	//if ( pc == 0x0038 )
    	//	mode_.setMode( MODE_STOP );
    
    	if ( mode_.getMode() == MODE_STOP )
    	{
            volatile Z80CPU::REGS  &z80regs  = cpu_.getRegs();
            volatile Z80CPU::FLAGS &z80flags = cpu_.getFlags();
    
    		if ( !regLines )
    			systemConsole_.printf( "\r\n                                  A  F  SZyHxPNC -BC- -DE- -HL- -IX- -IY- -SP-" );
    		regLines = ( regLines + 1 ) % 16;
    		disass_.setPC( pc );
    		systemConsole_.printf( "\r\n%04X ", pc );
    		const char *src = disass_.source();
    		nextpc = disass_.getPC();
    		int i = 0;
    		for ( ; pc < nextpc; ++pc, ++i )
    			systemConsole_.printf( "%02X", cpuram_.read( pc ) );
    		for ( ; i < 4; ++i )
    			systemConsole_.printf( "  " );
    		systemConsole_.printf( " %.20s", src );
    		char irq = cpu_.getIntEn()
    			? ( cpu_.getIRQ() ? '*' : '+' )
    			: ( cpu_.getIRQ() ? '=' : '-' );
    		systemConsole_.printf( "%02X %02X%c%d%d%d%d%d%d%d%d %04X %04X %04X %04X %04X %04X ",
    			z80regs.h.a, z80regs.h.f, irq,
    			z80flags.s&1, z80flags.z&1, z80flags.y&1, z80flags.h&1, z80flags.x&1, z80flags.p&1, z80flags.n&1, z80flags.c&1,
    			z80regs.x.bc, z80regs.x.de, z80regs.x.hl, z80regs.x.ix, z80regs.x.iy, z80regs.x.sp);
    
    		pc = cpu_.getPC();
    
    		int c = toupper( systemConsole_.getch() );
    
    		if ( !c )
    			return;
    
    		breakon = true;
    
    		if ( c == 'G' ) // GO
    		{
    			breakon = false;
    			regLines = 0;
    			mode_.setMode( MODE_RUN );
    			systemConsole_.println();
    		}
    		else if ( c == 'R' ) // SHOW REGISTER NAMES
    		{
    			regLines = 0;
    		}
    		else if ( c == 'B' ) // BREAKPOINT
    		{
    			char buf[33];
    			buf[0]=30;
    			buf[1]=buf[2]=0;
    			systemConsole_.puts( "\r\nBreakpoint: " );
    			char *str = systemConsole_.gets( buf );
    			if ( !str )
    			{
    				regLines = 0;
    				return;
    			}
    			if ( !buf[2] )
    			{
    				bkpt = lastbkpt;
    				systemConsole_.printf( "Breakpoint: %04X\r\n", bkpt );
    			}
    			else
    			{
    				sscanf( buf+2, "%x", &bkpt );
    				lastbkpt = bkpt;
    			}
    			mode_.setMode( MODE_RUN );
    		}
    		else if ( c == 'C' ) // CALL STEP
    		{
    			uchar x = cpuram_.read( pc );
    			uchar x1 = cpuram_.read( pc+1 );
    			if	(	x==0xCD									// CALL nnnn
    				||	( x>0xC0 && ( (x&7)==4 || (x&7)==7 ) )	// CALL nz|z|nc|c|...,nnnn ; RST  nn
    				||	( x==0xED && (x1 & 0xF4)==0xB0 )		// xxIR|xxDR
    				)
    			{
    				bkpt = nextpc;
    				mode_.setMode( MODE_RUN );
    				systemConsole_.println();
    			}
    			else
    			{
    				cpu_.sim();
    			}
    		}
    		else if ( c == 'X' ) // UNTIL RET
    		{
    			retsp = cpu_.getRegs().x.sp;
    			mode_.setMode( MODE_RET );
    			systemConsole_.println();
    		}
    		else if ( c == 'E' ) // EXEC UNTIL $BREAK
    		{
    			regLines = 0;
    			mode_.setMode( MODE_RUN );
    			systemConsole_.println();
    		}
    		else if ( c == 'H' ) // HEX DUMP
    		{
    			regLines = 0;
    			systemConsole_.println();
    			hexDump( systemConsole_, cpuram_, hexptr );
    		}
    		else if ( c == ';' ) // HEX DUMP NEXT PAGE
    		{
    			hexptr += 0x0100;
    			regLines = 0;
    			systemConsole_.println();
    			hexDump( systemConsole_, cpuram_, hexptr );
    		}
    		else if ( c == '-' ) // HEX DUMP PREV PAGE
    		{
    			hexptr -= 0x0100;
    			regLines = 0;
    			systemConsole_.println();
    			hexDump( systemConsole_, cpuram_, hexptr );
    		}
    		else if ( c == '.' ) // HEX DUMP 16 PAGES FORWARD
    		{
    			hexptr += 0x1000;
    			regLines = 0;
    			systemConsole_.println();
    			hexDump( systemConsole_, cpuram_, hexptr );
    		}
    		else if ( c == '_' ) // HEX DUMP 16 PAGES BACKWARD
    		{
    			hexptr -= 0x1000;
    			regLines = 0;
    			systemConsole_.println();
    			hexDump( systemConsole_, cpuram_, hexptr );
    		}
    		else if ( c == 'M' ) // HEX DUMP PTRS
    		{
    			regLines = 0;
    			systemConsole_.println();
    			systemConsole_.puts( "BC=" );
    			hexDumpLine( systemConsole_, cpuram_, z80regs.x.bc );
    			systemConsole_.puts( "DE=" );
    			hexDumpLine( systemConsole_, cpuram_, z80regs.x.de );
    			systemConsole_.puts( "HL=" );
    			hexDumpLine( systemConsole_, cpuram_, z80regs.x.hl );
    			systemConsole_.puts( "IX=" );
    			hexDumpLine( systemConsole_, cpuram_, z80regs.x.ix );
    			systemConsole_.puts( "IY=" );
    			hexDumpLine( systemConsole_, cpuram_, z80regs.x.iy );
    		}
    		else if ( c == ' ' || c == 'I' ) // INSTRUCTION STEP
    		{
    			cpu_.sim();
    		}
    		else if ( c == 'F' ) // HELP
    		{
    			showCharSet( systemConsole_ );
    			regLines = 0;
    		}
    		else if ( c == '?' ) // HELP
    		{
    			showHelp( systemConsole_ );
    			regLines = 0;
    		}
    	}
    	else
    	{
    		if ( !ttyTimer-- )
    		{
    			ttyTimer = 10;
    			if ( systemConsole_.kbhit() )
    			{
    				if ( Z80IntEnFlag )
    				{
    					cpu_.trigIRQ( 1 );
    				}
    
    				int ch = systemConsole_.getch();
    				// next commented block not working with ansi esc sequences !!
    				//while ( ch != 0x1B && systemConsole_.kbhit() )
    				//	ch = systemConsole_.getch();
    				systemConsole_.ungetch( ch );
    	//			mode_.setMode( MODE_STOP );
    			}
    		}
    
#if USE_BUTTON
            if ( !buttonTimer-- )
            {
                buttonTimer = 100000;
                userButton = !digitalRead(USER_BUTTON);
            }
#endif
    
    		lastpc = cpu_.getPC();
    		cpu_.Z80CPU::sim();
    
    		if ( !breakon && mode_.getMode() == MODE_STOP )
    		{
    			if	(	cpuram_.read( lastpc ) == 0xED
    				&&	cpuram_.read( lastpc+1 ) == 0xF5
    				)
    			{
    				mode_.setMode( MODE_RUN );
    			}
    		}
    
            if ( debugMode && userButton )
            {
                mode_.setMode( MODE_STOP );
                userButton = 0;
            }
    
    		if ( cpu_.getPC() == bkpt )
    		{
    			mode_.setMode( MODE_STOP );
    			bkpt = 0;
    		}
    		else if ( mode_.getMode() == MODE_RET )
    		{
    			uchar x = cpuram_.read( lastpc );
    			uchar x1 = cpuram_.read( lastpc+1 );
    			if	(	x == 0xC9						// RET
    				||	( x >= 0xC0 && (x&7) == 0 )		// RET nz|z|nc|c|...
    				)
    			{
    				auto &sp = cpu_.getRegs().x.sp;
    				if	(	cpu_.getPC() != lastpc+1
    					&&	sp > retsp
    					&&	sp < retsp+0x20
    					)
    				{
    					mode_.setMode( MODE_STOP );
    				}
    			}
    			else if	(	x == 0x31						// LD SP,nnnn
    					||	x == 0xF9						// LD SP,HL
    					||	x == 0xED && x1 == 0x7B			// LD SP,(nnnn)
    					)
    			{
    				mode_.setMode( MODE_STOP );
    			}
    		}
    
    
    		if ( mode_.getMode() == MODE_STOP )
    		{
    			disass_.setPC( lastpc );
    			systemConsole_.printf( "\r\n%04X ", lastpc );
    			const char *src = disass_.source();
    			int i = 0;
    			for ( ; lastpc < disass_.getPC(); ++lastpc, ++i )
    				systemConsole_.printf( "%02X", cpuram_.read( lastpc ) );
    			for ( ; i < 4; ++i )
    				systemConsole_.printf( "  " );
    			systemConsole_.printf( " %.19s", src );
    			regLines = 0;
    		}
    	}
    }
}
