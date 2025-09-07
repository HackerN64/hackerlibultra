#ifndef __MACROS_H__
#define __MACROS_H__

#define ALIGNED(x) __attribute__((aligned(x)))

#define ARRLEN(x) ((s32)(sizeof(x) / sizeof(x[0])))

#define STUBBED_PRINTF(x) ((void)(x))

#define UNUSED __attribute__((unused))

#define FALLTHROUGH __attribute__((fallthrough))

#ifndef __GNUC__
#define __attribute__(x)
#endif

#define ALIGN8(val) (((val) + 7) & ~7)

#define STACK(stack, size) u64 stack[ALIGN8(size) / sizeof(u64)]

#define STACK_START(stack) ((u8*)(stack) + sizeof(stack))

#ifndef MIN
#define MIN(a, b)                                                                                                      \
    ({                                                                                                                 \
        __auto_type _a = (a);                                                                                          \
        __auto_type _b = (b);                                                                                          \
        _a < _b ? _a : _b;                                                                                             \
    })
#endif // MIN

#ifndef MAX
#define MAX(a, b)                                                                                                      \
    ({                                                                                                                 \
        __auto_type _a = (a);                                                                                          \
        __auto_type _b = (b);                                                                                          \
        _a > _b ? _a : _b;                                                                                             \
    })
#endif // MAX

// Integer limits and clamping
#define S8_MAX  127
#define S8_MIN  -128
#define U8_MAX  255
#define S16_MAX 32767
#define S16_MIN -32768
#define U16_MAX 65535
#define S32_MAX 2147483647
#define S32_MIN -2147483648
#define U32_MAX 4294967295

// Clamp a value inbetween a range
#define CLAMP(x, low, high) MIN(MAX((x), (low)), (high))

// Clamp a value to the range of a specific data type
#define CLAMP_U8(x)  CLAMP((x), 0, U8_MAX)
#define CLAMP_S8(x)  CLAMP((x), S8_MIN, S8_MAX)
#define CLAMP_U16(x) CLAMP((x), 0, U16_MAX)
#define CLAMP_S16(x) CLAMP((x), S16_MIN, S16_MAX)

#endif
