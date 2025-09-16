#include <ultra64.h>
#include <PR/os_internal_error.h>
#include <PR/os_system.h>
#include <stdarg.h>
#include <string.h>

#include "assert.h"
#include "crash_screen.h"
#include "disasm.h"
#include "map_parser.h"
#include "stacktrace.h"

extern u16 sRenderedFramebuffer;
extern void audio_signal_game_loop_tick(void);
extern void stop_sounds_in_continuous_banks(void);
extern void read_controller_inputs(s32 threadID);

static OSFaultContext __osCurrentFaultContext;

// Configurable Defines
#define X_KERNING 6
#define GLYPH_WIDTH 8
#define GLYPH_HEIGHT 12
#define FONT_ROWS 16
#define LEFT_MARGIN 10 // for crash screen prints

enum crashPages {
    PAGE_SIMPLE,
    PAGE_CONTEXT,
    PAGE_STACKTRACE,
    PAGE_DISASM,
    PAGE_ASSERTS,
    PAGE_COUNT
};

static char *crashPageNames[PAGE_COUNT + NUM_USER_PAGES] = {
    [PAGE_SIMPLE] = "(Overview)",
    [PAGE_CONTEXT] = "(Context)",
    [PAGE_STACKTRACE] = "(Stack Trace)",
    [PAGE_DISASM] = "(Disassembly)",
    [PAGE_ASSERTS] = "(Assert)",
};

static u8 sCrashScreenCharToGlyph[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
};

static u32 sCrashScreenFont[GLYPH_HEIGHT * FONT_ROWS * 2 + 1] = {
    0x00000000,0x00000000,0x00000000,0x40000000,0x007070f0,0x20000000,0x00888888,0x00000000,0x0098b888,0x00f00000,0x00a8a8f0,0x00880000,0x00c8b880,0x00880000,0x00888080,0x00880000,0x00707880,0x00f00000,0x00000000,0x00800000,0x00000000,0x00800000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x20202070,0x00000000,0x20e05088,0x00000000,0x20205088,0x78780000,0x20208888,0x88880000,0x2020f888,0x88880000,0x002088a8,0x98880000,0x20f88870,0x68780000,0x00000008,0x00080000,0x00000000,0x00080000,0x00000000,0x00000000,0x00000000,0x00000000,0x50000000,0x00000000,0x5070f0f0,0x80000000,0x50888888,0x80000000,0x00088888,0xf0b00000,0x0010f0f0,0x88c80000,0x00208888,0x88800000,0x00408888,0x88800000,0x00f8f088,0xf0800000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x50707070,0x00000000,0xf8888888,0x00000000,0x50088080,0x70780000,0xf8308070,0x88800000,0x50088008,0x80700000,0x00888888,0x80080000,0x00707070,0x78f00000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x20000000,0x00000000,0x7010f0f8,0x08200000,0xa8308820,0x08200000,0xa0508820,0x78700000,0x70908820,0x88200000,0x28f88820,0x88200000,0xa8108820,0x88200000,0x7038f020,0x78180000,0x20000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x48f8f888,0x00000000,0xa8808088,0x00000000,0xb0f08088,0x70880000,0x7008f088,0x88880000,0x68088088,0xf8880000,0xa8888088,0x80980000,0x9070f870,0x78680000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x6070f888,0x18000000,0x90808088,0x20000000,0xa0f08088,0x70880000,0x4088f050,0x20880000,0xa8888050,0x20500000,0x90888020,0x20500000,0x68708020,0x20200000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x20000000,0x00000000,0x20f87088,0x00000000,0x20088888,0x00000000,0x00088088,0x78a80000,0x001098a8,0x88a80000,0x002088a8,0x88a80000,0x002088d8,0x88a80000,0x00207088,0x78500000,0x00000000,0x08000000,0x00000000,0x70000000,0x00000000,0x00000000,0x00000000,0x00000000,0x10000000,0x00000000,0x20708888,0x80000000,0x40888888,0x80000000,0x40888850,0xf0880000,0x4070f820,0x88500000,0x40888850,0x88200000,0x40888888,0x88500000,0x20708888,0x88880000,0x10000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x40000000,0x00000000,0x2070f888,0x20000000,0x10882088,0x00000000,0x10882088,0x60880000,0x10882050,0x20880000,0x10782020,0x20880000,0x10082020,0x20880000,0x2070f820,0x70780000,0x40000000,0x00080000,0x00000000,0x00700000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x200008f8,0x20000000,0xa8600808,0x00000000,0x70600810,0x60f80000,0x70000820,0x20100000,0xa8608840,0x20200000,0x20608880,0x20400000,0x000070f8,0x20f80000,0x00000000,0x20000000,0x00000000,0xc0000000,0x00000000,0x00000000,0x00000000,0x00100000,0x00000030,0x00200000,0x00008820,0x80200000,0x20609020,0x80200000,0x2060a020,0x90200000,0xf800c020,0xa0400000,0x2060a020,0xe0200000,0x20609020,0x90200000,0x00208820,0x88200000,0x00400030,0x00200000,0x00000000,0x00100000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000080,0x00200000,0x00088040,0x60200000,0x00108040,0x20200000,0x00208020,0x20200000,0x00408020,0x20200000,0x60208010,0x20200000,0x60108010,0x20200000,0x2008f808,0x70200000,0x40000008,0x00200000,0x00000004,0x00000000,0x00000000,0x00000000,0x00000000,0x00400000,0x00000060,0x00200000,0x00008820,0x00200000,0x0000d820,0x00200000,0x00f8a820,0xf0200000,0x7000a820,0xa8100000,0x00f88820,0xa8200000,0x00008820,0xa8200000,0x00008820,0xa8200000,0x00000060,0x00200000,0x00000000,0x00400000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00408820,0x00000000,0x0020c850,0x00000000,0x0010a888,0xb0680000,0x00089800,0xc8b00000,0x00108800,0x88000000,0x60208800,0x88000000,0x60408800,0x88000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x04000000,0x00000000,0x08707000,0x00000000,0x08888800,0x00000000,0x10088800,0x70000000,0x10108800,0x88000000,0x20208800,0x88000000,0x20008800,0x88000000,0x40207000,0x70000000,0x40000000,0x00000000,0x800000fc,0x00000000,0x00000000,0x00000000
};

u8 crashPage = 0;
u8 updateBuffer = TRUE;

static char crashScreenBuf[0x200];

char *gCauseDesc[18] = {
    "Interrupt",
    "TLB modification",
    "TLB exception on load",
    "TLB exception on store",
    "Address error on load",
    "Address error on store",
    "Bus error on inst.",
    "Bus error on data",
    "Failed Assert: See Assert Page",
    "Breakpoint exception",
    "Reserved instruction",
    "Coprocessor unusable",
    "Arithmetic overflow",
    "Trap exception",
    "Virtual coherency on inst.",
    "Floating point exception",
    "Watchpoint exception",
    "Virtual coherency on data",
};

char *gFpcsrDesc[6] = {
    "Unimplemented operation", "Invalid operation", "Division by zero", "Overflow", "Underflow",
    "Inexact operation",
};

static u32 sProgramPosition = 0;
static u16 gCrashScreenTextColor = 0xFFFF;

static void set_text_color(u32 r, u32 g, u32 b) {
    gCrashScreenTextColor = GPACK_RGBA5551(r, g, b, 255);
}

static void reset_text_color(void) {
    gCrashScreenTextColor = 0xFFFF;
}

void crash_screen_draw_rect(s32 x, s32 y, s32 w, s32 h) {
    u16 *ptr;
    s32 i, j;

    ptr = __osCurrentFaultContext.cfb + __osCurrentFaultContext.width * y + x;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            *ptr = 0x0001;
            ptr++;
        }
        ptr += __osCurrentFaultContext.width - w;
    }
}

void crash_screen_draw_glyph(s32 x, s32 y, s32 glyph) {
    const u32 *data;
    u16 *ptr;
    u32 bit;
    u32 rowMask;
    s32 i, j;

    if (glyph > 0x7F) return;

    data = &sCrashScreenFont[((glyph&0xF)*GLYPH_HEIGHT * 2) + (glyph >= 64)];

    ptr = __osCurrentFaultContext.cfb + __osCurrentFaultContext.width * y + x;

    u16 color = gCrashScreenTextColor;

    for (i = 0; i < GLYPH_HEIGHT; i++) {
        bit = 0x80000000U >> ((glyph >> 4) * GLYPH_WIDTH);
        rowMask = *data++;
        data ++;

        for (j = 0; j < (GLYPH_WIDTH); j++) {
            if (bit & rowMask) {
                *ptr = color;
            }
            ptr++;
            bit >>= 1;
        }
        ptr += __osCurrentFaultContext.width - (GLYPH_WIDTH);
    }
}

static char *write_to_buf(char *buffer, const char *data, size_t size) {
    return (char *) memcpy(buffer, data, size) + size;
}

void crash_screen_print_with_newlines(s32 x, s32 y, const s32 xNewline, const char *fmt, ...) {
    char *ptr;
    u32 glyph;
    s32 size;
    s32 xOffset = x;

    va_list args;
    va_start(args, fmt);

    size = _Printf(write_to_buf, crashScreenBuf, fmt, args);

    if (size > 0) {
        ptr = crashScreenBuf;

        while (*ptr && size-- > 0) {
            if (xOffset >= __osCurrentFaultContext.width - (xNewline + X_KERNING)) {
                y += 10;
                xOffset = xNewline;
            }

            glyph = sCrashScreenCharToGlyph[*ptr & 0x7f];

            if (*ptr == '\n') {
                y += 10;
                xOffset = x;
                ptr++;
                continue;
            } else if (glyph != 0xff) {
                crash_screen_draw_glyph(xOffset, y, glyph);
            }

            ptr++;
            xOffset += X_KERNING;
        }
    }

    va_end(args);
}

void crash_screen_print(s32 x, s32 y, const char *fmt, ...) {
    char *ptr;
    u32 glyph;
    s32 size;

    va_list args;
    va_start(args, fmt);

    size = _Printf(write_to_buf, crashScreenBuf, fmt, args);

    if (size > 0) {
        ptr = crashScreenBuf;

        while (*ptr && size-- > 0) {
            glyph = sCrashScreenCharToGlyph[*ptr & 0x7f];

            if (glyph != 0xff) {
                crash_screen_draw_glyph(x, y, glyph);
            }

            ptr++;
            x += X_KERNING;
        }
    }

    va_end(args);
}

void crash_screen_sleep(s32 ms) {
    u64 cycles = ms * 1000LL * osClockRate / 1000000ULL;
    osSetTime(0);
    while (osGetTime() < cycles) { }
}

void crash_screen_print_float_reg(s32 x, s32 y, s32 regNum, void *addr) {
    u32 bits = *(u32 *) addr;
    s32 exponent = ((bits & 0x7f800000U) >> 0x17) - 0x7F;

    if ((exponent >= -0x7E && exponent <= 0x7F) || bits == 0x0) {
        crash_screen_print(x, y, "F%02d:%.3e",  regNum, *(f32 *) addr);
    } else {
        crash_screen_print(x, y, "F%02d:%08XD", regNum, *(u32 *) addr);
    }
}

void crash_screen_print_fpcsr(u32 fpcsr) {
    s32 i;
    u32 bit = FPCSR_CE;

    crash_screen_print(100, 220, "FPCSR:%08XH", fpcsr);
    for (i = 0; i < 6; i++) {
        if (fpcsr & bit) {
            crash_screen_print(222, 220, "(%s)", gFpcsrDesc[i]);
            return;
        }
        bit >>= 1;
    }
}

void draw_crash_overview(OSThread *thread, s32 cause) {
    __OSThreadContext *tc = &thread->context;

    crash_screen_draw_rect(0, 20, 320, 240);

    crash_screen_print(LEFT_MARGIN, 20, "Thread %d (%s)", thread->id, gCauseDesc[cause]);

#ifdef DEBUG_EXPORT_SYMBOLS
    symtable_info_t info = get_symbol_info(tc->pc);

    crash_screen_print(LEFT_MARGIN, 40, "Crash at: %s", info.func == NULL ? "Unknown" : info.func);
    if (info.line != -1) {
        crash_screen_print(LEFT_MARGIN, 60, "File: %s", info.file);
#ifdef DEBUG_EXPORT_ALL_LINES
        // This line only shows the correct value if every line is in the sym file
        crash_screen_print(LEFT_MARGIN, 72, "Line: %d", info.line);
#endif // DEBUG_EXPORT_ALL_LINES
    }
#endif // DEBUG_EXPORT_SYMBOLS

    crash_screen_print(LEFT_MARGIN, 84, "Address: 0x%08X", tc->pc);
}

void draw_crash_context(OSThread *thread, s32 cause) {
    __OSThreadContext *tc = &thread->context;
    crash_screen_draw_rect(0, 20, 320, 240);
    crash_screen_print(LEFT_MARGIN, 20, "Thread:%d (%s)", thread->id, gCauseDesc[cause]);
    crash_screen_print(LEFT_MARGIN, 30, "PC:%08XH   SR:%08XH   VA:%08XH", tc->pc, tc->sr, tc->badvaddr);
    osWritebackDCacheAll();
#ifdef DEBUG_EXPORT_SYMBOLS
    char *fname = parse_map(tc->pc, TRUE);
    crash_screen_print(LEFT_MARGIN, 40, "Crash at: %s", fname == NULL ? "Unknown" : fname);
#endif // DEBUG_EXPORT_SYMBOLS
    crash_screen_print(LEFT_MARGIN,  52, "AT:%08XH   V0:%08XH   V1:%08XH", (u32) tc->at, (u32) tc->v0, (u32) tc->v1);
    crash_screen_print(LEFT_MARGIN,  62, "A0:%08XH   A1:%08XH   A2:%08XH", (u32) tc->a0, (u32) tc->a1, (u32) tc->a2);
    crash_screen_print(LEFT_MARGIN,  72, "A3:%08XH   T0:%08XH   T1:%08XH", (u32) tc->a3, (u32) tc->t0, (u32) tc->t1);
    crash_screen_print(LEFT_MARGIN,  82, "T2:%08XH   T3:%08XH   T4:%08XH", (u32) tc->t2, (u32) tc->t3, (u32) tc->t4);
    crash_screen_print(LEFT_MARGIN,  92, "T5:%08XH   T6:%08XH   T7:%08XH", (u32) tc->t5, (u32) tc->t6, (u32) tc->t7);
    crash_screen_print(LEFT_MARGIN, 102, "S0:%08XH   S1:%08XH   S2:%08XH", (u32) tc->s0, (u32) tc->s1, (u32) tc->s2);
    crash_screen_print(LEFT_MARGIN, 112, "S3:%08XH   S4:%08XH   S5:%08XH", (u32) tc->s3, (u32) tc->s4, (u32) tc->s5);
    crash_screen_print(LEFT_MARGIN, 122, "S6:%08XH   S7:%08XH   T8:%08XH", (u32) tc->s6, (u32) tc->s7, (u32) tc->t8);
    crash_screen_print(LEFT_MARGIN, 132, "T9:%08XH   GP:%08XH   SP:%08XH", (u32) tc->t9, (u32) tc->gp, (u32) tc->sp);
    crash_screen_print(LEFT_MARGIN, 142, "S8:%08XH   RA:%08XH",            (u32) tc->s8, (u32) tc->ra);
#ifdef DEBUG_EXPORT_SYMBOLS
    fname = parse_map(tc->ra, TRUE);
    crash_screen_print(LEFT_MARGIN, 152, "RA at: %s", fname == NULL ? "Unknown" : fname);
#endif // DEBUG_EXPORT_SYMBOLS

    crash_screen_print_fpcsr(tc->fpcsr);

    osWritebackDCacheAll();
    crash_screen_print_float_reg( 10, 170,  0, &tc->fp0.f.f_even);
    crash_screen_print_float_reg(100, 170,  2, &tc->fp2.f.f_even);
    crash_screen_print_float_reg(190, 170,  4, &tc->fp4.f.f_even);
    crash_screen_print_float_reg( 10, 180,  6, &tc->fp6.f.f_even);
    crash_screen_print_float_reg(100, 180,  8, &tc->fp8.f.f_even);
    crash_screen_print_float_reg(190, 180, 10, &tc->fp10.f.f_even);
    crash_screen_print_float_reg( 10, 190, 12, &tc->fp12.f.f_even);
    crash_screen_print_float_reg(100, 190, 14, &tc->fp14.f.f_even);
    crash_screen_print_float_reg(190, 190, 16, &tc->fp16.f.f_even);
    crash_screen_print_float_reg( 10, 200, 18, &tc->fp18.f.f_even);
    crash_screen_print_float_reg(100, 200, 20, &tc->fp20.f.f_even);
    crash_screen_print_float_reg(190, 200, 22, &tc->fp22.f.f_even);
    crash_screen_print_float_reg( 10, 210, 24, &tc->fp24.f.f_even);
    crash_screen_print_float_reg(100, 210, 26, &tc->fp26.f.f_even);
    crash_screen_print_float_reg(190, 210, 28, &tc->fp28.f.f_even);
    crash_screen_print_float_reg( 10, 220, 30, &tc->fp30.f.f_even);
}


#ifdef PUPPYPRINT_DEBUG
void draw_crash_log(void) {
    s32 i;
    crash_screen_draw_rect(0, 20, 320, 210);
    osWritebackDCacheAll();
#define LINE_HEIGHT (25 + ((LOG_BUFFER_SIZE - 1) * 10))
    for (i = 0; i < LOG_BUFFER_SIZE; i++) {
        crash_screen_print(LEFT_MARGIN, (LINE_HEIGHT - (i * 10)), consoleLogTable[i]);
    }
#undef LINE_HEIGHT
}
#endif

void draw_stacktrace(OSThread *thread,  s32 cause) {
    __OSThreadContext *tc = &thread->context;

    crash_screen_draw_rect(0, 20, 320, 240);
    crash_screen_print(LEFT_MARGIN, 25, "Stack Trace from %08X:", (u32) tc->sp);

#if defined(DEBUG_EXPORT_SYMBOLS) && defined(DEBUG_FULL_STACK_TRACE)
    // Current Func (EPC)
    crash_screen_print(LEFT_MARGIN, 35, "%08X (%s)", tc->pc, parse_map(tc->pc, TRUE));

    // Previous Func (RA)
    u32 ra = tc->ra;
    symtable_info_t info = get_symbol_info(ra);

    crash_screen_print(LEFT_MARGIN, 45, "%08X (%s:%d)", ra, info.func, info.line);

    osWritebackDCacheAll();

    static u32 generated = 0;

    if (stackTraceGenerated == FALSE) {
        generated = generate_stack(thread);
        stackTraceGenerated = TRUE;
    }
    for (u32 i = 0; i < generated; i++) {
        crash_screen_print(LEFT_MARGIN, 55 + (i * 10), get_stack_entry(i));
    }
#else // defined(DEBUG_EXPORT_SYMBOLS) && defined(DEBUG_FULL_STACK_TRACE)
    // simple stack trace
    u32 sp = tc->sp;

    for (int i = 0; i < STACK_LINE_COUNT; i++) {
        crash_screen_print(LEFT_MARGIN, 55 + (i * 10), "%3d: %08X", i, *((u32*)(sp + (i * 4))));
        crash_screen_print(120, 55 + (i * 10), "%3d: %08X", i + STACK_LINE_COUNT, *((u32*)(sp + ((i + STACK_LINE_COUNT) * 4))));
    }
#endif // defined(DEBUG_EXPORT_SYMBOLS) && defined(DEBUG_FULL_STACK_TRACE)
}

void draw_disasm(OSThread *thread) {
    __OSThreadContext *tc = &thread->context;

    crash_screen_draw_rect(0, 20, 320, 240);
    if (sProgramPosition == 0) {
        sProgramPosition = (tc->pc - 36);
    }
    crash_screen_print(LEFT_MARGIN, 25, "Program Counter: %08X", sProgramPosition);
    osWritebackDCacheAll();

    int skiplines = 0;
#ifdef DEBUG_EXPORT_SYMBOLS
    int currline = 0;
#endif // DEBUG_EXPORT_SYMBOLS

    for (int i = 0; i < 19; i++) {
        u32 addr = (sProgramPosition + (i * 4));

        char *disasm = insn_disasm((InsnData *)addr);


        if (disasm[0] == 0) {
            crash_screen_print(LEFT_MARGIN + 22, 35 + (skiplines * 10) + (i * 10), "%08X", addr);
        } else {
#ifdef DEBUG_EXPORT_SYMBOLS
            symtable_info_t info = get_symbol_info(addr);

            if (info.func_offset == 0 && info.distance == 0 && currline != info.line) {
                currline = info.line;
                set_text_color(239, 196, 15);
                crash_screen_print(LEFT_MARGIN, 35 + (skiplines * 10) + (i * 10), "<%s:>", info.func);
                reset_text_color();
                skiplines++;
            }
#ifndef DEBUG_EXPORT_ALL_LINES
            // catch `jal` and `jalr` callsites
            if (disasm[0] == 'j' && disasm[1] == 'a') {
#endif // DEBUG_EXPORT_ALL_LINES
                if (info.line != -1) {
                    set_text_color(200, 200, 200);
                    crash_screen_print(LEFT_MARGIN, 35 + (skiplines * 10) + (i * 10), "%d:", info.line);
                    reset_text_color();
                }
#ifndef DEBUG_EXPORT_ALL_LINES
            }
#endif // DEBUG_EXPORT_ALL_LINES

#endif // DEBUG_EXPORT_SYMBOLS
            if (addr == tc->pc) {
                set_text_color(255, 0, 0);
            } else {
                reset_text_color();
            }
            crash_screen_print(LEFT_MARGIN + 22, 35 + (skiplines * 10) + (i * 10), "%s", disasm);
        }

    }

    reset_text_color();
    osWritebackDCacheAll();
}

void draw_assert( OSThread *thread) {
    crash_screen_draw_rect(0, 20, 320, 240);

    crash_screen_print(LEFT_MARGIN, 25, "Assert");

    if (__n64Assert_Filename != NULL) {
        crash_screen_print(LEFT_MARGIN, 35, "File: %s", __n64Assert_Filename);
        crash_screen_print(LEFT_MARGIN, 45, "Line %d", __n64Assert_LineNum);
        crash_screen_print(LEFT_MARGIN, 55, "Condition:");
        crash_screen_print(LEFT_MARGIN, 65, "(%s)", __n64Assert_Condition);
        if (__n64Assert_MessageBuf[0] != 0) {
            crash_screen_print(LEFT_MARGIN, 75, "Message:");
            crash_screen_print(LEFT_MARGIN, 85, " %s", __n64Assert_MessageBuf);
        }
    } else {
        crash_screen_print(LEFT_MARGIN, 35, "No failed assert to report.");
    }

    osWritebackDCacheAll();
}

void draw_crash_screen(OSThread *thread) {
    __OSThreadContext *tc = &thread->context;

    s32 cause = ((tc->cause >> 2) & 0x1F);
    if (cause == 23) { // EXC_WATCH
        cause = 16;
    }
    if (cause == 31) { // EXC_VCED
        cause = 17;
    }

    // if (gPlayer1Controller->buttonPressed & R_TRIG) {
    //     crashPage++;
    //     if (crashPage == PAGE_ASSERTS && tc->cause != EXC_SYSCALL) crashPage++;
    //     updateBuffer = TRUE;
    // }
    // if (gPlayer1Controller->buttonPressed & (L_TRIG | Z_TRIG)) {
    //     crashPage--;
    //     if (crashPage == PAGE_ASSERTS && tc->cause != EXC_SYSCALL) crashPage--;
    //     updateBuffer = TRUE;
    // }

    // if (crashPage == PAGE_DISASM) {
    //     if (gPlayer1Controller->buttonDown & D_CBUTTONS) {
    //         sProgramPosition += 4;
    //         updateBuffer = TRUE;
    //     }
    //     if (gPlayer1Controller->buttonDown & U_CBUTTONS) {
    //         sProgramPosition -= 4;
    //         updateBuffer = TRUE;
    //     }
    // }

    if ((crashPage >= PAGE_COUNT) && (crashPage != 255)) {
        crashPage = 0;
    }
    if (crashPage == 255) {
        crashPage = (PAGE_COUNT - 1);
        if (crashPage == PAGE_ASSERTS && tc->cause != EXC_SYSCALL) crashPage--;
    }
    if (updateBuffer) {
        crash_screen_draw_rect(0, 0, 320, 20);
        crash_screen_print(LEFT_MARGIN, 5, "Page:%02d %-22s L/Z: Left   R: Right", crashPage, crashPageNames[crashPage]);
        switch (crashPage) {
            case PAGE_SIMPLE:     draw_crash_overview(thread, cause); break;
            case PAGE_CONTEXT:    draw_crash_context(thread, cause); break;
#ifdef PUPPYPRINT_DEBUG
            case PAGE_LOG: 		  draw_crash_log(); break;
#endif
            case PAGE_STACKTRACE: draw_stacktrace(thread, cause); break;
            case PAGE_DISASM:     draw_disasm(thread); break;
            case PAGE_ASSERTS:    draw_assert(thread); break;
        }

        osWritebackDCacheAll();
        osViBlack(FALSE);
        osViSwapBuffer(__osCurrentFaultContext.cfb);
        updateBuffer = FALSE;
    }
}

OSThread *get_crashed_thread(void) {
    OSThread *thread = __osGetCurrFaultedThread();

    while (thread->priority != -1) {
        if (thread->priority > OS_PRIORITY_IDLE && thread->priority < OS_PRIORITY_APPMAX
            && ((thread->flags & (BIT(0) | BIT(1))) != 0)) {
            return thread;
        }
        thread = thread->tlnext;
    }
    return NULL;
}

void osFaultMain(void *arg) {
    OSMesg mesg;
    OSThread *thread = NULL;

    osSetEventMesg(OS_EVENT_CPU_BREAK, &__osCurrentFaultContext.mesgQueue, (OSMesg) 1);
    osSetEventMesg(OS_EVENT_FAULT,     &__osCurrentFaultContext.mesgQueue, (OSMesg) 2);
    while (TRUE) {
        if (thread == NULL) {
            osRecvMesg(&__osCurrentFaultContext.mesgQueue, &mesg, 1);
            thread = get_crashed_thread();
            if (thread) {
                __osCurrentFaultContext.thread.priority = 15;
                crash_screen_sleep(200);
                audio_signal_game_loop_tick();
                crash_screen_sleep(200);
                // If an assert happened, go straight to that page
                if (thread->context.cause == EXC_SYSCALL) {
                    crashPage = PAGE_ASSERTS;
                }
                continue;
            }
        } else {
//             if (gControllerBits) {
// #if ENABLE_RUMBLE
//                 block_until_rumble_pak_free();
// #endif
//                 osContStartReadDataEx(&gSIEventMesgQueue);
//             }
            read_controller_inputs(FAULT_THREAD);
            draw_crash_screen(thread);
        }
    }
}

void osCreateFaultHandler(void) {
    osCreateMesgQueue(&__osCurrentFaultContext.mesgQueue, &__osCurrentFaultContext.mesg, 1);
    osCreateThread(&__osCurrentFaultContext.thread, FAULT_THREAD, osFaultMain, NULL,
                   (u8 *) __osCurrentFaultContext.stack + sizeof(__osCurrentFaultContext.stack),
                   OS_PRIORITY_APPMAX
                  );
    osStartThread(&__osCurrentFaultContext.thread);
}

