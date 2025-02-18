#include "PR/os_internal.h"
#include "PR/ultraerror.h"
#include "PR/rcp.h"
#include "PRinternal/osint.h"

// TODO: this comes from a header
#ident "$Revision: 1.17 $"

/**
 * Submits an audio buffer to be consumed by the Audio DAC. The audio interface can queue a second DMA
 * while another is in progress and automatically begin the next one as soon as the current DMA
 * completes. If there is already a second DMA queued (DMA is full), -1 is returned to indicate the
 * buffer could not be submitted.
 *
 * @param bufPtr Next audio buffer. Must be an 8-byte aligned KSEG0 (0x80XXXXXX) address.
 * @param size Length of next audio buffer in bytes, maximum size 0x40000 bytes / 256 KiB. Should be a
 * multiple of 8.
 * @return 0 if the DMA was enqueued successfully, -1 if the DMA could not yet be queued.
 */
s32 osAiSetNextBuffer(void* bufPtr, u32 size) {
    static u8 hdwrBugFlag = FALSE;
    u8* bptr;

    if (__osAiDeviceBusy()) {
        return -1;
    }

#ifdef _DEBUG
    if ((u32)bufPtr & (8 - 1)) {
        __osError(ERR_OSAISETNEXTBUFFER_ADDR, 1, bufPtr);
        return -1;
    }

    if ((u32)size & (8 - 1)) {
        __osError(ERR_OSAISETNEXTBUFFER_SIZE, 1, size);
        return -1;
    }
#endif

    bptr = bufPtr;

    if (hdwrBugFlag) {
        bptr = (u8*)bufPtr - 0x2000;
    }

    if ((((u32)bufPtr + size) & 0x1fff) == 0) {
        hdwrBugFlag = TRUE;
    } else {
        hdwrBugFlag = FALSE;
    }

    IO_WRITE(AI_DRAM_ADDR_REG, osVirtualToPhysical(bptr));
    IO_WRITE(AI_LEN_REG, size);
    return 0;
}
