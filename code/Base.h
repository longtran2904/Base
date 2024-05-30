/* date = July 24th 2022 6:30 pm */

#ifndef _BASE_H
#define _BASE_H

//~ TODO(long):
// [ ] GCC/CLANG testing
// [ ] Fuzzing
// [ ] Custom printf format
// [ ] readonly

//~ NOTE(long): Context Cracking

//- NOTE(long): Compiler
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

//- NOTE(long): OS
#ifdef _WIN32
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

//- NOTE(long): Architecture
#if COMPILER_CL
#ifdef _M_AMD64
#define ARCH_X64 1
#elif defined(_M_I68)
#define ARCH_X86 1
#elif defined(_M_ARM)
#define ARCH_ARM 1
// TODO(long): ARM64
#else
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
// TODO(long): Maybe error out?
#endif

#else // TODO(long): verify this works on clang and gcc
#ifdef __amd64__
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

//- NOTE(long): Language
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

//- NOTE(long): Pointer
#if ARCH_X64 || ARCH_ARM64
# define ARCH_SIZE 64
#else
# define ARCH_SIZE 32
#endif

//- NOTE(long): Warning
#if COMPILER_CL
#    define WarnPush(...) warning(push, __VA_ARGS__)
#    define WarnPop() warning(pop)
#    define  WarnEnable(warn) warning(default: warn)
#    define WarnDisable(warn) warning(disable: warn)
#elif COMPILER_GCC
#    define WarnPush(...) GCC diagnostic push
#    define WarnPop() GCC diagnostic pop
#    define  WarnEnable(warn) GCC diagnostic warning warn
#    define WarnDisable(warn) GCC diagnostic ignored warn
#elif COMPILER_CLANG
#    define WarnPush(...) clang diagnostic push
#    define WarnPop() clang diagnostic pop
#    define  WarnEnable(warn) clang diagnostic warning warn
#    define WarnDisable(warn) clang diagnostic ignored warn
#else
#    error warnings are not modifiable in code for this compiler
#endif

//- NOTE(long): Extension
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

#if COMPILER_CL
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
#else
#error CHECK_PRINTF isn't defined for this compiler
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

//~ NOTE(long): Helper Macros

#define UNIQUE(name) Concat(name, __LINE__)
#define Stmnt(S) do { S; } while (0)
#define UNUSED(x) ((void)(x))
#define DEBUG(x, ...) Stmnt(__VA_ARGS__; UNUSED(x))

#ifndef ENABLE_ASSERT
#define ENABLE_ASSERT 1
#endif

#if ENABLE_ASSERT
#define Assert(c) Stmnt(if (!(c)) \
                        { \
                            OSWriteFile(StdErr, StrLit("\n\n" __FILE__ "(" Stringify(__LINE__) "): " __FUNCSIG__ ": " \
                                                       "Assertion \"" Stringify(c) "\" failed\n")); \
                            AssertBreak(); \
                        })
#else
#define Assert(...)
#endif
#define StaticAssert(c, ...) typedef u8 Concat(_##__VA_ARGS__, __LINE__) [(c)?1:-1]

#define PANIC(str) Stmnt(OSWriteFile(StdErr, StrLit("\n\n" __FILE__ "(" Stringify(__LINE__) "): " __FUNCSIG__ ": " \
                                                    "PANIC \"" str "\"\n")); \
                         AssertBreak())
#define ALWAYS(x) ((x) ? (x) : (AssertBreak(), (x)))
#define  NEVER(x) ((x) ? (AssertBreak(), (x)) : (x))

#define Stringify_(s) #s
#define Stringify(s) Stringify_(s)
#define Concat_(a, b) a##b
#define Concat(a, b) Concat_(a, b)

#define EnumCount(type) Concat(type, _Count)
#define ArrayCount(a) (sizeof(a)/sizeof(*(a)))
#define ArrayExpand(type, ...) (type[]){ __VA_ARGS__ }, ArrayCount((type[]){ __VA_ARGS__ })

#define HasAnyFlags(flags, fl) ((flags) & (fl))
#define HasAllFlags(flags, fl) (((flags) & (fl)) == (fl))
#define     NoFlags(flags, fl) (!HasAnyFlags(flags, fl))

#define Implies(a,b) (!(a) || (b))

#define IntFromPtr(p) (unsigned long long)((char*)p - (char*)0)
#define PtrFromInt(n) (void*)((char*)0 + (n))

#define Member(T, m) (((T*)0)->m)
#define OffsetOf(T, m) IntFromPtr(&Member(T, m))

#define BitCast(type, var) (*((type)*)(&(var))) // NOTE(long): UB
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
#define Lerp(t, a, b) ((1 - (t)) * (a) + (t) * (b))
#define UnLerp(x, a, b) (((x) - (a))/((b) - (a)))
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

#define global   static
#define local    static
#define function static

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

//~ NOTE(long): Linked List Macros

#define DLLPushBack_NP(f, l, n, next, prev) (((f)==0?\
                                              (f)=(l)=(n):\
                                              ((l)->next=(n),(l)=(n))),\
                                             (n)->next=(n)->prev=0)
#define DLLPushBack(f, l, n) DLLPushBack_NP(f, l, n, next, prev)
#define DLLPushFront(f, l, n) DLLPushBack_NP(l, f, n, prev, next)

#define DLLRemove_NP(f, l, n, next, prev) (((f)==(n)?\
                                            ((f)=(f)->next, (f)->prev=0):\
                                            (l)==(n)?\
                                            ((l)=(l)->prev,(l)->next=0):\
                                            ((n)->next->prev=(n)->prev,\
                                             (n)->prev->next=(n)->next)))
#define DLLRemove(f, l, n) DLLRemove_NP(f, l, n, next, prev)

#define SLLQueuePush(f, l, n) ((f)==0?\
                               (f)=(l)=(n):\
                               ((l)->next=(n),(l)=(n)),\
                               (n)->next=0)
#define SLLQueuePushFront(f, l, n) ((f)==0?\
                                    ((f)=(l)=(n),(n)->next=0):\
                                    ((n)->next=(f),(f)=(n)))
#define SLLQueuePop(f, l) ((f)==(l)?\
                           ((f)=(l)=0):\
                           ((f)=(f)->next))

#define SLLStackPush(f, n) ((n)->next=(f),(f)=(n))
#define SLLStackPop(f) ((f)==0?0:((f)=(f)->next))

//~ NOTE(long): Basic Types

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

#define  I8(x) (x)
#define I16(x) (x)
#define I32(x) (x)
#define I64(x) (x ## LL)
#define  U8(x) (x)
#define U16(x) (x)
#define U32(x) (x ## U)
#define U64(x) (x ## ULL)

#ifdef LANG_C
#define false 0
#define true  1
#endif

typedef i8 b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;

typedef float f32;
typedef double f64;

typedef void VoidFunc(void);
typedef void VoidFuncVoid(void*);

//~ NOTE(long): Basic Constants

#define MIN_I8  I8(0x80)
#define MIN_I16 I16(0x8000)
#define MIN_I32 I32(0x80000000)
#define MIN_I64 I64(0x8000000000000000)

#define MAX_I8  I8(0x7f)
#define MAX_I16 I16(0x7fff)
#define MAX_I32 I32(0x7fffffff)
#define MAX_I64 I64(0x7fffffffffffffff)

#define MAX_U8  U8(0xff)
#define MAX_U16 U16(0xffff)
#define MAX_U32 U32(0xffffffff)
#define MAX_U64 U64(0xffffffffffffffff)

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

//~ NOTE(long): Symbolic Constants

typedef enum Axis Axis;
enum Axis
{
    Axis_X,
    Axis_Y,
    Axis_Z,
    Axis_W,
};

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

//~ NOTE(long): Time

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

//~ NOTE(long): File Properties

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

//~ NOTE(long): Base Memory Pre-Requisites

#if !defined(MemReserve)
# error missing definition for 'MemReserve' type: (U64)->void* 
#endif
#if !defined(MemCommit)
# error missing definition for 'MemCommit' type: (void*,U64)->B32
#endif
#if !defined(MemDecommit)
# error missing definition for 'MemDecommit' type: (void*,U64)->void
#endif
#if !defined(MemRelease)
# error missing definition for 'MemRelease' type: (void*,U64)->void
#endif

//~ NOTE(long): Arena Types

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

#define SCRATCH_POOL_COUNT 4

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

//~ NOTE(long): String Types

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
    FindStr_NoCase    = 1 << 0,
    FindStr_LastMatch = 1 << 1,
};

typedef u32 StringSplitFlags;
enum
{
    SplitStr_NoCase           = 1 << 0,
    SplitStr_IncludeSeperator = 1 << 1,
    SplitStr_AllowEmptyMember = 1 << 2,
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

#define  Str8Stream(str, ptr, opl) for (u8  *ptr = (str).str, *opl = (str).str + (str).size; ptr < opl;)
#define Str16Stream(str, ptr, opl) for (u16 *ptr = (str).str, *opl = (str).str + (str).size; ptr < opl;)
#define Str32Stream(str, ptr, opl) for (u32 *ptr = (str).str, *opl = (str).str + (str).size; ptr < opl;)

//~ NOTE(long): Math Functions

//- NOTE(long) Float Constant Functions
function f32 Inf_f32(void);
function f32 NegInf_f32(void);
function b32 InfOrNan_f32(f32 x);

function f64 Inf_f64(void);
function f64 NegInf_f64(void);
function b32 InfOrNan_f64(f64 x);

//- NOTE(long): Numeric Functions
function i32 AbsI32(i32 x);
function i64 AbsI64(i64 x);
function f32 Abs_f32(f32 x);
function f64 Abs_f64(f64 x);

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

//- NOTE(long): Trigonometric Functions
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

//~ NOTE(long): Arena Functions

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
#define ArenaPos(arena, pos) ((arena)->curr->basePos + (pos))
#define ArenaCurrPos(arena) ArenaPos(arena, (arena)->curr->pos)

#define PtrAdd(ptr, offset) ((u8*)(ptr) + (offset))

#define   PushStruct(arena, type)        (type*)  ArenaPush((arena), sizeof(type))
#define PushStructNZ(arena, type)        (type*)ArenaPushNZ((arena), sizeof(type))
#define    PushArray(arena, type, count) (type*)  ArenaPush((arena), sizeof(type) * (count))
#define  PushArrayNZ(arena, type, count) (type*)ArenaPushNZ((arena), sizeof(type) * (count))

function TempArena TempBegin(Arena* arena);
function TempArena GetScratch(Arena** conflictArray, u32 count);
function void      TempEnd(TempArena temp);

#define ScratchName(name) Concat(_tempArenaOf_, name)
#define ScratchClear(scratch) TempEnd(ScratchName(scratch))

#define ScratchBegin(name, ...) \
    TempArena ScratchName(name) = GetScratch(ArrayExpand(Arena*, 0, __VA_ARGS__)); Arena* name = ScratchName(name).arena; \
    i32 Concat(_debugVarOf_, name);
#define ScratchEnd(scratch) Stmnt(ScratchClear(scratch); ScratchName(scratch) = (TempArena){0}; scratch = 0; \
                                  Concat(_debugVarOf_, scratch);)

#define TempBlock(name, arena) for (TempArena name = TempBegin(arena); name.arena != 0; TempEnd(name), name = (TempArena){0})
#define TempPoisonBlock(name, arena) \
    for (TempArena name = (ArenaPushPoisonEx((arena), 0, MEM_POISON_SIZE, 0), TempBegin(arena)), UNIQUE(name) = {0}; \
         UNIQUE(name).pos == 0; \
         UNIQUE(name).pos = ArenaCurrPos(arena), TempEnd(name), \
         ArenaPushPoisonEx((arena), 0, UNIQUE(name).pos - name.pos, MEM_POISON_SIZE))

#define ScratchBlock(name, ...) struct { i32 i; TempArena temp; } UNIQUE(_i_) = \
    { .temp = GetScratch(ArrayExpand(Arena*, 0, __VA_ARGS__)) }; \
    for (Arena* name = UNIQUE(_i_).temp.arena; \
         UNIQUE(_i_).i == 0; \
         UNIQUE(_i_).i++, UNIQUE(_i_).temp = (TempEnd(UNIQUE(_i_).temp), (TempArena){0}))

//~ NOTE(long): String Functions

//- NOTE(long): Constructor Functions
#define Str(str, size) (String){ (str), (size) }
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

//- NOTE(long): Allocation Functions
function String StrCopy(Arena* arena, String str);
#define StrCloneCStr(arena, cstr) StrCopy((arena), StrFromCStr(cstr))
#define StrToCStr(a, s) StrCopy((a), (s)).str

#define GetEnumStr(type, e) (InRange(e, 0, EnumCount(type) - 1) ? (Concat(type, _names)[(i32)(e)]) : StrLit("Invalid"))
#define GetEnumName(type, e) GetEnumStr(type, e).str
#define GetFlagStr(arena, type, flags) GetFlagName_((arena), Concat(type, _names), \
                                                    ArrayCount(Concat(type, _names)), (flags))
#define GetFlagName(arena, type, flags) GetFlagStr(arena, type, flags).str

function StringList StrList(Arena* arena, String* strArr, u64 count);
function StringList StrListExplicit(StringNode* nodes, String* strs, u64 count);

function void StrListPush(Arena* arena, StringList* list, String str);
function void StrListPushNode(StringList* list, String str, StringNode* nodeMem);

function String StrListPop     (StringList* list);
function String StrListPopFront(StringList* list);

// NOTE(long): Need to put const here because of C6298
function String StrPushfv(Arena* arena, char* fmt, va_list args);
function String StrPushf (Arena* arena, CHECK_PRINTF char* fmt, ...);
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

function StringList StrSplitList(Arena* arena, String str, StringList* splits, StringSplitFlags flags);
function StringList StrSplitArr (Arena* arena, String str, String      splits, StringSplitFlags flags);
function StringList StrSplit    (Arena* arena, String str, String      split , StringSplitFlags flags);

function String StrReplaceList(Arena* arena, String str, StringList* oldStr, String newStr, b32 noCase);
function String StrReplaceArr (Arena* arena, String str, String      oldArr, String newStr, b32 noCase);
function String StrReplace    (Arena* arena, String str, String      oldStr, String newStr, b32 noCase);

//- NOTE(long): Comparision Functions
function b32 ChrCompare(char a, char b, b32 noCase);
function b32 StrCompare(String a, String b, b32 noCase);
function b32 ChrCompareArr(char chr, String arr, b32 noCase);
function b32 StrListCompare(String str, StringList* values, b32 noCase);
function b32 StrIsWhitespace(String str); // Empty strings will return true

function i64 StrFindStr(String str, String val, StringFindFlags flags);
function i64 StrFindArr(String str, String arr, StringFindFlags flags);
function StringNode* StrFindList(String str, StringList* list, StringFindFlags flags);

function String StrGetSubstr(String a, String b, StringFindFlags flags);
function String StrChopAfter(String str, String arr, StringFindFlags flags);
function String StrSkipUntil(String str, String arr, StringFindFlags flags);

#define ChrIsUpper(c) ((c) >= 'A' && (c) <= 'Z')
#define ChrIsLower(c) ((c) >= 'a' && (c) <= 'z')
#define ChrToUpper(c) ((c) & (MAX_U8-32))
#define ChrToLower(c) ((c) | 32)
#define ChrCompareNoCase(a, b) (ChrToUpper(a) == ChrToUpper(b))

#define StrIsIdentical(a, b) ((a).str == (b).str && (a).size == (b).size)
#define StrIsSubstr(str, substr) ((str).str <= (substr).str && ((str).str + (str).size >= (substr).str + (substr).size))
#define StrIsPrefix(str, prefix) ((str).str == (prefix).str && (str).size >= (prefix).size)
#define StrIsPostfix(str, postfix) (((str).str + (str).size == (postfix).str + (postfix).size) && (str).str <= (postfix).str)

#define StrStartsWith(str, val, noCase) (StrCompare(StrPrefix ((str), (val).size), (val), noCase))
#define   StrEndsWith(str, val, noCase) (StrCompare(StrPostfix((str), (val).size), (val), noCase))

#define StrFindChr(str, chr, flags) StrFindArr((str), StrLit(chr), (flags))
#define StrContainsChr(str, chr) (StrFindArr((str), StrLit(chr), 0) > -1)
#define StrContainsStr(str, val) (StrFindStr((str), (val), 0) > -1)
#define StrContainsNums(str) (StrFindChr((str), Digits, 0) > -1)
#define StrContainsChrs(str) (StrFindChr((str), Characters, 0) > -1)

//- NOTE(long): Mutable Functions
function String StrWriteToStr(String src, u64 srcOffset, String dst, u64 dstOffset);
#define StrWriteToMem(data, offset, dst, size) StrWriteToStr((data), (offset), Str(dst, size), 0)

function void StrToLower(String str);
function void StrToUpper(String str);
function void StrSwapChr(String str, char o, char n);

//- NOTE(long): Unicode Functions
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
function u32 RuneFolding(u32 rune); // NOTE(long): Should StrToLow(Upp)er use this?
function b32 RuneIsBlank(u32 rune); // NOTE(long): Should StrIsWhitespace use this?

//- NOTE(long): Convert Functions
function String StrFromF32(Arena* arena, f32 x, u32 prec);
function String StrFromF64(Arena* arena, f64 x, u32 prec);
function String StrFromI32(Arena* arena, i32 x, u32 radix);
function String StrFromI64(Arena* arena, i64 x, u32 radix);

function f32 F32FromStr(String str, b32* error);
function f64 F64FromStr(String str, b32* error);
function i32 I32FromStr(String str, u32 radix, b32* error);
function i64 I64FromStr(String str, u32 radix, b32* error);

function String StrFromTime(Arena* arena, DateTime time);

//~ NOTE(long): Logs/Errors

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
function void   LogBeginEx(LogInfo info);
function Logger LogEnd(Arena* arena);
function StringList StrListFromLogger(Arena* arena, Logger* logger);

function LogInfo* LogGetInfo(void);
function void LogFmtStd(Arena* arena, Record* record, char* fmt, va_list args);
function void LogFmtANSIColor(Arena* arena, Record* record, char* fmt, va_list args);
function void LogPushf(i32 level, char* file, i32 line, CHECK_PRINTF char* fmt, ...);
#define LogPush(level, log, ...) LogPushf((level), __FILE__, __LINE__, (log), __VA_ARGS__)

#define ErrorBegin(...) LogBegin(.level = LOG_ERROR, __VA_ARGS__)
#define ErrorEnd(arena) LogEnd(arena)
#define ErrorSet(error, errno) ((errno) = 1, ErrorFmt(error))
#define ErrorFmt(error, ...) LogPush(LOG_ERROR, error, __VA_ARGS__)

//~ NOTE(long): Buffer Functions

function String  BufferInterleave  (Arena* arena, void** in, u64 laneCount, u64 elementSize, u64 elementCount);
function String* BufferUninterleave(Arena* arena, void*  in, u64 laneCount, u64 elementSize, u64 elementCount);
#define BufferRead(buffer, offset, value) StrWriteToMem((buffer), (offset), (value), sizeof(*(value)))

//~ NOTE(long): PRNG Functions

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

#endif //_BASE_H
