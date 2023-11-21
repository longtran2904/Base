
//~ NOTE(long): Symbolic Constants

global String Compiler_names[] =
{
    ConstStr("None"),
    ConstStr("CLANG"),
    ConstStr("CL"),
    ConstStr("GCC"),
};

global String Arch_names[] =
{
    ConstStr("None"),
    ConstStr("X64"),
    ConstStr("X86"),
    ConstStr("ARM"),
    ConstStr("ARM64"),
};

global String OS_names[] =
{
    ConstStr("None"),
    ConstStr("Win"),
    ConstStr("Linux"),
    ConstStr("Mac"),
};

global String Month_names[] =
{
    ConstStr("None"),
    ConstStr("Jan"),
    ConstStr("Feb"),
    ConstStr("Mar"),
    ConstStr("Apr"),
    ConstStr("May"),
    ConstStr("Jun"),
    ConstStr("Jul"),
    ConstStr("Aug"),
    ConstStr("Sep"),
    ConstStr("Oct"),
    ConstStr("Nov"),
    ConstStr("Dec"),
};

global String Day_names[] =
{
    ConstStr("None"),
    ConstStr("Sunday"),
    ConstStr("Monday"),
    ConstStr("Tuesday"),
    ConstStr("Wednesday"),
    ConstStr("Thursday"),
    ConstStr("Friday"),
    ConstStr("Saturday"),
};

//~ NOTE(long): Math Functions

#include <math.h>

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

function i32 Abs(i32 x)
{
    return abs(x);
}

function i64 AbsI64(i64 x)
{
    return llabs(x);
}

#define GenerateFloatFunc(name, arg32, arg64, defaultValue, value32, value64) \
    function f32 Concat(name, _f32)(arg32) \
    { \
        f32 f = defaultValue; \
        u32 u = *(u32*)&f; \
        u value32; \
        return *(f32*)&u; \
    } \
    function f64 Concat(name, _f64)(arg64) \
    { \
        f64 f = defaultValue; \
        u64 u = *(u64*)&f; \
        u value64; \
        return *(f64*)&u; \
    } \

#define BindingFloatFunc(oldName, newName) \
    f32 Concat(newName, _f32)(f32 x) { return Concat(oldName, f)(x); } \
    f64 Concat(newName, _f64)(f64 x) { return oldName(x); }

FLOAT_FUNCS();
#undef GenerateFloatFunc
#undef BindingFloatFunc

function u32 GetNoise1D(i32 position, u32 seed)
{
    u32 result = (u32)position;
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
        result = GetNoise1D(result, *((u64*)values + i) >> (i % 16));
    for (u8* i = values + (count / 8) * 8; i < values + count; ++i)
        result += BIT_NOISE4 * (*i) + BIT_NOISE5;
    return result;
}

//~ NOTE(long): Time

function DateTime ToDateTime(DenseTime time)
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

function DenseTime ToDenseTime(DateTime* time)
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
StaticAssert(sizeof(MemArena) <= INITIAL_COMMIT, CheckArenaSize);

function MemArena* MakeReservedArena(u64 reserve)
{
    MemArena* result = 0;
    if (reserve >= INITIAL_COMMIT)
    {
        void* memory = ReserveMem(reserve);
        if (CommitMem(memory, INITIAL_COMMIT))
        {
            result = (MemArena*)memory;
            result->cap = reserve;
            result->pos = AlignUpPow2(sizeof(MemArena), 64);
            result->commitPos = INITIAL_COMMIT;
        }
    }
    
    Assert(result != 0);
    return result;
}

function MemArena* MakeArena(void)
{
    return MakeReservedArena(DEFAULT_RESERVE_SIZE);
}

function void ReleaseArena(MemArena* arena)
{
    ReleaseMem(arena);
}

function void* PushArena(MemArena* arena, u64 size)
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
            
            if (CommitMem(((u8*)arena) + arena->commitPos, commitSize))
                arena->commitPos = nextCommitPos;
            else
                result = 0;
        }
    }
    
    Assert(result != 0);
    return result;
}

function void PopArenaTo(MemArena* arena, u64 pos)
{
    if (pos < arena->pos)
    {
        arena->pos = pos;
        
        u64 alignedPos = AlignUpPow2(arena->pos, COMMIT_BLOCK_SIZE - 1);
        u64 nextCommitPos = ClampTop(alignedPos, arena->cap);
        
        if (nextCommitPos < arena->commitPos)
        {
            u64 decommitSize = arena->commitPos - nextCommitPos;
            DecommitMem(arena + nextCommitPos, decommitSize);
            arena->commitPos = nextCommitPos;
        }
    }
}

function void* PushZeroArena(MemArena* arena, u64 size)
{
    void* result = PushArena(arena, size);
    ZeroMem(result, size);
    return result;
}

function void  AlignArena(MemArena* arena, u64 alignment)
{
    u64 alignedPos = AlignUpPow2(arena->pos, alignment);
    u64 size = alignedPos - arena->pos;
    if (size > 0)
        PushArena(arena, size);
}

function void  AlignZeroArena(MemArena* arena, u64 aligment)
{
    u64 alignedPos = AlignUpPow2(arena->pos, aligment);
    u64 size = alignedPos - arena->pos;
    if (size > 0)
        PushZeroArena(arena, size);
}

function TempArena BeginTemp(MemArena* arena)
{
    return (TempArena){ arena, arena->pos };
}

function void EndTemp(TempArena temp)
{
    PopArenaTo(temp.arena, temp.pos);
}

#if 1
threadvar MemArena* scratchPool[SCRATCH_POOL_COUNT] = {0};
function TempArena GetScratch(MemArena** conflictArray, u32 count)
{
    MemArena** pool = scratchPool;
    if (pool[0] == 0)
    {
        MemArena** scratchSlot = pool;
        for (u64 i = 0; i < SCRATCH_POOL_COUNT; ++i, ++scratchSlot)
            *scratchSlot = MakeArena();
    }
    
    TempArena result = {0};
    MemArena** scratchSlot = pool;
    for (u64 i = 0; i < SCRATCH_POOL_COUNT; ++i, ++scratchSlot)
    {
        b32 noConflict = true;
        MemArena** conflictPtr = conflictArray;
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
            result = BeginTemp(*scratchSlot);
            break;
        }
    }
    
    return result;
}
#endif

//~ NOTE(long): String Functions

function String Str(u8* str, u64 size)
{
    return (String){ str, size };
}

function String StrRange(u8* first, u8* opl)
{
    return (String){ first, (u64)(opl - first) };
}

function String StrFromCStr(u8* cstr)
{
    u8* ptr = cstr;
    for (;*ptr != 0; ++ptr);
    return StrRange(cstr, ptr);
}

function String CopyStr(MemArena* arena, String str, b32 addNullTerminator)
{
    String result = { 0, str.size };
    result.str = PushZeroArray(arena, u8, result.size + (addNullTerminator ? 1 : 0));
    CopyMem(result.str, str.str, str.size);
    return result;
}

function String GetFlagName_(MemArena* arena, String* names, u64 nameCount, u64 flags)
{
    BeginScratch(scratch, arena);
    StringList list = {0};
    
    for (u64 i = 0; i < nameCount; ++i)
        if (flags & (1ULL << i))
            PushStrList(scratch, &list, names[i]);
    String result = JoinStr(arena, &list, &(StringJoin){ .mid = StrLit(" | ") }, true);
    
    EndScratch(scratch);
    return result;
}

#define SUB_STRING(name, ptrOffset, newSize) \
    function String name##Str(String str, u64 size) \
    { \
        u64 clampedSize = ClampTop(size, str.size); \
        u64 remainingSize = str.size - clampedSize; \
        return (String){ (str.str + ptrOffset), (newSize) }; \
    }

SUB_STRING(Prefix, 0, clampedSize)
SUB_STRING(Chop, 0, remainingSize)
SUB_STRING(Postfix, remainingSize, clampedSize)
SUB_STRING(Skip, clampedSize, remainingSize)

function String ChopStrAfter(String str, String characters, StringMatchFlags flags)
{
    u32 size = 0;
    b32 inclusive = flags & StringMatchFlag_Inclusive;
    b32 result = false;
    for (i64 i = str.size - 1; i >= 0; --i, ++size)
        if (result = CompareCharArray(str.str[i], characters, flags))
            break;
    if (!result)
    {
        size = 0;
        inclusive = true;
    }
    return ChopStr(str, size + (inclusive ? 0 : 1));
}

function String SkipStrUntil(String str, String characters, StringMatchFlags flags)
{
    u64 size;
    b32 inclusive = flags & StringMatchFlag_Inclusive;
    b32 result = false;
    for (size = 0; size < str.size; ++size)
        if (result = CompareCharArray(str.str[size], characters, flags))
            break;
    if (!result)
    {
        size = 0;
        inclusive = true;
    }
    return SkipStr(str, size + (inclusive ? 0 : 1));
}

function String PreSubStr(String str, String substr)
{
    u32 size = 0;
    if (InRange(substr.str, str.str, str.str + str.size))
        size = substr.str - str.str;
    return PrefixStr(str, size);
}

function String PostSubStr(String str, String substr)
{
    u32 size = 0;
    if (InRange(substr.str + substr.size, str.str, str.str + str.size))
        size = (str.str + str.size) - (substr.str + substr.size);
    return PostfixStr(str, size);
}

function String SubStr(String str, u64 first, u64 opl)
{
    u64 clampedOpl = ClampTop(str.size, opl);
    u64 clampedFirst = ClampTop(clampedOpl, first);
    return (String){ str.str + clampedFirst, clampedOpl - clampedFirst };
}

function String SubStrRange(String str, u64 first, u64 range)
{
    return SubStr(str, first, first + range);
}

function StringList StrList(MemArena* arena, String* strArr, u64 count)
{
    StringList result = {0};
    if (strArr != 0 && count != 0)
    {
        result.first = PushArray(arena, StringNode, count);
        result.last = result.first + count - 1;
        for (u64 i = 0; i < count; ++i)
            PushStrListExplicit(&result, strArr[i], &result.first[i]);
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
            PushStrListExplicit(&result, strs[i], nodes + i);
    }
    return result;
}

function void PushStrListExplicit(StringList* list, String str, StringNode* nodeMem)
{
    nodeMem->string = str;
    SLLQueuePush(list->first, list->last, nodeMem);
    list->nodeCount++;
    list->totalSize += str.size;
}

function void PushStrList(MemArena* arena, StringList* list, String str)
{
    StringNode* node = PushStruct(arena, StringNode);
    PushStrListExplicit(list, str, node);
}

function String JoinStr(MemArena* arena, StringList* list, StringJoin* optionalJoin, b32 terminate)
{
    local StringJoin dummy = {0};
    StringJoin* join = optionalJoin;
    if (join == 0)
        join = &dummy;
    
    u64 size = (join->pre.size +
                join->post.size +
                join->mid.size * (ClampBot(list->nodeCount , 1) - 1) +
                list->totalSize);
    
    u64 actualSize = size;
    if (terminate)
        ++actualSize;
    
    u8* str = PushZeroArray(arena, u8, actualSize);
    u8* ptr = str;
    
    CopyMem(ptr, join->pre.str, join->pre.size);
    ptr += join->pre.size;
    
    b32 isMid = false;
    for (StringNode* node = list->first; node != 0; node = node->next)
    {
        if (isMid)
        {
            CopyMem(ptr, join->mid.str, join->mid.size);
            ptr += join->mid.size;
        }
        
        CopyMem(ptr, node->string.str, node->string.size);
        ptr += node->string.size;
        
        isMid = true;
    }
    
    CopyMem(ptr, join->post.str, join->post.size);
    ptr += join->post.size;
    
    Assert(ptr == (str + size));
    
    return (String){ str, size };
}

function String JoinStr3(MemArena* arena, StringJoin* join, b32 terminate)
{
    String result = {0};
    if (join)
    {
        StringNode pre, mid, post;
        StringList list = {0};
        PushStrListExplicit(&list, join->pre, &pre);
        PushStrListExplicit(&list, join->mid, &mid);
        PushStrListExplicit(&list, join->post, &post);
        result = JoinStr(arena, &list, 0, terminate);
    }
    return result;
}

function String ReplaceStr(MemArena* arena, String str, String oldStr, String newStr, StringMatchFlags flags, b32 terminate)
{
    b32 isDelete = newStr.str == 0;
    if (isDelete)
        flags &= ~StringMatchFlag_Inclusive;
    StringList list = SplitStr(arena, str, oldStr, flags);
    if (!isDelete)
    {
        for (StringNode* node = list.first; node; node = node->next)
        {
            if (flags & StringMatchFlag_IsCharArray)
            {
                if (node->string.size == 1)
                {
                    u64 matchIndex = GetMatchIndex(node->string.str[0], newStr, flags);
                    if (matchIndex >= 0)
                        node->string = SubStrRange(newStr, matchIndex, 1);
                }
            }
            
            else if (CompareStr(node->string, oldStr, flags))
                node->string = newStr;
        }
    }
    return JoinStr(arena, &list, 0, terminate);
}

function StringList SplitStr(MemArena* arena, String str, String splits, StringMatchFlags flags)
{
    StringList result = {0};
    u8* ptr = str.str;
    u8* firstWord = ptr;
    u8* opl = str.str + str.size;
    b32 isCharArray = flags & StringMatchFlag_IsCharArray;
    u64 size = Max(isCharArray ? 1 : splits.size, 1); // NOTE(long): splits can be empty
    
    // NOTE(long): < rather than <= because firstWord = ptr + 1 can crash at the end of buffer
    for (;ptr < opl; ++ptr)
    {
        if (isCharArray ? CompareCharArray(*ptr, splits, flags) : CompareStr(StrRange(ptr, ptr + splits.size), splits, flags))
        {
            // NOTE(long): try to emit word, < rather than <= because we don't allow empty members
            // EX: split "A,B,,C" with "," -> { "A", "B", "C" }
            if (firstWord < ptr)
                PushStrList(arena, &result, StrRange(firstWord, ptr));
            if (flags & StringMatchFlag_Inclusive)
                PushStrList(arena, &result, Str(ptr, size));
            ptr += size - 1;
            firstWord = ptr + 1;
        }
    }
    
    // try to emit the final word
    if (firstWord < ptr)
        PushStrList(arena, &result, StrRange(firstWord, ptr));
    
    return result;
}

#include <stdarg.h>
#include <stdio.h>

function String PushStrfv(MemArena* arena, b32 terminate, char* fmt, va_list args)
{
    // in case we need to try a second time
    va_list args2;
    va_copy(args2, args);
    
    // try to build the string in 1024 bytes
    u64 bufferSize = 1024;
    u8* buffer = PushZeroArray(arena, u8, bufferSize);
    // NOTE(long): vsnprintf doens't count the null terminator
    u64 actualSize = vsnprintf((char*)buffer, bufferSize, fmt, args);
    u64 allocSize = actualSize + (terminate ? 1: 0);
    
    String result = {0};
    if (allocSize <= bufferSize)
    {
        // if first try worked, put back the remaining
        PopArenaAmount(arena, bufferSize - allocSize);
        result = Str(buffer, actualSize);
    }
    else
    {
        // if first try failed, reset and try again with the correct size
        PopArenaAmount(arena, bufferSize);
        u8* newBuffer = PushZeroArray(arena, u8, allocSize);
        vsnprintf((char*)newBuffer, allocSize, fmt, args2);
        result = Str(newBuffer, actualSize);
    }
    
    va_end(args2);
    return result;
}

function String PushStrf(MemArena* arena, b32 terminate, char*fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String result = PushStrfv(arena, terminate, fmt, args);
    va_end(args);
    return result;
}

function void PushStrListf(MemArena* arena, b32 terminate, StringList* list, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String string = PushStrfv(arena, terminate, fmt, args);
    va_end(args);
    PushStrList(arena, list, string);
}

function b32 CompareChar(char a, char b, StringMatchFlags flags)
{
    b32 result = flags & StringMatchFlag_NotEqual;
    if (a == b)
        result = !result;
    else if ((flags & StringMatchFlag_NoCase) && CompareCharNoCase(a, b))
        result = !result;
    return result;
}

function b32 CompareCharArray(char c, String characters, StringMatchFlags flags)
{
    b32 result = flags & StringMatchFlag_NotEqual;
    for (u64 i = 0; i < characters.size; ++i)
        if (CompareChar(c, characters.str[i], flags & ~StringMatchFlag_NotEqual))
            return !result;
    return result;
}

function u64 GetMatchIndex(char c, String characters, StringMatchFlags flags)
{
    for (u64 i = 0; i < characters.size; ++i)
        if (CompareChar(c, characters.str[i], flags))
            return i;
    return -1;
}

function b32 CompareStr(String a, String b, StringMatchFlags flags)
{
    b32 result = false;
    
    if (a.str == 0 || b.str == 0)
        result = (a.size == b.size) == (flags & ~StringMatchFlag_NotEqual);
    else if (flags & StringMatchFlag_IsCharArray)
    {
        result = flags & StringMatchFlag_NotEqual;
        for (u64 i = 0; i < a.size; ++i)
            if (CompareCharArray(a.str[i], b, flags & ~StringMatchFlag_NotEqual))
                return !result;
    }
    else if (a.size == b.size)
    {
        result = true;
        for (u64 i = 0; i < a.size; ++i)
            if (!CompareChar(a.str[i], b.str[i], flags))
                return false;
    }
    
    return result;
}

function b32 CompareStrList(String str, StringList* matches, StringMatchFlags flags)
{
    b32 result = flags & StringMatchFlag_NotEqual;
    for (StringNode* node = matches->first; node; node = node->next)
        if (CompareStr(str, node->string, flags & ~StringMatchFlag_NotEqual))
            return !result;
    return result;
}

function b32 AtEndOfStr(String str, u8* opl)
{
    return opl == (str.str + str.size);
}

function b32 InsideStr(String str, u8* ptr)
{
    return (ptr <= (str.str + str.size)) && (ptr >= str.str);
}

//~ NOTE(long): String Convert Functions

function f32 StrToF32(String str, b32* error)
{
    String numberStr = SkipStrUntil(str, StrLit(".e"), StringMatchFlag_NoCase);
    if (numberStr.size != str.size)
        numberStr = ChopStr(PreSubStr(str, numberStr), 1);
    String fracStr = SkipStr(ChopStrAfter(PostSubStr(str, numberStr), StrLit("e"), StringMatchFlag_NoCase), 1);
    String expStr = SkipStr(str, numberStr.size + fracStr.size + 1);
    
    i64 number = StrToI64(numberStr, error);
    
    i64 exp = StrToI64(expStr, error);
    f64 e = 1;
    for (u64 i = 0; i < Abs(exp); ++i)
        e *= 10;
    if (exp < 0)
        e = 1. / e;
    
    f32 coefficient = 1;
    f32 fractional = 0;
    StringIter(fracStr, i)
    {
        if (CompareCharArray(fracStr.str[i], StrLit(Digits), StringMatchFlag_NotEqual))
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

function f64 StrToF64(String str)
{
    // TODO(long)
}

function i64 StrToI64(String str, b32* error)
{
    if (str.str == 0)
        return 0;
    
    i64 result = 0;
    i64 sign = 1;
    str = SkipStrUntil(str, StrLit(WspaceStr), StringMatchFlag_Inclusive|StringMatchFlag_NotEqual);
    if (str.str[0] == '-')
        sign = -1;
    
    str = SkipStrUntil(str, StrLit(Digits), StringMatchFlag_Inclusive);
    String digits = StrLit(Digits);
    if (str.size >= 2)
    {
        if (str.str[0] == '0')
        {
            digits = StrLit(OctalDigits);
            str = SkipStr(str, 1);
            if (CompareCharNoCase(str.str[0], 'x'))
            {
                digits = StrLit(HexadecimalDigits);
                str = SkipStr(str, 1);
            }
            else if (CompareCharNoCase(str.str[0], 'b'))
            {
                // TODO(long): Implement binary
                digits = StrLit(Binary);
                str = SkipStr(str, 1);
            }
        }
    }
    
    StringIter(str, i)
    {
        if (CompareCharArray(str.str[i], digits, StringMatchFlag_NotEqual|StringMatchFlag_NoCase))
        {
            if (error)
                *error = true;
            break;
        }
        result *= digits.size;
        char c = UpperCase(str.str[i]);
        if (c >= 'A')
            c = c - 'A' + '9' + 1;
        result += c - '0';
    }
    return result * sign;
}

//~ NOTE(long): Errors

threadvar String errorStr = {0};
global MemArena* permArena = {0};

function b32 HasError(void)
{
    return errorStr.size != 0 && errorStr.str != 0;
}

function String GetError(void)
{
    return errorStr;
}

function void SetError(String error)
{
	// TODO(long): Either free the previous error or add it to a string list
    errorStr = error;
}

function void SetErrorf(char* format, ...)
{
    va_list args;
    va_start(args, format);
	errorStr = PushStrfv(permArena, false, format, args);
    va_end(args);
}

//~ TODO(long): Unicode Functions

#if 0
function StringDecode DecodeUTF8Str(u8* str, u32 cap)
{
    local u8 utf8ClassTable[] =
    {
        0, 0, 0, 0, // 000xx
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        1, 1, 1, 1, // 100xx
        1, 1, 1, 1,
        2, 2, 2, 2, // 110xx
        3, 3,       // 1110x
        4,          // 11110
        5,          // 11111
    };
    
    StringDecode result = {'#', 1};
    if (cap > 0)
    {
        u8 byte = *str;
        u8 utf8Class = utf8ClassTable[byte >> 3];
        switch (utf8Class)
        {
            case 0:
            {
                result.codepoint = byte;
                result.size = 1;
            } break;
            
            case 2:
            {
                if (cap >= 2 && utf8ClassTable[str[1] >> 3] == 1)
                {
                    result.codepoint = ((byte & 0x1F) << 6) | (str[1] & 0x3F);
                    result.size = 2;
                }
            } break;
            
            case 3:
            {
                if (cap)
            }
        }
    }
}
#endif