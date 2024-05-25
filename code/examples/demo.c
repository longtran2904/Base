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
    
    DEMO("Context")
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
            {  1, ldexp(50883641005312716., -172), },
            {  2, ldexp(38162730753984537., -170), },
            {  3, ldexp(50832789069151999., -101), },
            {  4, ldexp(51822367833714164., -109), },
            {  5, ldexp(66840152193508133., -172), },
            {  6, ldexp(55111239245584393., -138), },
            {  7, ldexp(71704866733321482., -112), },
            {  8, ldexp(67160949328233173., -142), },
            {  9, ldexp(53237141308040189., -152), },
            { 10, ldexp(62785329394975786., -112), },
            { 11, ldexp(48367680154689523., - 77), },
            { 12, ldexp(42552223180606797., -102), },
            { 13, ldexp(63626356173011241., -112), },
            { 14, ldexp(43566388595783643., - 99), },
            { 15, ldexp(54512669636675272., -159), },
            { 16, ldexp(52306490527514614., -167), },
            { 17, ldexp(52306490527514614., -168), },
            { 18, ldexp(41024721590449423., - 89), },
            { 19, ldexp(37664020415894738., -132), },
            { 20, ldexp(37549883692866294., - 93), },
            { 21, ldexp(69124110374399839., -104), },
            { 22, ldexp(69124110374399839., -105), },
            
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
                String fStr = StrFromF64(arena, test[i].f, test[i].prec);
                printf("Float: %.9g, String: %.*s\n", test[i].f, StrExpand(fStr));
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
            
            info->level = LOG_DEBUG;
            info->callback = DemoLogCallback;
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
        
        LogBlock(arena, logs, .callback = DemoLogCallback)
        {
            printf("\n-Stress Test-\n");
            for (u64 i = 0; i < 256; ++i)
                LogPush(i % LogType_Count, "Log: %lld", i);
        }
        for (StringNode* node = logs.first; node; node = node->next)
            printf("%.*s\n", StrExpand(node->string));
    }
    
    DEMO("OS")
    {
        String currentName;
        FileProperties currentProp;
        OSFileIter iter = FileIterInit(StrLit("code"));
        while(FileIterNext(arena, &iter, &currentName, &currentProp))
        {
            currentName = StrJoin3(arena, currentName, StrLit(","));
            printf("Name: %-17.*s Created: %llu, Date modified: %llu, Size: %llu\n",
                   StrExpand(currentName), currentProp.createTime, currentProp.modifyTime, currentProp.size);
        }
    }
    
    ArenaRelease(arena);
    return 0;
}
