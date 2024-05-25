#include "DefaultMemory.h"
#include "Base.h"
#include "LongOS.h"
#include "Base.c"
#include "LongOS_Win32.c"
#include <stdio.h>

typedef struct TestCtx TestCtx;
struct TestCtx
{
    i32 testCount;
    i32 passCount;
};
TestCtx ctx;

global i32 snapshot;

void TestLogCallback(Arena* arena, Record* record, char* fmt, va_list args)
{
    ScratchBlock(scratch, arena)
    {
        fmt = (char*)StrPushf(scratch, "%s, snapshot: %d", fmt, snapshot++).str;
        LogFmtStd(arena, record, fmt, args);
    }
    
    ScratchBlock(scratch, arena)
    {
        record->log = StrPushf(scratch, "%.*s, arena: %llu", StrExpand(record->log), arena->pos);
        record->log = StrCopy(arena, record->log);
    }
}

void IncSnapshot(void)
{
    LogPush(LOG_TRACE, "Snapshot: %d", snapshot);
    snapshot++;
}

function void TestBegin(char* name)
{
    String str = StrFromCStr(name);
    i32 spaces = ClampBot(25 - (i32)str.size, 0);
    printf("\"%.*s\"%.*s [", StrExpand(str), spaces, " ------------------------------");
    
    ctx.testCount = 0;
    ctx.passCount = 0;
}

function void TestResult(b32 result)
{
    ctx.testCount++;
    ctx.passCount += !!result;
    printf(result ? "." : "X");
    
    Assert(result);
}

function void TestEnd()
{
    i32 spaces = ClampBot(40 - ctx.testCount, 0);
    
    printf("]%.*s ", spaces, "                                                                                ");
    printf("[%2i/%-2i] %2i passed, %2i tests, ", ctx.passCount, ctx.testCount, ctx.passCount, ctx.testCount);
    printf(ctx.testCount == ctx.passCount ? "SUCCESS ( )\n" : "FAILED (X)\n");
}

#define TEST(name) DeferBlock(TestBegin(name), TestEnd())

int main(void)
{
    i32 space = Max(sizeof(CURRENT_COMPILER_NAME), Max(sizeof(CURRENT_OS_NAME), sizeof(CURRENT_ARCH_NAME))) - 1;
    printf(  "------------------------------CONTEXT------------------------------\n");
    printf("-----[COM:  %*s]-----\n", space, CURRENT_COMPILER_NAME);
    printf("-----[OS:   %*s]-----\n", space, CURRENT_OS_NAME);
    printf("-----[ARCH: %*s]-----\n", space, CURRENT_ARCH_NAME);
    printf("\n------------------------------TESTING------------------------------\n");
    
    TEST("Context")
    {
        TestResult(ArrayCount(Day_names) == 8);
        TestResult(StrCompare(GetEnumStr(Day, -1), StrLit("Invalid"), 0));
        TestResult(StrCompare(GetEnumStr(Arch, 0), StrLit("None"), 0));
        TestResult(StrCompare(GetEnumStr(Arch, Arch_ARM), StrLit("ARM"), 0));
        TestResult(StrCompare(GetEnumStr(Compiler, CURRENT_COMPILER_NUMBER), StrLit(CURRENT_COMPILER_NAME), 0));
        TestResult(StrCompare(GetEnumStr(OS,             CURRENT_OS_NUMBER), StrLit(      CURRENT_OS_NAME), 0));
        TestResult(StrCompare(GetEnumStr(Arch,         CURRENT_ARCH_NUMBER), StrLit(    CURRENT_ARCH_NAME), 0));
        
        i32 foo[100];
        for (i32 i = 0; i < ArrayCount(foo); ++i)
            foo[i] = i;
        TestResult(ArrayCount(foo) == 100);
        
        i32 bar[100];
        CopyFixedArr(bar, foo);
        TestResult(bar[0] == 0 && bar[50] == 50 && bar[99] == 99);
        TestResult(CompareMem(foo, bar, sizeof(foo)));
        
        typedef struct TestStruct TestStruct;
        struct TestStruct
        {
            i32 a, b, c, d;
        };
        
        TestResult(OffsetOf(TestStruct, a) ==  0);
        TestResult(OffsetOf(TestStruct, b) ==  4);
        TestResult(OffsetOf(TestStruct, c) ==  8);
        TestResult(OffsetOf(TestStruct, d) == 12);
        
        TestStruct t = { 1, 2, 3, 4 };
        TestResult(t.a == 1 && t.b == 2 && t.c == 3 && t.d == 4);
        ZeroStruct(&t);
        TestResult(t.a == 0 && t.b == 0 && t.c == 0 && t.d == 0);
        
        TestResult(ChrIsUpper('A') == 1);
        TestResult(ChrIsUpper('c') == 0);
        TestResult(ChrIsUpper(';') == 0);
        TestResult(ChrIsLower('d') == 1);
        TestResult(ChrIsLower('E') == 0);
        TestResult(ChrIsLower('0') == 0);
        
        TestResult(ChrToUpper('A') == 'A');
        TestResult(ChrToUpper('d') == 'D');
        TestResult(ChrToUpper('@') == '@');
        TestResult(ChrToUpper('^') == '^');
        TestResult(ChrToLower('a') == 'a');
        TestResult(ChrToLower('D') == 'd');
        TestResult(ChrToLower('2') == '2');
        TestResult(ChrToLower('`') == '`');
    }
    
    TEST("Helper")
    {
        TestResult(Min(  1, 100) ==   1);
        TestResult(Min(100,  20) ==  20);
        TestResult(Min(-50, -20) == -50);
        TestResult(Max(  1,  40) ==  40);
        TestResult(Max(200,  10) == 200);
        TestResult(Max(-50,  10) ==  10);
        
        TestResult(Clamp( 1, 10, 50) == 10);
        TestResult(Clamp(80,  0,  7) ==  7);
        TestResult(Clamp(20,  5, 30) == 20);
        
        TestResult(  Lerp( .75f, 1.f, 10.f) == 7.75f);
        TestResult(UnLerp(7.75f, 1.f, 10.f) ==  .75f);
        
        i32 a = 10, b = 5;
        Swap(i32, a, b);
        TestResult(a == 5 && b == 10);
        
        TestResult(GetSign(-2) == -1);
        TestResult(GetSign(+2) == +1);
        TestResult(GetSign( 0) ==  0);
        TestResult(GetUnsigned( 0) == +1);
        TestResult(GetUnsigned(-1) == -1);
        
        TestResult(AlignUpPow2(64, 32) == 64);
        TestResult(AlignUpPow2(15, 64) == 64);
        TestResult(AlignUpPow2(84, 32) == 96);
        
        TestResult(AlignDownPow2(64, 32) == 64);
        TestResult(AlignDownPow2(72, 32) == 64);
        TestResult(AlignDownPow2(72, 64) == 64);
        
        TestResult(IsPow2OrZero(32) == 1);
        TestResult(IsPow2OrZero(64) == 1);
        TestResult(IsPow2OrZero( 5) == 0);
        TestResult(IsPow2OrZero( 0) == 1);
        TestResult(IsPow2OrZero(KB( 4)) == 1);
        TestResult(IsPow2OrZero(KB(10)) == 0);
    }
    
    TEST("Float")
    {
        TestResult(AbsI32(-3506708) == 3506708);
        TestResult(AbsI64(MIN_I64 + MAX_I32) == 9223372034707292161);
        
        TestResult(Inf_f32() == -NegInf_f32());
        TestResult(Abs_f32(-2.5f) == 2.5f);
        TestResult(Abs_f32(-Inf_f32()) == Inf_f32());
        
        TestResult(Inf_f64() == -NegInf_f64());
        TestResult(Abs_f64(-357.39460) == 357.39460);
        TestResult(Abs_f64(NegInf_f64()) == Inf_f64());
    }
    
#define IsNanF32(x) (InfOrNan_f32(x) && (x) != Inf_f32() && (x) != NegInf_f32())
#define IsNanF64(x) (InfOrNan_f64(x) && (x) != Inf_f64() && (x) != NegInf_f64())
    
    TEST("Round F32")
    {
        f32 zero_f32 = 0.f;
        
        TestResult(Trunc_f32(+2.0f) == +2.0f);
        TestResult(Trunc_f32(+2.3f) == +2.0f);
        TestResult(Trunc_f32(+2.5f) == +2.0f);
        TestResult(Trunc_f32(+2.6f) == +2.0f);
        TestResult(Trunc_f32(-2.0f) == -2.0f);
        TestResult(Trunc_f32(-2.3f) == -2.0f);
        TestResult(Trunc_f32(-2.5f) == -2.0f);
        TestResult(Trunc_f32(-2.6f) == -2.0f);
        
        TestResult(Trunc_f32(+1.f/zero_f32) == Inf_f32() && Trunc_f32(-1.f/zero_f32) == NegInf_f32());
        TestResult(IsNanF32(Trunc_f32(0.f/zero_f32)));
        
        TestResult(Floor_f32(+2.0f) == +2.0f);
        TestResult(Floor_f32(+2.3f) == +2.0f);
        TestResult(Floor_f32(+2.5f) == +2.0f);
        TestResult(Floor_f32(+2.6f) == +2.0f);
        TestResult(Floor_f32(-2.0f) == -2.0f);
        TestResult(Floor_f32(-2.3f) == -3.0f);
        TestResult(Floor_f32(-2.5f) == -3.0f);
        TestResult(Floor_f32(-2.6f) == -3.0f);
        
        TestResult(Floor_f32(+1.f/zero_f32) == Inf_f32() && Floor_f32(-1.f/zero_f32) == NegInf_f32());
        TestResult(IsNanF32(Floor_f32(0.f/zero_f32)));
        
        TestResult(Ceil_f32(+2.0f) == +2.0f);
        TestResult(Ceil_f32(+2.3f) == +3.0f);
        TestResult(Ceil_f32(+2.5f) == +3.0f);
        TestResult(Ceil_f32(+2.6f) == +3.0f);
        TestResult(Ceil_f32(-2.0f) == -2.0f);
        TestResult(Ceil_f32(-2.3f) == -2.0f);
        TestResult(Ceil_f32(-2.5f) == -2.0f);
        TestResult(Ceil_f32(-2.6f) == -2.0f);
        
        TestResult(Ceil_f32(+1.f/zero_f32) == Inf_f32() && Ceil_f32(-1.f/zero_f32) == NegInf_f32());
        TestResult(IsNanF32(Ceil_f32(0.f/zero_f32)));
        
        TestResult(Round_f32(+2.0f) == +2.0f);
        TestResult(Round_f32(+2.3f) == +2.0f);
        TestResult(Round_f32(+2.5f) == +2.0f);
        TestResult(Round_f32(-2.0f) == -2.0f);
        TestResult(Round_f32(-2.3f) == -2.0f);
        TestResult(Round_f32(-2.5f) == -2.0f);
        TestResult(Round_f32(+2.6f) == +3.0f);
        TestResult(Round_f32(-2.6f) == -3.0f);
        
        TestResult(Round_f32(+1.f/zero_f32) == Inf_f32() && Round_f32(-1.f/zero_f32) == NegInf_f32());
        TestResult(IsNanF32(Round_f32(0.f/zero_f32)));
    }
    
    TEST("Round F64")
    {
        f64 zero_f64 = 0.;
        
        TestResult(Trunc_f64(+2.0) == +2.0);
        TestResult(Trunc_f64(+2.3) == +2.0);
        TestResult(Trunc_f64(+2.5) == +2.0);
        TestResult(Trunc_f64(+2.6) == +2.0);
        TestResult(Trunc_f64(-2.0) == -2.0);
        TestResult(Trunc_f64(-2.3) == -2.0);
        TestResult(Trunc_f64(-2.5) == -2.0);
        TestResult(Trunc_f64(-2.6) == -2.0);
        
        TestResult(Trunc_f64(+1./zero_f64) == Inf_f64() && Trunc_f64(-1./zero_f64) == NegInf_f64());
        TestResult(IsNanF64(Trunc_f64(0./zero_f64)));
        
        TestResult(Floor_f64(+2.0) == +2.0);
        TestResult(Floor_f64(+2.3) == +2.0);
        TestResult(Floor_f64(+2.5) == +2.0);
        TestResult(Floor_f64(+2.6) == +2.0);
        TestResult(Floor_f64(-2.0) == -2.0);
        TestResult(Floor_f64(-2.3) == -3.0);
        TestResult(Floor_f64(-2.5) == -3.0);
        TestResult(Floor_f64(-2.6) == -3.0);
        
        TestResult(Floor_f64(+1./zero_f64) == Inf_f64() && Floor_f64(-1./zero_f64) == NegInf_f64());
        TestResult(IsNanF64(Floor_f64(0./zero_f64)));
        
        TestResult(Ceil_f64(+2.0) == +2.0);
        TestResult(Ceil_f64(+2.3) == +3.0);
        TestResult(Ceil_f64(+2.5) == +3.0);
        TestResult(Ceil_f64(+2.6) == +3.0);
        TestResult(Ceil_f64(-2.0) == -2.0);
        TestResult(Ceil_f64(-2.3) == -2.0);
        TestResult(Ceil_f64(-2.5) == -2.0);
        TestResult(Ceil_f64(-2.6) == -2.0);
        
        TestResult(Ceil_f64(+1./zero_f64) == Inf_f64() && Ceil_f64(-1./zero_f64) == NegInf_f64());
        TestResult(IsNanF64(Ceil_f64(0./zero_f64)));
        
        TestResult(Round_f64(+2.0f) == +2.0f);
        TestResult(Round_f64(+2.3f) == +2.0f);
        TestResult(Round_f64(+2.5f) == +2.0f);
        TestResult(Round_f64(-2.0f) == -2.0f);
        TestResult(Round_f64(-2.3f) == -2.0f);
        TestResult(Round_f64(-2.5f) == -2.0f);
        TestResult(Round_f64(+2.6f) == +3.0f);
        TestResult(Round_f64(-2.6f) == -3.0f);
        
        TestResult(Round_f64(+1.f/zero_f64) == Inf_f64() && Round_f64(-1.f/zero_f64) == NegInf_f64());
        TestResult(IsNanF64(Round_f64(0.f/zero_f64)));
    }
    
    TEST("Numeric F32")
    {
        TestResult(Sqrt_f32( +4.0f) ==  2.f);
        TestResult(Sqrt_f32(1369.f) == 37.f);
        TestResult(Sqrt_f32( +.25f) ==  .5f);
        TestResult(Sqrt_f32( +2.0f) ==  1.4142135f);
        TestResult(IsNanF32(Sqrt_f32( -1.0f)));
        
        f32 epsilon = 0.0005f;
        f32 debug = 0;
        
        TestResult((debug = Abs_f32(RSqrt_f32( +4.0f) - 1.f /  2.f)) <= epsilon);
        TestResult((debug = Abs_f32(RSqrt_f32(1369.f) - 1.f / 37.f)) <= epsilon);
        TestResult((debug = Abs_f32(RSqrt_f32( +.25f) - 1.f /  .5f)) <= epsilon);
        TestResult(IsNanF32(RSqrt_f32( -1.0f)));
        
        TestResult((debug = Abs_f32(Ln_f32(E_F32) - 1.0f)) <= epsilon);
        TestResult((debug = Abs_f32(Ln_f32(+2.0f) - .69314718f)) <= epsilon);
        TestResult(IsNanF32(Ln_f32(-1.0f)));
        
        TestResult(Pow_f32(10.f,  9.0f) == (f32)Billion(1));
        TestResult(Pow_f32(2.0f, 12.0f) == (f32)KB(4));
        TestResult(Pow_f32(2.0f, -1.0f) == 0.5f);
        TestResult(Pow_f32(0.0f, 10.0f) == 0.0f);
        TestResult(Pow_f32(1.0f, 10.0f) == 1.0f);
        TestResult(Pow_f32(3.0f,  0.0f) == 1.0f);
        TestResult(Pow_f32(0.0f,  0.0f) == 1.0f);
        
        i32 exp = 0;
        TestResult(FrExp_f32(8.0f, &exp) == 0.5f && exp == +4);
        TestResult(FrExp_f32(-.5f, &exp) == -.5f && exp ==  0);
        TestResult(FrExp_f32(1.0f, &exp) == 0.5f && exp == +1);
        TestResult(FrExp_f32(.25f, &exp) == 0.5f && exp == -1);
        TestResult(FrExp_f32(0x1p23f, &exp) == 0.5f && exp == 24);
    }
    
    TEST("Numeric F64")
    {
        TestResult(Sqrt_f64( +4.0) ==  2.);
        TestResult(Sqrt_f64(1369.) == 37.);
        TestResult(Sqrt_f64( +.25) ==  .5);
        TestResult(Sqrt_f64( +2.0) ==  1.41421356237309504);
        TestResult(IsNanF64(Sqrt_f64( -1.0)));
        
        f64 epsilon = 0.0005;
        f64 debug = 0;
        
        TestResult((debug = Abs_f64(Ln_f64(E_F64) - 1.0)) <= epsilon);
        TestResult((debug = Abs_f64(Ln_f64( +2.0) - .69314718)) <= epsilon);
        TestResult(IsNanF64(Ln_f64(-1.0)));
        
        TestResult(Pow_f64(10.,  9.0) == (f64)Billion(1));
        TestResult(Pow_f64(2.0, 12.0) == (f64)KB(4));
        TestResult(Pow_f64(2.0, -1.0) == 0.5);
        TestResult(Pow_f64(0.0, 10.0) == 0.0);
        TestResult(Pow_f64(1.0, 10.0) == 1.0);
        TestResult(Pow_f64(3.0,  0.0) == 1.0);
        TestResult(Pow_f64(0.0,  0.0) == 1.0);
        
        i32 exp = 0;
        TestResult(FrExp_f64(8.0, &exp) == 0.5 && exp == +4);
        TestResult(FrExp_f64(-.5, &exp) == -.5 && exp ==  0);
        TestResult(FrExp_f64(1.0, &exp) == 0.5 && exp == +1);
        TestResult(FrExp_f64(.25, &exp) == 0.5 && exp == -1);
        TestResult(FrExp_f64(0x1p23, &exp) == 0.5f && exp == 24);
    }
    
    TEST("Trigonometric F32")
    {
        f32 debug = 0;
        f32 epsilon = 0.0000009f;
        
        TestResult(        (debug = Sin_f32(0.0f        )) == 0.0f);
        TestResult(Abs_f32((debug = Sin_f32(+PI_F32/2.0f)) - +1.0f) < epsilon);
        TestResult(Abs_f32((debug = Sin_f32(+PI_F32     )) -  0.0f) < epsilon);
        TestResult(Abs_f32((debug = Sin_f32(+PI_F32*1.5f)) - -1.0f) < epsilon);
        TestResult(Abs_f32((debug = Sin_f32(+PI_F32/6.0f)) - +0.5f) < epsilon);
        TestResult(Abs_f32((debug = Sin_f32(-PI_F32/2.0f)) - -1.0f) < epsilon);
        TestResult(Abs_f32((debug = Sin_f32(-PI_F32     )) -  0.0f) < epsilon);
        TestResult(Abs_f32((debug = Sin_f32(-PI_F32*1.5f)) - +1.0f) < epsilon);
        TestResult(Abs_f32((debug = Sin_f32(-PI_F32/6.0f)) - -0.5f) < epsilon);
        
        TestResult(        (debug = Cos_f32(0.0f        )) == 1.0f);
        TestResult(Abs_f32((debug = Cos_f32(+PI_F32/2.0f)) -  0.0f) < epsilon);
        TestResult(Abs_f32((debug = Cos_f32(+PI_F32     )) - -1.0f) < epsilon);
        TestResult(Abs_f32((debug = Cos_f32(+PI_F32*1.5f)) -  0.0f) < epsilon);
        TestResult(Abs_f32((debug = Cos_f32(+PI_F32/3.0f)) - +0.5f) < epsilon);
        TestResult(Abs_f32((debug = Cos_f32(-PI_F32/2.0f)) -  0.0f) < epsilon);
        TestResult(Abs_f32((debug = Cos_f32(-PI_F32     )) - -1.0f) < epsilon);
        TestResult(Abs_f32((debug = Cos_f32(-PI_F32*1.5f)) -  0.0f) < epsilon);
        TestResult(Abs_f32((debug = Cos_f32(-PI_F32/3.0f)) - +0.5f) < epsilon);
        
        TestResult(        (debug = Tan_f32(0.0f            )) == 0.0f);
        TestResult(Abs_f32((debug = Tan_f32(PI_F32          )) -  0.0f) < epsilon);
        TestResult(Abs_f32((debug = Tan_f32(PI_F32*1.0f/4.0f)) - +1.0f) < epsilon);
        TestResult(Abs_f32((debug = Tan_f32(PI_F32*3.0f/4.0f)) - -1.0f) < epsilon);
        TestResult(Abs_f32((debug = Tan_f32(PI_F32*5.0f/4.0f)) - +1.0f) < epsilon);
        TestResult(Abs_f32((debug = Tan_f32(PI_F32*7.0f/4.0f)) - -1.0f) < epsilon);
        
        TestResult(        (debug = Atan_f32( 0.0f)) == 0.0f);
        TestResult(Abs_f32((debug = Atan_f32(+1.0f)) - +PI_F32*1.0f/4.0f) < epsilon);
        TestResult(Abs_f32((debug = Atan_f32(-1.0f)) - -PI_F32*1.0f/4.0f) < epsilon);
        
        TestResult(Abs_f32((debug = Atan2_f32(+1.0f, +1.0f)) - +PI_F32*1.0f/4.0f) < epsilon);
        TestResult(Abs_f32((debug = Atan2_f32(+1.0f, -1.0f)) - +PI_F32*3.0f/4.0f) < epsilon);
        TestResult(Abs_f32((debug = Atan2_f32(-1.0f, -1.0f)) - -PI_F32*3.0f/4.0f) < epsilon);
        TestResult(Abs_f32((debug = Atan2_f32(-1.0f, +1.0f)) - -PI_F32*1.0f/4.0f) < epsilon);
        
        TestResult(Abs_f32((debug = Atan2_f32(+0.0f, +0.0f)) - 0.0f) < epsilon);
        TestResult(Abs_f32((debug = Atan2_f32(+0.0f, -0.0f)) - PI_F32) < epsilon);
        TestResult(Abs_f32((debug = Atan2_f32(+7.0f, +0.0f)) - PI_F32/2.0f) < epsilon);
        TestResult(Abs_f32((debug = Atan2_f32(+7.0f, -0.0f)) - PI_F32/2.0f) < epsilon);
    }
    
    TEST("Trigonometric F64")
    {
        f64 debug = 0;
        f64 epsilon = 0.0000009f;
        
        TestResult(        (debug = Sin_f64(0.0f        )) == 0.0f);
        TestResult(Abs_f64((debug = Sin_f64(+PI_F64/2.0f)) - +1.0f) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(+PI_F64     )) -  0.0f) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(+PI_F64*1.5f)) - -1.0f) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(+PI_F64/6.0f)) - +0.5f) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(-PI_F64/2.0f)) - -1.0f) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(-PI_F64     )) -  0.0f) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(-PI_F64*1.5f)) - +1.0f) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(-PI_F64/6.0f)) - -0.5f) < epsilon);
        
        TestResult(        (debug = Cos_f64(0.0f        )) == 1.0f);
        TestResult(Abs_f64((debug = Cos_f64(+PI_F64/2.0f)) -  0.0f) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(+PI_F64     )) - -1.0f) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(+PI_F64*1.5f)) -  0.0f) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(+PI_F64/3.0f)) - +0.5f) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(-PI_F64/2.0f)) -  0.0f) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(-PI_F64     )) - -1.0f) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(-PI_F64*1.5f)) -  0.0f) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(-PI_F64/3.0f)) - +0.5f) < epsilon);
        
        TestResult(        (debug = Tan_f64(0.0f            )) == 0.0f);
        TestResult(Abs_f64((debug = Tan_f64(PI_F64          )) -  0.0f) < epsilon);
        TestResult(Abs_f64((debug = Tan_f64(PI_F64*1.0f/4.0f)) - +1.0f) < epsilon);
        TestResult(Abs_f64((debug = Tan_f64(PI_F64*3.0f/4.0f)) - -1.0f) < epsilon);
        TestResult(Abs_f64((debug = Tan_f64(PI_F64*5.0f/4.0f)) - +1.0f) < epsilon);
        TestResult(Abs_f64((debug = Tan_f64(PI_F64*7.0f/4.0f)) - -1.0f) < epsilon);
        
        TestResult(        (debug = Atan_f64( 0.0f)) == 0.0f);
        TestResult(Abs_f64((debug = Atan_f64(+1.0f)) - +PI_F64*1.0f/4.0f) < epsilon);
        TestResult(Abs_f64((debug = Atan_f64(-1.0f)) - -PI_F64*1.0f/4.0f) < epsilon);
        
        TestResult(Abs_f64((debug = Atan2_f64(+1.0f, +1.0f)) - +PI_F64*1.0f/4.0f) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(+1.0f, -1.0f)) - +PI_F64*3.0f/4.0f) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(-1.0f, -1.0f)) - -PI_F64*3.0f/4.0f) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(-1.0f, +1.0f)) - -PI_F64*1.0f/4.0f) < epsilon);
        
        TestResult(Abs_f64((debug = Atan2_f64(+0.0f, +0.0f)) - 0.0f) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(+0.0f, -0.0f)) - PI_F64) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(+7.0f, +0.0f)) - PI_F64/2.0f) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(+7.0f, -0.0f)) - PI_F64/2.0f) < epsilon);
    }
    
    Arena* arena = ArenaMake();
    
    TEST("Conversion")
    {
        TestResult(I64FromStr(StrLit(            "28"), 10, 0) == 28);
        TestResult(I64FromStr(StrLit(    "4000000024"), 10, 0) == 4000000024);
        TestResult(I64FromStr(StrLit(    "2000000022"), 10, 0) == 2000000022);
        TestResult(I64FromStr(StrLit(    "4000000000"), 10, 0) == 4000000000);
        TestResult(I64FromStr(StrLit(    "9000000000"), 10, 0) == 9000000000);
        TestResult(I64FromStr(StrLit(  "900000000001"), 10, 0) == 900000000001);
        TestResult(I64FromStr(StrLit( "9000000000002"), 10, 0) == 9000000000002);
        TestResult(I64FromStr(StrLit("90000000000004"), 10, 0) == 90000000000004);
        
        TestResult(I64FromStr(StrLit(                 "24"), 8, 0) == 024);
        TestResult(I64FromStr(StrLit(         "4000000024"), 8, 0) == 04000000024);
        TestResult(I64FromStr(StrLit(         "2000000022"), 8, 0) == 02000000022);
        TestResult(I64FromStr(StrLit(         "4000000000"), 8, 0) == 04000000000);
        TestResult(I64FromStr(StrLit(     "44000000000000"), 8, 0) == 044000000000000);
        TestResult(I64FromStr(StrLit(  "44400000000000001"), 8, 0) == 044400000000000001);
        TestResult(I64FromStr(StrLit("4444000000000000002"), 8, 0) == 04444000000000000002);
        TestResult(I64FromStr(StrLit("4444000000000000004"), 8, 0) == 04444000000000000004);
        
        TestResult(I64FromStr(StrLit(              "2a"), 16, 0) == 42);
        TestResult(I64FromStr(StrLit(        "A0000024"), 16, 0) == 2684354596);
        TestResult(I64FromStr(StrLit(        "20000022"), 16, 0) == 536870946);
        TestResult(I64FromStr(StrLit(        "A0000021"), 16, 0) == 2684354593);
        TestResult(I64FromStr(StrLit(  "8a000000000000"), 16, 0) == 38843546786070528);
        TestResult(I64FromStr(StrLit("8A40000000000010"), 16, 0) == -8484781697966014448);
        TestResult(I64FromStr(StrLit("4a44000000000020"), 16, 0) == 5351402257222991904);
        TestResult(I64FromStr(StrLit("8a44000000000040"), 16, 0) == -8483655798059171776);
        
        TestResult(I64FromStr(StrLit(       "10"), 2, 0) == 2);
        TestResult(I64FromStr(StrLit( "10000011"), 2, 0) == 131);
        TestResult(I64FromStr(StrLit("100000011"), 2, 0) == 259);
        TestResult(I64FromStr(StrLit( "10010100"), 2, 0) == 148);
        TestResult(I64FromStr(StrLit(                                "11111111111111111111111111111111"), 2, 0) == MAX_U32);
        TestResult(I64FromStr(StrLit("0000000000000000000000000000000000000000000000000000000000000000"), 2, 0) == 0);
        TestResult(I64FromStr(StrLit("0000000000000000000000000000000000000000000000000000000000000001"), 2, 0) == 1);
    }
    
    TEST("TODO: Float from String")
    {
#if 0
        TestResult(F32FromStr(StrLit("15.75")  , 0) == 15.75f);
        TestResult(F32FromStr(StrLit("1.575E1"), 0) == 1.575e1f);
        TestResult(F32FromStr(StrLit("1575e-2"), 0) == 1575e-2f);
        TestResult(F32FromStr(StrLit("-2.5e-3"), 0) == -2.5e-3f);
        TestResult(F32FromStr(StrLit("25E-4")  , 0) == 25e-4f);
        TestResult(F32FromStr(StrLit(".0075e2"), 0) == .0075e2f);
        TestResult(F32FromStr(StrLit("0.075e1"), 0) == 0.075e1f);
        TestResult(F32FromStr(StrLit(".075e1") , 0) == .075e1f);
        TestResult(F32FromStr(StrLit("75e-2")  , 0) == 75e-2f);
#endif
    }
    
    typedef struct FloatTest
    {
        u32 prec;
        f64 f;
        String str;
    } FloatTest;
    
    TEST("Str From F64 (24-bit)")
    {
        const FloatTest test[] =
        {
            {3, 12345000, StrLit("1.234e+7")},
            
            // Table 16
            {  1, ldexp(12676506, -102), }, 
            {  2, ldexp(12676506, -103), }, 
            {  3, ldexp(15445013, + 86), }, 
            {  4, ldexp(13734123, -138), }, 
            {  5, ldexp(12428269, -130), }, 
            {  6, ldexp(15334037, -146), }, 
            {  7, ldexp(11518287, - 41), }, 
            {  8, ldexp(12584953, -145), }, 
            {  9, ldexp(15961084, -125), }, 
            { 10, ldexp(14915817, -146), }, 
            { 11, ldexp(10845484, -102), }, 
            { 12, ldexp(16431059, - 61), }, 
            
            // Table 17
            {  1, ldexp(16093626, + 69), }, 
            {  2, ldexp( 9983778, + 25), }, 
            {  3, ldexp(12745034, +104), }, 
            {  4, ldexp(12706553, + 72), }, 
            {  5, ldexp(11005028, + 45), }, 
            {  6, ldexp(15059547, + 71), }, 
            {  7, ldexp(16015691, - 99), }, 
            {  8, ldexp( 8667859, + 56), }, 
            {  9, ldexp(14855922, - 82), }, 
            { 10, ldexp(14855922, - 83), }, 
            { 11, ldexp(10144164, -110), }, 
            { 12, ldexp(13248074, + 95), }, 
        };
        
        for (u64 i = 0; i < ArrayCount(test); ++i)
        {
            TempBlock(temp, arena)
            {
                u8 buf[1000];
                String str1 = StrFromF64(arena, test[i].f, 17);
                String str2 = Str(buf, sprintf(buf, "%.*e", 17, test[i].f));
                
                f64 f1 = atof(str1.str);
                f64 f2 = atof(str2.str);
                TestResult(f1 == f2);
            }
        }
    }
    
    TEST("Str From F64 (53-bit)")
    {
        const FloatTest test[] =
        {
            // Table 3
            { 0, ldexp(8511030020275656, -342), StrLit("9.e-88")},
            { 1, ldexp(5201988407066741, -824), StrLit("4.6e-233")},
            { 2, ldexp(6406892948269899, +237), StrLit("1.41e+87")},
            { 3, ldexp(8431154198732492,  +72), StrLit("3.981e+37")},
            { 4, ldexp(6475049196144587,  +99), StrLit("4.1040e+45")},
            { 5, ldexp(8274307542972842, +726), StrLit("2.92084e+234")},
            { 6, ldexp(5381065484265332, -456), StrLit("2.891946e-122")},
            { 7, ldexp(6761728585499734,-1057), StrLit("4.3787718e-303")},
            { 8, ldexp(7976538478610756, +376), StrLit("1.22770163e+129")},
            { 9, ldexp(5982403858958067, +377), StrLit("1.841552452e+129")},
            {10, ldexp(5536995190630837,  +93), StrLit("5.4835744350e+43")},
            {11, ldexp(7225450889282194, +710), StrLit("3.89190181146e+229")},
            {12, ldexp(7225450889282194, +709), StrLit("1.945950905732e+229")},
            {13, ldexp(8703372741147379, +117), StrLit("1.4460958381605e+51")},
            {14, ldexp(8944262675275217,-1001), StrLit("4.17367747458531e-286")},
            {15, ldexp(7459803696087692, -707), StrLit("1.107950772878888e-197")},
            {16, ldexp(6080469016670379, -381), StrLit("1.2345501366327440e-99")},
            {17, ldexp(8385515147034757, +721), StrLit("9.25031711960365024e+232")},
            {18, ldexp(7514216811389786, -828), StrLit("4.198047150284889840e-234")},
            {19, ldexp(8397297803260511, -345), StrLit("1.1716315319786511046e-88")},
            {20, ldexp(6733459239310543, +202), StrLit("4.32810072844612493629e+76")},
            {21, ldexp(8091450587292794, -473), StrLit("3.317710118160031081518e-127")},
            
            // Table 4
            { 0, ldexp(6567258882077402, +952), StrLit("3.e+302")},
            { 1, ldexp(6712731423444934, +535), StrLit("7.6e+176")},
            { 2, ldexp(6712731423444934, +534), StrLit("3.78e+176")},
            { 3, ldexp(5298405411573037, -957), StrLit("4.350e-273")},
            { 4, ldexp(5137311167659507, -144), StrLit("2.3037e-28")},
            { 5, ldexp(6722280709661868, +363), StrLit("1.26301e+125")},
            { 6, ldexp(5344436398034927, -169), StrLit("7.142211e-36")},
            { 7, ldexp(8369123604277281, -853), StrLit("1.3934574e-241")},
            { 8, ldexp(8995822108487663, -780), StrLit("1.41463449e-219")},
            { 9, ldexp(8942832835564782, -383), StrLit("4.539277920e-100")},
            {10, ldexp(8942832835564782, -384), StrLit("2.2696389598e-100")},
            {11, ldexp(8942832835564782, -385), StrLit("1.13481947988e-100")},
            {12, ldexp(6965949469487146, -249), StrLit("7.700366561890e-60")},
            {13, ldexp(6965949469487146, -250), StrLit("3.8501832809448e-60")},
            {14, ldexp(6965949469487146, -251), StrLit("1.92509164047238e-60")},
            {15, ldexp(7487252720986826, +548), StrLit("6.898586531774201e+180")},
            {16, ldexp(5592117679628511, +164), StrLit("1.3076622631878654e+65")},
            {17, ldexp(8887055249355788, +665), StrLit("1.36052020756121240e+216")},
            {18, ldexp(6994187472632449, +690), StrLit("3.592810217475959676e+223")},
            {19, ldexp(8797576579012143, +588), StrLit("8.9125197712484551899e+192")},
            {20, ldexp(7363326733505337, +272), StrLit("5.58769757362301140950e+97")},
            {21, ldexp(8549497411294502, -448), StrLit("1.176257830728540379990e-119")},
        };
        
        for (u64 i = 0; i < ArrayCount(test); ++i)
        {
            TempBlock(temp, arena)
            {
                u8 buf[1000];
                String str1 = StrFromF64(arena, test[i].f, 17);
                String str2 = Str(buf, sprintf(buf, "%.*e", 17, test[i].f));
                
                f64 f1 = atof(str1.str);
                f64 f2 = atof(str2.str);
                TestResult(f1 == f2);
            }
        }
    }
    
    TEST("Str From F64 (56-bit)")
    {
        const FloatTest test[] =
        {
            // Table 20
            {  1, ldexp(50883641005312716., -172), },
            {  2, ldexp(38162730753984537., -170), },
            {  3, ldexp(50832789069151999., -101), },
            {  4, ldexp(51822367833714164., -109), },
            {  5, ldexp(66840152193508133., -172), },
            {  6, ldexp(55111239245584393., -138), },
            {  7, ldexp(71704866733321482., -112), },
            {  8, ldexp(67160949328233173., -142), },
            {  9, ldexp(53237141308040189., -152), },
            { 10, ldexp(62785329394975786., -112), },
            { 11, ldexp(48367680154689523., - 77), },
            { 12, ldexp(42552223180606797., -102), },
            { 13, ldexp(63626356173011241., -112), },
            { 14, ldexp(43566388595783643., - 99), },
            { 15, ldexp(54512669636675272., -159), },
            { 16, ldexp(52306490527514614., -167), },
            { 17, ldexp(52306490527514614., -168), },
            { 18, ldexp(41024721590449423., - 89), },
            { 19, ldexp(37664020415894738., -132), },
            { 20, ldexp(37549883692866294., - 93), },
            { 21, ldexp(69124110374399839., -104), },
            { 22, ldexp(69124110374399839., -105), },
            
            // Table 21
            {  1, ldexp(49517601571415211., - 94), },
            {  2, ldexp(49517601571415211., - 95), },
            {  3, ldexp(54390733528642804., -133), },
            {  4, ldexp(71805402319113924., -157), },
            {  5, ldexp(40435277969631694., -179), },
            {  6, ldexp(57241991568619049., -165), },
            {  7, ldexp(65224162876242886., + 58), },
            {  8, ldexp(70173376848895368., -138), },
            {  9, ldexp(37072848117383207., - 99), },
            { 10, ldexp(56845051585389697., -176), },
            { 11, ldexp(54791673366936431., -145), },
            { 12, ldexp(66800318669106231., -169), },
            { 13, ldexp(66800318669106231., -170), },
            { 14, ldexp(66574323440112438., -119), },
            { 15, ldexp(65645179969330963., -173), },
            { 16, ldexp(61847254334681076., -109), },
            { 17, ldexp(39990712921393606., -145), },
            { 18, ldexp(59292318184400283., -149), },
            { 19, ldexp(69116558615326153., -143), },
            { 20, ldexp(69116558615326153., -144), },
            { 21, ldexp(39462549494468513., -152), },
            { 22, ldexp(39462549494468513., -153), },
        };
        
        for (u64 i = 0; i < ArrayCount(test); ++i)
        {
            TempBlock(temp, arena)
            {
                u8 buf[1000];
                String str1 = StrFromF64(arena, test[i].f, 17);
                String str2 = Str(buf, sprintf(buf, "%.*e", 17, test[i].f));
                
                f64 f1 = atof(str1.str);
                f64 f2 = atof(str2.str);
                TestResult(f1 == f2);
            }
        }
    }
    
    TEST("String")
    {
        String str = StrLit("Here's a simple test!");
        TestResult(ChrCompare('m', 'M', 1));
        TestResult(ChrCompareArr('M', str, 1));
        TestResult(ChrCompareArr('M', str, 0) == 0);
        TestResult(StrCompare(str, StrLit("Here's a simple test!"), 0));
        TestResult(StrCompare(str, StrLit("here's a simple tesT!"), 1));
        TestResult(StrContainsChr(str, "!sa"));
        
        TestResult(StrCompare( StrPrefix(str, str.size / 2), StrLit( "Here's a s"), 0));
        TestResult(StrCompare(   StrSkip(str, str.size / 2), StrLit("imple test!"), 0));
        TestResult(StrCompare(   StrChop(str, 5), StrLit("Here's a simple "), 0));
        TestResult(StrCompare(StrPostfix(str, 2), StrLit("t!"), 0));
        TestResult(StrCompare( Substr(str, 2, 8), StrLit("re's a"), 0));
        
        StringList list = StrSplitArr(arena, str, StrLit(" '"), 0);
        StrListPush(arena, &list, StrLit("Insert string"));
        for (StringNode* node = list.first; node; node = node->next)
        {
            if (node->next)
                TestResult(!StrContainsChr(node->string, " '"));
            else
                TestResult(StrCompare(node->string, StrLit("Insert string"), 0));
        }
        
        TestResult(StrCompare(SubstrSplit(str, list.first->next->next->next->string).pre, StrLit("Here's a "), 0));
        TestResult(StrCompare(SubstrSplit(str, list.first->next->next->string).post, StrLit(" simple test!"), 0));
        
        String newStr = StrJoin(arena, &list, .pre = StrLit("a"), .mid = StrLit("mc"), .post = StrLit(" mm "));
        TestResult(StrCompare(newStr, StrLit("aHeremcsmcamcsimplemctest!mcInsert string mm "), 0));
        
        String word = StrLit("Word");
        String data = StrPushf(arena, "%.*s %.*s!\nThe date is %d", StrExpand(StrLit("Hello")), StrExpand(word), 25);
        TestResult(StrCompare(data, StrLit("Hello Word!\nThe date is 25"), 0));
    }
    
    TEST("Log/Error")
    {
        StringList logs = {0};
        LogBlock(arena, logs)
        {
            LogPush(LOG_TRACE, "Log #%d", LOG_TRACE);
            LogPush(LOG_DEBUG, "Log #%d", LOG_DEBUG);
            LogPush(LOG_INFO , "Log #%d", LOG_INFO);
            LogPush(LOG_WARN , "Log #%d", LOG_WARN);
            LogPush(LOG_ERROR, "Log #%d", LOG_ERROR);
            LogPush(LOG_FATAL, "Log #%d", LOG_FATAL);
            
            LogInfo* info = LogGetInfo();
            info->level = LOG_INFO;
            LogPush(LOG_TRACE, "Log trace");
            LogPush(LOG_DEBUG, "Log debug");
            LogPush(LOG_INFO , "Log info");
            LogPush(LOG_WARN , "Log warn");
            LogPush(LOG_ERROR, "Log error");
            LogPush(LOG_FATAL, "Log fatal");
            
            info->level = LOG_DEBUG;
            info->callback = TestLogCallback;
            LogPush(LOG_TRACE, "Log trace");
            LogPush(LOG_DEBUG, "Log debug");
            LogPush(LOG_INFO , "Log info");
            LogPush(LOG_WARN , "Log warn");
            LogPush(LOG_ERROR, "Log error");
            LogPush(LOG_FATAL, "Log fatal");
            
            TestResult(snapshot == 5);
            LogBlock(arena, logs, .callback = TestLogCallback)
            {
                for (u64 i = 0; i < 1024; ++i)
                    LogPush(i % LogType_Count, "Log: %lld", i);
                
                LogBlock(arena, logs)
                {
                    LogPush(0, "Inner Log\n");
                    for (u64 i = 0; i < 1024; ++i)
                        LogPush(i % LogType_Count, "New Log: %lld", i);
                }
                
                TestResult(snapshot == 1029);
                TestResult(logs.nodeCount == 1025);
            }
            
            TestResult(logs.nodeCount == 1024);
        }
        
        TestResult(logs.nodeCount == 15);
    }
    
    TEST("OS")
    {
        OSInit(0, 0);
        
        {
#define FileName(name) StrLit("code/examples/"name)
            String fileData = OSReadFile(arena, FileName("Test.txt"), true);
            TestResult(StrCompare(fileData, StrLit("This is a test file!"), 0));
            TestResult(OSWriteFile(FileName("Test2.txt"), fileData));
            TestResult(StrCompare(OSReadFile(arena, FileName("Test2.txt"), 0), fileData, 0));
            TestResult(OSDeleteFile(FileName("Test2.txt")));
            
            FileProperties file = GetFileProperties(FileName("Test.txt"));
            OSRenameFile(FileName("Test.txt"), FileName("Test3.txt"));
            TestResult(OSReadFile(arena, FileName("Test.txt"), 0).size == 0);
            {
                FileProperties _file = GetFileProperties(FileName("Test3.txt"));
                TestResult(CompareMem(&file, &_file, sizeof(FileProperties)));
            }
            OSRenameFile(FileName("Test3.txt"), FileName("Test.txt"));
            TestResult(OSReadFile(arena, FileName("Test3.txt"), 0).size == 0);
        }
        
        {
            u64 nameLen = sizeof("\\code\\examples\\test_base.c") - 1;
            String currentDir = OSCurrentDir(arena);
            TestResult(StrCompare(currentDir, StrChop(StrLit(__FILE__), nameLen), 0));
            TestResult(StrCompare(OSProcessDir(), StrJoin3(arena, currentDir, StrLit("\\build")), 0));
            TestResult(StrCompare(OSAppDataDir(), StrLit("C:\\Users\\ADMIN"), 0));
            TestResult(StrCompare(OSAppTempDir(), StrLit("C:\\Users\\ADMIN\\AppData\\Local\\Temp"), 0));
        }
        
        u64 entropy;
        OSGetEntropy(&entropy, sizeof(entropy));
        TestResult(entropy > 1000 && entropy != MAX_U64 && entropy != MAX_I64 && entropy != MAX_U32 && entropy != MAX_I32);
        
        StringList logs = {0};
        LogBlock(arena, logs)
        {
            OSLib testLib = OSLoadLib(StrLit("build/TestDLL.dll"));
            i32 (*init)(VoidFunc*);
            PrcCast(init, OSGetProc(testLib, "DLLCallback"));
            snapshot = 0;
            TestResult(init(IncSnapshot) == 10 && snapshot == 1);
            u32(*func)(u32*, u64);
            PrcCast(func, OSGetProc(testLib, "Sum"));
            u32* array = (u32*)&entropy;
            TestResult(func(array, 2) == array[0] + array[1]);
            i32* dllVar = (i32*)OSGetProc(testLib, "globalInt");
            TestResult(*dllVar == 10);
            *dllVar = 100;
            TestResult(init(IncSnapshot) == 100 && snapshot == 2);
            OSFreeLib(testLib);
        }
        TestResult(logs.nodeCount == 2);
    }
    
    TEST("Timer")
    {
        for (u64 i = 0; i < 20; ++i)
        {
            u64 time = OSNowMS();
            DateTime dt = OSNowUniTime();
            
            RNG rng = GetRNG();
            i64 sleep = RandomRangeI32(rng, 20, 200);
            OSSleepMS((u32)sleep);
            u64 now = OSNowMS();
            DateTime dtNow = OSNowUniTime();
            TestResult(AbsI64((i64)(now - time) - sleep) <= ClampBot(sleep / 20, 4));
            TestResult(dtNow.min == dt.min && dtNow.hour == dt.hour &&
                       dtNow.day == dt.day && dtNow.mon == dt.mon && dtNow.year == dt.year);
        }
    }
    
    TEST("File Iter")
    {
        {
            u32 folderCount = 0;
            StringList exts = {0};
            StrListPush(arena, &exts, StrLit("c"));
            StrListPush(arena, &exts, StrLit("h"));
            StrListPush(arena, &exts, StrLit("txt"));
            
            String currentName;
            FileProperties currentProp;
            OSFileIter iter = FileIterInit(StrLit("code"));
            
            while(FileIterNext(arena, &iter, &currentName, &currentProp))
            {
                // smallest is 2 (md), largest is 16 (LongCompressor.c and LongGFX_OpenGL.h)
                TestResult(InRange(currentName.size, 2, 16) && currentProp.modifyTime >= currentProp.createTime);
                if (currentProp.flags & FilePropertyFlag_IsFolder)
                    folderCount++;
                else
                    TestResult(StrListCompare(StrSkipUntil(currentName, StrLit("."), FindStr_LastMatch), &exts, 0));
            }
            TestResult(folderCount == 3);
        }
    }
    
    TEST("Arena")
    {
        ArenaPopTo(arena, 0);
        TestResult(arena->cap == MEM_DEFAULT_RESERVE_SIZE);
        TestResult(arena->pos == AlignUpPow2(sizeof(Arena), 8));
        TestResult(arena->alignment == MEM_DEFAULT_ALIGNMENT);
        TestResult(arena->commitPos == MEM_COMMIT_BLOCK_SIZE);
        
        u64 minPos = arena->pos;
        TempBlock(temp, arena)
        {
            u64 arrayCount = 10;
            i64* array = PushArray(arena, i64, arrayCount);
            i64 arr[10] = {0};
            TestResult(CompareMem(arr, array, 10));
            TestResult(arena->pos == temp.pos + arrayCount * sizeof(u64));
            TestResult(temp.pos == minPos);
        }
        TestResult(arena->pos == minPos);
        
        u8* buffer1 = 0;
        u8* buffer2 = 0;
        TempBlock(temp, arena)
        {
            buffer1 = ArenaPushPoison(arena, 1000);
            buffer2 =   ArenaPush(arena, 1000);
            Assert(buffer2 == (buffer1 + 1000 + MEM_POISON_SIZE));
            Assert(buffer2 == (PtrAdd(arena, temp.pos) + 1000 + MEM_POISON_SIZE * 2));
            
#if ENABLE_SANITIZER
            TestResult(AsanIsPoison(buffer2 + 1000, 1));
            TestResult(AsanIsPoison(buffer1 + 1000, 1));
#endif
        }
        
#if ENABLE_SANITIZER
        TestResult(AsanIsPoison(buffer1, buffer2 + 1000 - buffer1));
#endif
        
        buffer1 = ArenaPush(arena, 1000);
        u64 offset = 0;
        TempPoisonBlock(temp, arena)
        {
            u8* aPtr = ArenaPush(arena, 50);
            u8* bPtr = ArenaPush(arena, 50);
            u8* cPtr = ArenaPush(arena, 50);
            u8* dPtr = ArenaPush(arena, 50);
            
            offset = dPtr - buffer1;
            TestResult(aPtr == buffer1 + AlignUpPow2(1000, arena->alignment) + MEM_POISON_SIZE);
            TestResult((uptr)bPtr == AlignUpPow2((uptr)aPtr + 50, arena->alignment));
            TestResult((uptr)cPtr == AlignUpPow2((uptr)bPtr + 50, arena->alignment));
            TestResult((uptr)dPtr == AlignUpPow2((uptr)cPtr + 50, arena->alignment));
        }
        
        buffer2 = ArenaPush(arena, 1000);
        TestResult(buffer2 == buffer1 + offset + AlignUpPow2(50, MEM_DEFAULT_ALIGNMENT) + MEM_POISON_SIZE);
        
#if ENABLE_SANITIZER
        u64 cap = arena->cap;
        ArenaRelease(arena);
        TestResult(AsanIsPoison(arena, cap));
#endif
    }
}
