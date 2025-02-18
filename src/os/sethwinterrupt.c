#include "PR/os_internal.h"

// A stack frame was added to hardware interrupt handlers in 2.0J

struct __osHwInt {
    s32 (*handler)(void);
    void* stackEnd;
};

extern struct __osHwInt __osHwIntTable[];

void __osSetHWIntrRoutine(OSHWIntr interrupt, s32 (*handler)(void), void* stackEnd) {
    register u32 saveMask = __osDisableInt();

    __osHwIntTable[interrupt].handler = handler;
    __osHwIntTable[interrupt].stackEnd = stackEnd;
    __osRestoreInt(saveMask);
}
