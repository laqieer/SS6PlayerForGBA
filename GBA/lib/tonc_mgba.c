//
//  mGBA logging functions
//
//! \file tonc_mgba.c
//! \author Nick Sells
//! \date 20210401
//

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "tonc_math.h"
#include "tonc_mgba.h"

// --------------------------------------------------------------------
// FUNCTIONS 
// --------------------------------------------------------------------

//! Outputs \a str to mGBA's logger with \a level priority
//! \note adapted from https://github.com/GValiente/butano/blob/master/butano/hw/src/bn_hw_log.cpp
void mgba_log(const u32 level, const char* str) {
    REG_LOG_ENABLE = 0xC0DE;
    u32 chars_left = strlen(str);

    while(chars_left) {
        u32 chars_to_write = min(chars_left, LOG_MAX_CHARS_PER_LINE);

        memcpy(REG_LOG_STR, str, chars_to_write);
        REG_LOG_LEVEL = level; //every time this is written to, mGBA creates a new log entry

        str += chars_to_write;
        chars_left -= chars_to_write;
    }
}

//! Outputs \a fmt formatted with varargs to mGBA's logger with \a level priority
void mgba_printf(const u32 level, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char buf[LOG_MAX_CHARS_PER_LINE];
	vsnprintf(buf, LOG_MAX_CHARS_PER_LINE, fmt, args);
	mgba_log(level, buf);

	va_end(args);
}

// EOF
