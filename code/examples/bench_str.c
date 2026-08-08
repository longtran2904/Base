#include "Base.h"
#include "Base.c"

neverinline u32 foo1(char* ptr)
{
    u32 result = 0;
    for (int i = 0; ptr[i]; ++i)
        result += ptr[i];
    return result;
}

neverinline u32 foo2(char* ptr, int size)
{
    u32 result = 0;
    for (int i = 0; i < size; ++i)
        result += ptr[i];
    return result;
}

neverinline int bar1(char* ptr, char value)
{
    for (int i = 0; ptr[i]; ++i)
        if (ptr[i] == value)
            return 1;
    return 0;
}

neverinline int bar2(char* ptr, int size, char value)
{
    for (int i = 0; i < size; ++i)
        if (ptr[i] == value)
            return 1;
    return 0;
}

neverinline char* init(int size)
{
    char* result = OSCommit(0, size);
    RNG rng = { .seed = BIT_NOISE1 };
    for (u64 i = 0; i < size - 1; ++i)
        result[i] = RandomRangeI32(rng, 1, 255);
    result[size - 1] = 0;
    return result;
}

int main(void)
{
    int testSize = GiB(1) >> 1; // 0.5GB
    int sampleCount = 32;
    
    char* ptr = 0;
    int size = testSize - 1;
    
    Outf("Initializing %.2f GiB of test data...", DivF64(testSize, GiB(1)));
    TIME_BLOCK(duration, Outf(" %.3fs\n", (f64)duration/1000.))
        ptr = init(testSize);
    
    char value = 0;
    
    Outf("Cold Warmup...");
    TIME_BLOCK(duration, Outf(" %.3fs\n", (f64)duration/1000.))
    {
        int f1 = foo1(ptr);
        int f2 = foo2(ptr, size);
        Assert(f1 == f2);
        
        int b1 = bar1(ptr, value);
        int b2 = bar2(ptr, size, value);
        Assert(b1 == b2);
    }
    
    volatile int dummy = 0;
    
    TIME_BLOCK(duration, Outf("foo1 completes %d times in %.3fs\n", sampleCount, DivF64(duration, 1000.)))
        for (int i = 0; i < sampleCount; ++i)
            dummy += foo1(ptr);
    
    TIME_BLOCK(duration, Outf("foo2 completes %d times in %.3fs\n", sampleCount, DivF64(duration, 1000.)))
        for (int i = 0; i < sampleCount; ++i)
            dummy += foo2(ptr, size);
    
    TIME_BLOCK(duration, Outf("bar1 completes %d times in %.3fs\n", sampleCount, DivF64(duration, 1000.)))
        for (int i = 0; i < sampleCount; ++i)
            dummy += bar1(ptr, value);
    
    TIME_BLOCK(duration, Outf("bar2 completes %d times in %.3fs\n", sampleCount, DivF64(duration, 1000.)))
        for (int i = 0; i < sampleCount; ++i)
            dummy += bar2(ptr, size, value);
    
    return 0;
}
