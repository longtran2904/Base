#define BASE_LIB_EXPORT_SYMBOLS 1
#include "Base.h"
#include "Base.c"

#define DEMO(name) DeferBlock(Outf("[%s]\n", name), Outf("\n"))

void DemoLogCallback(Arena* arena, Record* record, char* fmt, va_list args)
{
    ScratchBlock(scratch, arena)
    {
        LogFmtStd(arena, record, fmt, args);
        record->log = StrPushf(scratch, "%.*s, arena: %llu", StrExpand(record->log), arena->pos);
        record->log = StrCopy(arena, record->log);
    }
}

void DemoLogDLL(void)
{
    LogPush(0, "Main Callback");
}

int main(void)
{
#if 0
    DEMO("Char")
    {
        i64 dx = 0x77E435B08;
        while (dx)
            putchar(0x726F6C6564574820 >> (((dx >>= 3) & 7) << 3) & 0xFF);
        Outf("\n");
    }
    
    DEMO("Float")
    {
        i64 b = (1LL << 24)|1;
        f32 a = (f32)b;
        Outf("i64: %lld, f32: %f\n", b, a);
        a = 5.f;
        Outf("hex(5.0f) = %08x\n", *(i32*)&a);
        
        f32 x = 1.1f;
        if (x == 1.1f)
            Outf("x equals 1.1f\n");
        
        if (x != 1.1)
            Outf("x doesn't equals 1.1\n");
    }
#endif
    
    DEMO("Context")
    {
        Outf("cl      = %d\n", COMPILER_CL);
        Outf("clang   = %d\n", COMPILER_CLANG);
        Outf("gcc     = %d\n", COMPILER_GCC);
        Outf("widnows = %d\n", OS_WIN);
        Outf("linux   = %d\n", OS_LINUX);
        Outf("mac     = %d\n", OS_MAC);
        Outf("x64     = %d\n", ARCH_X64);
        Outf("x86     = %d\n", ARCH_X86);
        Outf("arm     = %d\n", ARCH_ARM);
        Outf("arm64   = %d\n", ARCH_ARM64);
    }
    
    Arena* arena = ArenaMake();
    
    DEMO("Convert")
    {
        typedef struct FloatTest
        {
            u32 prec;
            f64 f;
            String str;
        } FloatTest;
        
        const FloatTest test[] =
        {
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
        
        for (u64 i = 0; i < ArrayCount(test); ++i)
        {
            TempBlock(temp, arena)
            {
                String fStr = StrFromF64(arena, test[i].f, 17);
                Outf("Float: %22.17g String: %.*s\n", test[i].f, StrExpand(fStr));
            }
        }
    }
    
    OSCommit(arena, KB(4));
    
    DEMO("Log")
    {
        StringList logs = {0};
        LogBlock(arena, logs, .callback = LogFmtANSIColor)
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
        }
        
        Outf("-Normal-\n");
        StrListIter(&logs, node)
            Outf("%.*s\n", StrExpand(node->string));
        
        u64 time = OSNowMS();
        u64 pos = ArenaPos(arena);
        u64 overhead = ArenaPos(logThread.arena);
        LogBlock(arena, logs, .callback = DemoLogCallback)
        {
            Outf("\n-Stress Test-\n");
            for (u64 i = 0; i < 1024; ++i)
                LogPush(i % LogType_Count, "Log: %llu", i);
        }
        Outf("Elapsed: %llums\n", OSNowMS() - time);
        Outf("Memory: %llu bytes, Overhead: %llu bytes\n", ArenaPos(arena) - pos, ArenaPos(logThread.arena) - overhead);
        Outf("Count: %llu nodes, Size: %llu bytes\n", logs.nodeCount, logs.totalSize);
    }
    
    DEMO("OS")
    {
        Outf("---PATHS---\n");
        String currentDir = OSGetCurrDir(arena);
        String processDir = OSGetExeDir();
        String appDataDir = OSGetUserDir();
        String appTempDir = OSGetTempDir();
        Outf("Curr: %.*s\\\n", StrExpand(currentDir));
        Outf("Exe:  %.*s\\\n", StrExpand(processDir));
        Outf("Data: %.*s\\\n", StrExpand(appDataDir));
        Outf("Temp: %.*s\\\n", StrExpand(appTempDir));
        Outf("File: %s\n", __FILE__);
        
        Outf("\n");
        Outf("---TIMES---\n");
        u64 sleep = 100;
        Outf("Before: %llu ms\n", OSNowMS());
        Outf("Sleep:  %8llu ms\n", (OSSleepMS((u32)sleep), sleep));
        Outf("After:  %llu ms\n", OSNowMS());
        String now = StrFromTime(arena, OSNowUniTime());
        Outf("Today:  %.*s\n", StrExpand(now));
        
        Outf("\n");
        Outf("---STDIO---\n");
        //OSSleepMS(10000);
        OSWriteConsole(OS_STD_OUT, StrLit("Please enter your name: "));
        String input = OSReadConsole(arena, OS_STD_IN);
        if (StrCompare(input, StrLit("Long"), 1))
            OSWriteConsole(OS_STD_OUT, StrPushf(arena, "\"%.*s\" is the most beautiful name I've ever seen\n", StrExpand(input)));
        else if (StrCompare(input, StrLit("sad"), 1))
            OSWriteConsole(OS_STD_ERR, StrLit("Then just kill yourself\n"));
        else
            OSWriteConsole(OS_STD_ERR, StrPushf(arena, "ERROR: \"%.*s\" is a stupid name\n", StrExpand(input)));
        //input = OSReadConsole(arena, OS_STD_IN, 1);
        OSSleepMS(500);
        
        Outf("\n");
        Outf("---FILES---\n");
        FileIterBlock(arena, iter, StrLit("code"))
        {
            FileProperties prop = iter.props;
            String createTime = StrFromTime(arena, OSToLocTime(TimeToDate(prop.createTime)));
            String modifyTime = StrFromTime(arena, OSToLocTime(TimeToDate(prop.modifyTime)));
            
            b32 isFolder = prop.flags & FilePropertyFlag_IsFolder;
            u64 size = prop.size;
            u64 childCount = 0;
            if (isFolder)
            {
                // NOTE(long): This is only one level deep
                String folderName = StrJoin3(arena, StrLit("code\\"), iter.name);
                FileIterBlock(arena, folder, folderName, FileIterFlag_SkipFolders|FileIterFlag_Recursive)
                {
                    size += folder.props.size;
                    childCount++;
                }
            }
            
            if (isFolder)
                iter.name = StrPushf(arena, "%.*s: %llu,", StrExpand(iter.name), childCount);
            else
                iter.name = StrJoin3(arena, iter.name, StrLit(","));
            
            Outf("%s %-17.*s Created: %.*s, Date modified: %.*s, Size: %6.2f KB\n", isFolder ? "Dir: " : "File:",
                 StrExpand(iter.name), StrExpand(createTime), StrExpand(modifyTime), (f64)size / KB(1));
        }
        
        Outf("\n");
        Outf("---LIBS---\n");
        {
            OSLoadLib(StrLit("build\\test_base"));
            OSLib lib = OSLoadLib(StrLit("build\\TestDLL.dll"));
            i32 (*init)(VoidFunc*, b32);
            PrcCast(init, OSGetProc(lib, "DLLCallback"));
            
            StringList logs = {0};
            LogBlock(arena, logs)
                init(DemoLogDLL, 1);
            StrListIter(&logs, node)
                Outf("Main: %.*s\n", StrExpand(node->string));
        }
    }
    
    ArenaRelease(arena);
    return 0;
}
