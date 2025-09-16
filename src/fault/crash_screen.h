#pragma once

#define FAULT_THREAD 2

#define NUM_USER_PAGES 16

typedef struct {
    OSMesgQueue mesgQueue;
    OSMesg mesg;
    OSThread thread;
    OSContPad pad;
    u32 width;
    u32 height;
    u16* cfb;
    u64 stack[100];
} OSFaultContext;
