#include "PRinternal/macros.h"
#include "PR/os_internal.h"
#include "PRinternal/controller.h"
#include "PRinternal/siint.h"

#define GCN_C_STICK_THRESHOLD 38

static void __osPackReadData(void);
static u16 __osTranslateGCNButtons(u16, s32, s32);
static u16 __osTranslateN64Buttons(u16);

static OSContButtonMap __osDefaultControllerMap = {
    .buttonMap = {
        .l_jpad = L_JPAD,
        .r_jpad = R_JPAD,
        .d_jpad = D_JPAD,
        .u_jpad = U_JPAD,
        .z_trig = Z_TRIG,
        .r_trig = R_TRIG,
        .l_trig = L_TRIG,
        .a_button = A_BUTTON,
        .b_button = B_BUTTON,
        .x_button = GCN_X_BUTTON,
        .y_button = GCN_Y_BUTTON,
        .start_button = START_BUTTON,
        .get_origin = CONT_GCN_GET_ORIGIN,
        .use_origin = CONT_GCN_USE_ORIGIN,
    },
    .cStickDeadzone = 38,
    .cStickMap = {
        .left = L_CBUTTONS,
        .right = R_CBUTTONS,
        .up = U_CBUTTONS,
        .down = D_CBUTTONS,
    },
};

static OSContButtonMap* __osContCurButtonMap = &__osDefaultControllerMap;

s32 osContStartReadData(OSMesgQueue* mq) {
    s32 ret = 0;

    __osSiGetAccess();

    if (__osContLastCmd != CONT_CMD_READ_BUTTON) {
        __osPackReadData();
        ret = __osSiRawStartDma(OS_WRITE, __osContPifRam.ramarray);
        osRecvMesg(mq, NULL, OS_MESG_BLOCK);
    }

    ret = __osSiRawStartDma(OS_READ, __osContPifRam.ramarray);
    __osContLastCmd = CONT_CMD_READ_BUTTON;
    __osSiRelAccess();

    return ret;
}

void osContGetReadData(OSContPad* data) {
    u8* ptr = (u8*)__osContPifRam.ramarray;
    __OSContReadFormat readformat;
    __OSContGCNShortPollFormat readformatgcn;
    int i;

    for (i = 0; i < __osMaxControllers; i++, data++) {
        if (__osControllerTypes[i] == CONT_TYPE_GCN) {
            s32 stick_x, stick_y, c_stick_x, c_stick_y;
            readformatgcn = *(__OSContGCNShortPollFormat*)ptr;
            // The analog stick data is encoded unsigned, with (0, 0) being the bottom left of the stick plane,
            //  compared to the N64 where (0, 0) is the center. We correct it here so that the end user does not
            //  have to account for this discrepancy.
            stick_x = ((s32)readformatgcn.stick_x) - 128;
            stick_y = ((s32)readformatgcn.stick_y) - 128;
            data->stick_x = stick_x;
            data->stick_y = stick_y;
            c_stick_x = ((s32)readformatgcn.c_stick_x) - 128;
            c_stick_y = ((s32)readformatgcn.c_stick_y) - 128;
            data->c_stick_x = c_stick_x;
            data->c_stick_y = c_stick_y;
            data->button = __osTranslateGCNButtons(readformatgcn.button, c_stick_x, c_stick_y);
            data->l_trig = readformatgcn.l_trig;
            data->r_trig = readformatgcn.r_trig;
            ptr += sizeof(__OSContGCNShortPollFormat);
        } else {
            readformat = *(__OSContReadFormat*)ptr;
            data->errno = CHNL_ERR(readformat);

            if (data->errno != 0) {
                ptr += sizeof(__OSContReadFormat);
                continue;
            }

            data->button = __osTranslateN64Buttons(readformat.button);
            data->stick_x = readformat.stick_x;
            data->stick_y = readformat.stick_y;
            data->c_stick_x = 0;
            data->c_stick_y = 0;
            data->l_trig = 0;
            data->r_trig = 0;
            ptr += sizeof(__OSContReadFormat);
        }
    }
}

static void __osPackReadData(void) {
    u8* ptr = (u8*)__osContPifRam.ramarray;
    __OSContReadFormat readformat;
    __OSContGCNShortPollFormat readformatgcn;
    int i;

    for (i = 0; i < ARRLEN(__osContPifRam.ramarray); i++) {
        __osContPifRam.ramarray[i] = 0;
    }

    __osContPifRam.pifstatus = CONT_CMD_EXE;
    readformat.dummy = CONT_CMD_NOP;
    readformat.txsize = CONT_CMD_READ_BUTTON_TX;
    readformat.rxsize = CONT_CMD_READ_BUTTON_RX;
    readformat.cmd = CONT_CMD_READ_BUTTON;
    readformat.button = 0xFFFF;
    readformat.stick_x = -1;
    readformat.stick_y = -1;

    readformatgcn.dummy = CONT_CMD_NOP;
    readformatgcn.txsize = CONT_CMD_GCN_SHORTPOLL_TX;
    readformatgcn.rxsize = CONT_CMD_GCN_SHORTPOLL_RX;
    readformatgcn.cmd = CONT_CMD_GCN_SHORTPOLL;
    // Changing the analog mode only changes how some of the analog values are arranged in the packet.
    //  Mode 3 is considered "Normal" mode, and doesn't read the analog A/B buttons (on supported controllers).
    //  Source:
    //  https://github.com/dolphin-emu/dolphin/blob/f76ab863266d012281e52bceda355bc72f36edb8/Source/Core/Core/HW/SI/SI_DeviceGCController.cpp#L185-L228
    readformatgcn.analog_mode = 3;
    readformatgcn.rumble = 0;
    readformatgcn.button = 0xFFFF;
    readformatgcn.stick_x = -1;
    readformatgcn.stick_y = -1;

    for (i = 0; i < __osMaxControllers; i++) {
        if (__osControllerTypes[i] == CONT_TYPE_GCN) {
            readformatgcn.rumble = __osGamecubeRumbleEnabled[i];
            *(__OSContGCNShortPollFormat*)ptr = readformatgcn;
            ptr += sizeof(__OSContGCNShortPollFormat);
        } else {
            *(__OSContReadFormat*)ptr = readformat;
            ptr += sizeof(__OSContReadFormat);
        }
    }

    *ptr = CONT_CMD_END;
}

void osContSetControllerMap(OSContButtonMap* contMap) {
    __osContCurButtonMap = contMap;
}

void osContResetControllerMap(void) {
    __osContCurButtonMap = &__osDefaultControllerMap;
}

static u16 __osTranslateGCNButtons(u16 input, s32 c_stick_x, s32 c_stick_y) {
    u16 ret = 0;

    // Face buttons
    if (input & CONT_GCN_A) {
        ret |= __osContCurButtonMap->buttonMap.a_button;
    }
    if (input & CONT_GCN_B) {
        ret |= __osContCurButtonMap->buttonMap.b_button;
    }
    if (input & CONT_GCN_START) {
        ret |= __osContCurButtonMap->buttonMap.start_button;
    }
    if (input & CONT_GCN_X) {
        ret |= __osContCurButtonMap->buttonMap.x_button;
    }
    if (input & CONT_GCN_Y) {
        ret |= __osContCurButtonMap->buttonMap.y_button;
    }

    // Triggers & Z
    if (input & CONT_GCN_Z) {
        ret |= __osContCurButtonMap->buttonMap.z_trig;
    }
    if (input & CONT_GCN_R) {
        ret |= __osContCurButtonMap->buttonMap.r_trig;
    }
    if (input & CONT_GCN_L) {
        ret |= __osContCurButtonMap->buttonMap.l_trig;
    }

    // D-Pad
    if (input & CONT_GCN_UP) {
        ret |= __osContCurButtonMap->buttonMap.u_jpad;
    }
    if (input & CONT_GCN_DOWN) {
        ret |= __osContCurButtonMap->buttonMap.d_jpad;
    }
    if (input & CONT_GCN_LEFT) {
        ret |= __osContCurButtonMap->buttonMap.l_jpad;
    }
    if (input & CONT_GCN_RIGHT) {
        ret |= __osContCurButtonMap->buttonMap.r_jpad;
    }

    s32 deadzone = __osContCurButtonMap->cStickDeadzone;

    if (c_stick_x > deadzone) {
        ret |= __osContCurButtonMap->cStickMap.right;
    }
    if (c_stick_x < -deadzone) {
        ret |= __osContCurButtonMap->cStickMap.left;
    }
    if (c_stick_y > deadzone) {
        ret |= __osContCurButtonMap->cStickMap.up;
    }
    if (c_stick_y < -deadzone) {
        ret |= __osContCurButtonMap->cStickMap.down;
    }

    return ret;
}

static u16 __osTranslateN64Buttons(u16 input) {
    u16 ret = 0;

    // Face buttons
    if (input & A_BUTTON) {
        ret |= __osContCurButtonMap->buttonMap.a_button;
    }
    if (input & B_BUTTON) {
        ret |= __osContCurButtonMap->buttonMap.b_button;
    }
    if (input & START_BUTTON) {
        ret |= __osContCurButtonMap->buttonMap.start_button;
    }

    // Triggers & Z
    if (input & Z_TRIG) {
        ret |= __osContCurButtonMap->buttonMap.z_trig;
    }
    if (input & R_TRIG) {
        ret |= __osContCurButtonMap->buttonMap.r_trig;
    }
    if (input & L_TRIG) {
        ret |= __osContCurButtonMap->buttonMap.l_trig;
    }

    // D-Pad
    if (input & U_JPAD) {
        ret |= __osContCurButtonMap->buttonMap.u_jpad;
    }
    if (input & D_JPAD) {
        ret |= __osContCurButtonMap->buttonMap.d_jpad;
    }
    if (input & L_JPAD) {
        ret |= __osContCurButtonMap->buttonMap.l_jpad;
    }
    if (input & R_JPAD) {
        ret |= __osContCurButtonMap->buttonMap.r_jpad;
    }

    if (input & U_CBUTTONS) {
        ret |= __osContCurButtonMap->cStickMap.up;
    }
    if (input & D_CBUTTONS) {
        ret |= __osContCurButtonMap->cStickMap.down;
    }
    if (input & L_CBUTTONS) {
        ret |= __osContCurButtonMap->cStickMap.left;
    }
    if (input & R_CBUTTONS) {
        ret |= __osContCurButtonMap->cStickMap.right;
    }

    return ret;
}
