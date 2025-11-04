/* date = June 5th 2024 0:13 am */

#ifndef _LONG_TEST_H
#define _LONG_TEST_H

#ifndef LT_NAME_PADDING
#define LT_NAME_PADDING 20
#endif

#ifndef LT_TEST_PADDING
#define LT_TEST_PADDING 40
#endif

typedef struct LT_Ctx
{
    i32 testCount;
    i32 passCount;
} LT_Ctx;

function void LT_Begin(LT_Ctx* ctx, char* name, i32 padding);
function void LT_End  (LT_Ctx* ctx, i32 padding);
function b32  LT_Check(LT_Ctx* ctx, b32 result);
#define LT_Block(ctx, name) \
    DeferBlock(LT_Begin((ctx), (name), LT_NAME_PADDING), LT_End((ctx), LT_TEST_PADDING))

#endif //_LONG_TEST_H

#ifdef LONG_TEST_IMPLEMENTATION

#ifndef LT_PRINTF
#define LT_PRINTF(fmt, ...) Outf(fmt, ##__VA_ARGS__)
#endif

#ifndef LT_ASSERT
#define LT_ASSERT(x)
#endif

function void LT_Begin(LT_Ctx* ctx, char* name, i32 padding)
{
    String str = StrFromCStr(name);
    i32 spaces = ClampBot(padding - (i32)str.size, 0);
    LT_PRINTF("\"%.*s\"%.*s [", StrExpand(str), spaces, " ------------------------------");
    
    ctx->testCount = 0;
    ctx->passCount = 0;
}

function b32 LT_Check(LT_Ctx* ctx, b32 result)
{
    ctx->testCount++;
    ctx->passCount += !!result;
    LT_PRINTF(result ? "." : "X");
    
    LT_ASSERT(result);
    return result;
}

function void LT_End(LT_Ctx* ctx, i32 padding)
{
    i32 spaces = ClampBot(padding - ctx->testCount, 0);
    
    LT_PRINTF("]%.*s ", spaces,
              "                                                                                ");
    LT_PRINTF("[%2i/%-2i] %2i passed, %2i tests, ",
              ctx->passCount, ctx->testCount, ctx->passCount, ctx->testCount);
    LT_PRINTF(ctx->testCount == ctx->passCount ? "SUCCESS ( )\n" : "FAILED (X)\n");
}
#endif // LONG_TEST_IMPLEMENTATION
