#ifndef _FINALROM

#include "PRinternal/dbgproto.h"
#include "PRinternal/rmonint.h"

#include "PRinternal/macros.h"

// TODO: this comes from a header
#ident "$Revision: 1.4 $"

static int NotImplemented(KKHeader* dummy UNUSED) {
    return TV_ERROR_ILLEGAL_CALL;
}

static FUNPTR dispatchTable[] = {
    __rmonLoadProgram,   __rmonListProcesses, __rmonGetExeName,     __rmonListThreads, __rmonThreadStatus,
    NotImplemented,      __rmonStopThread,    __rmonRunThread,      NotImplemented,    NotImplemented,
    __rmonSetFault,      NotImplemented,      __rmonGetRegionCount, __rmonGetRegions,  __rmonGetGRegisters,
    __rmonSetGRegisters, __rmonGetFRegisters, __rmonSetFRegisters,  __rmonReadMem,     __rmonWriteMem,
    __rmonSetBreak,      __rmonClearBreak,    __rmonListBreak,      NotImplemented,    NotImplemented,
    NotImplemented,      NotImplemented,      NotImplemented,       NotImplemented,    NotImplemented,
    __rmonSetComm,       NotImplemented,      NotImplemented,       NotImplemented,    NotImplemented,
    NotImplemented,      NotImplemented,      NotImplemented,       NotImplemented,    NotImplemented,
    NotImplemented,      NotImplemented,      NotImplemented,       NotImplemented,    NotImplemented,
    NotImplemented,      NotImplemented,      NotImplemented,       NotImplemented,    __rmonGetSRegs,
    __rmonSetSRegs,      __rmonGetVRegs,      __rmonSetVRegs,       NotImplemented,
};

int __rmonExecute(KKHeader* request) {
    int retval;
    KKHeader reply;

    if (request->code >= ARRLEN(dispatchTable) - 1) {
        return TV_ERROR_ILLEGAL_CALL;
    }

    retval = dispatchTable[(int)request->code](request);
    if (retval < TV_ERROR_NO_ERROR) {
        reply.code = request->code;
        reply.error = retval;
        __rmonSendReply(&reply, sizeof(reply), KK_TYPE_REPLY);
    }
    return retval;
}

#endif
