#ifndef _FINALROM

#include "PR/os_internal.h"
#include "PR/rcp.h"
#include "PRinternal/osint.h"

#include "PRinternal/macros.h"

typedef struct {
    /* 0x0 */ unsigned int inst1;
    /* 0x4 */ unsigned int inst2;
    /* 0x8 */ unsigned int inst3;
    /* 0xC */ unsigned int inst4;
} __osExceptionVector;

extern __osExceptionVector __isExpJP;

void MonitorInitBreak(void);

#define ISV_BASE       gISVDbgPrnAdrs
#define ISV_MAGIC_ADDR (ISV_BASE + 0x00)
#define ISV_GET_ADDR   (ISV_BASE + 0x04)
#define ISV_PUT_ADDR   (ISV_BASE + 0x14)
#define ISV_BUFFER     (ISV_BASE + 0x20)

#define ISV_BUFFER_LEN (0x10000 - 0x20)

#define IS64_MAGIC 0x49533634 // 'IS64'

__osExceptionVector ramOldVector ALIGNED(0x8);
u32 gISVFlag;
u16 gISVChk;
u32 gISVDbgPrnAdrs = 0x13FF0000;
u32 leoComuBuffAdd;

static OSPiHandle* is_Handle;

void isPrintfInit(void) {
    is_Handle = osCartRomInit();

    osEPiWriteIo(is_Handle, ISV_PUT_ADDR, 0);
    osEPiWriteIo(is_Handle, ISV_GET_ADDR, 0);
    osEPiWriteIo(is_Handle, ISV_MAGIC_ADDR, IS64_MAGIC);
}

static void* is_proutSyncPrintf(void* arg, const u8* str, u32 count) {
    u32 data;
    s32 p;
    s32 start;
    s32 end;

    if (gISVDbgPrnAdrs == 0) {
        return 0;
    }

    osEPiReadIo(is_Handle, ISV_MAGIC_ADDR, &data);
    if (data != IS64_MAGIC) {
        return 1;
    }
    osEPiReadIo(is_Handle, ISV_GET_ADDR, &data);
    p = data;
    osEPiReadIo(is_Handle, ISV_PUT_ADDR, &data);

    start = data;
    end = start + count;

    if (end >= ISV_BUFFER_LEN) {
        end -= ISV_BUFFER_LEN;
        if (p < end || start < p) {
            return 1;
        }
    } else {
        if (start < p && p < end) {
            return 1;
        }
    }
    while (count) {
        if (*str != '\0') {
            s32 shift = start & 3;
            u32 addr = ISV_BUFFER + (start & 0xFFFFFFC);

            shift = (3 - shift) * 8;

            osEPiReadIo(is_Handle, addr, &data);
            osEPiWriteIo(is_Handle, addr, (data & ~(0xff << shift)) | (*str << shift));

            start++;
            if (start >= ISV_BUFFER_LEN) {
                start -= ISV_BUFFER_LEN;
            }
        }
        count--;
        str++;
    }
    osEPiWriteIo(is_Handle, ISV_PUT_ADDR, start);

    return 1;
}

void __osInitialize_isv(void) {
    void (*fn)(void);
    OSPiHandle* hnd;
    s32 pad;
    s32 pad2;

    if (gISVDbgPrnAdrs != 0) {
        __printfunc = is_proutSyncPrintf;
        isPrintfInit();
    }
    if (gISVChk & 2) {
        hnd = osCartRomInit();

        ramOldVector = *(__osExceptionVector*)E_VEC;
        *(__osExceptionVector*)E_VEC = __isExpJP;

        osWritebackDCache(&ramOldVector, 0x10);
        osInvalICache(&ramOldVector, 0x10);
        osWritebackDCache(0x80000000, 0x190);
        osInvalICache(0x80000000, 0x190);
        osEPiReadIo(hnd, 0xBFF00010, (u32*)&fn);
        fn();
    }
    if (gISVChk & 2) {
        MonitorInitBreak();
    }
}

#endif
