/* date = June 5th 2024 0:13 am */

#ifndef _LONG_TEST_H
#define _LONG_TEST_H

#ifndef LT_NAME_PADDING
#define LT_NAME_PADDING 20
#endif

#ifndef LT_TEST_PADDING
#define LT_TEST_PADDING 40
#endif

typedef struct TestCtx TestCtx;
struct TestCtx
{
    i32 testCount;
    i32 passCount;
};
global TestCtx ctx; // TODO(long): Change from using a global to using a stack

function void TestBegin(char* name, i32 padding);
function void TestEnd(i32 padding);
function b32  TestResult(b32 result);
#define TEST(name) DeferBlock(TestBegin(name, LT_NAME_PADDING), TestEnd(LT_TEST_PADDING))

#endif //_LONG_TEST_H

#ifdef LONG_TEST_IMPLEMENTATION

#ifndef LT_PRINTF
#define LT_PRINTF(fmt, ...) Outf(fmt, __VA_ARGS__)
#endif

#ifndef LT_ASSERT
#define LT_ASSERT(x)
#endif

function void TestBegin(char* name, i32 padding)
{
    String str = StrFromCStr(name);
    i32 spaces = ClampBot(padding - (i32)str.size, 0);
    Outf("\"%.*s\"%.*s [", StrExpand(str), spaces, " ------------------------------");
    
    ctx.testCount = 0;
    ctx.passCount = 0;
}

function b32 TestResult(b32 result)
{
    ctx.testCount++;
    ctx.passCount += !!result;
    Outf(result ? "." : "X");
    
    LT_ASSERT(result);
    return result;
}

function void TestEnd(i32 padding)
{
    i32 spaces = ClampBot(padding - ctx.testCount, 0);
    
    Outf("]%.*s ", spaces, "                                                                                ");
    Outf("[%2i/%-2i] %2i passed, %2i tests, ", ctx.passCount, ctx.testCount, ctx.passCount, ctx.testCount);
    Outf(ctx.testCount == ctx.passCount ? "SUCCESS ( )\n" : "FAILED (X)\n");
}
#endif // LONG_TEST_IMPLEMENTATION
