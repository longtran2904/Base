// NOTE(long): Inspired by Mārtiņš Možeiko (@mmozeiko)
// https://hero.handmade.network/forums/code-discussion/t/157-memory_bandwidth_+_implementing_memcpy

#include "Base.h"
#include "Base.c"
#include <string.h>

#include <xmmintrin.h> // SSE
#include <immintrin.h> // AVX

//~ long: IMPL

function void CopyWithMemcpy(u8* dst, u8* src, uptr size)
{
    memcpy(dst, src, size);
}

function void CopyWithRepMovsb(u8* dst, u8* src, uptr size)
{
    __movsb(dst, src, size);
}

function void CopyWithRepMovsbUnaligned(u8* dst, u8* src, uptr size)
{
    __movsb(dst + 1, src + 1, size - 1);
}

// size must be multiple of 4 bytes
function void CopyWithRepMovsd(u8* dst, u8* src, uptr size)
{
    __movsd((unsigned long*)dst, (unsigned long*)src, size / sizeof(u32));
}

// size must be multiple of 8 bytes
function void CopyWithRepMovsq(u8* dst, u8* src, uptr size)
{
    __movsq((u64*)dst, (u64*)src, size / sizeof(u64));
}

// dst and src must be 16-byte aligned
// size must be multiple of 16*8 = 128 bytes
function void CopyWithSSE(u8* dst, u8* src, uptr size)
{
    for (uptr stride = 8*sizeof(__m128i); size; size -= stride, src += stride, dst += stride)
    {
        __m128 a = _mm_load_ps((f32*)(src + 0*sizeof(__m128)));
        __m128 b = _mm_load_ps((f32*)(src + 1*sizeof(__m128)));
        __m128 c = _mm_load_ps((f32*)(src + 2*sizeof(__m128)));
        __m128 d = _mm_load_ps((f32*)(src + 3*sizeof(__m128)));
        __m128 e = _mm_load_ps((f32*)(src + 4*sizeof(__m128)));
        __m128 f = _mm_load_ps((f32*)(src + 5*sizeof(__m128)));
        __m128 g = _mm_load_ps((f32*)(src + 6*sizeof(__m128)));
        __m128 h = _mm_load_ps((f32*)(src + 7*sizeof(__m128)));
        
        _mm_store_ps((f32*)(dst + 0*sizeof(__m128)), a);
        _mm_store_ps((f32*)(dst + 1*sizeof(__m128)), b);
        _mm_store_ps((f32*)(dst + 2*sizeof(__m128)), c);
        _mm_store_ps((f32*)(dst + 3*sizeof(__m128)), d);
        _mm_store_ps((f32*)(dst + 4*sizeof(__m128)), e);
        _mm_store_ps((f32*)(dst + 5*sizeof(__m128)), f);
        _mm_store_ps((f32*)(dst + 6*sizeof(__m128)), g);
        _mm_store_ps((f32*)(dst + 7*sizeof(__m128)), h);
    }
}

// dst and src must be 16-byte aligned
// size must be multiple of 16*2 = 32 bytes
function void CopyWithSSESmall(u8* dst, u8* src, uptr size)
{
    for (uptr stride = 2*sizeof(__m128); size; size -= stride, src += stride, dst += stride)
    {
        __m128 a = _mm_load_ps((f32*)(src + 0*sizeof(__m128)));
        __m128 b = _mm_load_ps((f32*)(src + 1*sizeof(__m128)));
        _mm_store_ps((f32*)(dst + 0*sizeof(__m128)), a);
        _mm_store_ps((f32*)(dst + 1*sizeof(__m128)), b);
    }
}

// dst and src must be 16-byte aligned
// size must be multiple of 16*2 = 32 bytes
function void CopyWithSSENoCache(u8* dst, u8* src, uptr size)
{
    for (uptr stride = 2*sizeof(__m128); size; size -= stride, src += stride, dst += stride)
    {
        __m128 a = _mm_load_ps((f32*)(src + 0*sizeof(__m128)));
        __m128 b = _mm_load_ps((f32*)(src + 1*sizeof(__m128)));
        _mm_stream_ps((f32*)(dst + 0*sizeof(__m128)), a);
        _mm_stream_ps((f32*)(dst + 1*sizeof(__m128)), b);
    }
}

// dst and src must be 32-byte aligned
// size must be multiple of 32*16 = 512 bytes
function void CopyWithAVX(u8* dst, u8* src, uptr size)
{
    for (uptr stride = 16*sizeof(__m256i); size; size -= stride, src += stride, dst += stride)
    {
        __m256i a = _mm256_load_si256((__m256i*)src + 0);
        __m256i b = _mm256_load_si256((__m256i*)src + 1);
        __m256i c = _mm256_load_si256((__m256i*)src + 2);
        __m256i d = _mm256_load_si256((__m256i*)src + 3);
        __m256i e = _mm256_load_si256((__m256i*)src + 4);
        __m256i f = _mm256_load_si256((__m256i*)src + 5);
        __m256i g = _mm256_load_si256((__m256i*)src + 6);
        __m256i h = _mm256_load_si256((__m256i*)src + 7);
        __m256i i = _mm256_load_si256((__m256i*)src + 8);
        __m256i j = _mm256_load_si256((__m256i*)src + 9);
        __m256i k = _mm256_load_si256((__m256i*)src + 10);
        __m256i l = _mm256_load_si256((__m256i*)src + 11);
        __m256i m = _mm256_load_si256((__m256i*)src + 12);
        __m256i n = _mm256_load_si256((__m256i*)src + 13);
        __m256i o = _mm256_load_si256((__m256i*)src + 14);
        __m256i p = _mm256_load_si256((__m256i*)src + 15);
        
        _mm256_store_si256((__m256i*)dst + 0, a);
        _mm256_store_si256((__m256i*)dst + 1, b);
        _mm256_store_si256((__m256i*)dst + 2, c);
        _mm256_store_si256((__m256i*)dst + 3, d);
        _mm256_store_si256((__m256i*)dst + 4, e);
        _mm256_store_si256((__m256i*)dst + 5, f);
        _mm256_store_si256((__m256i*)dst + 6, g);
        _mm256_store_si256((__m256i*)dst + 7, h);
        _mm256_store_si256((__m256i*)dst + 8, i);
        _mm256_store_si256((__m256i*)dst + 9, j);
        _mm256_store_si256((__m256i*)dst + 10, k);
        _mm256_store_si256((__m256i*)dst + 11, l);
        _mm256_store_si256((__m256i*)dst + 12, m);
        _mm256_store_si256((__m256i*)dst + 13, n);
        _mm256_store_si256((__m256i*)dst + 14, o);
        _mm256_store_si256((__m256i*)dst + 15, p);
    }
}

// dst and src must be 32-byte aligned
// size must be multiple of 32*2 = 64 bytes
function void CopyWithAVXSmall(u8* dst, u8* src, uptr size)
{
    for (uptr stride = 2*sizeof(__m256i); size; size -= stride, src += stride, dst += stride)
    {
        __m256i a = _mm256_load_si256((__m256i*)src + 0);
        __m256i b = _mm256_load_si256((__m256i*)src + 1);
        _mm256_store_si256((__m256i*)dst + 0, a);
        _mm256_store_si256((__m256i*)dst + 1, b);
    }
}

// dst and src must be 32-byte aligned
// size must be multiple of 32*2 = 64 bytes
function void CopyWithAVXNoCache(u8* dst, u8* src, uptr size)
{
    for (uptr stride = 2*sizeof(__m256i); size; size -= stride, src += stride, dst += stride)
    {
        // NOTE(long): Based on some testing, using no-cache loads is slower
        // _mm_stream_load_si128 and _mm256_stream_load_si256
        
        __m256i a = _mm256_load_si256((__m256i*)src + 0);
        __m256i b = _mm256_load_si256((__m256i*)src + 1);
        _mm256_stream_si256((__m256i*)dst + 0, a);
        _mm256_stream_si256((__m256i*)dst + 1, b);
    }
}

//~ long: TEST SETUP

global u32 SampleCount = 8;
global u64 TestSize = GB(3ULL) >> 1; // 1.5 GiB
global b32 Debug = 0;

global u8* MemDst;
global u8* MemSrc;

// this is to prevent compiler to optimize out memory copies
volatile u8 dummy;

#define MAX_THREADS 64
#define ERROR_EXIT(...) Stmnt(Errf("\nERROR: " __VA_ARGS__); OSExit(0))

typedef void CopyFunc(u8* dst, u8* src, uptr size);
function void SetupThreads(u32 newThreadCount);
function void CopyWithThreads(char* name, CopyFunc* func);

function void CopyWarmup(char* name, CopyFunc* func)
{
    func(MemDst, MemSrc, TestSize);
    if (memcmp(MemDst, MemSrc, TestSize) != 0)
        ERROR_EXIT("%s\n", name);
    dummy += MemDst[TestSize-1];
    memset(MemDst, 0, TestSize);
}

i32 main(i32 argc, char** argv)
{
    ScratchBegin(scratch);
    u64 threadCountMask = 0;
    StaticAssert(sizeof(threadCountMask)*8 >= MAX_THREADS);
    b32 help = 0;
    
    StringList errors = {0};
    StringList args = OSSetArgs(argc, argv);
    CmdLine cmd = CmdLineFromList(scratch, &args);
    
    for (CmdLineOpt* opt = cmd.opts.first; opt; opt = opt->next)
    {
        b32 invalid = 0;
        if (opt->name.size != 1)
            invalid = 1;
        
        else switch (opt->name.str[0])
        {
#define ParseNum(str, max, stmt) Stmnt(u64 num = U64FromStr((str), 10, 0); \
                                       if (InRange(num, 1, (max))) \
                                       { \
                                           invalid = 0; \
                                           stmt; \
                                       })
            
            case 't':
            {
                invalid = 1;
                StrListIter(&opt->values, val)
                {
                    ParseNum(val->string, MAX_THREADS, threadCountMask |= 1ULL << (num-1));
                    if (invalid)
                        break;
                }
            } break;
            
#define ParseSingleNum(max, stmt) Stmnt(invalid = 1;\
                                        if (opt->values.nodeCount == 1) \
                                        { ParseNum(opt->values.first->string, max, stmt); })
            case 'c': ParseSingleNum(2048, SampleCount = (u32)num); break;
            case 's': ParseSingleNum(KB(8),  TestSize =  MB(num)); break;
            
            case 'h':
            case '?':   help = 1; break;
            case 'd':  Debug = 1; break;
            default: invalid = 1; break;
        }
        
        if (invalid)
            StrListPushf(scratch, &errors, "ERROR: Invalid argument or option - '%.*s'.", StrExpand(opt->name));
    }
    
    if (errors.nodeCount)
    {
        StrListIter(&errors, node)
            Errf("%.*s\n", StrExpand(node->string));
        ERROR_EXIT("Type \"bench_mem /?\" for usage help.\n");
    }
    
    if (help)
    {
        Outf("[SYNTAX]\n\n"
             "    bench_mem [options...]\n"
             "    Examples:\n"
             "        bench_mem -t=2,5,12 -c=8 -s=960\n"
             "        bench_mem -t=2,4,8 -d -c=4\n"
             "\n"
             "[OPTIONS]\n\n"
             "    /c     The number of iterations to run on a single test. By default, this is 8.\n\n"
             "    /s     The test data size in MiB (e.g. 1024 is 1 GiB, while 512 is 0.5 GiB).\n"
             "           Note that this number must be divisible by the number of threads times 512.\n"
             "           By default, this is 1536 MiB (1.5 GiB).\n\n"
             "    /t     The number of threads to run the test (maximum is 64). You can specify how many\n"
             "           numbers you want and the test will run once for each specified number of threads.\n"
             "           For example, `-t=1,2,8` will run all tests with one thread, two threads, and fours.\n"
             "           Note that running with one thread is the only time it runs on the main thread.\n\n"
             "    /d     Displays the internal thread logging and is only intended for debugging.\n"
             "           S1 means start copying on thread 1, while E5 means done copying on thread 5.\n\n"
             "    /?, /h Displays this help message.\n\n");
        OSExit(0);
    }
    
    SysInfo system = OSGetSysInfo();
    u32 coreCount = system.coreCount;
    if (!coreCount)
        ERROR_EXIT("Can't get the CPU's core count\n");
    Outf("Test: %d samples per function, CPU: %dC/%dT\n", SampleCount, coreCount, system.threadCount);
    
#define COPY_LIST(X) Stmnt(X("memcpy         ", CopyWithMemcpy); \
                           X("SSE            ", CopyWithSSE); \
                           X("SSE Small      ", CopyWithSSESmall); \
                           X("SSE No-Cache   ", CopyWithSSENoCache); \
                           X("AVX            ", CopyWithAVX); \
                           X("AVX Small      ", CopyWithAVXSmall); \
                           X("AVX No-Cache   ", CopyWithAVXNoCache); \
                           X("movsb          ", CopyWithRepMovsb); \
                           X("movsd          ", CopyWithRepMovsd); \
                           X("movsq          ", CopyWithRepMovsq); \
                           X("movsb unaligned", CopyWithRepMovsbUnaligned))
    
    Outf("Initializing %.2f GiB of test data...", (f64)TestSize/(f64)GB(1));
    TIME_BLOCK(duration, Outf(" %.3fs\n", (f64)duration/1000.))
    {
        MemSrc = OSCommit(0, TestSize);
        MemDst = OSCommit(0, TestSize);
        for (u64 i = 0; i < TestSize; ++i)
            MemSrc[i] = (u8)(i + i*i);
    }
    
    Outf("Cold Warmup...");
    TIME_BLOCK(duration, Outf(" %.3fs\n", (f64)duration/1000.))
        COPY_LIST(CopyWarmup);
    
    if (threadCountMask == 0)
    {
        Outf("\n---Main Thread---\n");
        SetupThreads(1);
        COPY_LIST(CopyWithThreads);
        
        if (coreCount != 2)
        {
            Outf("\n---Threads: 2---\n");
            SetupThreads(2);
            COPY_LIST(CopyWithThreads);
        }
        
        u32 maxThreadCount = ClampTop(coreCount, MAX_THREADS);
        Outf("\n---Threads: %u (Max)---\n", maxThreadCount);
        SetupThreads(maxThreadCount);
        COPY_LIST(CopyWithThreads);
    }
    
    else for (u64 idx = 0; idx < MAX_THREADS; ++idx)
    {
        if (!(threadCountMask & (1ULL << idx)))
            continue;
        
        u32 count = (u32)idx + 1;
        if (count == 1)
            Outf("\n---Main Thread---\n");
        else
            Outf("\n---Threads: %u---\n", count);
        
        SetupThreads(count);
        COPY_LIST(CopyWithThreads);
    }
    
    ScratchEnd(scratch);
    return 0;
}

//~ long: THREAD SETUP

global CopyFunc* TestFunc;
global volatile u32 ThreadCount;

#if OS_WIN
global HANDLE ThreadHandles[MAX_THREADS];
global HANDLE WorkHandle;
global HANDLE DoneHandle;

#define DEBUG_OUTF(...) Stmnt(if (Debug) Outf(__VA_ARGS__))

function DWORD WINAPI ThreadProc(LPVOID arg)
{
    u32 idx = (u32)IntFromPtr(arg);
    u64 size = TestSize/ThreadCount;
    u64 offset = idx * size;
    u8* dst = MemDst + offset;
    u8* src = MemSrc + offset;
    
    while (1)
    {
        WaitForSingleObject(WorkHandle, INFINITE);
        DEBUG_OUTF("S%u, ", idx);
        TestFunc(dst, src, size);
        DEBUG_OUTF("E%u, ", idx);
        ReleaseSemaphore(DoneHandle, 1, 0);
    }
}

function void SetupThreads(u32 newThreadCount)
{
    u32 oldCount = ThreadCount;
    switch (oldCount)
    {
        case 1: break;
        
        case 0:
        {
            WorkHandle = CreateSemaphore(0, 0, MAX_THREADS, 0);
            DoneHandle = CreateSemaphore(0, 0, MAX_THREADS, 0);
        } break;
        
        default:
        {
            for (u32 i = 0; i < oldCount && oldCount > 1; ++i)
                TerminateThread(ThreadHandles[i], 0);
        } break;
    }
    
    // NOTE(long): This is always true in this program, but remember this when copy-paste this code
    // Assert(newThreadCount <= MAX_THREADS);
    if (TestSize % (512*newThreadCount))
        ERROR_EXIT("%.2f GiB is not a multiple of 512*%u: `-s` %% (`-t` * 512)\n",
                   (f64)TestSize/(f64)GB(1), newThreadCount);
    ThreadCount = newThreadCount;
    
    if (newThreadCount > 1)
        for (u32 i = 0; i < newThreadCount; ++i)
            ThreadHandles[i] = CreateThread(0, 0, ThreadProc, PtrFromInt(i), 0, 0);
}

function void CopyWithThreads(char* name, CopyFunc* func)
{
    f64 speed = (f64)(SampleCount*TestSize);
    TIME_BLOCK_US(duration, speed /= (f64)Thousand(duration))
    {
        for (u32 i = 0; i < SampleCount; ++i)
        {
            if (ThreadCount > 1)
            {
                DEBUG_OUTF("Copying: ");
                
                TestFunc = func;
                ReleaseSemaphore(WorkHandle, (LONG)ThreadCount, 0);
                for (u32 idx = 0; idx < ThreadCount; ++idx)
                    WaitForSingleObject(DoneHandle, INFINITE);
                
                DEBUG_OUTF("Done\n");
            }
            else func(MemDst, MemSrc, TestSize);
            
            dummy += MemDst[TestSize-1];
        }
    }
    Outf("%s = %5.2f GiB/s\n", name, speed);
}

#else
#error This OS is not supported
#endif
