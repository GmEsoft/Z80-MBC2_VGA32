#pragma once

#include <fcntl.h>
#include <sys/stat.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef O_ACCMODE
#define O_ACCMODE	( ! ( O_CREAT | O_EXCL | O_TRUNC | O_APPEND | O_BINARY ) )
#endif
