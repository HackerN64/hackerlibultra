#include "PR/os_internal.h"
#include "assert.h"
#include "PRinternal/siint.h"

// Adjust line numbers to match assert

// TODO: this comes from a header
#ident "$Revision: 1.17 $"

s32 __osSiRawReadIo(u32 devAddr, u32* data) {
    assert((devAddr & 0x3) == 0);
    assert(data != NULL);

    if (__osSiDeviceBusy()) {
        return -1;
    }

    *data = IO_READ(devAddr);
    return 0;
}
