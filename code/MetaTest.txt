/* date = July 24th 2022 6:30 pm */

#ifndef _BASE_H
#define _BASE_H

// --------------------------------------------------
// NOTE: Context Cracking

#define ENABLE_ASSERT 1

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

// TODO: verify this works on clang
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
// TODO: ARM64
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

#if COMPILER_CL
#define sharedexport __declspec(dllexport)
#else
#error sharedexport not defined for this compiler
#endif

struct TestStruct
{
    u32 a;
    u16 b;
    i8 c;
    f64 d;
};

// --------------------------------------------------
// NOTE: Time

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

// --------------------------------------------------
// NOTE: Symbolic Constants

typedef u32 DataAccessFlags;
//enum
//{
//DataAccessFlag_Read = (1 << 0),
//DataAccessFlag_Write = (1 << 1),
//DataAccessFlag_Execute = (1 << 2),
//};
enum
{
    DataAccessFlag_Read,
    DataAccessFlag_Write,
    DataAccessFlag_Execute,
};

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

// --------------------------------------------------
// NOTE(allen): Base Memory Pre-Requisites

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

// --------------------------------------------------
// NOTE: Arena Types

typedef struct MemArena
{
    u64 cap;
    u64 pos;
    u64 commitPos;
} MemArena;

typedef struct TempArena
{
    MemArena* arena;
    u64 pos;
} TempArena;

// --------------------------------------------------
// NOTE: Arena Functions

#define DEFAULT_RESERVE_SIZE GB(1)
#define COMMIT_BLOCK_SIZE MB(64)

#define SCRATCH_POOL_COUNT 4

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

function TempArena GetScratch(MemArena** conflictArray, u32 count);
#define            ReleaseScratch(temp) EndTemp(temp)

#define ScratchName(name) Concat(_tempArenaOf_, name)
#define BeginScratch(name, ...) TempArena ScratchName(name) = GetScratch(ArrayExpand(MemArena*, 0, __VA_ARGS__)); MemArena* name = ScratchName(name).arena
#define ResetScratch(scratch) EndTemp(ScratchName(scratch))
#define EndScratch(scratch) Stmnt(ResetScratch(scratch); ScratchName(scratch) = (TempArena){0}; scratch = 0;)

// --------------------------------------------------
// NOTE: String Types

struct StringJoin;

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

typedef struct StringJoin
{
    String pre;
    String mid;
    struct String post;
    struct A
    {
        u32 sad;
        u64 fuck;
    } a;
    struct B
    {
        f32 b;
    };
    struct
    {
        i32 c;
    };
} StringJoin;

struct
{
    i32 bTest;
    i32 cTest;
};

typedef u32 StringMatchFlags;
enum
{
    StringMatchFlag_NoCase = 1 << 0,
    StringMatchFlag_NotEqual = 1 << 1,
    StringMatchFlag_Inclusive = 1 << 2, // For SkipStrUntil, ChopStrAfter, and SplitStr
    StringMatchFlag_IsCharArray = 1 << 3, // For CompareStr, ReplaceStr, and SplitStr
};

#define StringIter(str, i) for (u64 i = 0; i < (str).size; ++i)

// --------------------------------------------------
// NOTE: String Functions

function String Str(u8* str, u64 size);
function String StrRange(u8* first, u8* opl);
function String StrFromCStr(u8* cstr);
function String CopyStr(MemArena* arena, String str, b32 addNullTerminator);

#define ConstStrLit(s) { (u8*)(s), sizeof(s) - 1 }
#define StrLit(s) Str((u8*)(s), sizeof(s) - 1)
#define StrExpand(s) (int)((s).size), ((s).str)

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

function void PushStrListExplicit(StringList* list, String str, StringNode* nodeMem);
function void PushStrList(MemArena* arena, StringList* list, String str);
function String JoinStr(MemArena* arena, StringList* list, StringJoin* optionalJoin, b32 terminate);
function String ReplaceStr(MemArena* arena, String str, String oldStr, String newStr, StringMatchFlags flags, b32 terminate);
function StringList SplitStr(MemArena* arena, String str, String splits, StringMatchFlags flags);
function StringList StrList(MemArena* arena, String* strArr, u64 count);

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

function b32 IsNullTerminated(String str);
function b32 AtEndOfStr(String str, u8* opl);
function b32 IsInsideStr(String str, u8* ptr);

// --------------------------------------------------
// NOTE: String Convert Functions

function f32 StrToF32(String str, b32* error);
function f64 StrToF64(String str);
function i64 StrToI64(String str, b32* error);

// --------------------------------------------------
// TODO: Unicode Functions

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

// --------------------------------------------------
// TODO: Interleaving/Swizzling Functions

function String  bop_interleave(MemArena* arena, void **in,
                                u64 laneCount, u64 elSize, u64 elCount);

function String* bop_uninterleave(MemArena* arena, void *in,
                                  u64 laneCount, u64 elSize, u64 elCount);

#endif //_BASE_H
