
//~ NOTE(long): Symbolic Constants

global String Compiler_names[] =
{
    StrConst("None"),
    StrConst("CLANG"),
    StrConst("CL"),
    StrConst("GCC"),
};

global String Arch_names[] =
{
    StrConst("None"),
    StrConst("X64"),
    StrConst("X86"),
    StrConst("ARM"),
    StrConst("ARM64"),
};

global String OS_names[] =
{
    StrConst("None"),
    StrConst("Win"),
    StrConst("Linux"),
    StrConst("Mac"),
};

global String Month_names[] =
{
    StrConst("None"),
    StrConst("Jan"),
    StrConst("Feb"),
    StrConst("Mar"),
    StrConst("Apr"),
    StrConst("May"),
    StrConst("Jun"),
    StrConst("Jul"),
    StrConst("Aug"),
    StrConst("Sep"),
    StrConst("Oct"),
    StrConst("Nov"),
    StrConst("Dec"),
};

global String Day_names[] =
{
    StrConst("None"),
    StrConst("Sunday"),
    StrConst("Monday"),
    StrConst("Tuesday"),
    StrConst("Wednesday"),
    StrConst("Thursday"),
    StrConst("Friday"),
    StrConst("Saturday"),
};

//~ NOTE(long): Math Functions

#include <math.h>

//- NOTE(long): Trigonometric Functions
#ifndef EXTRA_PRECISION
#define EXTRA_PRECISION
#endif

// https://web.archive.org/web/20060503192740/http://www.devmaster.net/forums/showthread.php?t=5784
function f64 SinPi64(f64 x)
{
    f64 y = 4 / PI_F64 * x + -4 / (PI_F64 * PI_F64) * x * Abs_f64(x);
    
#ifdef EXTRA_PRECISION
    //  f64 Q = 0.775;
    f64 P = 0.225;
    y = P * (y * Abs_f64(y) - y) + y;   // Q * y + P * y * abs(y)
#endif
    
    return y;
}

function f64 SinTurn64(f64 x)
{
    f64 y = 4 * x + -4 * x * Abs_f64(x);
#ifdef EXTRA_PRECISION
    y = 0.225 * (y * Abs_f64(y) - y) + y;
#endif
    return y;
}

function f32 SinPi32(f32 x)
{
    return (f32)SinPi64(x);
}

function f32 SinTurn32(f32 x)
{
    return (f32)SinTurn64(x);
}

function f64 CosPi64(f64 x)
{
    x += PI_F64 / 2;
    x -= (x > PI_F64) * (2 * PI_F64);
    return SinTurn64(x);
}

function f32 Sin_f32(f32 x) { return sinf(x); }
function f32 Cos_f32(f32 x) { return cosf(x); }
function f32 Tan_f32(f32 x) { return tanf(x); }

function f64 Sin_f64(f64 x) { return sin(x); }
function f64 Cos_f64(f64 x) { return cos(x); }
function f64 Tan_f64(f64 x) { return tan(x); }

function f32 Atan_f32(f32 x) { return atanf(x); }
function f64 Atan_f64(f64 x) { return atan (x); }

function f32 Atan2_f32(f32 x, f32 y) { return atan2f(x, y); }
function f64 Atan2_f64(f64 x, f64 y) { return atan2 (x, y); }

//- NOTE(long): Float Constant Functions
function f32 Inf_f32(void)
{
    f32 f = 0;
    u32 u = *(u32*)&f;
    u = 0x7f800000;
    return *(f32*)&u;
}

function f32 NegInf_f32(void)
{
    f32 f = 0;
    u32 u = *(u32*)&f;
    u = 0xff800000;
    return *(f32*)&u;
}

function b32 InfOfNan_f32(f32 x)
{
    u32 u = *(u32*)&x;
    return (u & 0x7f800000) == 0x7f800000;
}

function f64 Inf_f64(void)
{
    f64 f = 0;
    u64 u = *(u64*)&f;
    u = 0x7ff0000000000000;
    return *(f64*)&u;
}

function f64 NegInf_f64(void)
{
    f64 f = 0;
    u64 u = *(u64*)&f;
    u = 0xfff0000000000000;
    return *(f64*)&u;
}

function b64 InfOfNan_f64(f64 x)
{
    u64 u = *(u64*)&x;
    return (u & 0xff800000) == 0xff800000;
}

//- NOTE(long): Numeric Functions
function i32 AbsI32(i32 x)
{
    i32 mask = x >> 31;
    return (x ^ mask) - mask; // or (n + mask) ^ mask
}
function i64 AbsI64(i64 x)
{
    i64 mask = x >> 63;
    return (x ^ mask) - mask; // or (n + mask) ^ mask
}

function f32 Abs_f32(f32 x)
{
    u32 u = *(u32*)&x;
    u &= 0x7fffffff;
    return x;
}
function f64 Abs_f64(f64 x)
{
    u64 u = *(u64*)&x;
    u &= 0x7fffffffffffffff;
    return x;
}

function f32 Trunc_f32(f32 x) { return (f32)((i32)x); }
function f32 Floor_f32(f32 x)
{
    f32 r = Trunc_f32(x);
    return r > x ? r - 1 : r;
}
function f32 Ceil_f32(f32 x)
{
    f32 r = Trunc_f32(x);
    return r < x ? r + 1 : r;
}

function f64 Trunc_f64(f64 x) { return (f64)((i64)x); }
function f64 Floor_f64(f64 x)
{
    f64 r = Trunc_f64(x);
    return r > x ? r - 1 : r;
}
function f64 Ceil_f64(f64 x)
{
    f64 r = Trunc_f64(x);
    return r < x ? r + 1 : r;
}

function f32 Sqrt_f32(f32 x) { return sqrtf(x); }
function f64 Sqrt_f64(f64 x) { return sqrt (x); }

function f32 Ln_f32(f32 x) { return logf(x); }
function f64 Ln_f64(f64 x) { return log (x); }

function f32 Pow_f32(f32 base, f32 x) { return powf(base, x); }
function f64 Pow_f64(f64 base, f64 x) { return pow (base, x); }

function f32 FrExp_f32(f32 x, i32* exp) { return frexpf(x, exp); }
function f64 FrExp_f64(f64 x, i32* exp) { return frexp (x, exp); }

//~ NOTE(long): PRNG Functions

function u32 Noise1D(u32 pos, u32 seed)
{
    u32 result = pos;
    result *= BIT_NOISE1;
    result += seed;
    result ^= (result >> 8);
    result += BIT_NOISE2;
    result ^= (result << 8);
    result *= BIT_NOISE3;
    result ^= (result >> 8);
    return result;
}

function u64 Hash64(u8* values, u64 count)
{
    u64 result = BIT_NOISE5;
    for (u64 i = 0; i < count / 8; ++i)
        result = Noise1D(result, *((u64*)values + i) >> (i % 16));
    for (u8* i = values + (count / 8) * 8; i < values + count; ++i)
        result += BIT_NOISE4 * (*i) + BIT_NOISE5;
    return result;
}

//~ NOTE(long): Time

function DateTime TimeToDate(DenseTime time)
{
    DateTime result = {0};
    result.msec = time % 1000;
    time /= 1000;
    result.sec = time % 61;
    time /= 61;
    result.min = time % 60;
    time /= 60;
    result.hour = time%24;
    time /= 24;
    result.day = time % 31;
    time /= 31;
    result.mon = time % 12;
    time /= 12;
    i32 encodedYear = (i32)time;
    result.year = (encodedYear - 0x8000);
    return result;
}

function DenseTime TimeToDense(DateTime* time)
{
    DenseTime result = 0;
    u32 encodedYear = (u32)((i32)time->year + 0x8000);
    result+= encodedYear;
    result *= 12;
    result += time->mon;
    result *= 31;
    result += time->day;
    result *= 24;
    result += time->hour;
    result *= 60;
    result += time->min;
    result *= 61;
    result += time->sec;
    result *= 1000;
    result += time->msec;
    return result;
}

//~ NOTE(long): Arena Functions

function void* ChangeMemoryNoOp(void* ptr, u64 size) { return ptr; }

#define INITIAL_COMMIT KB(4)
StaticAssert(sizeof(Arena) <= INITIAL_COMMIT, CheckArenaSize);

function Arena* ArenaReserve(u64 reserve)
{
    Arena* result = 0;
    if (reserve >= INITIAL_COMMIT)
    {
        void* memory = MemReserve(reserve);
        if (MemCommit(memory, INITIAL_COMMIT))
        {
            result = (Arena*)memory;
            result->cap = reserve;
            result->pos = AlignUpPow2(sizeof(Arena), 64);
            result->commitPos = INITIAL_COMMIT;
        }
    }
    
    Assert(result != 0);
    return result;
}

function void ArenaRelease(Arena* arena)
{
    MemRelease(arena);
}

function void* ArenaPush(Arena* arena, u64 size)
{
    void* result = 0;
    if (arena->pos + size <= arena->cap)
    {
        result = ((u8*)arena) + arena->pos;
        arena->pos += size;
        if (arena->pos > arena->highWaterMark)
            arena->highWaterMark = arena->pos;
        
        if (arena->pos > arena->commitPos)
        {
            u64 alignedPos = AlignUpPow2(arena->pos, COMMIT_BLOCK_SIZE - 1);
            u64 nextCommitPos = ClampTop(alignedPos, arena->cap);
            u64 commitSize = nextCommitPos - arena->commitPos;
            
            if (MemCommit(((u8*)arena) + arena->commitPos, commitSize))
                arena->commitPos = nextCommitPos;
            else
                result = 0;
        }
    }
    
    Assert(result != 0);
    return result;
}

function void ArenaPopTo(Arena* arena, u64 pos)
{
    if (pos < arena->pos)
    {
        arena->pos = pos;
        
        u64 alignedPos = AlignUpPow2(arena->pos, COMMIT_BLOCK_SIZE - 1);
        u64 nextCommitPos = ClampTop(alignedPos, arena->cap);
        
        if (nextCommitPos < arena->commitPos)
        {
            u64 decommitSize = arena->commitPos - nextCommitPos;
            MemDecommit(arena + nextCommitPos, decommitSize);
            arena->commitPos = nextCommitPos;
        }
    }
}

function void* ArenaPushZero(Arena* arena, u64 size)
{
    void* result = ArenaPush(arena, size);
    ZeroMem(result, size);
    return result;
}

function void  ArenaAlign(Arena* arena, u64 alignment)
{
    u64 alignedPos = AlignUpPow2(arena->pos, alignment);
    u64 size = alignedPos - arena->pos;
    if (size > 0)
        ArenaPush(arena, size);
}

function void  ArenaAlignZero(Arena* arena, u64 aligment)
{
    u64 alignedPos = AlignUpPow2(arena->pos, aligment);
    u64 size = alignedPos - arena->pos;
    if (size > 0)
        ArenaPushZero(arena, size);
}

function TempArena TempBegin(Arena* arena)
{
    return (TempArena){ arena, arena->pos };
}

function void TempEnd(TempArena temp)
{
    ArenaPopTo(temp.arena, temp.pos);
}

threadvar Arena* scratchPool[SCRATCH_POOL_COUNT] = {0};
function TempArena GetScratch(Arena** conflictArray, u32 count)
{
    Arena** pool = scratchPool;
    if (pool[0] == 0)
    {
        Arena** scratchSlot = pool;
        for (u64 i = 0; i < SCRATCH_POOL_COUNT; ++i, ++scratchSlot)
            *scratchSlot = ArenaMake();
    }
    
    TempArena result = {0};
    Arena** scratchSlot = pool;
    for (u64 i = 0; i < SCRATCH_POOL_COUNT; ++i, ++scratchSlot)
    {
        b32 noConflict = true;
        Arena** conflictPtr = conflictArray;
        for (u32 j = 0; j < count; ++j, ++conflictPtr)
        {
            if (*scratchSlot == *conflictPtr)
            {
                noConflict = false;
                break;
            }
        }
        
        if (noConflict)
        {
            result = TempBegin(*scratchSlot);
            break;
        }
    }
    
    return result;
}

//~ NOTE(long): String Functions

//- NOTE(long): Constructor Functions
function String StrFromCStr(u8* cstr)
{
    u8* ptr = cstr;
    for (;*ptr != 0; ++ptr);
    return StrRange(cstr, ptr);
}

function String StrChop(String str, u64 size)
{
    u64 clampedSize = ClampTop(size, str.size);
    return (String){ (str.str), (str.size - clampedSize) };
}

function String StrSkip(String str, u64 size)
{
    u64 clampedSize = ClampTop(size, str.size);
    return (String){ (str.str + clampedSize), (str.size - clampedSize) };
}

function String StrPrefix(String str, u64 size)
{
    u64 clampedSize = ClampTop(size, str.size);
    return (String){ (str.str), clampedSize };
}

function String StrPostfix(String str, u64 size)
{
    u64 clampedSize = ClampTop(size, str.size);
    return (String){ (str.str + str.size - clampedSize), (clampedSize) };
}

function String Substr(String str, u64 first, u64 opl)
{
    u64 clampedOpl = ClampTop(str.size, opl);
    u64 clampedFirst = ClampTop(clampedOpl, first);
    return (String){ str.str + clampedFirst, clampedOpl - clampedFirst };
}

function String SubstrRange(String str, u64 first, u64 range)
{
    return Substr(str, first, first + range);
}

function StringJoin SubstrSplit(String str, String substr)
{
    StringJoin result = {0};
    if (StrIsSubstr(str, substr))
    {
        result.pre = StrRange(str.str, substr.str);
        result.mid = substr;
        result.post = StrRange(substr.str + substr.size, str.str + str.size);
    }
    return result;
}

function String StrTrim(String str, String arr, i32 dir)
{
    u64 first = 0;
    if (dir <= 0)
    {
        while (first < str.size)
        {
            if (ChrCompareArr(str.str[first], arr, 0))
                first++;
            else
                break;
        }
    }
    
    u64 end = str.size - 1;
    if (dir >= 0)
    {
        // NOTE(long): This can be > when first is actually initialized (dir == 0)
        while (end >= first)
        {
            if (ChrCompareArr(str.str[end], arr, 0))
                end--;
            else
                break;
        }
    }
    
    return end >= first ? Substr(str, first, end + 1) : (String){0};
}

//- NOTE(long): Allocation Functions
function String StrCopy(Arena* arena, String str)
{
    String result = { PushZeroArray(arena, u8, str.size + 1), str.size };
    CopyMem(result.str, str.str, str.size);
    return result;
}

function String GetFlagName_(Arena* arena, String* names, u64 nameCount, u64 flags)
{
    BeginScratch(scratch, arena);
    StringList list = {0};
    
    for (u64 i = 0; i < nameCount; ++i)
        if (flags & (1ULL << i))
            StrListPush(scratch, &list, names[i]);
    String result = StrJoin(arena, &list, .mid = StrLit(" | "));
    
    EndScratch(scratch);
    return result;
}

function StringList StrList(Arena* arena, String* strArr, u64 count)
{
    StringList result = {0};
    if (strArr != 0 && count != 0)
    {
        result.first = PushArray(arena, StringNode, count);
        result.last = result.first + count - 1;
        for (u64 i = 0; i < count; ++i)
            StrListPushNode(&result, strArr[i], &result.first[i]);
    }
    return result;
}

function StringList StrListExplicit(StringNode* nodes, String* strs, u64 count)
{
    StringList result = {0};
    if (strs != 0 && nodes != 0 && count != 0)
    {
        result.first = nodes;
        result.last = nodes + count - 1;
        for (u64 i = 0; i < count; ++i)
            StrListPushNode(&result, strs[i], nodes + i);
    }
    return result;
}

function void StrListPushNode(StringList* list, String str, StringNode* nodeMem)
{
    nodeMem->string = str;
    SLLQueuePush(list->first, list->last, nodeMem);
    list->nodeCount++;
    list->totalSize += str.size;
}

function void StrListPush(Arena* arena, StringList* list, String str)
{
    StringNode* node = PushStruct(arena, StringNode);
    StrListPushNode(list, str, node);
}

// NOTE(long): I could remove stdarg but stb_sprintf has already included it so what's the point
#include <stdarg.h>

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_STATIC
#include "stb_sprintf.h"

function String StrPushfv(Arena* arena, char* fmt, va_list args)
{
    // in case we need to try a second time
    va_list args2;
    va_copy(args2, args);
    
    // try to build the string in 1024 bytes
    u64 bufferSize = 1024;
    u8* buffer = PushZeroArray(arena, u8, bufferSize);
    // NOTE(long): vsnprintf doens't count the null terminator
    u64 size = stbsp_vsnprintf((char*)buffer, bufferSize, fmt, args);
    u64 allocSize = size + 1;
    
    String result = {0};
    if (allocSize <= bufferSize)
    {
        // if first try worked, put back the remaining
        ArenaPopAmount(arena, bufferSize - allocSize);
        result = Str(buffer, size);
    }
    else
    {
        // if first try failed, reset and try again with the correct size
        ArenaPopAmount(arena, bufferSize);
        u8* newBuffer = PushZeroArray(arena, u8, allocSize);
        stbsp_vsnprintf((char*)newBuffer, allocSize, fmt, args2);
        result = Str(newBuffer, size);
    }
    
    va_end(args2);
    return result;
}

function String StrPushf(Arena* arena, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String result = StrPushfv(arena, fmt, args);
    va_end(args);
    return result;
}

function String StrPad(Arena* arena, String str, char chr, u32 count, i32 dir)
{
    // @RECONSIDER(long): This will always alloc a new string, even if the count is zero
    u64 size = (dir == 0 ? count * 2 : count) + str.size;
    String result = { PushZeroArray(arena, u8, size + 1), size };
    if (dir <= 0) SetMem(result.str, chr, count);
    if (dir >= 0) SetMem(result.str + size - count, chr, count);
    return result;
}

function String StrInsert(Arena* arena, String str, u64 index, String value)
{
    index = ClampTop(index, str.size - 1);
    String result = { 0, str.size + value.size };
    result.str = PushZeroArray(arena, u8, result.size + 1);
    CopyMem(result.str, str.str, index);
    CopyMem(result.str + index, value.str, value.size);
    CopyMem(result.str + index + value.size, str.str + index, value.size - index);
    return result;
}

function String StrRemove(Arena* arena, String str, u64 index, u64 count)
{
    index = ClampTop(index, str.size - 1);
    count = ClampTop(count, str.size - index - 1);
    if (index + count == str.size)
        return StrChop(str, count); // @RECONSIDER(long): Maybe just alloc a new null-terminated string
    
    String result = { 0, count };
    result.str = PushZeroArray(arena, u8, count + 1);
    CopyMem(result.str, str.str, index);
    CopyMem(result.str + index, str.str + index + count, count);
    return result;
}

function String StrRepeat(Arena* arena, String str, u64 count)
{
    u64 size = count * str.size;
    // @RECONSIDER(long): This will always alloc a new string, even if the count is zero
    String result = { PushArray(arena, u8, size + 1), size };
    for (u64 i = 0; i < count; ++i)
        CopyMem(result.str + i * count, str.str, str.size);
    result.str[size] = 0;
    return result;
}

function String ChrRepeat(Arena* arena, char chr, u64 count)
{
    // @RECONSIDER(long): This will always alloc a new string, even if the count is zero
    String result = { PushArray(arena, u8, count + 1), count };
    SetMem(result.str, chr, count);
    result.str[count] = 0;
    return result;
}

function String StrJoinList(Arena* arena, StringList* list, StringJoin* join)
{
    StringJoin dummy = {0};
    if (!join)
        join = &dummy;
    
    String pre = join->pre, mid = join->mid, post = join->post;
    
    u64 size = (pre.size + post.size + mid.size * (ClampBot(list->nodeCount , 1) - 1) + list->totalSize);
    u8* str = PushZeroArray(arena, u8, size + 1);
    u8* ptr = str;
    
    CopyMem(ptr, pre.str, pre.size);
    ptr += pre.size;
    
    b32 isMid = false;
    for (StringNode* node = list->first; node != 0; node = node->next)
    {
        if (isMid)
        {
            CopyMem(ptr, mid.str, mid.size);
            ptr += mid.size;
        }
        
        CopyMem(ptr, node->string.str, node->string.size);
        ptr += node->string.size;
        
        isMid = true;
    }
    
    CopyMem(ptr, post.str, post.size);
    ptr += post.size;
    
    Assert(ptr == (str + size));
    
    return (String){ str, size };
}

function String StrJoin3_(Arena* arena, StringJoin* join)
{
    StringList list = {0};
    StringNode pre, mid, post;
    StrListPushNode(&list, join->pre, &pre);
    StrListPushNode(&list, join->mid, &mid);
    StrListPushNode(&list, join->post, &post);
    return StrJoin(arena, &list);
}

function StringList StrSplitList(Arena* arena, String str, StringList* splits, StringSplitFlags flags)
{
    StringList result = {0};
    
    if (splits->totalSize && str.size)
    {
        b32 noCase     = flags & SplitStr_NoCase;
        b32 inclusive  = flags & SplitStr_IncludeSeperator;
        b32 allowEmpty = flags & SplitStr_AllowEmptyMember;
        
        u8* ptr = str.str;
        u8* firstWord = ptr;
        u8* opl = str.str + str.size;
        
        // NOTE(long): < rather than <= because firstWord = ptr + 1 can crash at the end of buffer
        for (;ptr < opl; ++ptr)
        {
            StringNode* match = StrFindList(StrRange(ptr, opl), splits, noCase);
            if (match)
            {
                // NOTE(long): try to emit word, <= rather than < will allow empty members
                // EX: split "A,B,,C" with "," -> { "A", "B", "", "C" } vs { "A", "B", "C" }
                if ((firstWord < ptr) || (firstWord == ptr && allowEmpty))
                    StrListPush(arena, &result, StrRange(firstWord, ptr));
                if (inclusive)
                    StrListPush(arena, &result, Str(ptr, match->string.size));
                ptr += match->string.size - 1;
                firstWord = ptr + 1;
            }
        }
        
        // try to emit the final word
        if (firstWord < ptr)
            StrListPush(arena, &result, StrRange(firstWord, ptr));
    }
    
    return result;
}

function StringList StrSplitArr(Arena* arena, String str, String splits, StringSplitFlags flags)
{
    StringList result = {0};
    
    if (splits.size && str.size)
    {
        b32 noCase     = flags & SplitStr_NoCase;
        b32 inclusive  = flags & SplitStr_IncludeSeperator;
        
        u8* ptr = str.str;
        u8* firstWord = ptr;
        u8* opl = str.str + str.size;
        u64 size = 1;
        
        // NOTE(long): < rather than <= because firstWord = ptr + 1 can crash at the end of buffer
        for (;ptr < opl; ++ptr)
        {
            if (ChrCompareArr(*ptr, splits, noCase))
            {
                // NOTE(long): try to emit word, < rather than <= because we don't allow empty members
                // EX: split "A,B,,C" with "," -> { "A", "B", "C" }
                if (firstWord < ptr)
                    StrListPush(arena, &result, StrRange(firstWord, ptr));
                if (inclusive)
                    StrListPush(arena, &result, Str(ptr, 1));
                firstWord = ptr + 1;
            }
        }
        
        // try to emit the final word
        if (firstWord < ptr)
            StrListPush(arena, &result, StrRange(firstWord, ptr));
    }
    
    return result;
}

function StringList StrSplit(Arena* arena, String str, String split, StringSplitFlags flags)
{
    StringNode node = {0}; StringList list = {0};
    StrListPushNode(&list, split, &node);
    return StrSplitList(arena, str, &list, flags);
}

function String StrReplaceList(Arena* arena, String str, StringList* oldStr, String newStr, b32 noCase)
{
    StringList list = StrSplitList(arena, str, oldStr,
                                   SplitStr_AllowEmptyMember|(noCase ? SplitStr_NoCase : 0));
    return StrJoin(arena, &list, .mid = newStr);
}

function String StrReplaceArr(Arena* arena, String str, String charArr, String newStr, b32 noCase)
{
    StringList list = StrSplitArr(arena, str, charArr, SplitStr_AllowEmptyMember|(noCase ? SplitStr_NoCase : 0));
    return StrJoin(arena, &list, .mid = newStr);
}

function String StrReplace(Arena* arena, String str, String oldStr, String newStr, b32 noCase)
{
    StringNode node = {0}; StringList list = {0};
    StrListPushNode(&list, oldStr, &node);
    return StrReplaceList(arena, str, &list, newStr, noCase);
}

//- NOTE(long): Comparision Functions
function b32 ChrCompare(char a, char b, b32 noCase)
{
    if (noCase) return ChrCompareNoCase(a, b);
    return a == b;
}

function b32 StrCompare(String a, String b, b32 noCase)
{
    b32 result = false;
    
    if (a.size == b.size)
    {
        if (a.str != b.str)
            for (u64 i = 0; i < a.size; ++i)
                if (!ChrCompare(a.str[i], b.str[i], noCase))
                    return false;
        
        return true;
    }
    
    return false;
}

function b32 ChrCompareArr(char chr, String arr, b32 noCase)
{
    for (u64 i = 0; i < arr.size; ++i)
        if (ChrCompare(chr, arr.str[i], noCase))
            return true;
    return false;
}

function b32 StrListCompare(String str, StringList* values, b32 noCase)
{
    for (StringNode* node = values->first; node; node = node->next)
        if (StrCompare(str, node->string, noCase))
            return true;
    return false;
}

function b32 StrIsWhitespace(String str)
{
    for (u64 i = 0; i < str.size; ++i)
        if (!IsWspace(str.str[i]))
            return false;
    return true;
}

function i64 StrFindStr(String str, String val, StringFindFlags flags)
{
    if (str.size == 0 || val.size == 0)
        return -1;
    
    b32 noCase    = flags & FindStr_NoCase;
    b32 lastMatch = flags & FindStr_LastMatch;
    
    String compare = lastMatch ? StrPostfix(str, val.size) : StrPrefix(str, val.size);
    while (compare.size >= val.size)
    {
        if (StrCompare(compare, val, noCase))
            return compare.str - str.str;
        StringJoin sub = SubstrSplit(str, compare);
        compare = lastMatch ? StrPrefix(sub.post, val.size) : StrPostfix(sub.pre, val.size);
    }
    
    return -1;
}

function i64 StrFindArr(String str, String arr, StringFindFlags flags)
{
    for (u64 i = 0; i < str.size; ++i)
    {
        u64 result = (flags & FindStr_LastMatch) ? (str.size - i - 1) : i;
        if (ChrCompareArr(str.str[result], arr, flags & FindStr_NoCase))
            return result;
    }
    return -1;
}

function StringNode* StrFindList(String str, StringList* list, StringFindFlags flags)
{
    for (StringNode* node = list->first; node; node = node->next)
        if (StrFindStr(str, node->string, flags) > -1)
            return node;
    return 0;
}

function String StrChopAfter(String str, String arr, StringFindFlags flags)
{
    i64 index = StrFindArr(str, arr, flags);
    return StrPrefix(str, index < 0 ? str.size : index);
}

function String StrSkipUntil(String str, String arr, StringFindFlags flags)
{
    i64 index = StrFindArr(str, arr, flags);
    return StrSkip(str, index + 1);
}

//- NOTE(long): Mutable Functions
function String StrWriteToStr(String src, u64 srcOffset, String dst, u64 dstOffset)
{
    String result = {0};
    if (srcOffset + src.size <= dstOffset + dst.size)
    {
        u8* ptr = dst.str + dstOffset;
        CopyMem(ptr, src.str + srcOffset, src.size);
        result = Str(ptr, src.size);
    }
    return result;
}

function void StrToLower(String str)
{
    for (u64 i = 0; i < str.size; ++i)
        if (IsCharacter(str.str[i]))
            str.str[i] = ChrToLower(str.str[i]);
}

function void StrToUpper(String str)
{
    for (u64 i = 0; i < str.size; ++i)
        if (IsCharacter(str.str[i]))
            str.str[i] = ChrToUpper(str.str[i]);
}

function void StrSwapChr(String str, char o, char n)
{
    for (u64 i = 0; i < str.size; ++i)
        if (str.str[i] == o)
            str.str[i] = n;
}

//- NOTE(long): Unicode Functions
function StringDecode StrDecodeUTF8(u8* str, u32 cap)
{
    local u8 length[] = {
        1, 1, 1, 1, // 000xx
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        0, 0, 0, 0, // 100xx
        0, 0, 0, 0,
        2, 2, 2, 2, // 110xx
        3, 3,       // 1110x
        4,          // 11110
        0,          // 11111
    };
    local u8 firstByteMask[] = { 0, 0x7F, 0x1F, 0x0F, 0x07 };
    local u8 finalShift[] = { 0, 18, 12, 6, 0 };
    
    StringDecode result = {0};
    if (cap > 0)
    {
        result.codepoint = '#';
        result.size = 1;
        
        u8 byte = str[0];
        u8 l = length[byte >> 3];
        if (0 < l && l <= cap)
        {
            u32 cp = (byte & firstByteMask[l]) << 18;
            switch (l)
            {
                case 4: cp |= ((str[3] & 0x3F) <<  0);
                case 3: cp |= ((str[2] & 0x3F) <<  6);
                case 2: cp |= ((str[1] & 0x3F) << 12);
                default: break;
            }
            cp >>= finalShift[l];
            
            result = (StringDecode){cp, l};
        }
    }
    
    return result;
}

function StringDecode StrDecodeWide(u16* str, u32 cap)
{
    StringDecode result = {'#', 1};
    
    u16 x = str[0];
    if (x < 0xD800 || 0xDFFF < x) result.codepoint = x;
    else if (cap >= 2)
    {
        u16 y = str[1];
        if (0xD800 <= x && x < 0xDC00 &&
            0xDC00 <= y && y < 0xE000)
        {
            u16 xj = x - 0xD800;
            u16 yj = y - 0xDc00;
            u32 xy = (xj << 10) | yj;
            result = (StringDecode){ xy + 0x10000, 2 };
        }
    }
    
    return result;
}

function u32 StrEncodeUTF8(u8* dst, u32 codepoint)
{
    u32 size = 0;
    if (codepoint < (1 << 7))
    {
        dst[0] = codepoint;
        size = 1;
    }
    
    else if (codepoint < (1 << 11))
    {
        dst[0] = 0xC0 | (codepoint >> 6);
        dst[1] = 0x80 | (codepoint & 0x3F);
        size = 2;
    }
    
    else if (codepoint < (1 << 16))
    {
        dst[0] = 0xE0 | (codepoint >> 12);
        dst[1] = 0x80 | ((codepoint >> 6) & 0x3F);
        dst[2] = 0x80 | (codepoint & 0x3F);
        size = 3;
    }
    
    else if (codepoint < (1 << 21))
    {
        dst[0] = 0xF0 | (codepoint >> 18);
        dst[1] = 0x80 | ((codepoint >> 12) & 0x3F);
        dst[2] = 0x80 | ((codepoint >> 6) & 0x3F);
        dst[3] = 0x80 | (codepoint & 0x3F);
        size = 4;
    }
    
    else
    {
        dst[0] = '#';
        size = 1;
    }
    return size;
}

function u32 StrEncodeWide(u16* dst, u32 codepoint)
{
    u32 size = 0;
    if (codepoint < 0x10000)
    {
        dst[0] = codepoint;
        size = 1;
    }
    else
    {
        u32 cpj = codepoint - 0x10000;
        dst[0] = (cpj >>   10) + 0xD800;
        dst[1] = (cpj & 0x3FF) + 0xDC00;
        size = 2;
    }
    return size;
}

function String32 Str8ToStr32(Arena* arena, String str)
{
    u64 expectedSize = str.size;
    u32* memory = PushArray(arena, u32, expectedSize + 1);
    
    u32* dptr = memory;
    Str8Stream(str, ptr, opl)
    {
        StringDecode decode = StrDecodeUTF8(ptr, (u64)(opl - ptr));
        if (!decode.size)
            return (String32){0};
        
        *dptr = decode.codepoint;
        ptr += decode.size;
        dptr += 1;
    }
    *dptr = 0;
    
    u64 size = (u64)(dptr - memory);
    ArenaPopAmount(arena, (expectedSize - size) * sizeof(*memory));
    return (String32){ memory, size };
}

function String16 Str8ToStr16(Arena* arena, String str)
{
    u64 expectedSize = str.size;
    u16* memory = PushArray(arena, u16, expectedSize + 1);
    
    u16* dptr = memory;
    Str8Stream(str, ptr, opl)
    {
        StringDecode decode = StrDecodeUTF8(ptr, (u64)(opl - ptr));
        if (!decode.size)
            return (String16){0};
        
        u32 encSize = StrEncodeWide(dptr, decode.codepoint);
        *dptr = decode.codepoint;
        ptr += decode.size;
        dptr += encSize;
    }
    *dptr = 0;
    
    u64 size = (u64)(dptr - memory);
    ArenaPopAmount(arena, (expectedSize - size) * sizeof(*memory));
    return (String16){ memory, size };
}

function String StrFromStr32(Arena* arena, String32 str)
{
    u64 expectedSize = str.size*4;
    u8* memory = PushArray(arena, u8, expectedSize + 1);
    
    u8* dptr = memory;
    Str32Stream(str, ptr, opl)
    {
        u32 encSize = StrEncodeUTF8(dptr, *ptr);
        ptr += 1;
        dptr += encSize;
    }
    *dptr = 0;
    
    u64 size = (u64)(dptr - memory);
    ArenaPopAmount(arena, (expectedSize - size) * sizeof(*memory));
    return(String){ memory, size };
}

function String StrFromStr16(Arena* arena, String16 str)
{
    u64 expectedSize = str.size*2;
    u8* memory = PushArray(arena, u8, expectedSize + 1);
    
    u8* dptr = memory;
    Str16Stream(str, ptr, opl)
    {
        StringDecode decode = StrDecodeWide(ptr, (u64)(opl - ptr));
        u16 encSize = StrEncodeUTF8(dptr, decode.codepoint);
        ptr += 1;
        dptr += encSize;
    }
    *dptr = 0;
    
    u64 size = (u64)(dptr - memory);
    ArenaPopAmount(arena, (expectedSize - size) * sizeof(*memory));
    return(String){ memory, size };
}

function u64 UTF8Len(String str)
{
    u64 result = 0;
    Str8Stream(str, ptr, opl)
    {
        StringDecode decode = StrDecodeUTF8(ptr, (u64)(opl - ptr));
        if (!decode.size)
            return 0;
        
        ptr += decode.size;
        result += 1;
    }
    return result;
}

function b32 UTF8Valid(String str)
{
    Str8Stream(str, ptr, opl)
    {
        StringDecode decode = StrDecodeUTF8(ptr, (u64)(opl - ptr));
        if (!decode.size)
            return 0;
        
        ptr += decode.size;
    }
    return 1;
}

//- NOTE(long): Convert Functions
function f64 F64FromStr(String str, b32* error)
{
    
}

function f32 StrToF32(String str, b32* error)
{
    String numberStr = StrSkipUntil(str, StrLit(".e"), FindStr_NoCase);
    if (numberStr.size != str.size)
        numberStr = StrChop(SubstrSplit(str, numberStr).pre, 1);
    String fracStr = StrSkip(StrChopAfter(SubstrSplit(str, numberStr).post, StrLit("e"), FindStr_NoCase|FindStr_LastMatch), 1);
    String expStr = StrSkip(str, numberStr.size + fracStr.size + 1);
    
    i64 number = StrToI64(numberStr, error);
    
    i64 exp = StrToI64(expStr, error);
    f64 e = 1;
    for (u64 i = 0; i < AbsI32(exp); ++i)
        e *= 10;
    if (exp < 0)
        e = 1. / e;
    
    f32 coefficient = 1;
    f32 fractional = 0;
    for (u64 i = 0; i < fracStr.size; ++i)
    {
        if (!ChrCompareArr(fracStr.str[i], StrLit(Digits), 0))
        {
            if (error)
                *error = true;
            break;
        }
        
        coefficient *= .1f;
        fractional += (fracStr.str[i] - '0') * coefficient;
    }
    f32 result = (number + fractional * GetUnsigned(number)) * e;
    return result;
}

function f64 StrToF64(String str, b32* error)
{
    // TODO(long)
}

function i64 I64FromStr(String str, u32 radix, b32* error)
{
    const u8 integer_symbol_reverse[256] = {
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    };
    
    b32 _err_;
    if (!error) error = &_err_;
    *error = 1;
    
    i64 x = 0;
    if (str.size)
    {
        b32 negative =  str.str[0] == '-';
        if (negative || str.str[0] == '+')
            str = StrSkip(str, 1);
        
        if (radix >= 2 && radix <= 16)
        {
            for (u64 i = 0; i < str.size; i += 1)
            {
                u8 symbol = integer_symbol_reverse[ChrToUpper(str.str[i])];
                if (symbol >= radix)
                    goto END;
                x = x * radix + symbol;
            }
        }
        
        if (negative)
            x = -x;
        *error = 0;
    }
    
    END:
    return x;
}

function i64 StrToI64(String str, b32* error)
{
    if (str.str == 0)
        return 0;
    
    i64 result = 0;
    i64 sign = 1;
    StrTrimWspace(str);
    if (str.str[0] == '-')
        sign = -1;
    
    str = StrSkip(str, StrFindArr(str, StrLit(Digits), 0));
    String digits = StrLit(Digits);
    if (str.size >= 2)
    {
        if (str.str[0] == '0')
        {
            digits = StrLit(OctalDigits);
            str = StrSkip(str, 1);
            if (ChrCompareNoCase(str.str[0], 'x'))
            {
                digits = StrLit(HexadecimalDigits);
                str = StrSkip(str, 1);
            }
            else if (ChrCompareNoCase(str.str[0], 'b'))
            {
                // TODO(long): Implement binary
                digits = StrLit(Binary);
                str = StrSkip(str, 1);
            }
        }
    }
    
    for (u64 i = 0; i < str.size; ++i)
    {
        if (!ChrCompareArr(str.str[i], digits, 1))
        {
            if (error)
                *error = true;
            break;
        }
        
        result *= digits.size;
        char c = str.str[i];
        {
            if (ChrIsLower(c))
                c = ChrToUpper(str.str[i]);
            
            if (c >= 'A')
                c = c - 'A' + '9' + 1;
        }
        result += c - '0';
    }
    return result * sign;
}

String StrFromF64(Arena* arena, f64 x, u32 prec)
{
    if (x == 0)
        return StrCopy(arena, StrLit("0"));
    
    i64 v, e;
    {
        i32 exp;
        f64 fr = FrExp_f64(x, &exp);
        v = (i64)(fr * (1LL<<53LL));
        e = exp - 53;
    }
    
    BeginScratch(scratch, arena);
    String result = {0};
    String realStr = StrFromI64(scratch, v, 10);
    if (realStr.size)
    {
        b32 negative = x < 0;
        String str = negative ? StrSkip(realStr, 1) : realStr;
        u64 n = str.size;
        
        for (; e > 0; --e)
        {
            u8 phi = str.str[0] >= '5' ? 1 : 0, x = 0;
            
            for (i64 i = n-1; i >= 0; --i)
            {
                x += 2*(str.str[i] - '0');
                str.str[i+phi] = x%10 + '0';
                x /= 10;
            }
            
            if (phi)
            {
                str.str[0] = '1';
                n++;
            }
        }
        u64 dp = n;
        
        for (; e < 0; e++)
        {
            if (str.str[n-1] % 2 != 0)
            {
                str.str[n] = '0';
                n++;
            }
            
            u8 phi = 0, x = 0;
            if (str.str[0] < '2')
            {
                phi = 1;
                x = str.str[0] - '0';
                n--;
                dp--;
            }
            
            for (i64 i = 0; i < n; ++i)
            {
                x = x*10 + str.str[i+phi] - '0';
                str.str[i] = x/2 + '0';
                x %= 2;
            }
        }
        
        str.size = n;
        if (prec > 0)
        {
            if (n > prec)
            {
                if ( str.str[prec]  > '5' ||
                    (str.str[prec] == '5' && (StrContainsChr(Substr(str, prec+1, n), "123456789") || str.str[prec-1]%2 == 1)))
                {
                    i64 i = prec - 1;
                    while (i >= 0 && str.str[i] == '9')
                    {
                        str.str[i] = '0';
                        i--;
                    }
                    
                    if (i < 0)
                    {
                        str.str[0] = '1';
                        dp++;
                    }
                    else str.str[i]++;
                }
                n = prec;
            }
            
            if (n < prec)
            {
                SetMem(str.str + n, '0', prec - n);
                n = prec;
            }
        }
        
        str.size = n;
        String frStr = StrSkip(str, 1);
        String deStr = StrPrefix(realStr, negative ? 2 : 1);
        result = StrPushf(arena, "%.*s.%.*se%+d", StrExpand(deStr), StrExpand(frStr), dp-1);
    }
    
    EndScratch(scratch);
    return result;
}

function String StrFromI64(Arena* arena, i64 x, u32 radix)
{
    const u8 symbols[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F', };
    String result = {0};
    
    if (radix >= 2 && radix <= 16)
    {
        if (x)
        {
            u8 space[64];
            u64 length = 0;
            
            u64 offset = 0;
            if (x < 0)
            {
                x = -x;
                space[offset++] = '-';
            }
            
            for (u64 i = x; i > 0; i /= radix, length += 1)
                space[length+offset] = symbols[i%radix];
            
            for (u64 j = 0, i = length - 1; j < i; j += 1, i -= 1)
                Swap(u8, space[i+offset], space[j+offset]);
            
            result = StrCopy(arena, Str(space, length + offset));
        }
        else
            result = StrCopy(arena, StrLit("0"));
    }
    return result;
}

//~ NOTE(long): Errors

global Arena* permArena = {0};

typedef struct ERR_List ERR_List;
struct ERR_List
{
    ERR_List* next;
    Arena* arena;
    StringList errors;
};

typedef struct ERR_Thread ERR_Thread;
struct ERR_Thread
{
    ERR_List* stack;
    ERR_List* free;
};

threadvar Arena* errArena;
threadvar ERR_Thread errThread = {0};

function void ErrorBegin(Arena* arena)
{
    if (!errArena)
        errArena = ArenaReserve(KB(4));
    
    ERR_List* list = errThread.free;
    if (list)
        errThread.free = errThread.free->next;
    else
        list = PushStruct(errArena, ERR_List);
    *list = (ERR_List){ .arena = arena };
    SLLStackPush(errThread.stack, list);
}

function StringList ErrorEnd(void)
{
    if (!errThread.stack) return (StringList){0};
    
    ERR_List* list = errThread.stack;
    SLLStackPop (errThread.stack);
    SLLStackPush(errThread.free, list);
    
    // NOTE(long): Format each message to whatever you want
    StringList result = list->errors;
    return result;
}

function void ErrorPush(String error)
{
    ErrorPushf("%.*s", StrExpand(error));
}

function void ErrorPushf(char* fmt, ...)
{
    ERR_List* list = errThread.stack;
    if (list)
    {
        String error;
        {
            va_list args;
            va_start(args, fmt);
            error = StrPushfv(list->arena, fmt, args);
            va_end(args);
        }
        StrListPush(list->arena, &list->errors, error);
    }
}

//~ NOTE(long): Buffer Functiosn

function String
BufferInterleave(Arena *arena, void **in,
                 u64 laneCount, u64 elementSize, u64 elementCount){
    // TODO(allen): look at disassembly for real speed work
    
    // setup buffer
    String result = {0};
    result.size = laneCount*elementSize*elementCount;
    result.str = PushArray(arena, u8, result.size);
    
    // fill loop
    u8 *out_ptr = result.str;
    u64 in_off = 0;
    for (u64 i = 0; i < elementCount; i += 1, in_off += elementSize){
        u8 **in_base_ptr = (u8**)in;
        for (u64 j = 0; j < laneCount; j += 1, in_base_ptr += 1){
            CopyMem(out_ptr, *in_base_ptr + in_off, elementSize);
            out_ptr += elementSize;
        }
    }
    
    return(result);
}

function String*
BufferUninterleave(Arena *arena, void *in,
                   u64 laneCount, u64 elementSize, u64 elementCount){
    // TODO(allen): look at disassembly for real speed work
    
    // compute sizes
    u64 bytes_per_lane = elementSize*elementCount;
    u64 total_size = laneCount*bytes_per_lane;
    
    // allocate outs
    String *result = PushArray(arena, String, laneCount);
    for (u64 i = 0; i < laneCount; i += 1){
        result[i].str = PushArray(arena, u8, bytes_per_lane);
        result[i].size = bytes_per_lane;
    }
    
    // fill loop
    u8 *in_ptr = (u8*)in;
    u64 out_off = 0;
    for (u64 i = 0; i < elementCount; i += 1, out_off += elementSize){
        String *out_buffer = result;
        for (u64 j = 0; j < laneCount; j += 1, out_buffer += 1){
            CopyMem(out_buffer->str + out_off, in_ptr, elementSize);
            in_ptr += elementSize;
        }
    }
    
    return(result);
}
