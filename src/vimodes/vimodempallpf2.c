/**
 * @file vimodempallpf2.c
 *
 * MPAL LPF2 Video Mode
 *
 * L = Low Resolution
 * P = Point Sampled
 * F = Interlaced
 * 2 = 32-bit Framebuffer
 */
#include "PR/os.h"
#include "PR/rcp.h"
#include "PRinternal/viint.h"

OSViMode osViModeMpalLpf2 = {
    OS_VI_MPAL_LPF2, // type
    {
        // comRegs
        VI_CTRL_TYPE_32 | VI_CTRL_GAMMA_DITHER_ON | VI_CTRL_GAMMA_ON | VI_CTRL_SERRATE_ON | VI_CTRL_ANTIALIAS_MODE_2
            | VI_CTRL_PIXEL_ADV_3, // ctrl
        WIDTH(320),                // width
        BURST(57, 30, 5, 70),      // burst
        VSYNC(524),                // vSync
        HSYNC(3088, 0),            // hSync
        LEAP(3100, 3100),          // leap
        HSTART(108, 748),          // hStart
        SCALE(2, 0),               // xScale
        VCURRENT(0),               // vCurrent
    },
    { // fldRegs
      {
          // [0]
          ORIGIN(1280),       // origin
          SCALE(1, 0.25),     // yScale
          HSTART(35, 509),    // vStart
          BURST(2, 2, 11, 0), // vBurst
          VINTR(2),           // vIntr
      },
      {
          // [1]
          ORIGIN(1280),       // origin
          SCALE(1, 0.75),     // yScale
          HSTART(37, 511),    // vStart
          BURST(4, 2, 14, 0), // vBurst
          VINTR(2),           // vIntr
      } },
};
