#include "DefaultMemory.h"
#include "Base.h"
#include "LongOS.h"
#include "Base.c"
#include "LongOS_Win32.c"
#include <stdio.h>

#define EvalPrint(x)    printf("%s = %d\n", #x, (int)(x))
#define EvalPrintLL(x)  printf("%s = %lld\n", #x, (x))
#define EvalPrintU(x)   printf("%s = %u\n", #x, (u32)(x))
#define EvalPrintULL(x) printf("%s = %llu\n", #x, (x))
#define EvalPrintF(x)   printf("%s = %f\n", #x, (x))
#define EvalPrintC(x)   printf("%s = %c\n", #x, (x))
#define EvalPrintS(x)   printf("%s = %s\n", #x, (x))
#define EvalPrintStr(x) do { String __str__ = (x); printf("%s = %.*s\n", #x, StrExpand(__str__)); } while (0)
#define EvalPrintPtr(x) printf("%s = %p\n", #x, (x))
#define EvalPrintLine   printf("\n")

#define CODE_PATH 1

#if CODE_PATH == 0
typedef struct Test
{
    int a;
    int b;
    int c;
    int d;
} Test;

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
        record->log = StrPushf(scratch, "%.*s, arena: %lld", StrExpand(record->log), arena->pos);
        record->log = StrCopy(arena, record->log);
    }
}

libexport void PrintSnapshot()
{
    printf("Snapshot: %d\n", snapshot++);
}

int main(void)
{
    {
        i64 dx = 0x77E435B08;
        while (dx)
            putchar(0x726F6C6564574820 >> (((dx >>= 3) & 7) << 3) & 0xFF);
        printf("\n");
    }
    
    i64 _b = (1LL << 23)|1;
    f32 _a = (f32)_b;
    printf("%lld %f\n", _b, _a);
    _a = 5.f;
    printf("%08x\n", *(i32*)&_a);
    
    {
        f32 x = 1.1f;
        if (x != 1.1f)
            printf("A\n");
        
        //x = 1.1;
        if (x != 1.1)
            printf("B\n");
    }
    
    printf("cl = %d\n", COMPILER_CL);
    printf("clang = %d\n", COMPILER_CLANG);
    printf("gcc = %d\n", COMPILER_GCC);
    printf("widnows = %d\n", OS_WIN);
    printf("linux = %d\n", OS_LINUX);
    printf("mac = %d\n", OS_MAC);
    printf("x64 = %d\n", ARCH_X64);
    printf("x86 = %d\n", ARCH_X86);
    printf("arm = %d\n", ARCH_ARM);
    printf("arm64 = %d\n", ARCH_ARM64);
    
    int foo[100];
    for (int i = 0; i < ArrayCount(foo); ++i)
        foo[i] = i;
    EvalPrint(ArrayCount(foo));
    
    int bar[100];
    CopyFixedArr(bar, foo);
    EvalPrint(bar[50]);
    EvalPrint(CompareMem(foo, bar, sizeof(foo)));
    
    EvalPrint(OffsetOf(Test, a));
    EvalPrint(OffsetOf(Test, b));
    EvalPrint(OffsetOf(Test, d));
    
    EvalPrintLine;
    
    Test t = { 1, 2, 3, 4 };
    EvalPrint(t.a);
    EvalPrint(t.c);
    ZeroStruct(&t);
    EvalPrint(t.a);
    EvalPrint(t.c);
    
    EvalPrintLine;
    
    EvalPrint(Min(1, 100));
    EvalPrint(Min(100, 20));
    EvalPrint(Max(1, 40));
    EvalPrint(Max(200, 10));
    EvalPrint(Clamp(1, 10, 100));
    EvalPrint(Clamp(80, 0, 7));
    EvalPrint(Clamp(20, 5, 30));
    
    EvalPrintLine;
    
    EvalPrint(ChrIsUpper('A'));
    EvalPrint(ChrIsUpper('c'));
    EvalPrint(ChrIsUpper(';'));
    EvalPrint(ChrIsLower('d'));
    EvalPrint(ChrIsLower('E'));
    EvalPrint(ChrIsLower('0'));
    
    EvalPrintC(ChrToUpper('A'));
    EvalPrintC(ChrToUpper('d'));
    EvalPrintC(ChrToUpper('@'));
    EvalPrintC(ChrToUpper('1'));
    EvalPrintC(ChrToLower('a'));
    EvalPrintC(ChrToLower('D'));
    EvalPrintC(ChrToLower('2'));
    EvalPrintC(ChrToLower('^'));
    
    EvalPrintLine;
    
    EvalPrint(MIN_I8 );
    EvalPrint(MIN_I16);
    EvalPrint(MIN_I32);
    EvalPrintLL(MIN_I64);
    
    EvalPrintLine;
    
    EvalPrint(MAX_I8 );
    EvalPrint(MAX_I16);
    EvalPrint(MAX_I32);
    EvalPrintLL(MAX_I64);
    
    EvalPrintLine;
    
    EvalPrintU(MAX_U8 );
    EvalPrintU(MAX_U16);
    EvalPrintU(MAX_U32);
    EvalPrintULL(MAX_U64);
    
    EvalPrintLine;
    
    EvalPrintF(EPSILON_F32);
    EvalPrintF(E_F32);
    EvalPrintF(PI_F32);
    EvalPrintF(TAU_F32);
    EvalPrintF(GOLD_BIG_F32);
    EvalPrintF(GOLD_SMALL_F32);
    
    EvalPrintLine;
    
    EvalPrintF(EPSILON_F64);
    EvalPrintF(E_F64);
    EvalPrintF(PI_F64);
    EvalPrintF(TAU_F64);
    EvalPrintF(GOLD_BIG_F64);
    EvalPrintF(GOLD_SMALL_F64);
    
    EvalPrintLine;
    
    EvalPrint(AbsI32(-3506708));
    EvalPrintLL(AbsI64(MIN_I64 + MAX_I32));
    
    EvalPrintLine;
    
    EvalPrintF(Inf_f32());
    EvalPrintF(NegInf_f32());
    EvalPrintF(Abs_f32(-2.5f));
    
    EvalPrintLine;
    
    EvalPrintF(Inf_f64());
    EvalPrintF(NegInf_f64());
    EvalPrintF(Abs_f64(-357.39460));
    
    EvalPrintLine;
    
    EvalPrintF(Lerp(.75f, 8.5f, 27.3f));
    EvalPrintF(Sqrt_f64(4.));
    EvalPrintF(Cos_f32(30.f));
    EvalPrintF(SinPi64(PI_F64));
    EvalPrintF(SinTurn32(.5f));
    EvalPrintF(SinTurn64(.25));
    
    EvalPrintLine;
    
    f32 zero_f32 = 0.f;
    f64 zero_f64 = 0.;
    
    printf("these should output 2.0\n");
    EvalPrintF(Trunc_f32(2.0f));
    EvalPrintF(Trunc_f64(2.0));
    EvalPrintF(Trunc_f32(2.3f));
    EvalPrintF(Trunc_f64(2.3));
    EvalPrintF(Trunc_f32(2.5f));
    EvalPrintF(Trunc_f64(2.5));
    EvalPrintF(Trunc_f32(2.6f));
    EvalPrintF(Trunc_f64(2.6));
    printf("these should output -2.0\n");
    EvalPrintF(Trunc_f32(-2.0f));
    EvalPrintF(Trunc_f64(-2.0));
    EvalPrintF(Trunc_f32(-2.3f));
    EvalPrintF(Trunc_f64(-2.3));
    EvalPrintF(Trunc_f32(-2.5f));
    EvalPrintF(Trunc_f64(-2.5));
    EvalPrintF(Trunc_f32(-2.6f));
    EvalPrintF(Trunc_f64(-2.6));
    printf("special values\n");
    EvalPrintF(Trunc_f32(1.f/zero_f32));  // +inf
    EvalPrintF(Trunc_f64(1./zero_f64));  // +inf
    EvalPrintF(Trunc_f32(-1.f/zero_f32)); // -inf
    EvalPrintF(Trunc_f64(-1./zero_f64)); // -inf
    EvalPrintF(Trunc_f32(0.f/zero_f32));  // nan
    EvalPrintF(Trunc_f64(0./zero_f64));  // nan
    
    EvalPrintLine;
    
    printf("these should output 2.0\n");
    EvalPrintF(Floor_f32(2.0f));
    EvalPrintF(Floor_f64(2.0));
    EvalPrintF(Floor_f32(2.3f));
    EvalPrintF(Floor_f64(2.3));
    EvalPrintF(Floor_f32(2.5f));
    EvalPrintF(Floor_f64(2.5));
    EvalPrintF(Floor_f32(2.6f));
    EvalPrintF(Floor_f64(2.6));
    printf("these should output -2.0\n");
    EvalPrintF(Floor_f32(-2.0f));
    EvalPrintF(Floor_f64(-2.0));
    printf("these should output -3.0\n");
    EvalPrintF(Floor_f32(-2.3f));
    EvalPrintF(Floor_f64(-2.3));
    EvalPrintF(Floor_f32(-2.5f));
    EvalPrintF(Floor_f64(-2.5));
    EvalPrintF(Floor_f32(-2.6f));
    EvalPrintF(Floor_f64(-2.6));
    printf("special values\n");
    EvalPrintF(Floor_f32(1.f/zero_f32));  // +inf
    EvalPrintF(Floor_f64(1./zero_f64));  // +inf
    EvalPrintF(Floor_f32(-1.f/zero_f32)); // -inf
    EvalPrintF(Floor_f64(-1./zero_f64)); // -inf
    EvalPrintF(Floor_f32(0.f/zero_f32));  // nan
    EvalPrintF(Floor_f64(0./zero_f64));  // nan
    
    EvalPrintLine;
    
    printf("these should output 2.0\n");
    EvalPrintF(Ceil_f32(2.0f));
    EvalPrintF(Ceil_f64(2.0));
    
    printf("these should output 3.0\n");
    EvalPrintF(Ceil_f32(2.3f));
    EvalPrintF(Ceil_f64(2.3));
    EvalPrintF(Ceil_f32(2.5f));
    EvalPrintF(Ceil_f64(2.5));
    EvalPrintF(Ceil_f32(2.6f));
    EvalPrintF(Ceil_f64(2.6));
    printf("these should output -2.0\n");
    EvalPrintF(Ceil_f32(-2.0f));
    EvalPrintF(Ceil_f64(-2.0));
    EvalPrintF(Ceil_f32(-2.3f));
    EvalPrintF(Ceil_f64(-2.3));
    EvalPrintF(Ceil_f32(-2.5f));
    EvalPrintF(Ceil_f64(-2.5));
    EvalPrintF(Ceil_f32(-2.6f));
    EvalPrintF(Ceil_f64(-2.6));
    printf("special values\n");
    EvalPrintF(Ceil_f32(1.f/zero_f32));  // +inf
    EvalPrintF(Ceil_f32(-1.f/zero_f32)); // -inf
    EvalPrintF(Ceil_f32(0.f/zero_f32));  // nan
    EvalPrintF(Ceil_f64(1./zero_f64));  // +inf
    EvalPrintF(Ceil_f64(-1./zero_f64)); // -inf
    EvalPrintF(Ceil_f64(0./zero_f64));  // nan
    
    EvalPrintLine;
    
    printf("these should output +-2.0\n");
    EvalPrintF(Round_f32(2.3f));
    EvalPrintF(Round_f64(2.3));
    EvalPrintF(Round_f32(2.5f));
    EvalPrintF(Round_f64(2.5));
    EvalPrintF(Round_f32(-2.0f));
    EvalPrintF(Round_f64(-2.0));
    EvalPrintF(Round_f32(-2.3f));
    EvalPrintF(Round_f64(-2.3));
    EvalPrintF(Round_f32(-2.5f));
    EvalPrintF(Round_f64(-2.5));
    
    printf("these should output +-3.0\n");
    EvalPrintF(Round_f32(2.6f));
    EvalPrintF(Round_f64(2.6));
    EvalPrintF(Round_f32(-2.6f));
    EvalPrintF(Round_f64(-2.6));
    
    printf("special values\n");
    EvalPrintF(Round_f32(1.f/zero_f32));  // +inf
    EvalPrintF(Round_f32(-1.f/zero_f32)); // -inf
    EvalPrintF(Round_f32(0.f/zero_f32));  // nan
    EvalPrintF(Round_f64(1./zero_f64));  // +inf
    EvalPrintF(Round_f64(-1./zero_f64)); // -inf
    EvalPrintF(Round_f64(0./zero_f64));  // nan
    
    EvalPrintLine;
    
    EvalPrintF(F32FromStr(StrLit("15.75")  , 0));
    EvalPrintF(F32FromStr(StrLit("1.575E1"), 0));
    EvalPrintF(F32FromStr(StrLit("1575e-2"), 0));
    EvalPrintF(F32FromStr(StrLit("-2.5e-3"), 0));
    EvalPrintF(F32FromStr(StrLit("25E-4")  , 0));
    EvalPrintF(F32FromStr(StrLit(".0075e2"), 0));
    EvalPrintF(F32FromStr(StrLit("0.075e1"), 0));
    EvalPrintF(F32FromStr(StrLit(".075e1") , 0));
    EvalPrintF(F32FromStr(StrLit("75e-2")  , 0));
    
    EvalPrintLine;
    
    EvalPrintLL(I64FromStr(StrLit("28"), 10, 0));
    EvalPrintLL(I64FromStr(StrLit("4000000024"), 10, 0));
    EvalPrintLL(I64FromStr(StrLit("2000000022"), 10, 0));
    EvalPrintLL(I64FromStr(StrLit("4000000000"), 10, 0));
    EvalPrintLL(I64FromStr(StrLit("9000000000"), 10, 0));
    EvalPrintLL(I64FromStr(StrLit("900000000001"), 10, 0));
    EvalPrintLL(I64FromStr(StrLit("9000000000002"), 10, 0));
    EvalPrintLL(I64FromStr(StrLit("90000000000004"), 10, 0));
    
    EvalPrintLine;
    
    EvalPrintLL(I64FromStr(StrLit("24"), 8, 0));
    EvalPrintLL(I64FromStr(StrLit("4000000024"), 8, 0));
    EvalPrintLL(I64FromStr(StrLit("2000000022"), 8, 0));
    EvalPrintLL(I64FromStr(StrLit("4000000000"), 8, 0));
    EvalPrintLL(I64FromStr(StrLit("44000000000000"), 8, 0));
    EvalPrintLL(I64FromStr(StrLit("44400000000000001"), 8, 0));
    EvalPrintLL(I64FromStr(StrLit("4444000000000000002"), 8, 0));
    EvalPrintLL(I64FromStr(StrLit("4444000000000000004"), 8, 0));
    
    EvalPrintLine;
    
    EvalPrint(I64FromStr(StrLit("2a"), 16, 0) == 42);
    EvalPrint(I64FromStr(StrLit("0XA0000024"), 16, 0) == 2684354596);
    EvalPrint(I64FromStr(StrLit("20000022"), 16, 0) == 536870946);
    EvalPrint(I64FromStr(StrLit("0XA0000021"), 16, 0) == 2684354593);
    EvalPrint(I64FromStr(StrLit("8a000000000000"), 16, 0) == 38843546786070528);
    EvalPrint(I64FromStr(StrLit("8A40000000000010"), 16, 0) == -8484781697966014448);
    EvalPrint(I64FromStr(StrLit("4a44000000000020"), 16, 0) == 5351402257222991904);
    EvalPrint(I64FromStr(StrLit("8a44000000000040"), 16, 0) == -8483655798059171776);
    
    EvalPrintLine;
    
    EvalPrint(I64FromStr(StrLit("10"), 2, 0) == 2);
    EvalPrint(I64FromStr(StrLit("10000011"), 2, 0) == 131);
    EvalPrint(I64FromStr(StrLit("100000011"), 2, 0) == 259);
    EvalPrint(I64FromStr(StrLit("10010100"), 2, 0) == 148);
    EvalPrint(I64FromStr(StrLit("11111111111111111111111111111111"), 2, 0) == MAX_U32);
    EvalPrint(I64FromStr(StrLit("0000000000000000000000000000000000000000000000000000000000000000"), 2, 0) == 0);
    EvalPrint(I64FromStr(StrLit("0000000000000000000000000000000000000000000000000000000000000001"), 2, 0) == 1);
    
    EvalPrintLine;
    
    Arch a = 0;
    EvalPrintS(GetEnumName(Arch, a));
    EvalPrintS(GetEnumName(Arch, Arch_ARM));
    EvalPrintS(GetEnumName(Compiler, CURRENT_COMPILER_NUMBER));
    EvalPrintS(CURRENT_COMPILER_NAME);
    EvalPrintS(CURRENT_ARCH_NAME);
    
    Month month = 7;
    EvalPrintStr(GetEnumStr(Month, month));
    EvalPrintStr(GetEnumStr(Month, 13));
    
    EvalPrint(ArrayCount(Day_names));
    EvalPrintS(GetEnumName(Day, -1));
    
    EvalPrintLine;
    
    Arena* arena = ArenaMake();
    
    OSInit(0, 0);
    
    EvalPrintPtr(arena);
    EvalPrintU(arena->cap);
    EvalPrintU(arena->pos);
    EvalPrintU(arena->commitPos);
    
    EvalPrintLine;
    
    TempBlock(temp, arena)
    {
        u64 arrayCount = 10;
        i64* array = PushArray(arena, i64, arrayCount);
        for (u64 i = 0; i < arrayCount; ++i)
            EvalPrint(array[i]);
        for (u64 i = 0; i < arrayCount; ++i)
            array[i] = i;
        for (u64 i = 0; i < arrayCount; ++i)
            EvalPrint(array[i]);
        
        EvalPrintPtr(arena);
        EvalPrintU(arena->cap);
        EvalPrintU(arena->pos);
        EvalPrintU(arena->commitPos);
        
        EvalPrintLine;
    }
    
    EvalPrintPtr(arena);
    EvalPrintU(arena->cap);
    EvalPrintU(arena->pos);
    EvalPrintU(arena->commitPos);
    
    EvalPrintLine;
    
    EvalPrintStr(StrFromI64(arena, 0b10001, 2));
    EvalPrintStr(StrFromI64(arena, 0xFF12ACDE00000000, 16));
    EvalPrintStr(StrFromI64(arena, -0xED532200000000, 10));
    EvalPrintStr(StrFromI64(arena, 0756, 8));
    EvalPrintStr(StrFromI64(arena, 1256, 10));
    EvalPrintStr(StrFromI64(arena, -1256, 10));
    
    EvalPrintLine;
    
    EvalPrintStr(StrFromF64(arena, 0, 2));
    EvalPrintStr(StrFromF64(arena, 0b10001, 7));
    EvalPrintStr(StrFromF64(arena, 0xFF12ACDE, 6));
    EvalPrintStr(StrFromF64(arena, -38.9, 8));
    EvalPrintStr(StrFromF64(arena, .75, 9));
    EvalPrintStr(StrFromF64(arena, .75, 0));
    EvalPrintStr(StrFromF64(arena, .625, 1));
    EvalPrintStr(StrFromF64(arena, .625, 2));
    EvalPrintStr(StrFromF64(arena, 1.17549435082e-38, 8));
    EvalPrintStr(StrFromF64(arena, 1.40129846432e-45, 8));
    
    EvalPrintLine;
    
    typedef struct FloatTest
    {
        u32 prec;
        f64 f;
        String str;
    } FloatTest;
    
    const FloatTest test[] =
    {
        {3, 12345000, StrLit("1.234e+7")},
        
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
    
#define MAX_TEST_COUNT 100
    
    b32 printErr = 0;
    
    u64 elapsed = OSNowMS();
    for (u64 _ = 0; _ < MAX_TEST_COUNT; ++_)
    {
        for (u64 i = 0; i < ArrayCount(test); ++i)
        {
            TempBlock(temp, arena)
            {
                String fStr = StrFromF64(arena, test[i].f, test[i].prec);
                if (printErr)
                    if (test[i].str.size && !StrCompare(fStr, test[i].str, 0))
                        printf("Expected: %.*s, Result: %.*s\n", StrExpand(test[i].str), StrExpand(fStr));
            }
        }
    }
    elapsed = OSNowMS() - elapsed;
    elapsed /= MAX_TEST_COUNT;
    EvalPrintULL(elapsed);
    
    elapsed = OSNowMS();
    for (u64 _ = 0; _ < MAX_TEST_COUNT; ++_)
    {
        for (u64 i = 0; i < ArrayCount(test); ++i)
        {
            u8 buf[1000];
            String fStr = Str(buf, sprintf(buf, "%.*e", (i32)test[i].prec, test[i].f));
            if (printErr)
                if (test[i].str.size && !StrCompare(fStr, test[i].str, 0))
                    printf("Expected: %.*s, Result: %.*s\n", StrExpand(test[i].str), StrExpand(fStr));
        }
    }
    elapsed = OSNowMS() - elapsed;
    elapsed /= MAX_TEST_COUNT;
    EvalPrintULL(elapsed);
    
    elapsed = OSNowMS();
    for (u64 _ = 0; _ < MAX_TEST_COUNT; ++_)
    {
        for (u64 i = 0; i < ArrayCount(test); ++i)
        {
            u8 buf[1000];
            String fStr = Str(buf, stbsp_sprintf(buf, "%.*e", (i32)test[i].prec, test[i].f));
            if (printErr)
                if (test[i].str.size && !StrCompare(fStr, test[i].str, 0))
                    printf("Expected: %.*s, Result: %.*s\n", StrExpand(test[i].str), StrExpand(fStr));
        }
    }
    elapsed = OSNowMS() - elapsed;
    elapsed /= MAX_TEST_COUNT;
    EvalPrintULL(elapsed);
    
    EvalPrintLine;
    
    String str = StrLit("Here's a simple test!");
    EvalPrintStr(str);
    EvalPrint(ChrCompare('m', 'M', 1));
    EvalPrint(ChrCompareArr('M', str, 1));
    EvalPrint(ChrCompareArr('M', str, 0));
    EvalPrint(StrCompare(str, StrLit("Here's a simple test!"), 0));
    EvalPrint(StrCompare(str, StrLit("Here's a simple tesT!"), 1));
    EvalPrint(StrContainsChr(str, "!sa"));
    
    EvalPrintLine;
    
    String prefix = StrPrefix(str, str.size / 2);
    EvalPrintStr(prefix);
    EvalPrintStr(StrSkip(str, str.size / 2));
    EvalPrintStr(StrChop(str, 5));
    EvalPrintStr(StrPostfix(str, 2));
    EvalPrintStr(Substr(str, 2, 8));
    
    EvalPrintLine;
    
    StringList list = StrSplitArr(arena, str, StrLit(" '"), 0);
    StrListPush(arena, &list, StrLit("Insert string"));
    for (StringNode* node = list.first; node; node = node->next)
        EvalPrintStr(node->string);
    
    EvalPrintStr(SubstrSplit(str, list.first->next->next->next->string).pre);
    EvalPrintStr(SubstrSplit(str, list.first->next->next->string).post);
    
    String newStr = StrJoin(arena, &list, .pre = StrLit("a"), .mid = StrLit("mc"), .post = StrLit(" mm "));
    EvalPrintStr(newStr);
    EvalPrintS(newStr.str);
    
    EvalPrintLine;
    
    String word = StrLit("Word");
    String data = StrPushf(arena, "%.*s %.*s!\nThe date is %d", StrExpand(StrLit("Hello")), StrExpand(word), 25);
    EvalPrintStr(data);
    
    /*EvalPrintStr(SkipStrUntil(StrLit("Explain! to me this!"), StrLit("!"), 0));
    EvalPrintStr(SkipStrUntil(StrLit("Explain to me this!"), StrLit("!"), StringMatchFlag_Inclusive));
    EvalPrintStr(StrChopAfter(StrLit("This's a greate library!"), StrLit("'"), StringMatchFlag_Inclusive));
    EvalPrintStr(StrChopAfter(StrLit("This's a greate library!"), StrLit("'"), 0));*/
    
    EvalPrintLine;
    
    String fileData = OSReadFile(arena, StrLit("data/Test.txt"), true);
    EvalPrintS(fileData.str);
    EvalPrint(OSWriteFile(StrLit("data/Test2.txt"), data));
    EvalPrintStr(data);
    
    EvalPrintStr(OSCurrentDir(arena));
    EvalPrintStr(OSProcessDir());
    EvalPrintStr(OSAppDataDir());
    EvalPrintStr(OSAppTempDir());
    
    EvalPrintLine;
    
    {
        String currentName;
        FileProperties currentProp;
        OSFileIter iter = FileIterInit(StrLit("code"));
        while(FileIterNext(arena, &iter, &currentName, &currentProp))
        {
            EvalPrintStr(currentName);
            EvalPrintULL(currentProp.createTime);
            EvalPrintULL(currentProp.modifyTime);
            EvalPrint(TimeToDate(currentProp.modifyTime).year);
        }
    }
    
    EvalPrintLine;
    
    u64 entropy;
    OSGetEntropy(&entropy, sizeof(entropy));
    EvalPrintULL(entropy);
    
    EvalPrintLine;
    
    OSLib testLib = OSLoadLib(StrLit("build/TestDLL.dll"));
    EvalPrintPtr(testLib.v);
    VoidFunc* init = OSGetProc(testLib, "DLLInit");
    init();
    u32(*func)(u32*, u64);
    PrcCast(func, OSGetProc(testLib, "Sum"));
    EvalPrintU(func((u32*)&entropy, 2));
    i32* dllVar = (i32*)OSGetProc(testLib, "globalInt");
    EvalPrint(*dllVar);
    init();
    
    OSRenameFile(StrLit("build/TestDLL.dll"), StrLit("build/NewTestDLL.dll"));
    
    FileProperties file = GetFileProperties(StrLit("build/NewTestDLL.dll"));
    EvalPrintULL(file.createTime);
    EvalPrintULL(file.modifyTime);
    EvalPrint(TimeToDate(file.modifyTime).year);
    
    EvalPrintLine;
    
    LogBegin(arena, .callback = LogFmtANSIColor);
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
        
        Logger logger = LogEnd();
        StringList errors = StrListFromLogger(arena, &logger);
        for (StringNode* node = errors.first; node; node = node->next)
            printf("%.*s\n", StrExpand(node->string));
    }
    
    EvalPrintLine;
    
    EvalPrintULL(OSNowMS());
    DateTime now = OSNowUniTime();
    EvalPrint(now.msec);
    EvalPrint(OSToLocTime(&now).msec);
    
    EvalPrintU(arena->highWaterMark);
    ArenaRelease(arena);
    
    return 0;
}

#elif CODE_PATH == 1
#include "LongGFX.h"
#include "LongGFX_Win32.c"
#include "LongGFX_OpenGL.h"
#include "Win32_OpenGL.c"
#include "LongGFX_D3D11.h"
#include "Win32_D3D11.c"

enum
{
	Renderer_GL,
	Renderer_D3D,
};

function void WindowResizeHandler(GFXWindow window, u32 width, u32 height)
{
    UNUSED(width);
    UNUSED(height);
    
	u32 renderer = (u32)IntFromPtr(GFXWindowGetUserData(window));
	switch (renderer)
	{
		case Renderer_GL:
		{
			BeginGLRender(window);
			
            GFXFlags flags = GFXWindowGetFlags(window);
			if (flags & FLAG_WINDOW_MAXIMIZED)
				glClearColor(0.f, 1.f, 1.f, 1.f);
			else if (flags & FLAG_WINDOW_MINIMIZED)
				glClearColor(1.f, 0.f, 1.f, 1.f);
			else if (flags & FLAG_MODE_FULLSCREEN)
				glClearColor(1.f, 1.f, 0.f, 1.f);
			else if (window % 2)
                glClearColor(0, 0, 0, 1);
            else
				glClearColor(1.0f, 1.0f, 1.0f, 1.f);
			
            GLEnum error = glGetError();
            
			glClear(GL_COLOR_BUFFER_BIT);
            
            GLFloat flts[4];
            glGetFloatv(GL_VIEWPORT, flts);
            
            error = glGetError();
            glBegin(GL_TRIANGLES);
            glColor3f (1, 1, 1);
            glVertex3f(-1, -1, 0);
            glVertex3f(1, -1, 0);
            glVertex3f(0, 1, 0);
            glEnd();
            error = glGetError();
            
            error = glGetError();
            
			EndGLRender();
		} break;
        
		case Renderer_D3D:
		{
			ID3D11RenderTargetView* view = BeginD3D11Render(window);
			ID3D11DeviceContext_ClearRenderTargetView(ctx, view, MidnightBlue);
			ID3D11DeviceContext_OMSetRenderTargets(ctx, 1, &view, 0);
			EndD3D11Render(view);
		} break;
	}
}

#define TEST_WINDOW_COUNT 6

int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd)
{
    W32WinMainInit(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    
    ScratchBlock(scratch)
    {
        GFXErrorBlock(scratch, 1, .callback = GFXErrorFmt)
        {
            GFXInit();
            InitGL();
            //InitD3D11();
            
            //ErrorFmt("Test Error 1");
            //ErrorFmt("Test Error 2");
        }
        
        GFXSetResizeFunc(WindowResizeHandler);
        
        GFXWindow windows[TEST_WINDOW_COUNT] = {0};
        for (u32 i = 0; i < TEST_WINDOW_COUNT; ++i)
        {
            i32 w = 400, h = 200;
            i32 isGL = /*i % 2*/0;
            
            w = w * (i + 1);
            h = h * (i + 1);
            
            GFXErrorBlock(scratch, 1)
            {
                windows[i] = GFXCreateWindowEx(StrPushf(scratch, "Window: %d", i), CW_USEDEFAULT, CW_USEDEFAULT, w, h);
                if (isGL == 0)
                    EquipGLWindow(windows[i]);
                else
                    EquipD3D11Window(windows[i]);
            }
            
            GFXWindowSetUserData(windows[i], PtrFromInt(isGL));
            GFXShowWindow(windows[i]);
            
            if (i == 3)
            {
                i32 x, y;
                GFXWindowGetInnerRect(windows[i-1], &x, &y, &w, &h);
                GFXWindowSetOuterRect(windows[i], x, y, w, h);
            }
        }
        
        u32 count = 0;
        for (TempArena temp = TempBegin(scratch); ; TempEnd(temp))
        {
            if (!GFXPeekInput())
                break;
            
            //OutputDebugString(StrPushf(scratch, "Count: %d\n", count).str);
            count++;
#define TEST_TOGGLE_TIMER 150
            if (count % TEST_TOGGLE_TIMER == 0)
            {
#define TEST_VISIBILITY 0
#define TEST_OUTERINNER 0
#define TEST_FULLSCREEN 0
                
#define TEST_RESIZABLE 0
#define TEST_UNFOCUSED 0
#define TEST_MINMAX    0
                
                if (TEST_VISIBILITY)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-1];
                    if (GFXWindowIsValid(window))
                    {
                        b32 visible = count % (TEST_TOGGLE_TIMER * 2);
                        GFXWindowSetFlags(window, FLAG_WINDOW_HIDDEN, !visible);
                        if (visible)
                            GFXWindowSetTitle(window, StrPushf(scratch, "Toggle Windows: %d", count / 100));
                        visible = !visible;
                    }
                }
                
                if (TEST_OUTERINNER)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-2];
                    if (GFXWindowIsValid(window))
                    {
                        i32 x, y, w, h;
                        if (count % (TEST_TOGGLE_TIMER * 2))
                        {
                            i32 innerX, innerY, innerW, innerH;
                            
                            GFXWindowGetOuterRect(window, &x, &y, &w, &h);
                            GFXWindowGetInnerRect(window, &innerX, &innerY, &innerW, &innerH);
                            
                            GFXWindowSetInnerRect(window, x, y, w, h);
                            GFXWindowGetInnerRect(window, &innerX, &innerY, &innerW, &innerH);
                        }
                        else
                        {
                            GFXWindowGetInnerRect(window, &x, &y, &w, &h);
                            GFXWindowSetOuterRect(window, x, y, w, h);
                        }
                    }
                }
                
                if (TEST_FULLSCREEN)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-3];
                    if (GFXWindowIsValid(window))
                    {
                        if (count % (TEST_TOGGLE_TIMER * 2))
                        {
                            GFXWindowSetFlags(window, FLAG_WINDOW_MAXIMIZED, 0);
                            GFXWindowSetFlags(window, FLAG_MODE_FULLSCREEN, 1);
                        }
                        else
                        {
                            GFXWindowSetFlags(window, FLAG_MODE_FULLSCREEN, 0);
                            GFXWindowSetFlags(window, FLAG_WINDOW_MAXIMIZED, 1);
                        }
                    }
                }
                
                if (TEST_RESIZABLE)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-4];
                    if (GFXWindowIsValid(window))
                        GFXWindowSetFlags(window, FLAG_WINDOW_RESIZABLE, count % (TEST_TOGGLE_TIMER * 2));
                }
                
                if (TEST_MINMAX)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-5];
                    if (GFXWindowIsValid(window))
                    {
                        u32 loop = (count % (TEST_TOGGLE_TIMER * 4)) / TEST_TOGGLE_TIMER;
                        switch (loop)
                        {
                            case 2:
                            case 0: GFXWindowSetFlags(window, FLAG_WINDOW_MINIMIZED|FLAG_WINDOW_MAXIMIZED, 0); break;
                            case 1: GFXWindowSetFlags(window, FLAG_WINDOW_MAXIMIZED, 1); break;
                            case 3: GFXWindowSetFlags(window, FLAG_WINDOW_MINIMIZED, 1); break;
                        }
                    }
                }
                
                if (TEST_UNFOCUSED)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-6];
                    if (GFXWindowIsValid(window))
                        GFXWindowSetFlags(window, FLAG_WINDOW_UNFOCUSED, count % (TEST_TOGGLE_TIMER * 2) == 0);
                }
            }
            
            u32 activeWindowCount = 0;
            for (u32 i = 0; i < TEST_WINDOW_COUNT; ++i)
            {
                if (GFXWindowIsValid(windows[i]))
                {
                    activeWindowCount++;
                    WindowResizeHandler(windows[i], 0, 0);
                    /*if (IsGFXWindowMinimized(windows[i]))
                    GFXCloseWindow(windows[i]);*/
                }
            }
            
            if (!activeWindowCount)
                break;
        }
        
        GFXErrorBlock(scratch, 1)
        {
            //FreeD3D11();
            FreeGL();
            InitGL();
            FreeGL();
        }
    }
    
	return 0;
}
#elif CODE_PATH == 2

#define MD_DEFAULT_SPRINTF 0
#define MD_IMPL_Vsnprintf stbsp_vsnprintf
#include "md\md.h"
#include "md\md.c"

#define C_LIKE_OPS_NO_SIDE_EFFECTS(X) \
    X(ArraySubscript,      "[]",        Postfix,                18) \
    X(Member,              ".",         Binary,                 18) \
    X(PointerMember,       "->",        Binary,                 18) \
    X(UnaryPlus,           "+",         Prefix,                 17) \
    X(UnaryMinus,          "-",         Prefix,                 17) \
    X(LogicalNot,          "!",         Prefix,                 17) \
    X(BitwiseNot,          "~",         Prefix,                 17) \
    X(Dereference,         "*",         Prefix,                 17) \
    X(AddressOf,           "&",         Prefix,                 17) \
    X(SizeOf,              "sizeof",    Prefix,                 17) \
    X(Multiplication,      "*",         Binary,                 15) \
    X(Division,            "/",         Binary,                 15) \
    X(Modulo,              "%",         Binary,                 15) \
    X(Addition,            "+",         Binary,                 14) \
    X(Subtraction,         "-",         Binary,                 14) \
    X(LeftShift,           "<<",        Binary,                 13) \
    X(RightShift,          ">>",        Binary,                 13) \
    X(LessThan,            "<",         Binary,                 11) \
    X(LessThanOrEqual,     "<=",        Binary,                 11) \
    X(GreaterThan,         ">",         Binary,                 11) \
    X(GreaterThanOrEqual,  ">=",        Binary,                 11) \
    X(Equal,               "==",        Binary,                 10) \
    X(NotEqual,            "!=",        Binary,                 10) \
    X(BitwiseAnd,          "&",         Binary,                  9) \
    X(BitwiseXor,          "^",         Binary,                  8) \
    X(BitwiseOr,           "|",         Binary,                  7) \
    X(LogicalAnd,          "&&",        Binary,                  6) \
    X(LogicalOr,           "||",        Binary,                  5)

#define C_LIKE_OPS_CALLS(X) \
    X(Call,                "()",        Postfix,                18)

#define C_LIKE_OPS_WITH_SIDE_EFFECTS(X) \
    X(PostFixIncrement,    "++",        Postfix,                18) \
    X(PostFixDecrement,    "--",        Postfix,                18) \
    X(PreFixIncrement,     "++",        Prefix,                 17) \
    X(PreFixDecrement,     "--",        Prefix,                 17) \
    X(Assign,              "=",         BinaryRightAssociative,  3) \
    X(AssignAddition,      "+=",        BinaryRightAssociative,  3) \
    X(AssignSubtraction,   "-=",        BinaryRightAssociative,  3) \
    X(AssignMultiplication,"*=",        BinaryRightAssociative,  3) \
    X(AssignDivision,      "/=",        BinaryRightAssociative,  3) \
    X(AssignModulo,        "%=",        BinaryRightAssociative,  3) \
    X(AssignLeftShift,     "<<=",       BinaryRightAssociative,  3) \
    X(AssignRightShift,    ">>=",       BinaryRightAssociative,  3) \
    X(AssignBitwiseAnd,    "&=",        BinaryRightAssociative,  3) \
    X(AssignBitwiseXor,    "^=",        BinaryRightAssociative,  3) \
    X(AssignBitwiseOr,     "|=",        BinaryRightAssociative,  3)

enum Op
{
#define DEF_ENUM(e,t,k,p) Op##e,
    C_LIKE_OPS_NO_SIDE_EFFECTS(DEF_ENUM)
        C_LIKE_OPS_CALLS(DEF_ENUM)
        C_LIKE_OPS_WITH_SIDE_EFFECTS(DEF_ENUM)
#undef DEF_ENUM
};

void print_expression(Arena* arena, StringList* list, MD_Expr *expr)
{
    MD_ExprOpr *op = expr->op;
    if (op == 0)
    {
        MD_Node *node = expr->md_node;
        if (node->raw_string.size != 0)
        {
            StrListPushf(arena, list, "%.*s", MD_S8VArg(node->raw_string));
        }
        else if (!MD_NodeIsNil(node->first_child))
        {
            char c1 = 0;
            char c2 = 0;
            
            if (node->flags & MD_NodeFlag_HasParenLeft  ) c1 = '(';
            if (node->flags & MD_NodeFlag_HasBraceLeft  ) c1 = '{';
            if (node->flags & MD_NodeFlag_HasBracketLeft) c1 = '[';
            
            if (node->flags & MD_NodeFlag_HasParenRight  ) c2 = ')';
            if (node->flags & MD_NodeFlag_HasBraceRight  ) c2 = '}';
            if (node->flags & MD_NodeFlag_HasBracketRight) c2 = ']';
            
            StrListPushf(arena, list, "%c...%c", c1, c2);
        }
        else
        {
            MD_CodeLoc loc = MD_CodeLocFromNode(node);
            MD_PrintMessage(stderr, loc, MD_MessageKind_Error,
                            MD_S8Lit("the expression system does not expect this kind of node"));
        }
    }
    else
    {
        switch (op->kind)
        {
            default:
            {
                MD_Node *node = expr->md_node;
                MD_CodeLoc loc = MD_CodeLocFromNode(node);
                MD_PrintMessage(stderr, loc, MD_MessageKind_FatalError,
                                MD_S8Lit("this is an unknown kind of operator"));
            } break;
            
            case MD_ExprOprKind_Prefix:
            {
                StrListPushf(arena, list, "%.*s(", MD_S8VArg(op->string));
                print_expression(arena, list, expr->unary_operand);
                StrListPushf(arena, list, ")");
            } break;
            
            case MD_ExprOprKind_Postfix:
            {
                StrListPushf(arena, list, "(");
                print_expression(arena, list, expr->unary_operand);
                MD_String8 op_string = op->string;
                if ((expr->md_node->flags & MD_NodeFlag_MaskSetDelimiters) != 0)
                    StrListPushf(arena, list, ")%c...%c", op_string.str[0], op_string.str[1]);
                else
                    StrListPushf(arena, list, ")%.*s", MD_S8VArg(op_string));
            } break;
            
            case MD_ExprOprKind_Binary:
            case MD_ExprOprKind_BinaryRightAssociative:
            {
                StrListPushf(arena, list, "(");
                print_expression(arena, list, expr->left);
                StrListPushf(arena, list, " %.*s ", MD_S8VArg(op->string));
                print_expression(arena, list, expr->right);
                StrListPushf(arena, list, ")");
            } break;
        }
    }
}

typedef struct Parser Parser;
struct Parser
{
    MD_Arena* arena;
    
    MD_Node* current;
    MD_Node* parent;
    
    MD_Message* error;
    MD_Node* list;
};

function b32 IsPreProcError(MD_Message* error)
{
    b32 result = 0;
    if (error)
        result = MD_S8Match(error->string, MD_S8Lit("Unexpected reserved symbol \"#\""), 0);
    return result;
}

function b32 ParserInc(Parser* parser, b32 skipPreProc)
{
    b32 result = 0;
    
    if (!MD_NodeIsNil(parser->current))
    {
        parser->current = parser->current->next;
        result = 1;
        
        if (skipPreProc)
        {
            REPEAT_1:
            if (parser->error)
            {
                if (!IsPreProcError(parser->error))
                {
                    parser->error = parser->error->next;
                    goto REPEAT_1;
                }
                else
                {
                    MD_CodeLoc procLoc = MD_CodeLocFromNode(parser->error->node);
                    
                    REPEAT_2:
                    if (!MD_NodeIsNil(parser->current))
                    {
                        MD_CodeLoc currLoc = MD_CodeLocFromNode(parser->current);
                        if (procLoc.line == currLoc.line)
                        {
                            parser->current = parser->current->next;
                            goto REPEAT_2;
                        }
                        else if (procLoc.line < currLoc.line)
                        {
                            parser->error = parser->error->next;
                            goto REPEAT_1;
                        }
                    }
                }
            }
        }
    }
    
    return result;
}

function MD_Node* GetNodeBody(MD_Node* node)
{
    MD_Node* result = 0;
    for (i32 i = 0; i < 2 && !MD_NodeIsNil(node) && !result; ++i, node = node->next)
        if (HasAllFlags(node->flags, MD_NodeFlag_HasBraceLeft|MD_NodeFlag_HasBraceRight))
            result = node;
    return result;
}

function MD_Node* GetNodeBase(MD_Node* node)
{
    return node->first_tag;
}

function MD_String8 GetNodeType(MD_Node* node, MD_Node* base)
{
    MD_String8 string = base->string;
    return string;
}

function void PrintNode(MD_Node* node)
{
    MD_ArenaTemp temp = MD_GetScratch(0, 0);
    MD_String8List stream = {0};
    MD_Node* name = node->ref_target;
    MD_DebugDumpFromNode(temp.arena, &stream, name, 0, MD_S8Lit(" "), MD_GenerateFlags_Tree|MD_GenerateFlag_NodeFlags);
    MD_String8 str = MD_S8ListJoin(temp.arena, stream, 0);
    MD_Node* base = GetNodeBase(node);
    MD_String8 type = GetNodeType(name, base);
    ScratchBegin(scratch);
    String pointers = ChrRepeat(scratch, '*', base->offset);
    fprintf(stdout, "// Base Type: %.*s%.*s\n%.*s\n\n", MD_S8VArg(type), StrExpand(pointers), MD_S8VArg(str));
    MD_ReleaseScratch(temp);
    ScratchEnd(scratch);
}

function MD_Node* PushParent(Parser* parser, MD_Node* parent)
{
    MD_Node* result = parser->parent;
    parser->parent = parent;
    return result;
}

function void PopParent(Parser* parser)
{
    if (!MD_NodeIsNil(parser->parent))
        parser->parent = parser->parent->parent;
}

function MD_Node* PushNode(Parser* parser, MD_Node* name, MD_Node* base, u32 pointerLevel)
{
    MD_Node* result = MD_PushNewReference(parser->arena, parser->list, name);
    result->parent = parser->parent;
    
    MD_Node* tag = MD_MakeNode(parser->arena, MD_NodeKind_Tag, base->string, base->raw_string, base->offset);
    tag->ref_target = base;
    tag->offset = pointerLevel;
    
    MD_PushTag(result, tag);
    PrintNode(result);
    
    return result;
}

function b32 ParseToken(Parser* parser, MD_String8 token)
{
    b32 result = 0;
    if (!MD_NodeIsNil(parser->current) && MD_S8Match(parser->current->string, token, 0))
    {
        result = 1;
        ParserInc(parser, 1);
    }
    return result;
}

function MD_Node* ParseFlag(Parser* parser, MD_NodeFlags flags)
{
    MD_Node* result = 0;
    if (!MD_NodeIsNil(parser->current) && ((parser->current->flags & flags)/* == flags*/))
    {
        result = parser->current;
        ParserInc(parser, 1);
    }
    return result;
}

function u32 ParsePointer(Parser* parser, b32* error)
{
    u32 result = 0;
    b32 _err_ = 0;
    if (!error) error = &_err_;
    
    REPEAT:
    if (!MD_NodeIsNil(parser->current))
    {
        MD_String8 str = parser->current->string;
        u32 i = 0;
        for (; i < str.size; ++i)
            if (str.str[i] != '*')
                break;
        
        if (i)
        {
            result += i;
            ParserInc(parser, 1);
            
            if (i == str.size)
                goto REPEAT;
            else
                *error = 1;
        }
    }
    return result;
}

function u32 ParseArray(Parser* parser)
{
    u32 result = 0;
    while (!MD_NodeIsNil(parser->current))
    {
        if (HasAllFlags(parser->current->flags, MD_NodeFlag_HasBracketLeft|MD_NodeFlag_HasBracketRight))
            result++;
        else
            break;
        ParserInc(parser, 1);
    }
    return result;
}

function MD_Node* ParseIdentifier(Parser* parser, b32 ignoreFlags)
{
    MD_Node* result = 0;
    if (!MD_NodeIsNil(parser->current))
    {
        MD_NodeFlags flags = MD_NodeFlag_IsBeforeSemicolon|MD_NodeFlag_IsBeforeComma;
        if (HasAnyFlags(parser->current->flags, MD_NodeFlag_Identifier) && (ignoreFlags || NoFlags(parser->current->flags, flags)))
        {
            result = parser->current;
            ParserInc(parser, 1);
        }
    }
    return result;
}

int main(int argc, char** argv)
{
    OSInit(0, 0);
    
    // setup the global arena
    MD_Arena* arena = MD_ArenaAlloc();
    
    // parse all files passed to the command line
    MD_Node *list = MD_MakeList(arena);
    for (int i = 1; i < argc; i += 1)
    {
        MD_String8 file_name = MD_S8CString(argv[i]);
        MD_ParseResult parse_result = MD_ParseWholeFile(arena, file_name);
        
        for (MD_Message *message = parse_result.errors.first; message != 0; message = message->next)
        {
            if (IsPreProcError(message))
            {
                MD_CodeLoc code_loc = MD_CodeLocFromNode(message->node);
                MD_PrintMessage(stdout, code_loc, message->kind, message->string);
                MD_String8List stream = {0};
                MD_DebugDumpFromNode(arena, &stream, message->node, 0, MD_S8Lit(" "), MD_GenerateFlags_Tree|MD_GenerateFlag_NodeFlags|MD_GenerateFlag_NodeKind);
                MD_String8 str = MD_S8ListJoin(arena, stream, 0);
                fprintf(stdout, "%.*s\n\n", MD_S8VArg(str));
            }
        }
        
        //if (parse_result.errors.max_message_kind < MD_MessageKind_Error)
        {
            MD_PushNewReference(arena, list, parse_result.node);
        }
        
        Parser* parser = &(Parser){
            .arena = arena,
            .current = parse_result.node->first_child,
            .error = parse_result.errors.first,
            .list = MD_MakeList(arena),
        };
        
        while (!MD_NodeIsNil(parser->current))
        {
            Parser restore = *parser;
            b32 handled = 0;
            
            MD_Node* name = 0;
            MD_Node* base = 0;
            
            if (base = ParseIdentifier(parser, 0))
            {
                REPEAT:
                b32 error = 0;
                u32 pointerLevel = ParsePointer(parser, &error);
                if (!error && (name = ParseIdentifier(parser, 1)))
                {
                    //- NOTE(long): Functions
                    if (ParseFlag(parser, MD_NodeFlag_HasParenLeft|MD_NodeFlag_HasParenRight))
                    {
                        PushNode(parser, name, base, pointerLevel);
                        handled = 1;
                    }
                    
                    //- NOTE(long): Types
                    else if (ParseFlag(parser, MD_NodeFlag_HasBraceLeft|MD_NodeFlag_HasBraceRight))
                    {
                        PushNode(parser, name, base, pointerLevel);
                        handled = 1;
                    }
                    
                    //- NOTE(long): Decls
                    else
                    {
                        u32 arrayCount = ParseArray(parser);
                        MD_NodeFlags flags = MD_NodeFlag_IsAfterSemicolon|MD_NodeFlag_IsAfterComma;
                        if (HasAnyFlags(parser->current->flags, flags) || MD_S8Match(parser->current->string, MD_S8Lit("="), 0))
                        {
                            MD_Node* decl = PushNode(parser, name, base, pointerLevel);
                            handled = 1;
                            
                            if (HasAnyFlags(parser->current->flags, MD_NodeFlag_IsAfterComma))
                                goto REPEAT;
                        }
                        
                        // TODO(long): Handle parens inside a declaration
                    }
                }
            }
            
            if (!handled)
            {
                *parser = restore;
                ParserInc(parser, 1);
            }
        }
    }
    
    // setup the expression system
    MD_ExprOprTable table = {0};
    {
        MD_ExprOprList exprList = {0};
        
#define PUSH_OP(e,t,k,p) \
    MD_ExprOprPush(arena, &exprList, MD_ExprOprKind_##k, p, MD_S8Lit(t), Op##e, 0);
        C_LIKE_OPS_NO_SIDE_EFFECTS(PUSH_OP);
        C_LIKE_OPS_CALLS(PUSH_OP);
        C_LIKE_OPS_WITH_SIDE_EFFECTS(PUSH_OP);
#undef PUSH_OP
        
        table = MD_ExprBakeOprTableFromList(arena, &exprList);
    }
    
#if 0
    for (MD_EachNode(root_it, list->first_child))
    {
        MD_Node *root = MD_ResolveNodeFromReference(root_it);
        for (MD_EachNode(node, root->first_child))
        {
            MD_String8List stream = {0};
            MD_DebugDumpFromNode(arena, &stream, node, 0, MD_S8Lit(" "),
                                 MD_GenerateFlags_Tree|MD_GenerateFlag_NodeFlags|MD_GenerateFlag_NodeKind);
            MD_String8 str = MD_S8ListJoin(arena, stream, 0);
            fprintf(stdout, "%.*s\n\n", MD_S8VArg(str));
        }
    }
#endif
}
#endif