#include "DefaultMemory.h"
#include "Base.h"

sharedexport int globalInt = 10;

sharedexport u32 Sum(u32* vals, u64 count)
{
    u32 sum = 0;
    for (u64 i = 0; i < count; ++i)
        sum += vals[i];
    return sum;
}