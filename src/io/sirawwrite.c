#include "PR/os_internal.h"
#include "PRinternal/siint.h"
#include "assert.h"

// TODO: this comes from a header
#ident "$Revision: 1.17 $"

s32 __osSiRawWriteIo(u32 devAddr, u32 data) {
    assert((devAddr & 0x3) == 0);

    if (__osSiDeviceBusy()) {
        return -1;
    }

    IO_WRITE(devAddr, data);
    return 0;
}
