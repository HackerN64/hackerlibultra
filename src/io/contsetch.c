#include "PR/os_internal.h"
#include "PRinternal/controller.h"
#include "PRinternal/siint.h"

/*
 * This function specifies the number of devices for the functions to access when those functions access
 * to multiple direct SI devices.
 */
s32 osContSetCh(u8 ch) {
    return osContSetMask((1 << ch) - 1);
}

/*
 * Use a mask to detect controller channels instead, allowing the PIF to skip channels instead of
 * spending time reading data.
 * This does not work on iQue Player.
 */
s32 osContSetMask(u8 ch) {
    s32 ret = 0;

#ifdef BBPLAYER
    __osControllerMask = (CONT_P1 | CONT_P2 | CONT_P3 | CONT_P4);
    __osMaxControllers = MAXCONTROLLERS;
    return ret;
#else
    __osSiGetAccess();

    if ((ch == 0) || (ch > (CONT_P1 | CONT_P2 | CONT_P3 | CONT_P4))) {
        __osControllerMask = (CONT_P1 | CONT_P2 | CONT_P3 | CONT_P4);
        __osMaxControllers = MAXCONTROLLERS;
    } else {
        __osControllerMask = ch;
        for (s32 i = 0; i < MAXCONTROLLERS; i++) {
            if (ch & (1 << i)) {
                __osMaxControllers = i;
            }
        }
        __osMaxControllers++;
    }

    __osContLastCmd = CONT_CMD_END;
    __osSiRelAccess();
    return ret;
#endif
}
