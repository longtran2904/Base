#define BASE_LIB_RUNTIME_IMPORT 1
#include "Base.h"
#include "Base.c"

libexport i32 globalInt = 10;

libexport u32 Sum(u32* vals, u64 count)
{
    u32 sum = 0;
    for (u64 i = 0; i < count; ++i)
        sum += vals[i];
    return sum;
}

libexport i32 DLLCallback(VoidFunc* func, b32 log)
{
    LogPush(0, "DLL Initialize #1");
    ScratchBlock(scratch)
    {
        StringList logs = {0};
        LogBlock(scratch, logs)
        {
            LogPush(0, "DLL Initialize #2");
            func();
        }
        
        if (log)
            StrListIter(&logs, node)
                Outf("DLL: %.*s\n", StrExpand(node->string));
    }
    return globalInt;
}
