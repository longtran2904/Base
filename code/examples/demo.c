#include "DefaultMemory.h"
#include "Base.h"
#include "LongOS.h"
#include "Base.c"
#include "LongOS_Win32.c"
#include <stdio.h>

#define DEMO(name) DeferBlock(printf("[%s]\n", name), printf("\n"))

void DemoLogCallback(Arena* arena, Record* record, char* fmt, va_list args)
{
    ScratchBlock(scratch, arena)
    {
        LogFmtStd(arena, record, fmt, args);
        record->log = StrPushf(scratch, "%.*s, arena: %llu", StrExpand(record->log), arena->pos);
        record->log = StrCopy(arena, record->log);
    }
}

int main(void)
{
    OSInit(0, 0);
    
#if 0
    DEMO("Char")
    {
        i64 dx = 0x77E435B08;
        while (dx)
            putchar(0x726F6C6564574820 >> (((dx >>= 3) & 7) << 3) & 0xFF);
        printf("\n");
    }
    
    DEMO("Float")
    {
        i64 b = (1LL << 24)|1;
        f32 a = (f32)b;
        printf("i64: %lld, f32: %f\n", b, a);
        a = 5.f;
        printf("hex(5.0f) = %08x\n", *(i32*)&a);
        
        f32 x = 1.1f;
        if (x == 1.1f)
            printf("x equals 1.1f\n");
        
        if (x != 1.1)
            printf("x doesn't equals 1.1\n");
    }
#endif
    
    DEMO("Context")
    {
        printf("cl      = %d\n", COMPILER_CL);
        printf("clang   = %d\n", COMPILER_CLANG);
        printf("gcc     = %d\n", COMPILER_GCC);
        printf("widnows = %d\n", OS_WIN);
        printf("linux   = %d\n", OS_LINUX);
        printf("mac     = %d\n", OS_MAC);
        printf("x64     = %d\n", ARCH_X64);
        printf("x86     = %d\n", ARCH_X86);
        printf("arm     = %d\n", ARCH_ARM);
        printf("arm64   = %d\n", ARCH_ARM64);
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
                printf("Float: %22.17g String: %.*s\n", test[i].f, StrExpand(fStr));
            }
        }
    }
    
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
            
            LogInfo* info = LogGetInfo();
            info->level = LOG_INFO;
            LogPush(LOG_TRACE, "Log trace");
            LogPush(LOG_DEBUG, "Log debug");
            LogPush(LOG_INFO , "Log info");
            LogPush(LOG_WARN , "Log warn");
            LogPush(LOG_ERROR, "Log error");
            LogPush(LOG_FATAL, "Log fatal");
        }
        
        printf("-Normal-\n");
        for (StringNode* node = logs.first; node; node = node->next)
            printf("%.*s\n", StrExpand(node->string));
        
        u64 time = OSNowMS();
        u64 pos = ArenaCurrPos(arena);
        u64 overhead = ArenaCurrPos(logThread.arena);
        LogBlock(arena, logs, .callback = DemoLogCallback)
        {
            printf("\n-Stress Test-\n");
            for (u64 i = 0; i < 1024; ++i)
                LogPush(i % LogType_Count, "Log: %llu", i);
        }
        printf("Elapsed: %llums\n", OSNowMS() - time);
        printf("Memory: %llu bytes, Overhead: %llu bytes\n", ArenaCurrPos(arena) - pos, ArenaCurrPos(logThread.arena) - overhead);
        printf("Count: %llu nodes, Size: %llu bytes\n", logs.nodeCount, logs.totalSize);
    }
    
    DEMO("OS")
    {
        printf("---PATHS---\n");
        String currentDir = OSCurrentDir(arena);
        String processDir = OSProcessDir();
        String appDataDir = OSAppDataDir();
        String appTempDir = OSAppTempDir();
        printf("Curr: %.*s\\\n", StrExpand(currentDir));
        printf("Exe:  %.*s\\\n", StrExpand(processDir));
        printf("Data: %.*s\\\n", StrExpand(appDataDir));
        printf("Temp: %.*s\\\n", StrExpand(appTempDir));
        printf("File: %s\n", __FILE__);
        
        printf("\n");
        printf("---TIMES---\n");
        u64 sleep = 100;
        printf("Before: %llu ms\n", OSNowMS());
        printf("Sleep:  %8llu ms\n", (OSSleepMS((u32)sleep), sleep));
        printf("After:  %llu ms\n", OSNowMS());
        String now = StrFromTime(arena, OSNowUniTime());
        printf("Today:  %.*s\n", StrExpand(now));
        
        printf("\n");
        printf("---FILES---\n");
        FileIterBlock(arena, iter, StrLit("code"))
        {
            FileProperties prop = iter.prop;
            String createTime = StrFromTime(arena, OSToLocTime(TimeToDate(prop.createTime)));
            String modifyTime = StrFromTime(arena, OSToLocTime(TimeToDate(prop.modifyTime)));
            
            b32 isFolder = prop.flags & FilePropertyFlag_IsFolder;
            u64 size = prop.size;
            u64 childCount = 0;
            if (isFolder)
            {
                // NOTE(long): This is only one level deep
                String folderName = StrJoin3(arena, StrLit("code\\"), iter.name);
                FileIterBlock(arena, folder, folderName)
                {
                    size += folder.prop.size;
                    childCount++;
                }
            }
            
            if (isFolder)
                iter.name = StrPushf(arena, "%.*s: %d,", StrExpand(iter.name), childCount);
            else
                iter.name = StrJoin3(arena, iter.name, StrLit(","));
            
            printf("%s %-17.*s Created: %.*s, Date modified: %.*s, Size: %6.2f KB\n", isFolder ? "Dir: " : "File:",
                   StrExpand(iter.name), StrExpand(createTime), StrExpand(modifyTime), (f32)size / KB(1));
        }
    }
    
    ArenaRelease(arena);
    return 0;
}
