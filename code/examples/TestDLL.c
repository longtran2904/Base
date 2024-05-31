
#include "Base.h"
#include <stdio.h>

libexport i32 globalInt = 10;

libexport u32 Sum(u32* vals, u64 count)
{
    u32 sum = 0;
    for (u64 i = 0; i < count; ++i)
        sum += vals[i];
    return sum;
}

libexport i32 DLLCallback(VoidFunc* func)
{
    func();
    return globalInt;
}
