/**
 * @file vimodempalhaf1.c
 *
 * MPAL HAF1 Video Mode
 *
 * H = High Resolution
 * A = Anti-Aliased
 * F = Deflickered Interlaced
 * 1 = 16-bit Framebuffer
 */
#include "PR/os.h"
#include "PR/rcp.h"
#include "PRinternal/viint.h"

OSViMode osViModeMpalHaf1 = {
    OS_VI_MPAL_HAF1, // type
    {
        // comRegs
        VI_CTRL_TYPE_16 | VI_CTRL_GAMMA_DITHER_ON | VI_CTRL_GAMMA_ON | VI_CTRL_DIVOT_ON | VI_CTRL_SERRATE_ON
            | VI_CTRL_ANTIALIAS_MODE_0 | VI_CTRL_PIXEL_ADV_3, // ctrl
        WIDTH(640),                                           // width
        BURST(57, 30, 5, 70),                                 // burst
        VSYNC(524),                                           // vSync
        HSYNC(3088, 0),                                       // hSync
        LEAP(3100, 3100),                                     // leap
        HSTART(108, 748),                                     // hStart
        SCALE(1, 0),                                          // xScale
        VCURRENT(0),                                          // vCurrent
    },
    { // fldRegs
      {
          //[0]
          ORIGIN(1280),       // origin
          SCALE(0.5, 0.5),    // yScale
          HSTART(35, 509),    // vStart
          BURST(2, 2, 11, 0), // vBurst
          VINTR(2),           // vIntr
      },
      {
          //[1]
          ORIGIN(2560),       // origin
          SCALE(0.5, 0.5),    // yScale
          HSTART(37, 511),    // vStart
          BURST(4, 2, 14, 0), // vBurst
          VINTR(2),           // vIntr
      } },
};
