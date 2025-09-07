#include "os.h"
#include "PRinternal/osint.h"
#include "osint_debug.h"
#include <stdarg.h>

void __assertBreak(void);

void __assert(const char* exp, const char* filename, int line) {
#ifndef _FINALROM
    osSyncPrintf("\nASSERTION FAULT: %s, %d: \"%s\"\n", filename, line, exp);
    __assertBreak();
#endif
}

void __assertf(const char* exp, const char* filename, int line, const char* fmt, ...) {
#ifndef _FINALROM
    va_list args;

    va_start(args, fmt);

    osSyncPrintf("\nASSERTION FAULT: %s, %d: \"%s\"\n", filename, line, exp);
    osSyncPrintf(fmt, args);
    osSyncPrintf("\n");

    va_end(args);
    __assertBreak(); // Doesn't actually do anything, but is needed for matching
#endif
}
