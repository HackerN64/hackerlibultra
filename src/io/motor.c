#include "PRinternal/macros.h"
#include "PR/os_internal.h"
#include "PR/os_version.h"
#include "PRinternal/controller.h"
#include "PRinternal/siint.h"

static OSPifRam __MotorDataBuf[MAXCONTROLLERS];

#define READFORMAT(ptr) ((__OSContRamReadFormat*)(ptr))

s32 __osMotorAccess(OSPfs* pfs, s32 flag) {
    int i;
    s32 ret = 0;
    u8* ptr = (u8*)&__MotorDataBuf[pfs->channel];

    if (!(pfs->status & PFS_MOTOR_INITIALIZED)) {
        return 5;
    }

    if (__osControllerTypes[pfs->channel] == CONT_TYPE_GCN) {
        __osGamecubeRumbleEnabled[pfs->channel] = flag;
        __osContLastCmd = CONT_CMD_END;
    } else {
        __osSiGetAccess();
        __MotorDataBuf[pfs->channel].pifstatus = CONT_CMD_EXE;
        ptr += pfs->channel;

        for (i = 0; i < BLOCKSIZE; i++) {
            READFORMAT(ptr)->data[i] = flag;
        }

        __osContLastCmd = CONT_CMD_END;
        __osSiRawStartDma(OS_WRITE, &__MotorDataBuf[pfs->channel]);
        osRecvMesg(pfs->queue, NULL, OS_MESG_BLOCK);
        ret = __osSiRawStartDma(OS_READ, &__MotorDataBuf[pfs->channel]);
        osRecvMesg(pfs->queue, NULL, OS_MESG_BLOCK);

        ret = READFORMAT(ptr)->rxsize & CHNL_ERR_MASK;
        if (!ret) {
            if (!flag) {
                if (READFORMAT(ptr)->datacrc != 0) {
                    ret = PFS_ERR_CONTRFAIL;
                }
            } else {
                if (READFORMAT(ptr)->datacrc != 0xEB) {
                    ret = PFS_ERR_CONTRFAIL;
                }
            }
        }
        __osSiRelAccess();
    }

    return ret;
}

static void __osMakeMotorData(int channel, OSPifRam* mdata) {
    u8* ptr = (u8*)mdata->ramarray;
    __OSContRamReadFormat ramreadformat;
    int i;

    ramreadformat.dummy = CONT_CMD_NOP;
    ramreadformat.txsize = CONT_CMD_WRITE_PAK_TX;
    ramreadformat.rxsize = CONT_CMD_WRITE_PAK_RX;
    ramreadformat.cmd = CONT_CMD_WRITE_PAK;
    ramreadformat.addrh = CONT_BLOCK_RUMBLE >> 3;
    ramreadformat.addrl = (u8)(__osContAddressCrc(CONT_BLOCK_RUMBLE) | (CONT_BLOCK_RUMBLE << 5));

    if (channel != 0) {
        for (i = 0; i < channel; i++) {
            *ptr++ = CONT_CMD_REQUEST_STATUS;
        }
    }

    *READFORMAT(ptr) = ramreadformat;
    ptr += sizeof(__OSContRamReadFormat);
    ptr[0] = CONT_CMD_END;
}

s32 osMotorInit(OSMesgQueue* mq, OSPfs* pfs, int channel) {
    s32 ret;
    u8 temp[32];

    pfs->queue = mq;
    pfs->channel = channel;
    pfs->activebank = 0xFF;
    pfs->status = 0;

    if (__osControllerTypes[pfs->channel] != CONT_TYPE_GCN) {
        ret = SELECT_BANK(pfs, 0xFE);

        if (ret == PFS_ERR_NEW_PACK) {
            ret = SELECT_BANK(pfs, 0x80);
        }

        if (ret != 0) {
            return ret;
        }

        ret = __osContRamRead(mq, channel, CONT_BLOCK_DETECT, temp);

        if (ret == PFS_ERR_NEW_PACK) {
            ret = PFS_ERR_CONTRFAIL;
        }

        if (ret != 0) {
            return ret;
        } else if (temp[31] == 254) {
            return PFS_ERR_DEVICE;
        }

        ret = SELECT_BANK(pfs, 0x80);
        if (ret == PFS_ERR_NEW_PACK) {
            ret = PFS_ERR_CONTRFAIL;
        }

        if (ret != 0) {
            return ret;
        }

        ret = __osContRamRead(mq, channel, CONT_BLOCK_DETECT, temp);
        if (ret == PFS_ERR_NEW_PACK) {
            ret = PFS_ERR_CONTRFAIL;
        }

        if (ret != 0) {
            return ret;
        } else if (temp[31] != 0x80) {
            return PFS_ERR_DEVICE;
        }

        if (!(pfs->status & PFS_MOTOR_INITIALIZED)) {
            __osMakeMotorData(channel, &__MotorDataBuf[channel]);
        }
    }

    pfs->status = PFS_MOTOR_INITIALIZED;
    return 0;
}
