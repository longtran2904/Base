/* date = July 24th 2022 6:30 pm */

#ifndef _BASE_H
#define _BASE_H

//~ TODO(long):
// [ ] Custom printf format
// [ ] Support for custom data structures

//~/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////        BASE DECLARATION        ////////////////
////////////////////////////////////////////////////////////////
//-/////////////////////////////////////////////////////////////

//~ long: Base Setup

#ifndef BASE_LIB_EXPORT_SYMBOLS
#define BASE_LIB_EXPORT_SYMBOLS 0
#endif
#ifndef BASE_LIB_IMPORT_SYMBOLS
#define BASE_LIB_IMPORT_SYMBOLS 0
#endif
#ifndef BASE_LIB_RUNTIME_IMPORT
#define BASE_LIB_RUNTIME_IMPORT 0
#endif

#if BASE_LIB_EXPORT_SYMBOLS && BASE_LIB_IMPORT_SYMBOLS
#error BASE_LIB_EXPORT_SYMBOLS and BASE_LIB_IMPORT_SYMBOLS can't both be true
#endif

#if BASE_LIB_EXPORT_SYMBOLS
#define BASE_SHARABLE(name) libexport name
#elif BASE_LIB_IMPORT_SYMBOLS
#define BASE_SHARABLE(name) libimport name
#elif BASE_LIB_RUNTIME_IMPORT
#define BASE_SHARABLE(name) global (*name)
#else
#define BASE_SHARABLE(name) function name
#endif

#ifndef global
#define global static
#endif
#ifndef local
#define local static
#endif

#ifndef function
#define function static
#endif
#ifndef internal
#define internal static
#endif

//~ long: Context Cracking

//- long: Compiler
#if defined(__clang__)
#define COMPILER_CLANG 1
#elif defined(_MSC_VER)
#define COMPILER_CL 1
#elif defined(__GNUC__)
#define COMPILER_GCC 1
#else
#error No context cracking for this compiler
#endif

#ifndef COMPILER_CL
#define COMPILER_CL 0
#endif
#ifndef COMPILER_CLANG
#define COMPILER_CLANG 0
#endif
#ifndef COMPILER_GCC
#define COMPILER_GCC 0
#endif

//- long: OS
#if defined(_WIN32)
#define OS_WIN 1
#elif defined(__gnu_linux__)
#define OS_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_MAC 1
#else
#error Missing OS detection
#endif

#ifndef OS_WIN
#define OS_WIN 0
#endif
#ifndef OS_LINUX
#define OS_LINUX 0
#endif
#ifndef OS_MAC
#define OS_MAC 0
#endif

// @RECONSIDER(long): Do I even need this?
#if COMPILER_CL && !OS_WIN
#error Missing ARCH detection
#endif

//- long: Architecture
#if COMPILER_CL
#if defined(_M_AMD64)
#define ARCH_X64 1
#elif defined(_M_I68)
#define ARCH_X86 1
#elif defined(_M_ARM)
#define ARCH_ARM 1
#else // TODO(long): ARM64
#error Missing ARCH detection
#endif

// NOTE(long): While GCC and Clang have predefined macros for SSE, MSVC doesn't
// https://stackoverflow.com/questions/18563978/detect-the-availability-of-sse-sse2-instruction-set-in-visual-studio
#if (defined(_M_AMD64) || defined(_M_X64))
#define __SSE2__ //SSE2 x64
#elif _M_IX86_FP == 2
#define __SSE2__ //SSE2 x32
#elif _M_IX86_FP == 1
#define __SSE__  //SSE x32
#else
#error You must have at least SSE1
#endif

#else // TODO(long): verify this works on clang and gcc
#if defined(__amd64__)
#define ARCH_X64 1
#elif defined(__i386__)
#define ARCH_X86 1
#elif defined(__arm__)
#define ARCH_ARM 1
#elif defined(__aarch64__)
#define ARCH_ARM64 1
#else
#error Missing ARCH detection
#endif
#endif

#ifndef ARCH_X64
#define ARCH_X64 0
#endif
#ifndef ARCH_X86
#define ARCH_X86 0
#endif
#ifndef ARCH_ARM
#define ARCH_ARM 0
#endif
#ifndef ARCH_ARM64
#define ARCH_ARM64 0
#endif

// @RECONSIDER(long)
#if COMPILER_CLANG
#define CURRENT_COMPILER_NUMBER 1
#define CURRENT_COMPILER_NAME   "CLANG"
#elif COMPILER_CL
#define CURRENT_COMPILER_NUMBER 2
#define CURRENT_COMPILER_NAME   "CL"
#elif COMPILER_GCC
#define CURRENT_COMPILER_NUMBER 3
#define CURRENT_COMPILER_NAME   "GCC"
#endif

#if OS_WIN
#define CURRENT_OS_NUMBER 1
#define CURRENT_OS_NAME   "WIN"
#elif OS_LINUX
#define CURRENT_OS_NUMBER 2
#define CURRENT_OS_NAME   "LINUX"
#elif OS_MAC
#define CURRENT_OS_NUMBER 3
#define CURRENT_OS_NAME   "MAC"
#endif

#if ARCH_X64
#define CURRENT_ARCH_NUMBER 1
#define CURRENT_ARCH_NAME   "X64"
#elif ARCH_X86
#define CURRENT_ARCH_NUMBER 2
#define CURRENT_ARCH_NAME   "x86"
#elif ARCH_ARM
#define CURRENT_ARCH_NUMBER 3
#define CURRENT_ARCH_NAME   "ARM"
#elif ARCH_ARM64
#define CURRENT_ARCH_NUMBER 4
#define CURRENT_ARCH_NAME   "ARM64"
#endif

#define MSVC(...)
#define CLANG(...)
#define GCC(...)

#if COMPILER_CL
#undef MSVC
#define MSVC(...) __VA_ARGS__
#endif

#if COMPILER_CLANG
#undef CLANG
#define CLANG(...) __VA_ARGS__
#endif

#if COMPILER_GCC
#undef GCC
#define GCC(...) __VA_ARGS__
#endif

//- long: Language
#if defined(__cplusplus)
# define LANG_CPP 1
#else
# define LANG_C 1
#endif

#ifndef LANG_CPP
# define LANG_CPP 0
#endif
#ifndef LANG_C
# define LANG_C 0
#endif

//- long: Pointer
#if ARCH_X64 || ARCH_ARM64
# define ARCH_SIZE 64
#else
# define ARCH_SIZE 32
#endif

//- long: Warning
#if COMPILER_CL
# define WarnPush(...) _Pragma(Stringify(warning(push, __VA_ARGS__)))
# define WarnPop() _Pragma("warning(pop)")
# define WarnEnable(warn) _Pragma(Stringify(warning(default: warn)))
# define WarnDisable(warn) _Pragma(Stringify(warning(disable: warn)))
#elif COMPILER_GCC
# define WarnPush(...) GCC diagnostic push
# define WarnPop() GCC diagnostic pop
# define WarnEnable(warn) GCC diagnostic warning warn
# define WarnDisable(warn) GCC diagnostic ignored warn
#elif COMPILER_CLANG
# define WarnPush(...) _Pragma("clang diagnostic push")
# define WarnPop() _Pragma("clang diagnostic pop")
# define WarnEnable(warn) _Pragma(Stringify(clang diagnostic warning warn))
# define WarnDisable(warn) _Pragma(Stringify(clang diagnostic ignored warn))
#else
# error warnings are not modifiable in code for this compiler
#endif

//- long: Extension
#if LANG_CPP
#define clinkage extern "C"
#else
#define clinkage extern
#endif

#if COMPILER_CL
#define threadvar __declspec(thread)
#elif COMPILER_GCC || COMPILER_CLANG
#define threadvar __thread
#else
#error threadvar is not defined for this compiler
#endif

#if COMPILER_CL || (COMPILER_CLANG && OS_WIN)
#define libexport __declspec(dllexport)
#define libimport __declspec(dllimport)
#elif COMPILER_GCC || COMPILER_CLANG
#define libexport __attribute__ ((visibility ("default")))
#define libimport
#else
#error libexport and libimport are not defined for this compiler
#endif

#if COMPILER_CL
#define AlignOf __alignof
#define AlignAs(alignment) __declspec(align(alignment))
#elif COMPILER_GCC || COMPILER_CLANG
#define AlignOf __alignof__
#define AlignAs(alignment) __attribute__(aligned(alignment))
#else
#error AlignOf and AlignAs are not defined for this compiler
#endif

#if COMPILER_CL
#define CHECK_PRINTF _Printf_format_string_ const
#define CHECK_PRINTF_FUNC(...)
#else
#define CHECK_PRINTF
#define CHECK_PRINTF_FUNC(pos) __attribute__((format(printf, pos, pos + 1)))
#endif

#if COMPILER_CL || (COMPILER_CLANG && OS_WIN)
#pragma section(".readonly", read)
#define readonly __declspec(allocate(".readonly"))
#elif COMPILER_CLANG && OS_LINUX
#define readonly __attribute__((section(".urmom")))
#else
#error No idea on how to do this in GCC
#endif

// https://nullprogram.com/blog/2022/06/26/
#ifndef AssertBreak
#  if COMPILER_GCC || COMPILER_CLANG
#    define AssertBreak() __builtin_trap()
#  elif COMPILER_CL
#    define AssertBreak() __debugbreak()
#  else
#    define AssertBreak() (*(volatile int *)0 = 0)
#  endif
#endif

#if OS_WIN
#pragma section(".CRT$XCU", read)
#define BeforeMain(n) static void n(void);      \
    __declspec(allocate(".CRT$XCU"))              \
    __pragma(comment(linker,"/include:" #n "__")) \
    void (*n##__)(void) = n;                      \
    static void n(void)
#elif OS_LINUX
#define BeforeMain(n) \
    __attribute__((constructor)) static void n(void)
#else
#error BeforeMain missing for this OS
#endif

//~ long: Helper Macros

#define Stmnt(S) do { S; } while (0)
#define fallthrough
#define UNUSED(x) ((void)(x))
#define DEBUG(x, ...) Stmnt(__VA_ARGS__; UNUSED(x))
#define DebugReturn() Stmnt(if (0) return)

#ifndef ENABLE_ASSERT
#define ENABLE_ASSERT 1
#endif

#if ENABLE_ASSERT
#define DebugPrint(str) OSWriteConsole(OS_STD_ERR, StrLit("\n\n" __FILE__ "(" Stringify(__LINE__) "): " __FUNCSIG__ ": " str))
#define Assert(c) Stmnt(if (!(c)) { DebugPrint("Assertion \"" Stringify(c) "\" failed\n"); AssertBreak(); })
#define PANIC(str) Stmnt(DebugPrint("PANIC \"" str "\"\n"); AssertBreak())
#else
#define DebugPrint(...) 0
#define Assert(...)
#define PANIC(...)
#endif

#define ALWAYS(x) ((x) ? 1 : (DebugPrint("ALWAYS(" Stringify(x) ") is false\n"), AssertBreak(), 0))
#define  NEVER(x) ((x) ? (DebugPrint("NEVER(" Stringify(x) ") is true\n"), AssertBreak(), 1) : 0)
#define StaticAssert(c, ...) typedef u8 Concat(_##__VA_ARGS__, __LINE__) [(c)?1:-1]

#define Stringify_(s) #s
#define Stringify(s) Stringify_(s)
#define Concat_(a, b) a##b
#define Concat(a, b) Concat_(a, b)
#define UNIQUE(name) Concat(name, __LINE__)

#define TempPointer(type, ptr) type UNIQUE(ptr) = {0}; if (!(ptr)) (ptr) = &UNIQUE(ptr);
#define TempBool(ptr) TempPointer(b32, ptr)

#define EnumCount(type) Concat(type, _Count)
#define ArrayCount(a) (sizeof(a)/sizeof(*(a)))
#define ArrayExpand(type, ...) (type[]){ __VA_ARGS__ }, ArrayCount(((type[]){ __VA_ARGS__ }))

#define HasAnyFlags(flags, fl)  ((flags) & (fl))
#define HasAllFlags(flags, fl) (((flags) & (fl)) == (fl))
#define     NoFlags(flags, fl) (!HasAnyFlags(flags, fl))

#define Implies(a,b) (!(a) || (b))

// @UB(long): Clang complains about these
#define IntFromPtr(p) (uptr)((char*)p - (char*)0)
#define PtrFromInt(n) (void*)((char*)0 + (n))

#define   Member(T, m) (((T*)0)->m)
#define OffsetOf(T, m) IntFromPtr(&Member(T, m))
#define MemberFromOffset(T, ptr, off) (T)((((u8*)ptr) + (off)))

#define BitCast(type, var) (*((type)*)(&(var))) // @UB(long)
#define PrcCast(a, b) ((*(VoidFunc**)(&(a))) = (VoidFunc*)(b))

#define Min(a, b) ((a)<(b)?(a):(b))
#define Max(a, b) ((a)>(b)?(a):(b))
#define InRange(t, a, b) (((t)>=(a)) && ((t)<=(b))) 

#define ClampTop(value, max) Min(value, max)
#define ClampBot(value, min) Max(value, min)
#define Clamp(value, min, max) ClampBot(ClampTop(value, max), min)

#define AlignUpPow2(x, p) (((x) + (p) - 1)&~((p) - 1))
#define AlignDownPow2(x, p) ((x) &~ ((p) - 1))
#define IsPow2OrZero(x) (((x)&((x)-1)) == 0)

#define Swap(type, a, b) do { type temp = (a); (a) = (b); (b) = temp; } while (0)
#define Lerp(a, b, t) ((1 - (t)) * (a) + (t) * (b))
#define LerpInt(a, b, t) ((i32)Lerp((f32)a, (f32)b, (f32)t))
#define UnLerp(a, b, x) (((x) - (a))/((b) - (a)))
#define GetSign(n) ((n) > 0 ? 1 : ((n) < 0 ? -1 : 0))
#define GetUnsigned(n) ((n) >= 0 ? 1 : -1)

#define MagicP(T,x,s) ((T)(x) << (s))
#define MagicU32(a,b,c,d) (MagicP(U32,a,0) | MagicP(U32,b,8) | MagicP(U32,c,16) | MagicP(U32,d,24))

#define Boolify(x) ((x) != 0) // @RECONSIDER(long): Do I need this? Can't I just use `!!`?

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) ((x) << 40)

#define Thousand(x) ((x)*1000)
#define Million(x)  ((x)*1000000llu)
#define Billion(x)  ((x)*1000000000llu)
#define Trillion(x) ((x)*1000000000000llu)

#define IsCharacter(c) (InRange(c, 'A', 'Z') || InRange(c, 'a', 'z'))
#define IsBinary(c) InRange(c, '0', '1')
#define IsDigit(c) InRange(c, '0', '9')
#define IsNonZeroDigit(c) InRange(c, '1', '9')
#define IsOctalDigit(c) InRange(c, '0', '7')
#define IsHexadecimalDigit(c) (InRange(c, 'A', 'F') || InRange(c, 'a', 'f') || IsDigit(c))

#define Characters "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define Binary "01"
#define Digits "0123456789"
#define NonZeroDigits "123456789"
#define OctalDigits "01234567"
#define HexadecimalDigits "ABCDEF"Digits

#define IsSlash(c) ((c) =='/' || (c) == '\\')
#define IsSpace(c)  ((c=='\t') || (c=='\f') || (c=='\v') || (c==' '))
#define IsNline(c)  ((c=='\n') || (c=='\r'))
#define IsWspace(c) (IsSpace(c) || IsNline(c))

#define SlashStr "/\\"
#define SpaceStr " \t\f\v"
#define NlineStr "\n\r"
#define WspaceStr Concat(SpaceStr, NlineStr)

#include <string.h> // TODO(long): Replace memset, memcpy, and memcmp
#define SetMem(ptr, val, size) memset((ptr), (val), (size))
#define ZeroMem(ptr, size) SetMem((ptr), 0, (size))
#define ZeroStruct(ptr) ZeroMem((ptr), sizeof(*(ptr)))
#define ZeroFixedArr(ptr) ZeroMem((ptr), sizeof(ptr))
#define ZeroTypedArr(ptr, size) ZeroMem((ptr), sizeof(*(ptr))*(size))

#define CopyMem(dest, src, size) memcpy((dest), (src), (size))
#define CopyStruct(dest, src) CopyMem((dest), (src), Min(sizeof(*(dest)), sizeof(*(src))))
#define CopyFixedArr(dest, src) CopyMem((dest), (src), Min(sizeof(*(dest)), sizeof(*(src)))*Min(ArrayCount(dest), ArrayCount(src)))
#define CopyTypedArr(dest, src, count) CopyMem((dest), (src), Min(sizeof(*(dest)), sizeof(*(src)))*(count))

#define CompareMem(a, b, size) (memcmp((a), (b), (size)) == 0)
#define CompareArr(a, b) CompareMem((a), (b), Min(ArrayCount(a) * sizeof(*(a)), ArrayCount(b) * sizeof(*(b))))
#define  CmpStruct(a, b) CompareMem(&(a), &(b), sizeof(a))

#if __SANITIZE_ADDRESS__
#define ENABLE_SANITIZER 1
#include <sanitizer/asan_interface.h>
#else
#define ENABLE_SANITIZER 0
#endif

#if ENABLE_SANITIZER
#define   AsanPoison(ptr, size)   __asan_poison_memory_region((ptr), (size))
#define AsanUnpoison(ptr, size) __asan_unpoison_memory_region((ptr), (size))
#define AsanIsPoison(ptr, size) (__asan_region_is_poisoned((ptr), (size)) != 0)
#else
#define   AsanPoison(...)
#define AsanUnpoison(...)
#define AsanIsPoison(...)
#endif

#define C4(str) (*(u32*)(str))
#define ExpandC4(x) (i32)(sizeof(x)), (i8*)(&(x))

#define DeferBlock(begin, end) for (int UNIQUE(_i_) = ((begin), 0); UNIQUE(_i_) == 0; (UNIQUE(_i_) += 1), (end))

#define CheckNil(nil, p) ((p) == 0 || (p) == nil)

//~ long: Linked List Macros

#define DLLPushBack_NPZ(nil, f, l, n, next, prev) (CheckNil(nil, f) ? \
                                                   ((f)=(l)=(n), (n)->next=(n)->prev=(nil)) : \
                                                   ((l)->next=(n), (l)=(n), (n)->prev=(l), (n)->next=(nil)))
#define DLLPushBack_NP(f, l, n, next, prev) DLLPushBack_NPZ(0, f, l, next, prev)
#define DLLPushBack(f, l, n) DLLPushBack_NP(f, l, n, next, prev)
#define DLLPushFront(f, l, n) DLLPushBack_NP(l, f, n, prev, next)

#define DLLRemove_NPZ(nil, f, l, n, next, prev) ((f)==(n) ? \
                                                 ((f)==(l) ? ((f)=(l)=(nil)) : ((f)=(f)->next,(f)->prev=(nil))) : \
                                                 ((l)==(n) ? ((l)=(l)->prev,(l)->next=(nil)) : ((n)->next->prev=(n)->prev, \
                                                                                                (n)->prev->next=(n)->next)))
#define DLLRemove_NP(f, l, n, next, prev) DLLRemove_NPZ(0, f, l, n, next, prev)
#define DLLRemove(f, l, n) DLLRemove_NP(f, l, n, next, prev)

#define SLLQueuePush_N(f, l, n, next) (((f)==0 ? ((f)=(l)=(n)) : ((l)->next=(n),(l)=(n))), (n)->next=0)
#define SLLQueuePush(f, l, n) SLLQueuePush_N(f, l, n, next)

#define SLLQueuePushFront_N(f, l, n, next) ((f)==0 ? ((f)=(l)=(n), (n)->next=0) : ((n)->next=(f), (f)=(n)))
#define SLLQueuePushFront(f, l, n) SLLQueuePushFront_N(f, l, n, next)

#define SLLQueuePop_N(f, l, next) ((f)==(l) ? ((f)=(l)=0) : ((f)=(f)->next))
#define SLLQueuePop(f, l) SLLQueuePop_N(f, l, next)

#define SLLStackPush_N(f, n, next) ((n)->next=(f), (f)=(n))
#define SLLStackPush(f, n) SLLStackPush_N(f, n, next)

#define SLLStackPop_N(f, next) ((f)==0 ? 0 :((f)=(f)->next))
#define SLLStackPop(f) SLLStackPop_N(f, next)

//~ long: Basic Types

#if COMPILER_CL
// https://learn.microsoft.com/vi-vn/cpp/cpp/int8-int16-int32-int64?view=msvc-160
#define  __INT8_TYPE__ __int8
#define __INT16_TYPE__ __int16
#define __INT32_TYPE__ __int32
#define __INT64_TYPE__ __int64

// C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\include\vcruntime.h
#if ARCH_SIZE == 64
#define __INTPTR_TYPE__ __int64
#else
#define __INTPTR_TYPE__ __int32
#endif

#define   __UINT8_TYPE__ unsigned __INT8_TYPE__ 
#define  __UINT16_TYPE__ unsigned __INT16_TYPE__
#define  __UINT32_TYPE__ unsigned __INT32_TYPE__
#define  __UINT64_TYPE__ unsigned __INT64_TYPE__
#define __UINTPTR_TYPE__ unsigned __INTPTR_TYPE__
#endif

// https://github.com/gcc-mirror/gcc/blob/master/gcc/ginclude/stdint-gcc.h
// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
typedef    __INT8_TYPE__ i8;
typedef   __INT16_TYPE__ i16;
typedef   __INT32_TYPE__ i32;
typedef   __INT64_TYPE__ i64;
typedef   __UINT8_TYPE__ u8;
typedef  __UINT16_TYPE__ u16;
typedef  __UINT32_TYPE__ u32;
typedef  __UINT64_TYPE__ u64;
typedef  __INTPTR_TYPE__ iptr;
typedef __UINTPTR_TYPE__ uptr;

StaticAssert(sizeof(i8 ) == 1,  CheckI8Size);
StaticAssert(sizeof(i16) == 2, CheckI16Size);
StaticAssert(sizeof(i32) == 4, CheckI32Size);
StaticAssert(sizeof(i64) == 8, CheckI64Size);
StaticAssert(sizeof(u8 ) == 1,  CheckU8Size);
StaticAssert(sizeof(u16) == 2, CheckU16Size);
StaticAssert(sizeof(u32) == 4, CheckU32Size);
StaticAssert(sizeof(u64) == 8, CheckU64Size);
StaticAssert(sizeof(iptr) == ARCH_SIZE/8, CheckIPTRSize);
StaticAssert(sizeof(uptr) == ARCH_SIZE/8, CheckUPTRSize);

#define  I8L(x) (x)
#define I16L(x) (x)
#define I32L(x) (x)
#define I64L(x) (x ## LL)
#define  U8L(x) (x)
#define U16L(x) (x)
#define U32L(x) (x ## U)
#define U64L(x) (x ## ULL)

#ifdef LANG_C
#define false 0
#define true  1
#endif

typedef i8  b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;

typedef u8  Flags8;
typedef u16 Flags16;
typedef u32 Flags32;
typedef u64 Flags64;

typedef float f32;
typedef double f64;

typedef void VoidFunc(void);
typedef void VoidFuncVoid(void*);

//~ long: Basic Constants

#define MIN_I8   I8L(0x80)
#define MIN_I16 I16L(0x8000)
#define MIN_I32 I32L(0x80000000)
#define MIN_I64 I64L(0x8000000000000000)

#define MAX_I8   I8L(0x7f)
#define MAX_I16 I16L(0x7fff)
#define MAX_I32 I32L(0x7fffffff)
#define MAX_I64 I64L(0x7fffffffffffffff)

#define MAX_U8   U8L(0xff)
#define MAX_U16 U16L(0xffff)
#define MAX_U32 U32L(0xffffffff)
#define MAX_U64 U64L(0xffffffffffffffff)

#if ARCH_SIZE == 64
#define MIN_IPTR MIN_I64
#define MAX_IPTR MAX_I64
#define MAX_UPTR MAX_U64
#else
#define MIN_IPTR MIN_I32
#define MAX_IPTR MAX_I32
#define MAX_UPTR MAX_U32
#endif

#define EPSILON_F32    ((f32)1.1920929e-7f )
#define E_F32          ((f32)2.71828182846f)
#define PI_F32         ((f32)3.14159265359f)
#define TAU_F32        ((f32)6.28318530718f)
#define GOLD_BIG_F32   ((f32)1.61803398875f)
#define GOLD_SMALL_F32 ((f32)0.61803398875f)

#define EPSILON_F64    ((f64)2.220446e-16 )
#define E_F64          ((f64)2.71828182846)
#define PI_F64         ((f64)3.14159265359)
#define TAU_F64        ((f64)6.28318530718)
#define GOLD_BIG_F64   ((f64)1.61803398875)
#define GOLD_SMALL_F64 ((f64)0.61803398875)

//~ long: Compound Types

//- long: 2D Vectors

typedef union v2f32 v2f32;
union v2f32
{
    struct { f32 x, y; };
    f32 v[2];
};

typedef union v2i64 v2i64;
union v2i64
{
    struct { i64 x, y; };
    i64 v[2];
};

typedef union v2i32 v2i32;
union v2i32
{
    struct { i32 x, y; };
    i32 v[2];
};

typedef union v2i16 v2i16;
union v2i16
{
    struct { i16 x, y; };
    i16 v[2];
};

//- long: 3D Vectors

typedef union v3f32 v3f32;
union v3f32
{
    struct { f32 x, y, z; };
    struct
    {
        v2f32 xy;
        f32 _z0;
    };
    struct
    {
        f32 _x0;
        v2f32 yz;
    };
    f32 v[3];
};

typedef union v3i32 v3i32;
union v3i32
{
    struct { i32 x, y, z; };
    struct
    {
        v2i32 xy;
        i32 _z0;
    };
    struct
    {
        i32 _x0;
        v2i32 yz;
    };
    i32 v[3];
};

//- TODO(long): 4D Vectors/Colors

//- TODO(long): 2x2/3x3/4x4 Matrix

//- long: 1D Range

typedef union r1i32 r1i32;
union r1i32
{
    struct { i32 min, max; };
    i32 v[2];
};

typedef union r1u64 r1u64;
union r1u64
{
    struct { u64 min, max; };
    u64 v[2];
};

typedef union r1f32 r1f32;
union r1f32
{
    struct { f32 min, max; };
    f32 v[2];
};

//- long: 2D Range (rectangles)

typedef union r2i32 r2i32;
union r2i32
{
    struct { v2i32 min, max; };
    struct { v2i32 p0, p1; };
    struct { i32 x0, y0, x1, y1; };
    v2i32 v[2];
};

typedef union r2f32 r2f32;
union r2f32
{
    struct { v2f32 min, max; };
    struct { v2f32 p0, p1; };
    struct { f32 x0, y0, x1, y1; };
    v2f32 v[2];
};

//~ long: Symbolic Types

enum Axis
{
    Axis_X,
    Axis_Y,
    Axis_Z,
    Axis_W,
};
typedef enum Axis Axis;

typedef enum Side Side;
enum Side
{
    Side_Min,
    Side_Max,
};

typedef enum Compiler Compiler;
enum Compiler
{
    Compiler_None,
    
    Compiler_CLANG,
    Compiler_CL,
    Compiler_GCC,
    
    Compiler_Count
};

typedef enum Arch Arch;
enum Arch
{
    Arch_None,
    
    Arch_X64,
    Arch_X86,
    Arch_ARM,
    Arch_ARM64,
    
    Arch_Count
};

typedef enum OS OS;
enum OS
{
    OS_None,
    
    OS_Win,
    OS_Linux,
    OS_Mac,
    
    OS_Count
};

typedef enum Month Month;
enum Month
{
    Month_None,
    
    Month_Jan,
    Month_Feb,
    Month_Mar,
    Month_Apr,
    Month_May,
    Month_Jun,
    Month_Jul,
    Month_Aug,
    Month_Sep,
    Month_Oct,
    Month_Nov,
    Month_Dec,
    
    Month_Count
};

typedef enum Day Day;
enum Day
{
    Day_None,
    
    Day_Sunday,
    Day_Monday,
    Day_Tuesday,
    Day_Wednesday,
    Day_Thursday,
    Day_Friday,
    Day_Saturday,
    
    Day_Count
};

//~ long: Time

typedef u64 DenseTime;

typedef struct DateTime DateTime;
struct DateTime
{
    u16 msec;
    u8 sec;
    u8 min;
    u8 hour;
    u8 day;
    u8 mon;
    i16 year;
};

function  DateTime TimeToDate (DenseTime time);
function DenseTime TimeToDense(DateTime  time);

//~ long: File Properties

typedef u32 DataAccessFlags;
enum
{
    DataAccessFlag_Read = (1 << 0),
    DataAccessFlag_Write = (1 << 1),
    DataAccessFlag_Execute = (1 << 2),
};

typedef u32 FilePropertyFlags;
enum
{
    FilePropertyFlag_IsFolder = (1 << 0)
};

typedef struct FileProperties FileProperties;
struct FileProperties
{
    u64 size;
    FilePropertyFlags flags;
    DataAccessFlags access;
    DenseTime createTime;
    DenseTime modifyTime;
};

//~ long: Arena Types

typedef struct Arena Arena;
struct Arena
{
    Arena* prev;
    Arena* curr;
    
    u64 alignment;
    b8 growing;
    b8 padding[7];
    
    // @RECONSIDER(long): Maybe postfix all relative fields with offset rather than pos
    // pos -> (used/allocated)Offset, commitPos -> commit(ted)Offset
    u64 basePos; // absolute
    u64 pos; // relative
    u64 commitPos; // relative
    u64 cap;
    u64 highWaterMark;
};

typedef struct TempArena TempArena;
struct TempArena
{
    Arena* arena;
    u64 pos;
};

// TODO(long): Make sure Mac (specifically M1/2/3) is the same
#define ARCH_PAGE_SIZE KB(4)
#if OS_WIN
#define ARCH_ALLOC_GRANULARITY KB(64)
#else
#define ARCH_ALLOC_GRANULARITY KB(4)
#endif

#define SCRATCH_POOL_COUNT 2

// @RECONSIDER(long): Rather than constants, arenas can take runtime values and these just become default values
#ifndef MEM_DEFAULT_RESERVE_SIZE
#define MEM_DEFAULT_RESERVE_SIZE MB(64)
#endif
#ifndef MEM_COMMIT_BLOCK_SIZE
#define MEM_COMMIT_BLOCK_SIZE KB(8)
#endif
#ifndef MEM_DEFAULT_ALIGNMENT
#define MEM_DEFAULT_ALIGNMENT sizeof(uptr)
#endif
#ifndef MEM_INITIAL_COMMIT
#define MEM_INITIAL_COMMIT KB(8)
#endif

#ifndef MEM_POISON_SIZE
#if ENABLE_SANITIZER
#define MEM_POISON_SIZE 128
#define MEM_POISON_ALIGNMENT 8
#else
#define MEM_POISON_SIZE ARCH_PAGE_SIZE
#define MEM_POISON_ALIGNMENT ARCH_PAGE_SIZE
#endif
#endif

StaticAssert(MEM_DEFAULT_RESERVE_SIZE >= MEM_COMMIT_BLOCK_SIZE, checkMemDefault);
StaticAssert(MEM_DEFAULT_ALIGNMENT <= ARCH_ALLOC_GRANULARITY, checkMemDefault);
StaticAssert(MEM_POISON_ALIGNMENT <= ARCH_ALLOC_GRANULARITY, checkMemDefault);
StaticAssert(ARCH_PAGE_SIZE <= ARCH_ALLOC_GRANULARITY, checkMemDefault);
StaticAssert(sizeof(Arena) <= MEM_INITIAL_COMMIT, checkMemDefault);

//~ long: String Types

typedef struct String String;
struct String
{
    u8* str;
    u64 size;
};

typedef struct String32 String32;
struct String32
{
    u32* str;
    u64 size;
};

typedef struct String16 String16;
struct String16
{
    u16* str;
    u64 size;
};

typedef struct StringNode StringNode;
struct StringNode
{
    StringNode* next;
    String string;
};

typedef struct StringMetaNode StringMetaNode;
struct StringMetaNode
{
    StringMetaNode* next;
    StringNode* node;
};

typedef struct StringList StringList;
struct StringList
{
    StringNode* first;
    StringNode* last;
    u64 nodeCount;
    u64 totalSize;
};

typedef struct StringJoin StringJoin;
struct StringJoin
{
    String pre;
    String mid;
    String post;
};

typedef u32 StringFindFlags;
enum
{
    FindStr_IgnoreCase  = 1 << 0,
    FindStr_IgnoreSlash = 1 << 1,
    FindStr_RightSloppy = 1 << 2,
    FindStr_LastMatch   = 1 << 3,
    SplitStr_KeepEmpties = 1 << 4
};

typedef enum PathStyle PathStyle;
enum PathStyle
{
    PathStyle_Relative,
    PathStyle_WindowsAbsolute,
    PathStyle_UnixAbsolute,
    
#if OS_WIN
    PathStyle_SystemAbsolute = PathStyle_WindowsAbsolute
#elif OS_LINUX
    PathStyle_SystemAbsolute = PathStyle_UnixAbsolute
#else
#error Absolute path style is undefined for this OS
#endif
};

typedef enum DecodeError DecodeError;
enum DecodeError
{
    DecodeError_EOF         = (1 << 0),
    DecodeError_Overlong    = (1 << 1),
    DecodeError_Surrogate   = (1 << 2),
    DecodeError_OutOfRange  = (1 << 3),
    DecodeError_InvalidBits = (1 << 4),
};

typedef struct StringDecode StringDecode;
struct StringDecode
{
    u32 codepoint;
    u32 size;
    u32 error;
};

#define StrListFirst(list) ((list)->first ? (list)->first->string : (String){0})
#define  StrListLast(list) ((list)-> last ? (list)-> last->string : (String){0})

#define StrListIter(list, name   ) for (StringNode* name = (list)->first; name; name = name->next)
#define  Str8Stream(str, ptr, opl) for (u8  *ptr = (str).str, *opl = (str).str + (str).size; ptr < opl;)
#define Str16Stream(str, ptr, opl) for (u16 *ptr = (str).str, *opl = (str).str + (str).size; ptr < opl;)
#define Str32Stream(str, ptr, opl) for (u32 *ptr = (str).str, *opl = (str).str + (str).size; ptr < opl;)

//~ long: Math Functions

//- long: Float Functions
function f32 Inf_f32(void);
function f32 NegInf_f32(void);
function b32 InfOrNan_f32(f32 x);

function f64 Inf_f64(void);
function f64 NegInf_f64(void);
function b32 InfOrNan_f64(f64 x);

function i32 AbsI32(i32 x);
function i64 AbsI64(i64 x);
function f32 Abs_f32(f32 x);
function f64 Abs_f64(f64 x);

//- long: Scalar Functions
function f32 Round_f32(f32 x);
function f32 Trunc_f32(f32 x);
function f32 Floor_f32(f32 x);
function f32 Ceil_f32(f32 x);
function f32 Mod_f32(f32 x, f32 m);
function f32 Sqrt_f32(f32 x);
function f32 RSqrt_f32(f32 x);
function f32 Ln_f32(f32 x);
function f32 Pow_f32(f32 base, f32 x);
function f32 FrExp_f32(f32 x, i32* exp);

function f64 Round_f64(f64 x);
function f64 Trunc_f64(f64 x);
function f64 Floor_f64(f64 x);
function f64 Ceil_f64(f64 x);
function f64 Mod_f64(f64 x, f64 m);
function f64 Sqrt_f64(f64 x);
function f64 Ln_f64(f64 x);
function f64 Pow_f64(f64 base, f64 x);
function f64 FrExp_f64(f64 x, i32* exp);

//- long: Trigonometric Functions
function f32 Sin_f32(f32 x);
function f32 Cos_f32(f32 x);
function f32 Tan_f32(f32 x);
function f32 Atan_f32(f32 x);
function f32 Atan2_f32(f32 x, f32 y);

function f64 Sin_f64(f64 x);
function f64 Cos_f64(f64 x);
function f64 Tan_f64(f64 x);
function f64 Atan_f64(f64 x);
function f64 Atan2_f64(f64 x, f64 y);

//- long: Vector Functions
function v2f32 V2F32(f32 x, f32 y);
function v2f32 AddV2F32(v2f32 a, v2f32 b);
function v2f32 SubV2F32(v2f32 a, v2f32 b);
function v2f32 MulV2F32(v2f32 a, v2f32 b);
function v2f32 DivV2F32(v2f32 a, v2f32 b);
function v2f32 ScaleV2F32(v2f32 v, f32 s);
function f32 DotV2F32(v2f32 a, v2f32 b);
function f32 SqrMagV2F32(v2f32 v);
function f32 MagV2F32(v2f32 v);
function f32 AngleV2F32(v2f32 v);
function v2f32 V2F32Angle(f32 theta, f32 radius);
function v2f32 NormV2F32(v2f32 v);
function v2f32 LerpV2F32(v2f32 a, v2f32 b, f32 t);

function v2i64 V2I64(i64 x, i64 y);
function v2i64 AddV2I64(v2i64 a, v2i64 b);
function v2i64 SubV2I64(v2i64 a, v2i64 b);
function v2i64 MulV2I64(v2i64 a, v2i64 b);
function v2i64 DivV2I64(v2i64 a, v2i64 b);
function v2i64 ScaleV2I64(v2i64 v, i64 s);
function i64 DotV2I64(v2i64 a, v2i64 b);
function i64 SqrMagV2I64(v2i64 v);
function i64 MagV2I64(v2i64 v);
function v2i64 NormV2I64(v2i64 v);
function v2i64 LerpV2I64(v2i64 a, v2i64 b, f32 t);

function v2i32 V2I32(i32 x, i32 y);
function v2i32 AddV2I32(v2i32 a, v2i32 b);
function v2i32 SubV2I32(v2i32 a, v2i32 b);
function v2i32 MulV2I32(v2i32 a, v2i32 b);
function v2i32 DivV2I32(v2i32 a, v2i32 b);
function v2i32 ScaleV2I32(v2i32 v, i32 s);
function i32 DotV2I32(v2i32 a, v2i32 b);
function i32 SqrMagV2I32(v2i32 v);
function i32 MagV2I32(v2i32 v);
function v2i32 NormV2I32(v2i32 v);
function v2i32 LerpV2I32(v2i32 a, v2i32 b, f32 t);

function v3f32 V3F32(f32 x, f32 y, f32 z);
function v3f32 AddV3F32(v3f32 a, v3f32 b);
function v3f32 SubV3F32(v3f32 a, v3f32 b);
function v3f32 MulV3F32(v3f32 a, v3f32 b);
function v3f32 DivV3F32(v3f32 a, v3f32 b);
function v3f32 ScaleV3F32(v3f32 v, f32 s);
function f32 DotV3F32(v3f32 a, v3f32 b);
function f32 SqrMagV3F32(v3f32 v);
function f32 MagV3F32(v3f32 v);
function v3f32 NormV3F32(v3f32 v);
function v3f32 LerpV3F32(v3f32 a, v3f32 b, f32 t);
function v3f32 CrossV3F32(v3f32 a, v3f32 b);

function v3i32 V3I32(i32 x, i32 y, i32 z);
function v3i32 AddV3I32(v3i32 a, v3i32 b);
function v3i32 SubV3I32(v3i32 a, v3i32 b);
function v3i32 MulV3I32(v3i32 a, v3i32 b);
function v3i32 DivV3I32(v3i32 a, v3i32 b);
function v3i32 ScaleV3I32(v3i32 v, i32 s);
function i32 DotV3I32(v3i32 a, v3i32 b);
function i32 SqrMagV3I32(v3i32 v);
function i32 MagV3I32(v3i32 v);
function v3i32 NormV3I32(v3i32 v);
function v3i32 LerpV3I32(v3i32 a, v3i32 b, f32 t);
function v3i32 CrossV3I32(v3i32 a, v3i32 b);

//- long: Range Functions
function r1i32 R1I32(i32 min, i32 max);
function r1i32 ShiftR1I32(r1i32 r, i32 x);
function r1i32 PadR1I32(r1i32 r, i32 x);
function i32 CenterR1I32(r1i32 r);
function b32 ContainsR1I32(r1i32 r, i32 x);
function i32 DimR1I32(r1i32 r);
function r1i32 UnionR1I32(r1i32 a, r1i32 b);
function r1i32 IntersectR1I32(r1i32 a, r1i32 b);
function i32 ClampR1I32(r1i32 r, i32 v);

function r1u64 R1U64(u64 min, u64 max);
function r1u64 ShiftR1u64(r1u64 r, u64 x);
function r1u64 PadR1u64(r1u64 r, u64 x);
function u64 CenterR1u64(r1u64 r);
function b32 ContainsR1u64(r1u64 r, u64 x);
function u64 DimR1u64(r1u64 r);
function r1u64 UnionR1u64(r1u64 a, r1u64 b);
function r1u64 IntersectR1u64(r1u64 a, r1u64 b);
function u64 ClampR1u64(r1u64 r, u64 v);

function r1f32 R1F32(f32 min, f32 max);
function r1f32 ShiftR1F32(r1f32 r, f32 x);
function r1f32 PadR1F32(r1f32 r, f32 x);
function f32 CenterR1F32(r1f32 r);
function b32 ContainsR1F32(r1f32 r, f32 x);
function f32 DimR1F32(r1f32 r);
function r1f32 UnionR1F32(r1f32 a, r1f32 b);
function r1f32 IntersectR1F32(r1f32 a, r1f32 b);
function f32 ClampR1F32(r1f32 r, f32 v);

#define R2I32P(x, y, z, w) R2I32(V2I32((x), (y)), V2I32((z), (w)))
function r2i32 R2I32(v2i32 min, v2i32 max);
function r2i32 ShiftR2I32(r2i32 r, v2i32 x);
function r2i32 PadR2I32(r2i32 r, i32 x);
function v2i32 CenterR2I32(r2i32 r);
function b32 ContainsR2I32(r2i32 r, v2i32 x);
function v2i32 DimR2I32(r2i32 r);
function r2i32 UnionR2I32(r2i32 a, r2i32 b);
function r2i32 IntersectR2I32(r2i32 a, r2i32 b);
function v2i32 ClampR2I32(r2i32 r, v2i32 v);

#define R2F32P(x, y, z, w) R2F32(V2F32((x), (y)), V2F32((z), (w)))
function r2f32 R2F32(v2f32 min, v2f32 max);
function r2f32 ShiftR2F32(r2f32 r, v2f32 x);
function r2f32 PadR2F32(r2f32 r, f32 x);
function v2f32 CenterR2F32(r2f32 r);
function b32 ContainsR2F32(r2f32 r, v2f32 x);
function v2f32 DimR2F32(r2f32 r);
function r2f32 UnionR2F32(r2f32 a, r2f32 b);
function r2f32 IntersectR2F32(r2f32 a, r2f32 b);
function v2f32 ClampR2F32(r2f32 r, v2f32 v);

//~ long: Arena Functions

function Arena* ArenaBuffer(u8* buffer, u64 size, u64 alignment);
function Arena* ArenaReserve(u64 reserve, u64 alignment, b32 growing);
function void   ArenaRelease(Arena* arena);

// NOTE(long): NZ stands for no zero. ArenaPush will zero the memory while ArenaPushNZ won't.
// If zero-on-pop is defined then Push/PushNZ will be the same.
#ifndef BASE_ZERO_ON_POP
#define BASE_ZERO_ON_POP 1
#endif

function void* ArenaPush  (Arena* arena, u64 size);
function void* ArenaPushNZ(Arena* arena, u64 size);

function void* ArenaPushPoisonEx(Arena* arena, u64 size, u64 preSize, u64 postSize);
#define ArenaPushPoison(arena, size) ArenaPushPoisonEx((arena), (size), MEM_POISON_SIZE, MEM_POISON_SIZE)

function void ArenaPop    (Arena* arena, u64 amount);
function void ArenaPopTo  (Arena* arena, u64 pos); // This is the abosolute pos, which means it must account for basePos
function void ArenaAlign  (Arena* arena, u64 alignment);
function void ArenaAlignNZ(Arena* arena, u64 alignment);

#define ArenaMake(...) ArenaReserve(MEM_DEFAULT_RESERVE_SIZE, MEM_DEFAULT_ALIGNMENT, (__VA_ARGS__ + 0))
#define ArenaStack(name, size, ...) u8 UNIQUE(_buffer)[(size) + sizeof(Arena)] = {0}; \
    Arena* name = ArenaBuffer(UNIQUE(_buffer), sizeof(UNIQUE(_buffer)), (__VA_ARGS__+0))
#define ArenaPos(arena, pos) ((arena)->curr->basePos + (pos))
#define ArenaCurrPos(arena) ArenaPos(arena, (arena)->curr->pos)

#define PtrAdd(ptr, offset) ((u8*)(ptr) + (offset))

#define   PushStruct(arena, type)        (type*)  ArenaPush((arena), sizeof(type))
#define PushStructNZ(arena, type)        (type*)ArenaPushNZ((arena), sizeof(type))
#define    PushArray(arena, type, count) (type*)  ArenaPush((arena), sizeof(type) * (count))
#define  PushArrayNZ(arena, type, count) (type*)ArenaPushNZ((arena), sizeof(type) * (count))

TempArena BASE_SHARABLE(GetScratch)(Arena** conflictArray, u64 count);
function TempArena TempBegin(Arena* arena);
function void      TempEnd(TempArena temp);

#define ScratchName(name) Concat(_tempArenaOf_, name)
#define ScratchClear(scratch) TempEnd(ScratchName(scratch))

#define ScratchBegin(name, ...) \
    TempArena ScratchName(name) = GetScratch(ArrayExpand(Arena*, 0, __VA_ARGS__)); Arena* name = ScratchName(name).arena; \
    i32 Concat(_debugVarOf_, name);
#define ScratchEnd(scratch) Stmnt(ScratchClear(scratch); ScratchName(scratch) = (TempArena){0}; scratch = 0; \
                                  (void)Concat(_debugVarOf_, scratch);)

#define TempBlock(name, _arena) for (TempArena name = TempBegin(_arena); name.arena != 0; TempEnd(name), name = (TempArena){0})
#define TempPoisonBlock(name, arena) \
    for (TempArena name = (ArenaPushPoisonEx((arena), 0, MEM_POISON_SIZE, 0), TempBegin(arena)), UNIQUE(name) = {0}; \
         UNIQUE(name).pos == 0; \
         UNIQUE(name).pos = ArenaCurrPos(arena), TempEnd(name), \
         ArenaPushPoisonEx((arena), 0, UNIQUE(name).pos - name.pos, MEM_POISON_SIZE))

// NOTE(long): This macro is heavily coupled with how the TempArena/Begin/End and GetScratch works
// It must be changed when any of those things change
#define ScratchBlock(name, ...) for (Arena* name = GetScratch(ArrayExpand(Arena*, 0, __VA_ARGS__)).arena, \
                                     UNIQUE(name) = { .pos = ArenaCurrPos(name) }; \
                                     UNIQUE(name).pos; \
                                     TempEnd((TempArena){ .arena = name, .pos = UNIQUE(name).pos }), UNIQUE(name).pos = 0)

//~ long: String Functions

//- long: Constructor Functions
#define Str(...) ((String){ __VA_ARGS__ })
#define StrRange(first, opl) (String){ (first), (opl) - (first) }
#define StrConst(s) { (u8*)(s), sizeof(s) - 1 }
#define StrLit(s) (String){ (u8*)(s), sizeof(s) - 1 }
#define StrExpand(s) (i32)((s).size), ((s).str)

function String StrChop   (String str, u64 size);
function String StrSkip   (String str, u64 size);
function String StrPrefix (String str, u64 size);
function String StrPostfix(String str, u64 size);

function String StrFromCStr(u8* cstr);
function String Substr(String str, u64 first, u64 opl);
function String SubstrRange(String str, u64 first, u64 range);
function StringJoin SubstrSplit(String str, String substr);

function String StrTrim(String str, String arr, i32 dir);
#define StrTrimStart(str, arr) StrTrim((str), StrLit(arr), -1)
#define StrTrimEnd  (str, arr) StrTrim((str), StrLit(arr), +1)

#define         StrTrimWspace(str) StrTrim((str), StrLit(WspaceStr),  0)
#define  StrTrimLeadingWspace(str) StrTrim((str), StrLit(WspaceStr), -1)
#define StrTrimTrailingWspace(str) StrTrim((str), StrLit(WspaceStr), +1)

//- long: Allocation Functions
function String StrCopy(Arena* arena, String str);
#define StrCloneCStr(arena, cstr) StrCopy((arena), StrFromCStr(cstr))
#define StrToCStr(a, s) StrCopy((a), (s)).str

function String StrFromFlags(Arena* arena, String* names, u64 nameCount, u64 flags);
#define GetEnumStr(type, e) (InRange(e, 0, EnumCount(type) - 1) ? (Concat(type, _names)[(i32)(e)]) : StrLit("Invalid"))
#define GetEnumName(type, e) GetEnumStr(type, e).str
#define GetFlagStr(arena, type, flags) StrFromFlags((arena), Concat(type, _names), ArrayCount(Concat(type, _names)), (flags))
#define GetFlagName(arena, type, flags) GetFlagStr(arena, type, flags).str

function StringList StrList(Arena* arena, String* strArr, u64 count);
function StringList StrListExplicit(StringNode* nodes, String* strs, u64 count);

// NOTE(long): IP stands for in place, which means the function will modify the passed-in pointer
// In this case, the function will mutate the `list` argument
// In other cases, the function might mutate the content of a string
// These functions usually don't take in any arena or return anything
function void StrListConcatIP(StringList* list, StringList* toPush);
function void StrListPush(Arena* arena, StringList* list, String str);
function void StrListPushNode(StringList* list, String str, StringNode* nodeMem);
#define StrListPushNodeMem(list, node) StrListPushNode(list, (node)->string, node)

function String StrListPop     (StringList* list);
function String StrListPopFront(StringList* list);

function String StrPushfv(Arena* arena, char* fmt, va_list args);
function String StrPushf (Arena* arena, CHECK_PRINTF char* fmt, ...) CHECK_PRINTF_FUNC(2);
#define StrListPushf(arena, list, fmt, ...) StrListPush((arena), (list), StrPushf((arena), (fmt), __VA_ARGS__))

function String StrPad(Arena* arena, String str, char chr, u32 count, i32 dir);
#define StrPadL(arena, str, chr, count) StrPad((arena), (str), (chr), (count), -1)
#define StrPadR(arena, str, chr, count) StrPad((arena), (str), (chr), (count), +1)

function String StrInsert(Arena* arena, String str, u64 index, String value);
function String StrRemove(Arena* arena, String str, u64 index, u64 count);
function String StrRepeat(Arena* arena, String str, u64 count);
function String ChrRepeat(Arena* arena, char   chr, u64 count);

function String StrJoinList(Arena* arena, StringList* list, StringJoin* join);
function String StrJoinMax3(Arena* arena, StringJoin* join);
#define StrJoin(arena, list, ...) StrJoinList((arena), (list), &(StringJoin){ .pre = {0}, .mid = {0}, .post = {0}, __VA_ARGS__ })
#define StrJoin3(arena, ...) StrJoinMax3((arena), &(StringJoin){ __VA_ARGS__ })

function StringList StrSplitList(Arena* arena, String str, StringList* splits, StringFindFlags flags);
function StringList StrSplitArr (Arena* arena, String str, String      splits, StringFindFlags flags);
function StringList StrSplit    (Arena* arena, String str, String      split , StringFindFlags flags);

function String StrReplaceList(Arena* arena, String str, StringList* oldStr, String newStr, StringFindFlags flags);
function String StrReplaceArr (Arena* arena, String str, String      oldArr, String newStr, StringFindFlags flags);
function String StrReplace    (Arena* arena, String str, String      oldStr, String newStr, StringFindFlags flags);

function String StrToLower(Arena* arena, String str);
function String StrToUpper(Arena* arena, String str);

//- long: Comparision Functions
function b32 ChrCompare(char a, char b, StringFindFlags flags); // IgnoreCase/Slash
function b32 StrCompare(String a, String b, StringFindFlags flags); // RightSloppy
function b32 ChrCompareArr(char chr, String arr, StringFindFlags flags);
function b32 StrListCompare(String str, StringList* values, StringFindFlags falgs);
function b32 StrIsWhitespace(String str); // Empty strings will return true

function i64 StrFindStr(String str, String val, StringFindFlags flags);
function i64 StrFindArr(String str, String arr, StringFindFlags flags);
function StringNode* StrFindList(String str, StringList* list, StringFindFlags flags);

function String StrChopAfter(String str, String arr, StringFindFlags flags);
function String StrSkipUntil(String str, String arr, StringFindFlags flags);

#define ChrIsUpper(c) ((c) >= 'A' && (c) <= 'Z')
#define ChrIsLower(c) ((c) >= 'a' && (c) <= 'z')
#define ChrToUpper(c) ((c) & (MAX_U8-32))
#define ChrToLower(c) ((c) | 32)
#define ChrCompareNoCase(a, b) (ChrToUpper(a) == ChrToUpper(b))

#define StrIsIdentical(a, b) ((a).str == (b).str && (a).size == (b).size)
#define StrIsSubstr(s, substr) ((s).str <= (substr).str && ((s).str + (s).size >= (substr).str + (substr).size))
#define StrIsPrefix(s, prefix) ((s).str == (prefix).str && (s).size >= (prefix).size)
#define StrIsPostfix(s, postfix) (((s).str + (s).size == (postfix).str + (postfix).size) && (s).str <= (postfix).str)
#define StrIsChr(s, chr) ((s).size == 1 && (s).str[0] == (chr))

#define StrStartsWith(str, val, noCase) (StrCompare(StrPrefix ((str), (val).size), (val), noCase))
#define   StrEndsWith(str, val, noCase) (StrCompare(StrPostfix((str), (val).size), (val), noCase))

#define StrFindChr(str, chr, flags) StrFindArr((str), StrLit(chr), (flags))
#define StrContainsChr(str, chr) (StrFindArr((str), StrLit(chr), 0) > -1)
#define StrContainsStr(str, val) (StrFindStr((str), (val), 0) > -1)
#define StrContainsNums(str) (StrFindChr((str), Digits, 0) > -1)
#define StrContainsChrs(str) (StrFindChr((str), Characters, 0) > -1)

//- long: Path Helpers

#if OS_WIN
#define OSPathMatchFlags FindStr_IgnoreSlash|FindStr_IgnoreCase
#else
#define OSPathMatchFlags FindStr_IgnoreSlash
#endif

#define StrChopLastSlash(str) StrChopAfter((str), StrLit("/"), FindStr_LastMatch|FindStr_IgnoreSlash)
#define StrSkipLastSlash(str) StrSkipUntil((str), StrLit("/"), FindStr_LastMatch|FindStr_IgnoreSlash)
#define   StrChopLastDot(str) StrChopAfter((str), StrLit("."), FindStr_LastMatch)
#define   StrSkipLastDot(str) StrSkipUntil((str), StrLit("."), FindStr_LastMatch)

#define StrSplitPath(arena, str) StrSplitArr(arena, str, StrLit(SlashStr), 0)
#define StrJoinPaths(arena, path, style) StrJoinList(arena, path, &(StringJoin){ .mid = StrLit("/"), \
                                                         .pre = (style) == PathStyle_UnixAbsolute ? StrLit("/") : (String){0} })

function PathStyle PathStyleFromStr(String str);

function String PathRelFromAbs(Arena* arena, String dst, String src);
function String PathAbsFromRel(Arena* arena, String dst, String src);
function String PathNormString(Arena* arena, String path);

function void PathListResolveDotsIP(StringList* path, PathStyle style);
function StringList PathListNormString(Arena* arena, String path, PathStyle* out);

//- long: Serialize Functions
function String StrWriteToStr(String src, u64 srcOffset, String dst, u64 dstOffset);
#define StrWriteToMem(data, offset, dst, size) StrWriteToStr((data), (offset), Str(dst, size), 0)

//- long: Unicode Functions
function StringDecode StrDecodeUTF8(u8 * str, u64 cap);
function StringDecode StrDecodeWide(u16* str, u64 cap);
function u32          StrEncodeUTF8(u8 * dst, u32 codepoint);
function u32          StrEncodeWide(u16* dst, u32 codepoint);

function String32 StrToStr32(Arena* arena, String   str);
function String16 StrToStr16(Arena* arena, String   str);
function String StrFromStr32(Arena* arena, String32 str);
function String StrFromStr16(Arena* arena, String16 str);
function String StrBackspace(String str);

function u64 UTF8Length(String str);
function u32 UTF8GetErr(String str, u64* index); // returns the first invalid character

#define StrFromRune(arena, rune) StrFromStr32((arena), (String32){ &(rune), 1 })
#define UTF8IsValid(str) (UTF8GetErr(str) == -1)

// TODO(long): https://dev.to/rdentato/utf-8-strings-in-c-3-3-2pc7
function u32 RuneFolding(u32 rune); // @CONSIDER(long): Should StrToLow(Upp)er use this?
function b32 RuneIsBlank(u32 rune); // @CONSIDER(long): Should StrIsWhitespace use this?

//- long: Convert Functions
function String StrFromF32(Arena* arena, f32 x, u32 prec);
function String StrFromF64(Arena* arena, f64 x, u32 prec);
function String StrFromI32(Arena* arena, i32 x, u32 radix);
function String StrFromI64(Arena* arena, i64 x, u32 radix);

function f32 F32FromStr(String str, b32* error);
function f64 F64FromStr(String str, b32* error);
function i32 I32FromStr(String str, u32 radix, b32* error);
function i64 I64FromStr(String str, u32 radix, b32* error);

function String StrFromTime(Arena* arena, DateTime time);
function String StrFromArg(char* arg, b32* isArg, String* argValue);

//~ long: Logs/Errors

function void Outf(CHECK_PRINTF char* fmt, ...) CHECK_PRINTF_FUNC(1);
function void Errf(CHECK_PRINTF char* fmt, ...) CHECK_PRINTF_FUNC(1);

typedef struct Record Record;
struct Record
{
    String log;
    char* file;
    i32 line;
    i32 level;
    DenseTime time;
};

typedef enum LogType LogType;
enum LogType
{
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
    
    LogType_Count
};

typedef void LogHandler(Arena* arena, Record* record, char* fmt, va_list args);

typedef struct LogInfo LogInfo;
struct LogInfo
{
    i64 level;
    LogHandler* callback;
};

typedef struct Logger Logger;
struct Logger
{
    Record* records;
    u64 count;
    LogInfo info;
};

#define LogBegin(...) LogBeginEx((LogInfo){ .callback = LogFmtStd, __VA_ARGS__ })
#define LogBlock(arena, list, ...) for (Logger UNIQUE(dummy) = (LogBegin(__VA_ARGS__), (Logger){ .count = MAX_U64 }); \
                                        UNIQUE(dummy).count == MAX_U64; \
                                        UNIQUE(dummy) = LogEnd(arena), list = StrListFromLogger(arena, &UNIQUE(dummy)))
#define LogPush(level, log, ...) LogPushf((level), __FILE__, __LINE__, (log), __VA_ARGS__)

void BASE_SHARABLE(LogBeginEx)(LogInfo info);
Logger BASE_SHARABLE(LogEnd)(Arena* arena);

void BASE_SHARABLE(LogPushf)(i32 level, char* file, i32 line, CHECK_PRINTF char* fmt, ...) CHECK_PRINTF_FUNC(4);
void BASE_SHARABLE(LogFmtStd)(Arena* arena, Record* record, char* fmt, va_list args);
void BASE_SHARABLE(LogFmtANSIColor)(Arena* arena, Record* record, char* fmt, va_list args);
LogInfo BASE_SHARABLE(LogGetInfo)(void);

function StringList StrListFromLogger(Arena* arena, Logger* logger);

#define ErrorBegin(...) LogBegin(.level = LOG_ERROR, __VA_ARGS__)
#define ErrorEnd(arena) LogEnd(arena)
#define ErrorSet(error, errno) ((errno) = 1, ErrorFmt(error))
#define ErrorFmt(error, ...) LogPush(LOG_ERROR, error, __VA_ARGS__)

//~ long: Buffer Functions

function String  BufferInterleave  (Arena* arena, void** in, u64 laneCount, u64 elementSize, u64 elementCount);
function String* BufferUninterleave(Arena* arena, void*  in, u64 laneCount, u64 elementSize, u64 elementCount);
#define BufferRead(buffer, offset, value) StrWriteToMem((buffer), (offset), (value), sizeof(*(value)))

//~ long: PRNG Functions

typedef struct RNG RNG;
struct RNG
{
    u32 seed;
    u32 pos;
};

#define BIT_NOISE1 0x68E31DA4
#define BIT_NOISE2 0xB5297A4D
#define BIT_NOISE3 0x1B56C4E9
#define BIT_NOISE4 0x0BD4BCB5
#define BIT_NOISE5 0x0063D68D

function u32 Noise1D(u32 pos, u32 seed);
function u64 Hash64 (u8* values, u64 count);

#define Noise2D(x, y, seed) Noise1D(x + (BIT_NOISE4 * y), seed)
#define Noise3D(x, y, seed) Noise1D(x + (BIT_NOISE4 * y) + (BIT_NOISE5 * z), seed)

#define RandomU32(rng) Noise1D(rng.pos++, rng.seed)
#define RandomF32(rng) (RandomU32(rng) / (f32)MAX_U32)
#define RandomRangeI32(rng, minIn, maxIn) ((i32)(minIn) + RandomU32(rng) % ((i32)(maxIn) - (i32)(minIn)))
#define RandomRangeF32(rng, minIn, maxIn) ((minIn) + RandomF32((maxIn) - (minIn)))
#define Random(rng, prob) (RandomF32(rng) < prob ? true : false)

//~/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////         OS DECLARATION         ////////////////
////////////////////////////////////////////////////////////////
//-/////////////////////////////////////////////////////////////

//~ long: OS Setup

function StringList OSGetArgs(void);
function void OSSetArgs(int argc, char **argv);
function void OSExit(u32 code);

//~ long: Memory Functions

function void* OSReserve(u64 size);
function void  OSRelease(void* ptr);
function b32    OSCommit(void* ptr, u64 size);
function void OSDecommit(void* ptr, u64 size);

//~ long: Console Handling

enum
{
    OS_STD_NONE,
    OS_STD_IN,
    OS_STD_OUT,
    OS_STD_ERR,
};

function String OSReadConsole(Arena* arena, i32 handle, b32 terminateData);
function b32   OSWriteConsole(i32 handle, String data);

//~ long: File Handling

function String OSReadFile(Arena* arena, String fileName, b32 terminateData);
function b32   OSWriteList(String fileName, StringList* data);
#define OSWriteFile(file, data) OSWriteList((file), &(StringList) \
                                            { \
                                                .first = &(StringNode){ .string = (data) }, \
                                                .last  = &(StringNode){ .string = (data) }, \
                                                .nodeCount = 1, .totalSize = (data).size, \
                                            })

function b32 OSDeleteFile(String fileName);
function b32 OSRenameFile(String oldName, String newName);
function b32 OSCreateDir(String path);
function b32 OSDeleteDir(String path);

function FileProperties OSFileProperties(String fileName);

//~ long: File Iteration

typedef struct OSFileIter
{
    String name;
    FileProperties prop;
    u8 v[640];
} OSFileIter;

function OSFileIter FileIterInit(String path);
function b32 FileIterNext(Arena* arena, OSFileIter* iter);
function void FileIterEnd(OSFileIter* iter);
#define FileIterBlock(arena, name, path) for (OSFileIter name = FileIterInit(path); \
                                              FileIterNext(arena, &name) ? 1 : (FileIterEnd(&name), 0);)

//~ long: Paths

function String OSGetCurrDir(Arena* arena);
function String  OSGetExeDir(void);
function String OSGetUserDir(void);
function String OSGetTempDir(void);

// @CONSIDER(long): Maybe have a OSSetCurrDir and OSGetInitDir
// https://devblogs.microsoft.com/oldnewthing/20101109-00/?p=12323

//~ long: Time

function void OSSleepMS(u32 ms);
function u64  OSNowMS(void);

function DateTime OSNowUniTime(void);
function DateTime OSToLocTime(DateTime universalTime);
function DateTime OSToUniTime(DateTime localTime);

//~ long: Libraries

typedef struct OSLib OSLib;
struct OSLib
{
    u64 v[1];
};

function OSLib     OSLoadLib(String path);
function void      OSFreeLib(OSLib lib);
function VoidFunc* OSGetProc(OSLib lib, char* name);

//~ long: Entropy

function void OSGetEntropy(void* data, u64 size);

//~ long: Clipboard

function void   OSSetClipboard(String string);
function String OSGetClipboard(Arena *arena);

//~ TODO(long): Processes/Threads

//~ TODO(long): Network/Sockets/IPC

#endif //_BASE_H
