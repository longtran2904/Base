#define BASE_LIB_EXPORT_SYMBOLS 1
#include "Base.h"
#include "Base.c"
#include <stdio.h>

#define LONG_TEST_IMPLEMENTATION
#define LT_ASSERT(x) Assert(x)
#include "LongTest.h"

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

function void TestFloat(f64 f, String str)
{
    ScratchBlock(scratch)
    {
        u8 buf[1000];
        String str1 = StrFromF64(scratch, f, 17);
        String str2 = Str(buf, sprintf(buf, "%.*e", 17, f));
        
        f64 f1 = atof(str1.str);
        f64 f2 = atof(str2.str);
        TestResult(f1 == f2 && f1 == f);
        
        f64 f3 = F64FromStr(str1, 0);
        f64 f4 = F64FromStr(str2, 0);
        TestResult(f3 == f4 && f3 == f);
        
        if (str.size)
        {
            f64 f5 = F64FromStr(str, 0);
            TestResult(f5 == f);
        }
    }
}

MSVC(WarnDisable(28182 6011 4723))
int main(void)
{
    u64 elapsed = OSNowMS();
    
    i32 space = Max(sizeof(CURRENT_COMPILER_NAME), Max(sizeof(CURRENT_OS_NAME), sizeof(CURRENT_ARCH_NAME))) - 1;
    Outf(  "------------------------------CONTEXT------------------------------\n");
    Outf("-----[COM:  %*s]-----\n", space, CURRENT_COMPILER_NAME);
    Outf("-----[OS:   %*s]-----\n", space, CURRENT_OS_NAME);
    Outf("-----[ARCH: %*s]-----\n", space, CURRENT_ARCH_NAME);
    Outf("\n");
    Outf("------------------------------TESTING------------------------------\n");
    
    TEST("Helper")
    {
        TestResult(StrCompare(GetEnumStr(Day, -1), StrLit("Invalid"), 0));
        TestResult(StrCompare(GetEnumStr(Arch, 0), StrLit("None"), 0));
        TestResult(StrCompare(GetEnumStr(Arch, Arch_ARM), StrLit("ARM"), 0));
        TestResult(StrCompare(GetEnumStr(Compiler, CURRENT_COMPILER_NUMBER), StrLit(CURRENT_COMPILER_NAME), 0));
        TestResult(StrCompare(GetEnumStr(OS,             CURRENT_OS_NUMBER), StrLit(      CURRENT_OS_NAME), 0));
        TestResult(StrCompare(GetEnumStr(Arch,         CURRENT_ARCH_NUMBER), StrLit(    CURRENT_ARCH_NAME), 0));
        
#define CheckDefinedArray(arr, size) StaticAssert(ArrayCount(arr) == (size) + 1)
        CheckDefinedArray(Day_names, 7);
        CheckDefinedArray(Month_names, 12);
        CheckDefinedArray(Compiler_names, 3);
        CheckDefinedArray(Arch_names, 4);
        CheckDefinedArray(OS_names, 3);
        
        i32 foo[100];
        for (i32 i = 0; i < ArrayCount(foo); ++i)
            foo[i] = i;
        StaticAssert(ArrayCount(foo) == 100);
        
        i32 bar[100];
        CopyFixedArr(bar, foo);
        TestResult(bar[0] == 0 && bar[50] == 50 && bar[99] == 99);
        TestResult(CompareMem(foo, bar, sizeof(foo)));
        
        typedef struct TestStruct TestStruct;
        struct TestStruct
        {
            i32 a, b, c, d;
        };
        
        StaticAssert(OffsetOf(TestStruct, a) ==  0);
        StaticAssert(OffsetOf(TestStruct, b) ==  4);
        StaticAssert(OffsetOf(TestStruct, c) ==  8);
        StaticAssert(OffsetOf(TestStruct, d) == 12);
        
        TestStruct t = { 1, 2, 3, 4 };
        TestResult(t.a == 1 && t.b == 2 && t.c == 3 && t.d == 4);
        ZeroStruct(&t);
        TestResult(t.a == 0 && t.b == 0 && t.c == 0 && t.d == 0);
        
        StaticAssert(ChrIsUpper('A') == 1);
        StaticAssert(ChrIsUpper('c') == 0);
        StaticAssert(ChrIsUpper(';') == 0);
        StaticAssert(ChrIsLower('d') == 1);
        StaticAssert(ChrIsLower('E') == 0);
        StaticAssert(ChrIsLower('0') == 0);
        
        StaticAssert(ChrToUpper('A') == 'A');
        StaticAssert(ChrToUpper('d') == 'D');
        StaticAssert(ChrToUpper('@') == '@');
        StaticAssert(ChrToUpper('^') == '^');
        StaticAssert(ChrToLower('a') == 'a');
        StaticAssert(ChrToLower('D') == 'd');
        StaticAssert(ChrToLower('2') == '2');
        StaticAssert(ChrToLower('`') == '`');
        
        i32 a = 10, b = 5;
        Swap(i32, a, b);
        TestResult(a == 5 && b == 10);
        
        StaticAssert(Min(  1, 100) ==   1);
        StaticAssert(Min(100,  20) ==  20);
        StaticAssert(Min(-50, -20) == -50);
        StaticAssert(Max(  1,  40) ==  40);
        StaticAssert(Max(200,  10) == 200);
        StaticAssert(Max(-50,  10) ==  10);
        
        StaticAssert(Clamp( 1, 10, 50) == 10);
        StaticAssert(Clamp(80,  0,  7) ==  7);
        StaticAssert(Clamp(20,  5, 30) == 20);
        
        StaticAssert(  Lerp(1.f, 10.f, 0.75f) == 7.75f);
        StaticAssert(UnLerp(1.f, 10.f, 7.75f) ==  .75f);
        
        StaticAssert(GetSign(-2) == -1);
        StaticAssert(GetSign(+2) == +1);
        StaticAssert(GetSign( 0) ==  0);
        StaticAssert(GetUnsigned( 0) == +1);
        StaticAssert(GetUnsigned(-1) == -1);
        
        StaticAssert(AlignUpPow2(64, 32) == 64);
        StaticAssert(AlignUpPow2(15, 64) == 64);
        StaticAssert(AlignUpPow2(84, 32) == 96);
        
        StaticAssert(AlignDownPow2(64, 32) == 64);
        StaticAssert(AlignDownPow2(72, 32) == 64);
        StaticAssert(AlignDownPow2(72, 64) == 64);
        
        StaticAssert(IsPow2OrZero(32) == 1);
        StaticAssert(IsPow2OrZero(64) == 1);
        StaticAssert(IsPow2OrZero( 5) == 0);
        StaticAssert(IsPow2OrZero( 0) == 1);
        StaticAssert(IsPow2OrZero(KB( 4)) == 1);
        StaticAssert(IsPow2OrZero(KB(10)) == 0);
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
        
        TestResult(Round_f64(+2.0) == +2.0);
        TestResult(Round_f64(+2.3) == +2.0);
        TestResult(Round_f64(+2.5) == +2.0);
        TestResult(Round_f64(-2.0) == -2.0);
        TestResult(Round_f64(-2.3) == -2.0);
        TestResult(Round_f64(-2.5) == -2.0);
        TestResult(Round_f64(+2.6) == +3.0);
        TestResult(Round_f64(-2.6) == -3.0);
        
        TestResult(Round_f64(+1./zero_f64) == Inf_f64() && Round_f64(-1./zero_f64) == NegInf_f64());
        TestResult(IsNanF64(Round_f64(0./zero_f64)));
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
        f64 epsilon = 0.0000009;
        
        TestResult(        (debug = Sin_f64(0.0        )) == 0.0);
        TestResult(Abs_f64((debug = Sin_f64(+PI_F64/2.0)) - +1.0) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(+PI_F64     )) - 0.0) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(+PI_F64*1.5)) - -1.0) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(+PI_F64/6.0)) - +0.5) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(-PI_F64/2.0)) - -1.0) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(-PI_F64     )) - 0.0) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(-PI_F64*1.5)) - +1.0) < epsilon);
        TestResult(Abs_f64((debug = Sin_f64(-PI_F64/6.0)) - -0.5) < epsilon);
        
        TestResult(        (debug = Cos_f64(0.0        )) == 1.0);
        TestResult(Abs_f64((debug = Cos_f64(+PI_F64/2.0)) -  0.0) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(+PI_F64     ))- -1.0) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(+PI_F64*1.5)) -  0.0) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(+PI_F64/3.0)) - +0.5) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(-PI_F64/2.0)) -  0.0) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(-PI_F64     ))- -1.0) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(-PI_F64*1.5)) -  0.0) < epsilon);
        TestResult(Abs_f64((debug = Cos_f64(-PI_F64/3.0)) - +0.5) < epsilon);
        
        TestResult(        (debug = Tan_f64(0.0            )) == 0.0);
        TestResult(Abs_f64((debug = Tan_f64(PI_F64          )) -  0.0) < epsilon);
        TestResult(Abs_f64((debug = Tan_f64(PI_F64*1.0/4.0)) - +1.0) < epsilon);
        TestResult(Abs_f64((debug = Tan_f64(PI_F64*3.0/4.0)) - -1.0) < epsilon);
        TestResult(Abs_f64((debug = Tan_f64(PI_F64*5.0/4.0)) - +1.0) < epsilon);
        TestResult(Abs_f64((debug = Tan_f64(PI_F64*7.0/4.0)) - -1.0) < epsilon);
        
        TestResult(        (debug = Atan_f64( 0.0)) == 0.0);
        TestResult(Abs_f64((debug = Atan_f64(+1.0)) - +PI_F64*1.0/4.0) < epsilon);
        TestResult(Abs_f64((debug = Atan_f64(-1.0)) - -PI_F64*1.0/4.0) < epsilon);
        
        TestResult(Abs_f64((debug = Atan2_f64(+1.0, +1.0)) - +PI_F64*1.0/4.0) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(+1.0, -1.0)) - +PI_F64*3.0/4.0) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(-1.0, -1.0)) - -PI_F64*3.0/4.0) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(-1.0, +1.0)) - -PI_F64*1.0/4.0) < epsilon);
        
        TestResult(Abs_f64((debug = Atan2_f64(+0.0, +0.0)) - 0.0) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(+0.0, -0.0)) - PI_F64) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(+7.0, +0.0)) - PI_F64/2.0) < epsilon);
        TestResult(Abs_f64((debug = Atan2_f64(+7.0, -0.0)) - PI_F64/2.0) < epsilon);
    }
    
    TEST("Numeric F32")
    {
        TestResult(AbsI32(-3506708) == 3506708);
        TestResult(Inf_f32() == -NegInf_f32());
        TestResult(Abs_f32(-2.5f) == 2.5f);
        TestResult(Abs_f32(-Inf_f32()) == Inf_f32());
        
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
        TestResult(AbsI64(MIN_I64 + MAX_I32) == 9223372034707292161);
        TestResult(Inf_f64() == -NegInf_f64());
        TestResult(Abs_f64(-357.39460) == 357.39460);
        TestResult(Abs_f64(NegInf_f64()) == Inf_f64());
        
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
        TestResult(FrExp_f64(0x1p23, &exp) == 0.5 && exp == 24);
    }
    
    Arena* arena = ArenaMake();
    
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
        
        {
            StringList list = StrSplitArr(arena, str, StrLit(" '"), 0);
            StrListPush(arena, &list, StrLit("Insert string"));
            StrListIter(&list, node)
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
        
        {
            StringList list = {0};
            StrListPush(arena, &list, StrLit("A"));
            StrListPush(arena, &list, StrLit("B"));
            StrListPush(arena, &list, StrLit("C"));
            StrListPush(arena, &list, StrLit("D"));
            TestResult(list.nodeCount == 4 && list.totalSize == 4);
            TestResult(StrCompare(list.first->string, StrLit("A"), 0));
            TestResult(StrCompare(list.last ->string, StrLit("D"), 0));
            TestResult(StrCompareList(StrLit("A"), &list, 0));
            TestResult(StrCompareList(StrLit("D"), &list, 0));
            
            String last = StrListPop(&list);
            TestResult(list.nodeCount == 3 && list.totalSize == 3);
            TestResult(StrCompare(list.first->string, StrLit("A"), 0) && StrCompare(list.last->string, StrLit("C"), 0));
            TestResult(StrCompare(last, StrLit("D"), 0));
            
            String first = StrListPopFront(&list);
            TestResult(list.nodeCount == 2 && list.totalSize == 2);
            TestResult(StrCompare(list.first->string, StrLit("B"), 0) && StrCompare(list.last->string, StrLit("C"), 0));
            TestResult(StrCompare(first, StrLit("A"), 0));
            
            TestResult(!StrCompareList(StrLit("A"), &list, 0));
            TestResult(!StrCompareList(StrLit("D"), &list, 0));
            
            list = StrList(arena, ArrayExpand(String, StrLit("Long"), StrLit("Minh"), StrLit("Lan")));
            TestResult(list.nodeCount == 3 && list.totalSize == StrLit("LongMinhLan").size);
            TestResult(list.first->next->next == list.last);
            TestResult(StrCompare(StrLit("Long"), list.first->string, 0));
            TestResult(StrCompare( StrLit("Lan"), list. last->string, 0));
        }
    }
    
    TEST("Str -> I64")
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
        
        TestResult(I64FromStr(StrLit(               "2a"), 16, 0) == 42);
        TestResult(I64FromStr(StrLit(         "A0000024"), 16, 0) == 2684354596);
        TestResult(I64FromStr(StrLit(         "20000022"), 16, 0) == 536870946);
        TestResult(I64FromStr(StrLit(         "A0000021"), 16, 0) == 2684354593);
        TestResult(I64FromStr(StrLit(  "+8a000000000000"), 16, 0) == 38843546786070528);
        TestResult(I64FromStr(StrLit("-75BFFFFFFFFFFFF0"), 16, 0) == -8484781697966014448);
        TestResult(I64FromStr(StrLit("+4a44000000000020"), 16, 0) == 5351402257222991904);
        TestResult(I64FromStr(StrLit("-75BBFFFFFFFFFFC0"), 16, 0) == -8483655798059171776);
        
        TestResult(I64FromStr(StrLit(       "10"), 2, 0) == 2);
        TestResult(I64FromStr(StrLit( "10000011"), 2, 0) == 131);
        TestResult(I64FromStr(StrLit("100000011"), 2, 0) == 259);
        TestResult(I64FromStr(StrLit("101101011"), 2, 0) == 363);
        TestResult(I64FromStr(StrLit( "10010100"), 2, 0) == 148);
        TestResult(I64FromStr(StrLit(                                "11111111111111111111111111111111"), 2, 0) == MAX_U32);
        TestResult(I64FromStr(StrLit("0000000000000000000000000000000000000000000000000000000000000000"), 2, 0) == 0);
        TestResult(I64FromStr(StrLit("0000000000000000000000000000000000000000000000000000000000000001"), 2, 0) == 1);
    }
    
    TEST("Str <- I64")
    {
        TestResult(StrCompare(StrFromI64(arena, 28, 10), StrLit("28"), 0));
        TestResult(StrCompare(StrFromI64(arena, 4000000024, 10), StrLit("4000000024"), 0));
        TestResult(StrCompare(StrFromI64(arena, 2000000022, 10), StrLit("2000000022"), 0));
        TestResult(StrCompare(StrFromI64(arena, 4000000000, 10), StrLit("4000000000"), 0));
        TestResult(StrCompare(StrFromI64(arena, 9000000000, 10), StrLit("9000000000"), 0));
        TestResult(StrCompare(StrFromI64(arena, 900000000001, 10), StrLit("900000000001"), 0));
        TestResult(StrCompare(StrFromI64(arena, 9000000000002, 10), StrLit("9000000000002"), 0));
        TestResult(StrCompare(StrFromI64(arena, 90000000000004, 10), StrLit("90000000000004"), 0));
        
        TestResult(StrCompare(StrFromI64(arena, 024, 8), StrLit("24"), 0));
        TestResult(StrCompare(StrFromI64(arena, 04000000024, 8), StrLit("4000000024"), 0));
        TestResult(StrCompare(StrFromI64(arena, 02000000022, 8), StrLit("2000000022"), 0));
        TestResult(StrCompare(StrFromI64(arena, 04000000000, 8), StrLit("4000000000"), 0));
        TestResult(StrCompare(StrFromI64(arena, 044000000000000, 8), StrLit("44000000000000"), 0));
        TestResult(StrCompare(StrFromI64(arena, 044400000000000001, 8), StrLit("44400000000000001"), 0));
        TestResult(StrCompare(StrFromI64(arena, 04444000000000000002, 8), StrLit("4444000000000000002"), 0));
        TestResult(StrCompare(StrFromI64(arena, 04444000000000000004, 8), StrLit("4444000000000000004"), 0));
        
        TestResult(StrCompare(StrFromI64(arena, 42, 16), StrLit("2a"), 1));
        TestResult(StrCompare(StrFromI64(arena, 2684354596, 16), StrLit("A0000024"), 1));
        TestResult(StrCompare(StrFromI64(arena, 536870946, 16), StrLit("20000022"), 1));
        TestResult(StrCompare(StrFromI64(arena, 2684354593, 16), StrLit("A0000021"), 1));
        TestResult(StrCompare(StrFromI64(arena, 38843546786070528, 16), StrLit("8a000000000000"), 1));
        TestResult(StrCompare(StrFromI64(arena, 8484781697966014448, 16), StrLit("75bffffffffffff0"), 1));
        TestResult(StrCompare(StrFromI64(arena, 5351402257222991904, 16), StrLit("4a44000000000020"), 1));
        TestResult(StrCompare(StrFromI64(arena, 8483655798059171776, 16), StrLit("75bbffffffffffc0"), 1));
        
        TestResult(StrCompare(StrFromI64(arena, 2, 2), StrLit("10"), 0));
        TestResult(StrCompare(StrFromI64(arena, 131, 2), StrLit("10000011"), 0));
        TestResult(StrCompare(StrFromI64(arena, 259, 2), StrLit("100000011"), 0));
        TestResult(StrCompare(StrFromI64(arena, 363, 2), StrLit("101101011"), 0));
        TestResult(StrCompare(StrFromI64(arena, 148, 2), StrLit("10010100"), 0));
        TestResult(StrCompare(StrFromI64(arena, MAX_U32, 2), StrLit("11111111111111111111111111111111"), 0));
        TestResult(StrCompare(StrFromI64(arena, 0, 2), StrLit("0"), 0));
        TestResult(StrCompare(StrFromI64(arena, 1, 2), StrLit("1"), 0));
    }
    
    typedef struct FloatTest
    {
        u32 prec;
        f64 f;
        String str;
    } FloatTest;
    
    TEST("Str <-> F64 #1")
    {
        const FloatTest small_test[] =
        {
            { 9, 12345000, StrLit("1.2345e7"), },
            { 9,    15.75, StrLit(   "15.75"), },
            { 9,  1.575e1, StrLit( "1.575E1"), },
            { 9,  1575e-2, StrLit( "1575e-2"), },
            { 9,  -2.5e-3, StrLit( "-2.5e-3"), },
            { 9,    25e-4, StrLit(   "25E-4"), },
            { 9,  .0075e2, StrLit( ".0075e2"), },
            { 9,  0.075e1, StrLit( "0.075e1"), },
            { 9,   .075e1, StrLit(  ".075e1"), },
            { 9,    75e-2, StrLit(   "75e-2"), },
        };
        
        for (u64 i = 0; i < ArrayCount(small_test); ++i)
            TestFloat(small_test[i].f, small_test[i].str);
    }
    
    TEST("Str <-> F64 #2")
    {
        const FloatTest test[] =
        {
            // Quarter Table 16
            {  1, ldexp(12676506, -102), },
            {  2, ldexp(12676506, -103), },
            {  3, ldexp(15445013, + 86), },
            {  4, ldexp(13734123, -138), },
            
            // Quarter Table 17
            {  1, ldexp(16093626, + 69), },
            {  2, ldexp( 9983778, + 25), },
            {  3, ldexp(12745034, +104), },
            {  4, ldexp(12706553, + 72), },
            
            // Quarter Table 3
            { 0, ldexp(8511030020275656, -342), StrLit("9.e-88")},
            { 1, ldexp(5201988407066741, -824), StrLit("4.6e-233")},
            { 2, ldexp(6406892948269899, +237), StrLit("1.41e+87")},
            { 3, ldexp(8431154198732492,  +72), StrLit("3.981e+37")},
            
            // Quarter Table 4
            { 0, ldexp(6567258882077402, +952), StrLit("3.e+302")},
            { 1, ldexp(6712731423444934, +535), StrLit("7.6e+176")},
            { 2, ldexp(6712731423444934, +534), StrLit("3.78e+176")},
            { 3, ldexp(5298405411573037, -957), StrLit("4.350e-273")},
            
            // Quarter Table 20
            {  1, ldexp(50883641005312716., -172), },
            {  2, ldexp(38162730753984537., -170), },
            {  3, ldexp(50832789069151999., -101), },
            {  4, ldexp(51822367833714164., -109), },
            
            // Quarter Table 21
            {  1, ldexp(49517601571415211., - 94), },
            {  2, ldexp(49517601571415211., - 95), },
            {  3, ldexp(54390733528642804., -133), },
            {  4, ldexp(71805402319113924., -157), },
        };
        
        for (u64 i = 0; i < ArrayCount(test); ++i)
            TestFloat(test[i].f, Str(0)/*test[i].str*/);
    }
    
    TEST("Log")
    {
        StringList logs = {0};
        LogBegin();
        i32 startLine = __LINE__;
        {
            LogPush(LOG_TRACE, "Log #%d", LOG_TRACE);
            LogPush(LOG_DEBUG, "Log #%d", LOG_DEBUG);
            LogPush(LOG_INFO , "Log #%d", LOG_INFO);
            LogPush(LOG_WARN , "Log #%d", LOG_WARN);
            LogPush(LOG_ERROR, "Log #%d", LOG_ERROR);
            LogPush(LOG_FATAL, "Log #%d", LOG_FATAL);
            
            LogPush(LOG_TRACE, "Log trace");
            LogPush(LOG_DEBUG, "Log debug");
            LogPush(LOG_INFO , "Log info");
            LogPush(LOG_WARN , "Log warn");
            LogPush(LOG_ERROR, "Log error");
            LogPush(LOG_FATAL, "Log fatal");
            
            LogBlock(arena, logs, .callback = TestLogCallback)
            {
                for (u64 i = 0; i < 1024; ++i)
                    LogPush(i % LogType_Count, "Log: %llu", i);
                
                LogBlock(arena, logs)
                {
                    LogPush(0, "Inner Log\n");
                    for (u64 i = 0; i < 1024; ++i)
                        LogPush(i % LogType_Count, "New Log: %llu", i);
                }
                
                TestResult(snapshot == 1024);
                TestResult(logs.nodeCount == 1025);
            }
            
            TestResult(logs.nodeCount == 1024);
        }
        i32 endLine = __LINE__;
        
        Logger logger = LogEnd(arena);
        TestResult(logger.count == 12);
        for (u64 i = 0; i < logger.count; ++i)
        {
            Record record = logger.records[i];
            TestResult(CompareMem(record.file, __FILE__, sizeof(__FILE__)));
            TestResult(InRange(record.line, startLine, endLine) && InRange(record.level, 0, LogType_Count - 1));
        }
    }
    
    TEST("Buffer")
    {
        u64 size = KB(64);
        u16* buffer = PushArrayNZ(arena, u16, size);
        for (u64 i = 0; i < size; ++i) // can't use u16 because of wrap around
            buffer[i] = (u16)i;
        
#define LANE_COUNT 4
        u16* lanes[LANE_COUNT];
        u64 count = size / LANE_COUNT;
        for (u64 i = 0; i < LANE_COUNT; ++i)
            lanes[i] = buffer + i * count;
        
        TestResult(lanes[0] - buffer == 0);
        TestResult(lanes[1] - lanes[0] == count);
        TestResult(lanes[2] - lanes[1] == count);
        TestResult(lanes[3] - lanes[2] == count);
        
        u64 elementSize = 2;
        String  result1 =   BufferInterleave(arena, (void**)lanes, LANE_COUNT, elementSize, count);
        String* result2 = BufferUninterleave(arena,        buffer, LANE_COUNT, elementSize, count);
        
        String* result3 = BufferUninterleave(arena, result1.str, LANE_COUNT, elementSize, count);
        for (u64 i = 0; i < LANE_COUNT; ++i)
            lanes[i] = (u16*)result3[i].str;
        String result4 = BufferInterleave(arena, (void**)lanes, LANE_COUNT, elementSize, count);
        
        TestResult(result1.size / elementSize == size);
        TestResult(result4.size / elementSize == size);
        for (u64 i = 0; i < LANE_COUNT; ++i)
        {
            TestResult(result2[i].size / elementSize == count);
            TestResult(result3[i].size / elementSize == count);
        }
        
        // result1
        {
            b32 success = 0;
            u16* ptr = (u16*)result1.str;
            for (u64 i = 0; i < count; ++i)
            {
                for (u64 lane = 0; lane < LANE_COUNT; ++lane)
                {
                    success = ptr[i * LANE_COUNT + lane] == buffer[lane * count + i];
                    if (!success)
                        break;
                }
            }
            TestResult(success);
        }
        
        // result2
        for (u64 lane = 0; lane < LANE_COUNT; ++lane)
        {
            u16* ptr = (u16*)result2[lane].str;
            b32 success = 0;
            
            for (u64 i = 0; i < count; ++i)
            {
                success = ptr[i] == buffer[i * LANE_COUNT + lane];
                if (!success)
                    break;
            }
            
            TestResult(success);
        }
        
        // result3/4
        TestResult(CompareMem(result4.str, result1.str, count));
        for (u64 lane = 0; lane < LANE_COUNT; ++lane)
        {
            u16* ptr = (u16*)result3[lane].str;
            TestResult(CompareMem(ptr, buffer + count * lane, count));
        }
#undef LANE_COUNT
    }
    
    TEST("OS")
    {
        {
#define FileName(name) StrLit("code/examples/"name)
            String fileData = OSReadFile(arena, FileName("Test.txt"), true);
            TestResult(StrCompare(fileData, StrLit("This is a test file!"), 0));
            TestResult(OSWriteFile(FileName("Test2.txt"), fileData));
            TestResult(StrCompare(OSReadFile(arena, FileName("Test2.txt"), 0), fileData, 0));
            TestResult(OSDeleteFile(FileName("Test2.txt")));
            
            FileProperties file = OSFileProperties(FileName("Test.txt"));
            OSRenameFile(FileName("Test.txt"), FileName("Test3.txt"));
            TestResult(OSReadFile(arena, FileName("Test.txt"), 0).size == 0);
            {
                FileProperties _file = OSFileProperties(FileName("Test3.txt"));
                TestResult(CompareMem(&file, &_file, sizeof(FileProperties)));
            }
            OSRenameFile(FileName("Test3.txt"), FileName("Test.txt"));
            TestResult(OSReadFile(arena, FileName("Test3.txt"), 0).size == 0);
#undef FileName
        }
        
        {
            u64 nameLen = sizeof("\\code\\examples\\test_base.c") - 1;
            String currentDir = OSGetCurrDir(arena);
            TestResult(StrCompare(currentDir, StrChop(StrLit(__FILE__), nameLen), 0));
            TestResult(StrCompare(OSGetExeDir(), StrJoin3(arena, currentDir, StrLit("\\build")), 0));
            TestResult(StrCompare(OSGetUserDir(), StrLit("C:\\Users\\ADMIN"), 1));
            TestResult(StrCompare(OSGetTempDir(), StrLit("C:\\Users\\ADMIN\\AppData\\Local\\Temp"), 1));
        }
        
        u64 entropy;
        OSGetEntropy(&entropy, sizeof(entropy));
        TestResult(entropy > 1000 && entropy != MAX_U64 && entropy != MAX_I64 && entropy != MAX_U32 && entropy != MAX_I32);
        
        StringList logs = {0};
        LogBlock(arena, logs)
        {
            OSLib testLib = OSLoadLib(StrLit("build/TestDLL.dll"));
            i32 (*init)(VoidFunc*, b32);
            PrcCast(init, OSGetProc(testLib, "DLLCallback"));
            snapshot = 0;
            TestResult(init(IncSnapshot, 0) == 10 && snapshot == 1);
            u32(*func)(u32*, u64);
            PrcCast(func, OSGetProc(testLib, "Sum"));
            u32* array = (u32*)&entropy;
            TestResult(func(array, 2) == array[0] + array[1]);
            i32* dllVar = (i32*)OSGetProc(testLib, "globalInt");
            TestResult(*dllVar == 10);
            *dllVar = 100;
            TestResult(init(IncSnapshot, 0) == 100 && snapshot == 2);
            OSFreeLib(testLib);
        }
        TestResult(logs.nodeCount == 2);
    }
    
    TEST("Timer")
    {
#if 0
        for (u64 i = 0; i < 15; ++i)
        {
            u64 time = OSNowMS();
            DateTime dt = OSNowUniTime();
            
            RNG rng;
            OSGetEntropy(&rng, sizeof(rng));
            i64 sleep = RandomRangeI32(rng, 20, 200);
            OSSleepMS((u32)sleep);
            u64 now = OSNowMS();
            DateTime dtNow = OSNowUniTime();
            TestResult(AbsI64((i64)(now - time) - sleep) <= ClampBot(sleep / 20, 4));
            TestResult(dtNow.min == dt.min && dtNow.hour == dt.hour &&
                       dtNow.day == dt.day && dtNow.mon == dt.mon && dtNow.year == dt.year);
        }
#endif
        
        DateTime dates[] =
        {
            { .min = 24, .hour = 10, .day = 31, .mon =  1, .year = 2024, },
            { .min =  0, .hour =  0, .day = 30, .mon =  4, .year = 2023, },
            { .min = 30, .hour = 12, .day = 29, .mon = 12, .year = 2010, },
            { .min =  5, .hour =  5, .day =  3, .mon =  5, .year = 1992, },
            { .min = 59, .hour = 15, .day =  1, .mon =  4, .year = 2003, },
            { .min =  0, .hour =  0, .day =  1, .mon =  1, .year = 2000, },
            { .min = 59, .hour = 23, .day = 31, .mon = 12, .year = 1999, },
            { .min =  0, .hour =  0, .day = 29, .mon = 04, .year = 2003, },
            { .min =  0, .hour =  0, .day = 10, .mon = 01, .year = 2005, },
            { .min =  0, .hour =  0, .day = 11, .mon = 03, .year = 2009, },
        };
        
        String strings[] =
        {
            StrLit("31/01/2024 10:24 AM"),
            StrLit("30/04/2023 12:00 AM"),
            StrLit("29/12/2010 12:30 PM"),
            StrLit("03/05/1992 05:05 AM"),
            StrLit("01/04/2003 03:59 PM"),
            StrLit("01/01/2000 12:00 AM"),
            StrLit("31/12/1999 11:59 PM"),
            StrLit("29/04/2003 12:00 AM"),
            StrLit("10/01/2005 12:00 AM"),
            StrLit("11/03/2009 12:00 AM"),
        };
        
        StaticAssert(ArrayCount(dates) == ArrayCount(strings));
        for (u64 i = 0; i < ArrayCount(dates); ++i)
        {
            String str = StrFromTime(arena, dates[i]);
            TestResult(StrCompare(str, strings[i], 0));
        }
    }
    
    TEST("File Iter")
    {
        StringList exts = {0};
        StrListPush(arena, &exts, StrLit("c"));
        StrListPush(arena, &exts, StrLit("h"));
        StrListPush(arena, &exts, StrLit("txt"));
        StrListPush(arena, &exts, StrLit("mdesk"));
        
        FileIterBlock(arena, iter, StrLit("code"))
        {
            TestResult(iter.props.modifyTime >= iter.props.createTime);
            if (iter.props.flags & FilePropertyFlag_IsFolder)
                TestResult(!StrContainsChr(iter.name, ".\\/"));
            else
                TestResult(StrCompareList(StrSkipUntil(iter.name, StrLit("."), FindStr_LastMatch), &exts, 0));
        }
        
        String txt = StrListPop(&exts);
        StringList names = StrList(arena, ArrayExpand(String, StrLit("md.h"), StrLit("md.c"), StrLit("md_stb_sprintf.h")));
        
        FileIterBlock(arena, iter, StrLit("code\\dependencies\\md"))
        {
            String ext = StrSkipUntil(iter.name, StrLit("."), FindStr_LastMatch);
            TestResult(StrCompareList(ext, &exts, 0) && !StrCompare(ext, txt, 0));
            TestResult(StrCompareList(iter.name, &names, 0) &&
                       iter.props.modifyTime >= iter.props.createTime && !(iter.props.flags & FilePropertyFlag_IsFolder));
        }
    }
    
    TEST("Arena")
    {
        ScratchBlock(scratch, arena)
            TestResult(scratch != arena);
        
        ArenaStack(stackArena, 1024);
        TestResult(stackArena && stackArena->pos == ArenaMinSize(stackArena) && !stackArena->growing);
        u8* stackPtr = ArenaPushNZ(stackArena, 1024);
        TestResult(stackArena->pos == stackArena->cap && stackArena->commitPos == stackArena->cap);
        TestResult(stackPtr - (u8*)stackArena == sizeof(Arena)); 
        
        ArenaPopTo(arena, 0);
        TestResult(arena->cap == MEM_DEFAULT_RESERVE_SIZE);
        TestResult(arena->pos == ArenaMinSize(arena));
        TestResult(arena->alignment == MEM_DEFAULT_ALIGNMENT);
        TestResult(arena->commitPos == MEM_COMMIT_BLOCK_SIZE);
        
        u64 minPos = arena->pos;
        u64 bigCount = 1000;
        TempBlock(temp, arena)
        {
            i64* array = PushArray(arena, i64, bigCount);
            i64 arr[10] = {0};
            TestResult(CompareMem(arr, array, 10));
            TestResult(arena->pos == temp.pos + bigCount * sizeof(u64));
            TestResult(temp.pos == minPos);
        }
        TestResult(arena->pos == minPos);
        
        u8* buffer1 = 0;
        u8* buffer2 = 0;
        TempBlock(temp, arena)
        {
            buffer1 = ArenaPushPoison(arena, bigCount);
            buffer2 =   ArenaPush(arena, bigCount);
            TestResult(buffer2 == (buffer1 + bigCount + MEM_POISON_SIZE));
            u8* ptr = PtrAdd(arena, AlignUpPow2(temp.pos, MEM_POISON_ALIGNMENT));
            ptr += AlignUpPow2(bigCount, MEM_POISON_ALIGNMENT) + MEM_POISON_SIZE * 2;
            TestResult(buffer2 == ptr);
            
#if ENABLE_SANITIZER
            TestResult(AsanIsPoison(buffer2 + bigCount, 1));
            TestResult(AsanIsPoison(buffer1 + bigCount, 1));
#endif
        }
        
#if ENABLE_SANITIZER
        TestResult(AsanIsPoison(buffer1, buffer2 + bigCount - buffer1));
#endif
        
        buffer1 = ArenaPush(arena, bigCount);
        i64 offset = 0;
        u64 smallCount = 50;
        TempPoisonBlock(temp, arena)
        {
            u8* aPtr = ArenaPush(arena, smallCount);
            u8* bPtr = ArenaPush(arena, smallCount);
            u8* cPtr = ArenaPush(arena, smallCount);
            u8* dPtr = ArenaPush(arena, smallCount);
            
            offset = dPtr - (u8*)arena;
            i64 start = AlignUpPow2(buffer1 - (u8*)arena + bigCount, MEM_POISON_ALIGNMENT) + MEM_POISON_SIZE;
            TestResult(start == (i64)temp.pos);
            
            TestResult(aPtr - (u8*)arena == start);
            start += AlignUpPow2(smallCount, arena->alignment);
            TestResult(bPtr - (u8*)arena == start);
            start += AlignUpPow2(smallCount, arena->alignment);
            TestResult(cPtr - (u8*)arena == start);
            start += AlignUpPow2(smallCount, arena->alignment);
            TestResult(dPtr - (u8*)arena == start);
        }
        
        buffer2 = ArenaPush(arena, bigCount);
        offset = AlignUpPow2(offset + smallCount, MEM_POISON_ALIGNMENT);
        TestResult(buffer2 - (u8*)arena == offset + MEM_POISON_SIZE);
        
#if ENABLE_SANITIZER
        u64 cap = arena->cap;
        ArenaRelease(arena);
        TestResult(AsanIsPoison(arena, cap));
#endif
    }
    
    elapsed = OSNowMS() - elapsed;
    Outf("\nTime Elapsed: %llu ms\n", elapsed);
    return 0;
}
MSVC(WarnEnable(28182 6011 4723))
