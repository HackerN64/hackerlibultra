#ifndef _VIINT_H
#define _VIINT_H
#include "PR/os_internal.h"

#define OS_TV_TYPE_PAL  0
#define OS_TV_TYPE_NTSC 1
#define OS_TV_TYPE_MPAL 2

#define VI_STATE_MODE_UPDATED   0x01
#define VI_STATE_XSCALE_UPDATED 0x02
#define VI_STATE_YSCALE_UPDATED 0x04
#define VI_STATE_CTRL_UPDATED   0x08 // related to control regs changing
#define VI_STATE_BUFFER_UPDATED 0x10 // swap buffer
#define VI_STATE_BLACK          0x20 // probably related to a black screen
#define VI_STATE_REPEATLINE     0x40 // repeat line?
#define VI_STATE_FADE           0x80 // fade

#define VI_SCALE_MASK      0xfff // see rcp scale_x/scale_y
#define VI_2_10_FPART_MASK 0x3ff
#define VI_SUBPIXEL_SH     0x10

#define BURST(hsync_width, color_width, vsync_width, color_start)                                                      \
    (((u32)(hsync_width) & 0xFF) | (((u32)(color_width) & 0xFF) << 8) | (((u32)(vsync_width) & 0xF) << 16)             \
     | (((u32)(color_start) & 0xFFFF) << 20))
#define WIDTH(v)              (v)
#define VSYNC(v)              (v)
#define HSYNC(duration, leap) (((u32)(leap) << 16) | ((u32)(duration) & 0xFFFF))
#define LEAP(upper, lower)    (((u32)(upper) << 16) | ((u32)(lower) & 0xFFFF))
#define START(start, end)     (((u32)(start) << 16) | ((u32)(end) & 0xFFFF))

#define FTOFIX(val, i, f) ((u32)((val) * (f32)(1 << (f))) & ((1 << ((i) + (f))) - 1))

#define F210(val)           FTOFIX(val, 2, 10)
#define SCALE(scaleup, off) (F210((1.0f / (f32)(scaleup))) | (F210((f32)(off)) << 16))

#define VCURRENT(v) v // seemingly unused
#define ORIGIN(v)   v
#define VINTR(v)    v
#define HSTART      START

typedef struct {
    /* 0x0 */ f32 factor;
    /* 0x4 */ u16 offset;
    /* 0x8 */ u32 scale;
} __OSViScale;

typedef struct {
    /* 0x0 */ u16 state;
    /* 0x2 */ u16 retraceCount;
    /* 0x4 */ void* framep;
    /* 0x8 */ OSViMode* modep;
    /* 0xC */ u32 control;
    /* 0x10 */ OSMesgQueue* msgq;
    /* 0x14 */ OSMesg msg;
    /* 0x18 */ __OSViScale x;
    /* 0x24 */ __OSViScale y;
} __OSViContext; // 0x30 bytes

void __osViSwapContext(void);
extern __OSViContext* __osViCurr;
extern __OSViContext* __osViNext;
extern u32 __additional_scanline;
__OSViContext* __osViGetCurrentContext(void);
void __osViInit(void);
extern OSDevMgr __osViDevMgr;
#endif
