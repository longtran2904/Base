#define BASE_LOG_COLOR
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

#define CODE_PATH 0

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
    BeginScratch(scratch, arena);
    fmt = StrPushf(scratch, "%s, snapshot: %d", fmt, snapshot++).str;
    LogFmtStd(arena, record, fmt, args);
    EndScratch(scratch);
}

int main(void)
{
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
    
    InitOSMain(0, 0);
    
    EvalPrintPtr(arena);
    EvalPrintU(arena->cap);
    EvalPrintU(arena->pos);
    EvalPrintU(arena->commitPos);
    
    EvalPrintLine;
    
    TempArena temp = TempBegin(arena);
    
    u64 arrayCount = 10;
    int* array = PushZeroArray(arena, int, arrayCount);
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
    
    u32 testVal = 1 << 24 | 3;
    f32 testFloat = testVal;
    
    EvalPrintLine;
    
    TempEnd(temp);
    
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
        i64 prec;
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
        {  1, ldexp(50883641005312716, -172), },
        {  2, ldexp(38162730753984537, -170), },
        {  3, ldexp(50832789069151999, -101), },
        {  4, ldexp(51822367833714164, -109), },
        {  5, ldexp(66840152193508133, -172), },
        {  6, ldexp(55111239245584393, -138), },
        {  7, ldexp(71704866733321482, -112), },
        {  8, ldexp(67160949328233173, -142), },
        {  9, ldexp(53237141308040189, -152), },
        { 10, ldexp(62785329394975786, -112), },
        { 11, ldexp(48367680154689523, - 77), },
        { 12, ldexp(42552223180606797, -102), },
        { 13, ldexp(63626356173011241, -112), },
        { 14, ldexp(43566388595783643, - 99), },
        { 15, ldexp(54512669636675272, -159), },
        { 16, ldexp(52306490527514614, -167), },
        { 17, ldexp(52306490527514614, -168), },
        { 18, ldexp(41024721590449423, - 89), },
        { 19, ldexp(37664020415894738, -132), },
        { 20, ldexp(37549883692866294, - 93), },
        { 21, ldexp(69124110374399839, -104), },
        { 22, ldexp(69124110374399839, -105), },
        
        // Table 21
        {  1, ldexp(49517601571415211, - 94), },
        {  2, ldexp(49517601571415211, - 95), },
        {  3, ldexp(54390733528642804, -133), },
        {  4, ldexp(71805402319113924, -157), },
        {  5, ldexp(40435277969631694, -179), },
        {  6, ldexp(57241991568619049, -165), },
        {  7, ldexp(65224162876242886, + 58), },
        {  8, ldexp(70173376848895368, -138), },
        {  9, ldexp(37072848117383207, - 99), },
        { 10, ldexp(56845051585389697, -176), },
        { 11, ldexp(54791673366936431, -145), },
        { 12, ldexp(66800318669106231, -169), },
        { 13, ldexp(66800318669106231, -170), },
        { 14, ldexp(66574323440112438, -119), },
        { 15, ldexp(65645179969330963, -173), },
        { 16, ldexp(61847254334681076, -109), },
        { 17, ldexp(39990712921393606, -145), },
        { 18, ldexp(59292318184400283, -149), },
        { 19, ldexp(69116558615326153, -143), },
        { 20, ldexp(69116558615326153, -144), },
        { 21, ldexp(39462549494468513, -152), },
        { 22, ldexp(39462549494468513, -153), },
    };
    
#define MAX_TEST_COUNT 100
    
    u64 elapsed = NowMicroseconds();
    for (u64 _ = 0; _ < MAX_TEST_COUNT; ++_)
    {
        for (u64 i = 0; i < ArrayCount(test); ++i)
        {
            TempArena temp = TempBegin(arena);
            String fStr = StrFromF64(arena, test[i].f, test[i].prec);
            /*if (test[i].str.size && !StrCompare(fStr, test[i].str, 0))
            printf("Expected: %.*s, Result: %.*s\n", StrExpand(test[i].str), StrExpand(fStr));*/
            TempEnd(temp);
        }
    }
    elapsed = NowMicroseconds() - elapsed;
    elapsed /= MAX_TEST_COUNT;
    EvalPrintULL(elapsed);
    
    elapsed = NowMicroseconds();
    for (u64 _ = 0; _ < MAX_TEST_COUNT; ++_)
    {
        for (u64 i = 0; i < ArrayCount(test); ++i)
        {
            u8 buf[1000];
            String fStr = Str(buf, sprintf(buf, "%.*e", (i32)test[i].prec, test[i].f));
            /*if (test[i].str.size && !StrCompare(fStr, test[i].str, 0))
            printf("Expected: %.*s, Result: %.*s\n", StrExpand(test[i].str), StrExpand(fStr));*/
        }
    }
    elapsed = NowMicroseconds() - elapsed;
    elapsed /= MAX_TEST_COUNT;
    EvalPrintULL(elapsed);
    
    elapsed = NowMicroseconds();
    for (u64 _ = 0; _ < MAX_TEST_COUNT; ++_)
    {
        for (u64 i = 0; i < ArrayCount(test); ++i)
        {
            u8 buf[1000];
            String fStr = Str(buf, stbsp_sprintf(buf, "%.*e", (i32)test[i].prec, test[i].f));
            /*if (test[i].str.size && !StrCompare(fStr, test[i].str, 0))
            printf("Expected: %.*s, Result: %.*s\n", StrExpand(test[i].str), StrExpand(fStr));*/
        }
    }
    elapsed = NowMicroseconds() - elapsed;
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
    
    String fileData = ReadOSFile(arena, StrLit("data/Test.txt"), true);
    EvalPrintS(fileData.str);
    EvalPrint(WriteOSFile(StrLit("data/Test2.txt"), data));
    EvalPrintStr(data);
    
    EvalPrintStr(GetCurrDir(arena));
    EvalPrintStr(GetProcDir());
    EvalPrintStr(GetUserDir());
    EvalPrintStr(GetTempDir());
    
    EvalPrintLine;
    
    {
        String currentName;
        FileProperties currentProp;
        OSFileIter iter = InitFileIter(StrLit("code"));
        while(NextFileIter(arena, &iter, &currentName, &currentProp))
        {
            EvalPrintStr(currentName);
            EvalPrintULL(currentProp.createTime);
            EvalPrintULL(currentProp.modifyTime);
            EvalPrint(TimeToDate(currentProp.modifyTime).year);
        }
    }
    
    EvalPrintLine;
    
    u64 entropy;
    GetOSEntropy(&entropy, sizeof(entropy));
    EvalPrintULL(entropy);
    
    EvalPrintLine;
    
    OSLib testLib = LoadOSLib(StrLit("build/TestDLL.dll"));
    EvalPrintPtr(testLib.v);
    u32(*func)(u32*, u64) = (u32 (*)(u32*, u64))GetOSProc(testLib, "Sum");
    EvalPrintU(func((u32*)&entropy, 2));
    i32* dllVar = (i32*)GetOSProc(testLib, "globalInt");
    EvalPrint(*dllVar);
    
    RenameOSFile(StrLit("build/TestDLL.dll"), StrLit("build/NewTestDLL.dll"));
    
    FileProperties file = GetFileProperties(StrLit("build/NewTestDLL.dll"));
    EvalPrintULL(file.createTime);
    EvalPrintULL(file.modifyTime);
    EvalPrint(TimeToDate(file.modifyTime).year);
    
    EvalPrintLine;
    
    LogBegin(arena);
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
        StringList list = StrListFromLogger(arena, &logger);
        for (StringNode* node = list.first; node; node = node->next)
            printf("%.*s\n", StrExpand(node->string));
    }
    
    EvalPrintLine;
    
    EvalPrintULL(NowMicroseconds());
    DateTime now = NowUniversalTime();
    EvalPrint(now.msec);
    EvalPrint(ToLocalTime(&now).msec);
    
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
	u32 renderer = IntFromPtr(GetGFXWindowUserData(window));
	switch (renderer)
	{
		case Renderer_GL:
		{
			BeginGLRender(window);
			
			if (IsGFXWindowMaximized(window))
				glClearColor(0.f, 1.f, 1.f, 1.f);
			else if (IsGFXWindowMinimized(window))
				glClearColor(1.f, 0.f, 1.f, 1.f);
			else if (IsGFXWindowFullScreen(window))
				glClearColor(1.f, 1.f, 0.f, 1.f);
			else
				glClearColor(0.f, 0.f, 1.f, 1.f);
			
			glClear(GL_COLOR_BUFFER_BIT);
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

int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd)
{
    W32WinMainInit(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    BeginScratch(scratch);
    
    ErrorBegin(scratch, .callback = GFXErrorFmt);
    {
        InitGFX();
        InitGL();
        InitD3D11();
        
        //ErrorFmt("Test Error 1");
        //ErrorFmt("Test Error 2");
    }
    GFXCheckError(1);
    
	SetGFXResizeFunc(WindowResizeHandler);
	
#define TEST_WINDOW_COUNT 4
	GFXWindow windows[TEST_WINDOW_COUNT] = {0};
	for (u32 i = 0; i < TEST_WINDOW_COUNT; ++i)
	{
		i32 w = 400, h = 200;
		i32 isGL = i % 2;
		//if (i > 0) GetGFXWindowInnerRect(windows[i - 1], 0, 0, &w, &h);
        
		w = w * (i + 1);
		h = h * (i + 1);
        
        ErrorBegin(scratch);
        {
            windows[i] = CreateGFXWindowEx(StrPushf(scratch, "Window: %d", i), CW_USEDEFAULT, CW_USEDEFAULT, w, h);
            if (isGL == 0)
                EquipGLWindow(windows[i]);
            else
                EquipD3D11Window(windows[i]);
        }
        GFXCheckError(1);
        
		SetGFXWindowUserData(windows[i], PtrFromInt(isGL));
		ShowGFXWindow(windows[i]);
	}
	
    ID3D11DeviceContext* ctx = GetD3D11DeviceCtx();
    
	u32 count = 0;
	b32 visible = false;
    for (;;)
    {
        if (!WaitForGFXInput())
			break;
        
		//OutputDebugString(StrPushf(scratch, "Count: %d\n", count).str);
		count++;
		if (count % 50 == 0)
		{
			if (false)
			{
				GFXWindow window = windows[TEST_WINDOW_COUNT-1];
				if (IsGFXWindowValid(window))
				{
					SetGFXWindowVisible(window, visible);
					if (visible)
						SetGFXWindowTitle(window, StrPushf(scratch, "Toggle Windows: %d", count / 100));
					visible = !visible;
				}
			}
			
			if (false)
			{
				GFXWindow window = windows[TEST_WINDOW_COUNT-2];
				if (IsGFXWindowValid(window))
				{
					i32 x, y, w, h;
					GetGFXWindowOuterRect(window, &x, &y, &w, &h);
					SetGFXWindowInnerRect(window, x, y, w, h);
				}
			}
			
			if (false)
			{
				GFXWindow window = windows[TEST_WINDOW_COUNT-2];
				if (IsGFXWindowValid(window))
					SetGFXWindowFullScreen(window, count % 100 == 0);
			}
			
			{
				GFXWindow window = windows[1];
				if (IsGFXWindowValid(window))
				{
					OutputDebugString(StrPushf(scratch, "Before: %d\n", IsGFXWindowResizable(window)).str);
					SetGFXWindowResizable(window, count % 100);
					OutputDebugString(StrPushf(scratch, "After: %d\n", IsGFXWindowResizable(window)).str);
				}
			}
		}
		
		u32 activeWindowCount = 0;
		for (u32 i = 0; i < TEST_WINDOW_COUNT; ++i)
		{
			if (IsGFXWindowValid(windows[i]))
			{
				activeWindowCount++;
				WindowResizeHandler(windows[i], 0, 0);
				if (IsGFXWindowMinimized(windows[i]))
					CloseGFXWindow(windows[i]);
			}
		}
		
		if (!activeWindowCount)
			break;
	}
	
	{
		ID3D11Device_Release(device);
		ID3D11DeviceContext_Release(ctx);
		ID3D11Debug_Release(dbg);
	}
	
	EndScratch(scratch);
	return 0;
}
#endif