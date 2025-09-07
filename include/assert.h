#ifndef __ASSERT_H__
#define __ASSERT_H__

#ifdef NDEBUG
#undef assert
#define assert(EX) ((void)0)
#else
extern void __assert(const char*, const char*, int);
extern void __assertf(const char* exp, const char* filename, int line, const char* fmt, ...);
#define assert(EX)            ((EX) ? ((void)0) : __assert(#EX, __FILE__, __LINE__))
#define assertf(EX, fmt, ...) ((EX) ? ((void)0) : __assertf(#EX, __FILE__, __LINE__, fmt, ##__VA_ARGS__))
#endif /* NDEBUG */

#endif /* !__ASSERT_H__ */
