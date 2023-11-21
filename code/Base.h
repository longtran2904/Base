/* date = July 24th 2022 6:30 pm */

#ifndef _BASE_H
#define _BASE_H

//~ NOTE(long): Context Cracking

#define ENABLE_ASSERT 1

//- NOTE(long): Compiler/OS/Architecture
#if defined(__clang__)
#define COMPILER_CLANG 1

#ifdef _WIN32
#define OS_WIN 1
#elif defined(__gnu_linux__)
#define OS_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_MAC 1
#else
#error Missing OS detection
#endif

// TODO(long): verify this works on clang
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

#elif defined(_MSC_VER)
#define COMPILER_CL 1

#ifdef _WIN32
#define OS_WIN 1
#else
#error Missing OS detection!
#endif

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

#elif defined(_GNUC__)
#define COMPILER_GCC 1

#ifdef _WIN32
#define OS_WIN 1
#elif defined(__gnu_linux__)
#define OS_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_MAC 1
#else
#error Missing OS detection
#endif

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

#ifndef OS_WIN
#define OS_WIN 0
#endif

#ifndef OS_LINUX
#define OS_LINUX 0
#endif

#ifndef OS_MAC
#define OS_MAC 0
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
#define CURRENT_COMPILER_NAME "CLANG"
#elif COMPILER_CL
#define CURRENT_COMPILER_NUMBER 2
#define CURRENT_COMPILER_NAME "CL"
#elif COMPILER_GCC
#define CURRENT_COMPILER_NUMBER 3
#define CURRENT_COMPILER_NAME "GCC"
#endif

#if OS_WIN
#define CURRENT_OS_NUMBER 1
#define CURRENT_OS_NAME "WIN"
#elif OS_LINUX
#define CURRENT_OS_NUMBER 2
#define CURRENT_OS_NAME "LINUX"
#elif OS_MAC
#define CURRENT_OS_NUMBER 3
#define CURRENT_OS_NAME "MAC"
#endif

#if ARCH_X64
#define CURRENT_ARCH_NUMBER 1
#define CURRENT_ARCH_NAME "X64"
#elif ARCH_X86
#define CURRENT_ARCH_NUMBER 2
#define CURRENT_ARCH_NAME "x86"
#elif ARCH_ARM
#define CURRENT_ARCH_NUMBER 3
#define CURRENT_ARCH_NAME "ARM"
#elif ARCH_ARM64
#define CURRENT_ARCH_NUMBER 4
#define CURRENT_ARCH_NAME "ARM64"
#endif

//- NOTE(long): Language
#if defined(__cplusplus)
# define LANG_CXX 1
#else
# define LANG_C 1
#endif

#if !defined(LANG_CXX)
# define LANG_CXX 0
#endif
#if !defined(LANG_C)
# define LANG_C 0
#endif

//- NOTE(long): Pointer
#if ARCH_X64 || ARCH_ARM64
# define ARCH_ADDRSIZE 64
#else
# define ARCH_ADDRSIZE 32
#endif

//~ NOTE(long): Helper Macros

#if 1 // @RECONSIDER(long)
#define tdef(type, name) typedef type name name; type name
#define sdef(name) tdef(struct, name)
#define edef(name) tdef(enum, name)
#define udef(name) tdef(union, name)
#endif

#define Stmnt(S) do { S } while (0)

#ifndef AssertBreak
#define AssertBreak() (*(int*)0 = 0)
#endif

#if ENABLE_ASSERT
#define Assert(c) Stmnt(if (!(c)) { AssertBreak(); })
#else
#define Assert(...)
#endif

#define StaticAssert(c, l) typedef u8 Concat(l, __LINE__) [(c)?1:-1]

#define Stringify_(s) #s
#define Stringify(s) Stringify_(s)
#define Concat_(a, b) a##b
#define Concat(a, b) Concat_(a, b)

#define EnumCount(type) Concat(type, _Count)
#define ArrayCount(a) (sizeof(a)/sizeof(*(a)))
#define ArrayExpand(type, ...) (type[]){ __VA_ARGS__ }, ArrayCount((type[]){ __VA_ARGS__ })

#define IntFromPtr(p) (unsigned long long)((char*)p - (char*)0)
#define PtrFromInt(n) (void*)((char*)0 + (n))

#define Member(T, m) (((T*)0)->m)
#define OffsetOf(T, m) IntFromPtr(&Member(T, m))

#define Implies(a,b) (!(a) || (b))

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
#define Lerp(t, a, b) ((a) + (t) * ((b) - (a)))
#define GetSign(n) ((n) > 0 ? 1 : ((n) < 0 ? -1 : 0))
#define GetUnsigned(n) ((n) >= 0 ? 1 : -1)

#define BitCast(type, var) (*((type)*)(&(var))) // NOTE(long): UB

#define MagicP(T,x,s) ((T)(x) << (s))
#define MagicU32(a,b,c,d) (MagicP(U32,a,0) | MagicP(U32,b,8) | MagicP(U32,c,16) | MagicP(U32,d,24))

#define Boolify(x) ((x) != 0) // NOTE(long): Do I need this? Can't I just use `!!`

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) ((x) << 40)

#define Thousand(x) ((x)*1000)
#define Million(x)  ((x)*1000000llu)
#define Billion(x)  ((x)*1000000000llu)
#define Trillion(x) ((x)*1000000000000llu)

#define IsBinary(c) InRange(c, '0', '1')
#define IsDigit(c) InRange(c, '0', '9')
#define IsNonDigit(c) (InRange(c, 'A', 'Z') || InRange(c, 'a', 'z') || (c) == '_')
#define IsNonZeroDigit(c) InRange(c, '1', '9')
#define IsOctalDigit(c) InRange(c, '0', '7')
#define IsHexadecimalDigit(c) (InRange(c, 'A', 'F') || InRange(c, 'a', 'f') || IsDigit(c))

#define Binary "01"
#define Digits "0123456789"
#define NonDigits "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"
#define NonZeroDigits "123456789"
#define OctalDigits "01234567"
#define HexadecimalDigits "ABCDEF"Digits

#define IsSpace(c)  ((c=='\t') || (c=='\f') || (c=='\v') || (c==' '))
#define IsNline(c)  ((c=='\n') || (c=='\r'))
#define IsWspace(c) (IsSpace(c) || IsNline(c))

#define SpaceStr " \t\f\v"
#define NlineStr "\n\r"
#define WspaceStr Concat(SpaceStr, NlineStr)

#define global   static
#define local    static
#define function static

#if LANG_CXX
# define c_linkage extern "C"
#else
# define c_linkage extern
#endif

#if COMPILER_CL
#define threadvar __declspec(thread)
#elif COMPILER_CLANG
#define threadvar __thread
#else
#error threadvar defined for this compiler
#endif

#if COMPILER_CL
#define sharedexport __declspec(dllexport)
#else
#error sharedexport not defined for this compiler
#endif

#include <string.h>
#define ZeroMem(ptr, size) memset((ptr), 0, (size))
#define ZeroStruct(ptr) ZeroMem((ptr), sizeof(*(ptr)))
#define ZeroArray(ptr) ZeroMem((ptr), sizeof(ptr))
#define ZeroTypedArray(ptr, size) ZeroMem((ptr), sizeof(*(ptr))*(size))

#define CopyMem(dest, src, size) memmove((dest), (src), (size))
#define CopyStruct(dest, src) CopyMem((dest), (src), Min(sizeof(*(dest)), sizeof(*(src))))
#define CopyArray(dest, src) CopyMem((dest), (src), Min(sizeof(*(dest)), sizeof(*(src)))*Min(ArrayCount(dest), ArrayCount(src)))
#define CopyTypedArray(dest, src, count) CopyMem((dest), (src), Min(sizeof(*(dest)), sizeof(*(src)))*(count))

#define CompareMem(a, b, size) (memcmp((a), (b), (size)) == 0)
#define CompareArr(a, b) CompareMem((a), (b), Min(ArrayCount(a) * sizeof(*(a)), ArrayCount(b) * sizeof(*(b))))

#define ASCII4(str) (*(u32*)(str))
#define ExpandASCII4(x) (int)(sizeof(x)), (char*)(&(x))

// @RECONSIDER(long)
#define Using(type, name, ...) for (b32 done = !Begin##type(__VA_ARGS__); !done; done = End##type(__VA_ARGS__))

//~ NOTE(long): Linked List Macros

#define PushBackDLL_NP(f, l, n, next, prev) (((f)==0?\
                                              (f)=(l)=(n):\
                                              ((l)->next=(n),(l)=(n))),\
                                             (n)->next=(n)->prev=0)
#define PushBackDLL(f, l, n) DLLPushBack_NP(f, l, n, next, prev)
#define PushFrontDLL(f, l, n) DLLPushBack_NP(l, f, n, prev, next)

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
                           (f)=(l)=0:\
                           (f)=(f)->next)

#define SLLStackPush(f, n) ((n)->next=(f),(f)=(n))
#define SLLStackPop(f) ((f)==0?0:((f)=(f)->next))

//~ NOTE(long): Basic Types

#include <stdint.h>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef intptr_t iptr;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uintptr_t uptr;

#include <stdbool.h>
typedef i8 b8;
typedef i16 b16;
typedef i32 b32;
typedef i64 b64;

typedef float f32;
typedef double f64;

typedef void VoidFunc(void);
typedef void VoidFuncVoid(void*);

//~ NOTE(long): Basic Constants

#define MIN_I8  ((i8)0x80)
#define MIN_I16 ((i16)0x8000)
#define MIN_I32 ((i32)0x80000000)
#define MIN_I64 ((i64)0x8000000000000000ll)

#define MAX_I8  ((i8)0x7f)
#define MAX_I16 ((i16)0x7fff)
#define MAX_I32 ((i32)0x7fffffff)
#define MAX_I64 ((i64)0x7fffffffffffffffll)

#define MAX_U8  ((u8)0xff)
#define MAX_U16 ((u16)0xffff)
#define MAX_U32 ((u32)0xffffffff)
#define MAX_U64 ((u64)0xffffffffffffffffllu)

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

typedef enum Axis
{
    Axis_X,
    Axis_Y,
    Axis_Z,
    Axis_W,
} Axis;

typedef enum Side
{
    Side_Min,
    Side_Max,
} Side;

#if 0
#define ENUM_NAME Compiler
#define ENUM_VALUE(X) X(CLANG) X(CL) X(GCC)
#include "XEnum.h"

#define ENUM_NAME Arch
#define ENUM_VALUE(X) X(X64) X(X86) X(ARM) X(ARM64)
#include "XEnum.h"

#define ENUM_NAME OS
#define ENUM_VALUE(X) X(Win) X(Linux) X(Mac)
#include "XEnum.h"

#define ENUM_NAME Month
#define ENUM_VALUE(X) \
    X(Jan) X(Feb) X(Mar) X(Apr) X(May) X(Jun) X(Jul) X(Aug) X(Sep) X(Oct) X(Nov) X(Dec)
#include "XEnum.h"

#define ENUM_NAME Day
#define ENUM_VALUE(X) \
    X(Sunday) X(Monday) X(Tuesday) X(Wednesday) X(Thursday) X(Friday) X(Saturday)
#include "XEnum.h"
#else
typedef enum Compiler
{
    Compiler_None,
    
    Compiler_CLANG,
    Compiler_CL,
    Compiler_GCC,
    
    Compiler_Count
} Compiler;

typedef enum Arch
{
    Arch_None,
    
    Arch_X64,
    Arch_X86,
    Arch_ARM,
    Arch_ARM64,
    
    Arch_Count
} Arch;

typedef enum OS
{
    OS_None,
    
    OS_Win,
    OS_Linux,
    OS_Mac,
    
    OS_Count
} OS;

typedef enum Month
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
} Month;

typedef enum Day
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
} Day;
#endif

//~ NOTE(long): Time

typedef u64 DenseTime;

typedef struct DateTime
{
    u16 msec;
    u8 sec;
    u8 min;
    u8 hour;
    u8 day;
    u8 mon;
    i16 year;
} DateTime;

function DateTime ToDateTime(DenseTime time);
function DenseTime ToDenseTime(DateTime* time);

//~ NOTE(long): File Properties

typedef u32 DataAccessFlags;
typedef enum
{
    DataAccessFlag_Read = (1 << 0),
    DataAccessFlag_Write = (1 << 1),
    DataAccessFlag_Execute = (1 << 2),
} DataAccessFlag;

typedef u32 FilePropertyFlags;
enum
{
    FilePropertyFlag_IsFolder = (1 << 0)
};

typedef struct FileProperties
{
    u64 size;
    FilePropertyFlags flags;
    DenseTime createTime;
    DenseTime modifyTime;
    DataAccessFlags access;
} FileProperties;

//~ NOTE(long): Base Memory Pre-Requisites

#if !defined(ReserveMem)
# error missing definition for 'ReserveMem' type: (U64)->void* 
#endif
#if !defined(CommitMem)
# error missing definition for 'CommitMem' type: (void*,U64)->B32
#endif
#if !defined(DecommitMem)
# error missing definition for 'DecommitMem' type: (void*,U64)->void
#endif
#if !defined(ReleaseMem)
# error missing definition for 'ReleaseMem' type: (void*,U64)->void
#endif

//~ NOTE(long): Arena Types

typedef struct MemArena
{
    u64 cap;
    u64 pos;
    u64 commitPos;
    u64 highWaterMark;
} MemArena;

typedef struct TempArena
{
    MemArena* arena;
    u64 pos;
} TempArena;

#ifndef DEFAULT_RESERVE_SIZE
#define DEFAULT_RESERVE_SIZE KB(64)
#endif
#ifndef COMMIT_BLOCK_SIZE
#define COMMIT_BLOCK_SIZE KB(64)
#endif

#define SCRATCH_POOL_COUNT 4

//~ NOTE(long): Arena Functions

function MemArena* MakeReservedArena(u64 reserve);
function MemArena* MakeArena(void);

function void ReleaseArena(MemArena* arena);

function void* PushArena(MemArena* arena, u64 size);
function void  PopArenaTo(MemArena* arena, u64 pos);
#define PopArenaAmount(arena, amount) PopArenaTo((arena), (arena)->pos - (amount))

function void* PushZeroArena(MemArena* arena, u64 size);
function void  AlignArena(MemArena* arena, u64 alignment);
function void  AlignZeroArena(MemArena* arena, u64 aligment);

#define PushStruct(arena, type) (type*)PushZeroArena((arena), sizeof(type))
#define PushArray(arena, type, count) (type*)PushArena((arena), sizeof(type) * (count))
#define PushZeroArray(arena, type, count) (type*)PushZeroArena((arena), sizeof(type) * (count))

function TempArena BeginTemp(MemArena* arena);
function void      EndTemp(TempArena temp);

function TempArena GetScratch(MemArena** conflictArray, u32 cout);
#define            ReleaseScratch(temp) EndTemp(temp)

#define ScratchName(name) Concat(_tempArenaOf_, name)
#define ResetScratch(scratch) EndTemp(ScratchName(scratch))

#if 0
#define BeginScratch(name) TempArena ScratchName(name) = GetScratch(&threadCtx); \
    MemArena* name = ScratchName(name).arena
#define EndScratch(scratch) Stmnt(ResetScratch(scratch); ScratchName(scratch) = (TempArena){0}; \
                                  scratch = 0; threadCtx.callerScratch--;)
#else
#define BeginScratch(name, ...) TempArena ScratchName(name) = GetScratch(ArrayExpand(MemArena*, 0, __VA_ARGS__)); \
    MemArena* name = ScratchName(name).arena
#define EndScratch(scratch) Stmnt(ResetScratch(scratch); ScratchName(scratch) = (TempArena){0}; scratch = 0;)
#endif

//~ NOTE(long): String Types

typedef struct String
{
    u8* str;
    u64 size;
} String;

typedef struct StringNode StringNode;
struct StringNode
{
    StringNode* next;
    String string;
};

typedef struct StringList
{
    StringNode* first;
    StringNode* last;
    u64 nodeCount;
    u64 totalSize;
} StringList;

typedef struct StringJoin
{
    String pre;
    String mid;
    String post;
} StringJoin;

//~ TODO(long): Improve the string manipulation functions and helpers

// TODO(long): Make the String API works with IsCharArray
typedef u32 StringMatchFlags;
enum
{
    StringMatchFlag_NoCase = 1 << 0,
    StringMatchFlag_NotEqual = 1 << 1,
    StringMatchFlag_Inclusive = 1 << 2, // For SkipStrUntil, ChopStrAfter, and SplitStr
    StringMatchFlag_IsCharArray = 1 << 3, // For CompareStr, ReplaceStr, and SplitStr
};

#define StringIter(str, i) for (u64 i = 0; i < (str).size; ++i)

//~ NOTE(long): String Functions

function String Str(u8* str, u64 size);
function String StrRange(u8* first, u8* opl);
function String StrFromCStr(u8* cstr);
function String CopyStr(MemArena* arena, String str, b32 addNullTerminator);

#define ConstStr(s) { (u8*)(s), sizeof(s) - 1 }
#define StrLit(s) Str((u8*)(s), sizeof(s) - 1)
#define StrExpand(s) (i32)((s).size), ((s).str)

#define GetEnumStr(type, e) (InRange(e, 0, EnumCount(type) - 1) ? (Concat(type, _names)[(i32)(e)]) : StrLit("Invalid"))
#define GetEnumName(type, e) GetEnumStr(type, e).str
#define GetFlagStr(arena, type, flags) GetFlagName_((arena), Concat(type, _names), \
                                                    ArrayCount(Concat(type, _names)), (flags))
#define GetFlagName(arena, type, flags) GetFlagStr(arena, type, flags).str

function String PrefixStr (String str, u64 size);
function String ChopStr (String str, u64 size);
function String PostfixStr (String str, u64 size);
function String SkipStr (String str, u64 size);

function String ChopStrAfter(String str, String characters, StringMatchFlags flags);
function String SkipStrUntil(String str, String characters, StringMatchFlags flags);

function String PreSubStr(String str, String substr);
function String PostSubStr(String str, String substr);
function String SubStr(String str, u64 first, u64 opl);
function String SubStrRange(String str, u64 first, u64 range);

function StringList StrList(MemArena* arena, String* strArr, u64 count);
function StringList StrListExplicit(StringNode* nodes, String* strs, u64 count);
function void PushStrListExplicit(StringList* list, String str, StringNode* nodeMem);
function void PushStrList(MemArena* arena, StringList* list, String str);

function String JoinStr(MemArena* arena, StringList* list, StringJoin* join, b32 terminate);
function String JoinStr3(MemArena* arena, StringJoin* optionalJoin, b32 terminate);
function String ReplaceStr(MemArena* arena, String str, String oldStr, String newStr,
                           StringMatchFlags flags, b32 terminate);
function StringList SplitStr(MemArena* arena, String str, String splits, StringMatchFlags flags);

function String PushStrfv(MemArena* arena, b32 terminate, char* fmt, va_list args);
function String PushStrf(MemArena* arena, b32 terminate, char*fmt, ...);
function void   PushStrListf(MemArena* arena, b32 terminate, StringList* list, char* fmt, ...);

#define IsUpperCase(c) ((c) >= 'A' && (c) <= 'Z')
#define IsLowerCase(c) ((c) >= 'a' && (c) <= 'z')
#define UpperCase(c)   (IsLowerCase(c) ? ((c) + 'A' - 'a') : (c))
#define LowerCase(c)   (IsUpperCase(c) ? ((c) + 'a' - 'A') : (c))
#define CompareCharNoCase(a, b) (UpperCase(a) == UpperCase(b))

function b32 CompareChar(char a, char b, StringMatchFlags flags);
function b32 CompareCharArray(char c, String characters, StringMatchFlags flags);
function u64 GetMatchIndex(char c, String characters, StringMatchFlags flags);
function b32 CompareStr(String a, String b, StringMatchFlags flags);
function b32 CompareStrList(String str, StringList* matches, StringMatchFlags flags);

function b32 AtEndOfStr(String str, u8* opl);
function b32 IsInsideStr(String str, u8* ptr);

//~ NOTE(long): String Convert Functions

function f32 StrToF32(String str, b32* error);
function f64 StrToF64(String str);
function i64 StrToI64(String str, b32* error);

//~ NOTE(long): Errors

function b32    HasError(void);
function String GetError(void);
function void   SetError(String error);
function void   SetErrorf(char* format, ...);
#define SetConstError(error) SetError(StrLit(error))

//~ TODO(long): Unicode Functions

typedef struct StringDecode
{
    u32 codepoint;
    u32 size;
} StringDecode;

#if 0
function StringDecode DecodeUTF8Str(u8* str, u32 cap);
function u32          EncodeUTF8Str(u8* dest, u32 codepoint);
function StringDecode DecodeUTF16Str(u8* str, u32 cap);
function u32          EncodeUTF16Str(u16* dest, u32 codepoint);

function String32 Str8ToStr32(MemArena* arena, String str);
function String16 Str8ToStr16(MemArena* arena, String str);
function String   Str32ToStr8(MemArena* arena, String32 string);
function String   Str16ToStr8(MemArena* arena, String16 string);
#endif

//~ TODO(long): Interleaving/Swizzling Functions

function String  bop_interleave(MemArena* arena, void **in,
                                u64 laneCount, u64 elSize, u64 elCount);

function String* bop_uninterleave(MemArena* arena, void *in,
                                  u64 laneCount, u64 elSize, u64 elCount);

//~ NOTE(long): Math Functions

function i32 Abs(i32 x);
function i64 AbsI64(i64 x);

#define FLOAT_FUNCS() \
    GenerateFloatFunc(Inf, void, void, 0, = 0x7f800000, = 0xff800000); \
    GenerateFloatFunc(NegInf, void, void, 0, = 0xff800000, = 0xfff0000000000000); \
    GenerateFloatFunc(Abs, f32 x, f64 x, x, &= 0x7fffffff, &= 0x7fffffffffffffff); \
    \
    BindingFloatFunc(sqrt, Sqrt); \
    BindingFloatFunc(sin, Sin); \
    BindingFloatFunc(cos, Cos); \
    BindingFloatFunc(tan, Tan); \

#define GenerateFloatFunc(name, arg32, arg64, ...) \
    function f32 name##_f32(arg32); \
    function f64 name##_f64(arg64);

#define BindingFloatFunc(oldName, newName) \
    f32 newName##_f32(f32 x); \
    f64 newName##_f64(f64 x);

FLOAT_FUNCS();
#undef GenerateFloatFunc
#undef BindingFloatFunc

typedef struct RNG
{
    u32 seed;
    u32 pos;
} RNG;

#define BIT_NOISE1 0x68E31DA4
#define BIT_NOISE2 0xB5297A4D
#define BIT_NOISE3 0x1B56C4E9
#define BIT_NOISE4 0x0BD4BCB5
#define BIT_NOISE5 0x0063D68D

function u32 GetNoise(u32 pos, u32 seed);
function u64 Hash64(u8* values, u64 count);
#define GetNoise2D(x, y, seed) GetNoise(x + (BIT_NOISE4 * y), seed)
#define GetNoise3D(x, y, seed) GetNoise(x + (BIT_NOISE4 * y) + (BIT_NOISE5 * z), seed)
#define RandomU32(rng) GetNoise1D(rng->pos++, rng->speed)
#define RandomF32(rng) (RandomU32(rng) / (f32)MAX_U32)
#define RandomRangeI32(rng, minIn, maxIn) ((i32)(minIn) + RandomU32(rng) % ((i32)(maxIn) - (i32)(minIn)))
#define RandomRangeF32(rng, minIn, maxIn) ((minIn) + RandomF32((maxIn) - (minIn)))
#define Random(rng, prob) (RandomF32(rng) < prob ? true : false)

#endif //_BASE_H
