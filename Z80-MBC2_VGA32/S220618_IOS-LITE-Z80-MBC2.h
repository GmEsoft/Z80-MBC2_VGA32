#pragma once

#include "runtime.h"

typedef uint	word;
typedef uchar	byte;

#if 0
const word  boot_A_StrAddr = 0x0000;// Payload A image starting address
extern const byte  boot_A_[] ;				// Payload A image (S210718 uBIOS + Basic)
extern const word  boot_A_size;
#endif

#if 1
const word  boot_B_StrAddr = 0xfd10;// Payload B image starting address
extern const byte  boot_B_[] ;				// Payload B image (S200718 iLoad)
extern const word  boot_B_size;
#endif

#if 0
const word  boot_C_StrAddr = 0x0100;// Payload C image starting address
extern const byte  boot_C_[] ;				// Payload C image (Forth)
extern const word  boot_C_size;
#endif
