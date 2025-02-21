#include "PR/os_internal.h"
#include "PRinternal/controller.h"
#include "PRinternal/siint.h"

/*
 * This function specifies the number of devices for the functions to access when those functions access
 * to multiple direct SI devices.
 */
s32 osContSetCh(u8 ch) {
    s32 ret = 0;

    __osSiGetAccess();

    if (ch > MAXCONTROLLERS) {
        __osMaxControllers = MAXCONTROLLERS;
    } else {
        __osMaxControllers = ch;
    }

    __osContLastCmd = CONT_CMD_END;
    __osSiRelAccess();
    return ret;
}

s32 osContSetMask(u8 mask) {
    s32 ret = 0;
    s32 i;

    __osSiGetAccess();

    if (mask > (CONT_P1 | CONT_P2 | CONT_P3 | CONT_P4)) {
        __osControllerMask = CONT_P1 | CONT_P2 | CONT_P3 | CONT_P4;
    } else {
        __osControllerMask = mask;
    }

    __osContLastCmd = CONT_CMD_END;
    __osSiRelAccess();
    return ret;
}
