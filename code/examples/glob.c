#ifndef GLOB_STATIC_LIB
#define GLOB_STATIC_LIB 0
#endif

#if !GLOB_STATIC_LIB
//#define ENABLE_ASSERT 0
#endif

#include "Base.h"
#if !GLOB_STATIC_LIB
#include "Base.c"
#endif

enum
{
    GLOB_SYNTAX_ERROR = 1,
    GLOB_SYNTAX_EXTENSION = 2,
};

function Flags32 GetGlobError(String pattern)
{
    Flags32 result = 0;
    
    u64 patternIndex = 0;
    while (patternIndex < pattern.size)
    {
#define SYNTAX_ERROR() Stmnt(result |= GLOB_SYNTAX_ERROR; goto END;);
        
        switch (pattern.str[patternIndex])
        {
            case '\\':
            {
                patternIndex++;
                if (patternIndex == pattern.size)
                    SYNTAX_ERROR();
            } break;
            
            case '[':
            {
                u64 startIndex = ++patternIndex;
                if (startIndex < pattern.size && pattern.str[startIndex] == '!')
                    startIndex++;
                if (startIndex + 1 >= pattern.size)
                    SYNTAX_ERROR();
                
                while (patternIndex < pattern.size)
                {
                    if (startIndex != patternIndex)
                    {
                        switch (pattern.str[patternIndex])
                        {
                            case ']': goto BREAK;
                            case '-':
                            {
                                if (patternIndex + 1 == pattern.size)
                                    SYNTAX_ERROR();
                                
                                if (pattern.str[patternIndex + 1] != ']')
                                {
                                    b32 ext1 = patternIndex - startIndex >= 3 && pattern.str[patternIndex-2] == '-';
                                    b32 ext2 = (patternIndex + 2 < pattern.size &&
                                                pattern.str[patternIndex+1] == '-' &&
                                                pattern.str[patternIndex+2] != ']');
                                    result |= ext1 || ext2 ? GLOB_SYNTAX_EXTENSION : 0;
                                    patternIndex++;
                                }
                            } break;
                        }
                    }
                    
                    patternIndex++;
                }
                
                if (pattern.str[pattern.size] != ']')
                    SYNTAX_ERROR();
                
            } BREAK:
            break;
        }
        
        patternIndex++;
    }
    
    END:
    return result;
#undef SYNTAX_ERROR
}

function b32 glob(String pattern, String text, Flags32* error)
{
    b32 result = 0;
    NilPtr(Flags32, error);
    
    u64 textIndex = 0, patternIndex = 0;
    while (patternIndex < pattern.size && textIndex < text.size)
    {
#define SYNTAX_ERROR(result) Stmnt(*error |= GLOB_SYNTAX_ERROR; return result;)
        
        switch (pattern.str[patternIndex])
        {
            case '?': break;
            
            case '*':
            {
                if (glob(StrSkip(pattern, patternIndex + 1), StrSkip(text, textIndex), error))
                {
                    result = 1;
                    goto END;
                }
                
                textIndex++; // NOTE(long): Advance text not pattern
            } continue;
            
            case '\\':
            {
                patternIndex++;
                if (patternIndex == pattern.size)
                    SYNTAX_ERROR(text.str[textIndex] == '\\' && textIndex + 1 == text.size);
            } fallthrough;
            
            default:
            {
                if (pattern.str[patternIndex] != text.str[textIndex])
                    goto END;
            } break;
            
            case '[':
            {
                b32 matched = 0;
                b32 negate = 0;
                u64 startIndex = ++patternIndex;
                
                if (startIndex < pattern.size && pattern.str[startIndex] == '!')
                {
                    startIndex++;
                    negate = 1;
                }
                
                if (startIndex + 1 >= pattern.size)
                    SYNTAX_ERROR(0);
                
                while (patternIndex < pattern.size)
                {
                    if (startIndex != patternIndex)
                    {
                        switch (pattern.str[patternIndex])
                        {
                            case ']': goto EXIT;
                            
                            case '-':
                            {
                                if (patternIndex + 1 == pattern.size)
                                    SYNTAX_ERROR((matched ^ negate) || text.str[textIndex] == '-'); // unclosed [
                                
                                if (pattern.str[patternIndex + 1] != ']')
                                {
                                    // NOTE(long): "[*---c]" is extensions and will work
                                    if ((patternIndex - startIndex >= 3     && pattern.str[patternIndex-2] == '-') || // "[a-d-g]"
                                        (patternIndex + 2 < pattern.size    && // "[*--b]"
                                         pattern.str[patternIndex+1] == '-' && pattern.str[patternIndex+2] != ']'))
                                        *error |= GLOB_SYNTAX_EXTENSION;
                                    
                                    // NOTE(long): This will match '\' just fine
                                    matched |= (pattern.str[patternIndex - 1] <= text.str[textIndex] &&
                                                pattern.str[patternIndex + 1] >= text.str[textIndex]);
                                    patternIndex++;
                                    goto NEXT;
                                }
                            } break;
                        }
                    }
                    
                    matched |= pattern.str[patternIndex] == text.str[textIndex];
                    NEXT:
                    patternIndex++;
                }
                
                if (pattern.str[pattern.size] != ']')
                    SYNTAX_ERROR(matched ^ negate);
                
                EXIT:
                if (matched == negate)
                    goto END;
            } break;
        }
        
        patternIndex++;
        textIndex++;
    }
    
    if (textIndex == text.size)
    {
        while (patternIndex < pattern.size && pattern.str[patternIndex] == '*')
            patternIndex++;
        if (patternIndex == pattern.size)
            result = 1;
    }
    
    END:
    return result;
#undef SYNTAX_ERROR
}

#if !GLOB_STATIC_LIB
enum
{
    Glob_CLI_Help      = 1 << 0,
    Glob_CLI_File      = 1 << 1,
    Glob_CLI_Name      = 1 << 2,
    Glob_CLI_Ext       = 1 << 3,
    Glob_CLI_Debug     = 1 << 4,
    Glob_CLI_Recursive = 1 << 5,
};

global u64 getHandleMs = 0;
global u64 getFileMs = 0;
global u64 loadFileMs = 0;
global u64 globFileMs = 0;

global String userPattern;
global u64 matchCount;

function void GlobFile(String text, String file)
{
    TIME_BLOCK(duration, globFileMs += duration)
    {
        for (i64 index, row = 1; text.size; text = StrSkip(text, index + 1), ++row)
        {
            index = StrFindChr(text, "\n");
            if (index < 0)
                break;
            if (index > 0 && text.str[index-1] == '\r')
                index--;
            
            if (index > 0)
            {
                String line = StrPrefix(text, index);
                if (glob(userPattern, line, 0))
                {
                    matchCount++;
                    Outf("%.*s(%lld): %.*s\n", StrExpand(file), row, StrExpand(line));
                }
            }
        }
    }
}

#define READ_ASYNC 1

// NOTE(long): Async reads are ~1.5x faster on my HDD drive, while ~3-5x faster on my SSD drive (~3GB tests)

#if READ_ASYNC
typedef struct FileEntry FileEntry;
struct FileEntry
{
    FileEntry* next;
    Arena* arena;
    
    String path;
    String data;
    
    HANDLE file;
    OVERLAPPED overlapped;
};

global HANDLE iocp;

function b32 ReadFileAsync(Arena* arena, FileEntry* entry)
{
    b32 result = 0;
    u64 len = entry->data.size;
    
    TIME_BLOCK(duration, loadFileMs += duration)
    {
        u8* buffer = 0;
        if (CreateIoCompletionPort(entry->file, iocp, (ULONG_PTR)entry, 0))
        {
            DWORD bytesRead;
            //buffer = ArenaPush(arena, len);
            buffer = malloc(len);
            
            if (!buffer)
                Errf("ERROR: \"%.*s\" is too big (%.2f MiB)\n", StrExpand(entry->path),
                     DivF64(entry->data.size, MiB(1)));
            else if (ReadFile(entry->file, buffer, (DWORD)len, &bytesRead, &entry->overlapped))
                result = bytesRead == len;
            else
                result = GetLastError() == ERROR_IO_PENDING;
        }
        
        if (result)
        {
            entry->data.str = buffer;
            entry->arena = arena;
        }
    }
    
    return result;
}

function FileEntry* WaitForFile(void)
{
    FileEntry* entry = 0;
    u64 startMs = OSNowMS();
    if (!GetQueuedCompletionStatus(iocp, &(DWORD){0}, (PULONG_PTR)&entry, &(LPOVERLAPPED){0}, INFINITE))
        Errf("\nERROR: Failed to wait for file\n");
    loadFileMs += OSNowMS() - startMs;
    return entry;
}

global HANDLE semaphore;
//global FileEntry* volatile workEntry;
global volatile u64 workCount;

#include <stdlib.h>

function DWORD WINAPI ThreadProc(LPVOID arg)
{
    UNUSED(arg);
    
    u64 oldCount = 0;
    while (1)
    {
#if 1
        FileEntry* entry = WaitForFile();
        if (ALWAYS(entry))
        {
            GlobFile(entry->data, entry->path);
            CloseHandle(entry->file);
            free(entry->data.str);
            u64 count = AtomicDec64(&workCount);
            if (count != oldCount - 1)
                Outf("Pending: %llu\n", count);
            oldCount = count;
        }
        
#else
        WaitForSingleObject(semaphore, INFINITE);
        u64 count = 0;
        
        for (FileEntry* entry = (FileEntry*)AtomicXch64(&workEntry, 0); entry; count++, entry = entry->next)
        {
            GlobFile(entry->data, entry->path);
            CloseHandle(entry->file);
            free(entry->data.str);
            AtomicDec64(&workCount);
        }
        
        if (count > maxCount)
        {
            maxCount = count;
            Outf("Pop: %llu\n", count);
        }
#endif
    }
}

function u64 ReadAndProcess(FileEntry* firstEntry, u64 desiredSize)
{
    //u64 swapCount = 0, popCount = 0;
    //u64 currArena = 0, nextArena = 0;
    //u64 totalSizes[2] = {0};
    Arena* arenas[2] = { ArenaReserve(desiredSize, 1, 1), ArenaReserve(desiredSize, 1, 1) };
    //u64 maxFileCount = 0;
    u64 fileCount = 0;
    
    //PUSH_READ:
    //u64 remainSize = ClampBot((i64)(desiredSize - totalSizes[currArena]), 0);
    for (FileEntry* entry = firstEntry; entry /*&& totalSizes[nextArena] < remainSize*/; firstEntry = entry = entry->next)
    {
        if (ReadFileAsync(arenas[/*nextArena*/0], entry))
        {
            //totalSizes[nextArena] += entry->data.size;
            fileCount++;
            AtomicInc64(&workCount);
        }
    }
    
    /*if (fileCount > maxFileCount)
    maxFileCount = fileCount;
    
    // Only true the first time
    if (nextArena == currArena)
    {
    nextArena = 1;
    Outf("Start Loads: %llu\n", fileCount);
    }*/
    
#if 0
    FileEntry* entry = WaitForFile();
    if (ALWAYS(entry))
    {
        u64 entryIdx = arenas[currArena] == entry->arena ? currArena : nextArena;
        totalSizes[entryIdx] -= entry->data.size;
        if (totalSizes[entryIdx] == 0)
        {
            popCount++;
            if (entryIdx == currArena)
            {
                swapCount++;
                Swap(u64, currArena, nextArena);
            }
        }
        
        SLLAtomicPush(workEntry, entry);
        AtomicInc64(&workCount);
        ReleaseSemaphore(semaphore, 1, NULL);
        
        if (--fileCount)
            goto PUSH_READ;
        while (workEntry);
    }
    
#elif 0
    FileEntry* entry = WaitForFile();
    if (ALWAYS(entry))
    {
        // Process file
        fileCount--;
        GlobFile(entry->data, entry->path);
        CloseHandle(entry->file);
        
        u64 entryIdx = arenas[currArena] == entry->arena ? currArena : nextArena;
        totalSizes[entryIdx] -= entry->data.size;
        if (totalSizes[entryIdx] == 0)
        {
            ArenaPopTo(arenas[entryIdx], 0);
            popCount++;
            
            if (entryIdx == currArena) // Swap when all reads are done
            {
                swapCount++;
                Swap(u64, currArena, nextArena);
            }
        }
        
        // Is there any file still waiting? The firstEntry can be null because the loop checks for it.
        if (fileCount)
            goto PUSH_READ;
    }
#endif
    
    while (workCount);
    //Outf("Swaps: %llu, Pops: %llu, Async: %llu, Work Count: %llu\n", swapCount, popCount, maxFileCount, totalFileCount);
    Outf("Work Count: %llu\n", fileCount);
    return arenas[0]->highWaterMark + arenas[1]->highWaterMark;
}

function void PushRead(Arena* arena, FileEntry** first, String path)
{
    HANDLE file = INVALID_HANDLE_VALUE;
    u64 len = 0;
    
    TIME_BLOCK(duration, getHandleMs += duration)
    {
        W32WidePath(wpath, path, file = CreateFileW(wpath.str, GENERIC_READ, FILE_SHARE_READ, 0,
                                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, 0));
        if (!(GetFileSizeEx(file, (PLARGE_INTEGER)&len) && InRange(len, 1, MAX_U32)))
            return;
    }
    
    FileEntry* entry = PushStruct(arena, FileEntry);
    SLLStackPush(*first, entry);
    entry->path = path;
    entry->file = file;
    entry->data.size = len;
}
#endif

int main(i32 argc, char** argv)
{
    char* helpMessage =
        "[SYNTAX]\n\n"
        "    glob [options...] \"pattern\" [paths...]\n\n"
        "[OPTIONS]\n\n"
        "    /e[-]  Turns on all the extensions. By default, /e is on. /e- disables all the ns.\n"
        "    /f     Rather than match the file's path, match the file's content instead. Will override /n.\n"
        "    /n     Rather than match the file's path, match the file's name instead. Will override /f.\n"
        "    /r     Recursively searches files starting from specified directories.\n"
        "    /d     Displays the internal memory usage. This is for debugging only.\n"
        "    /?, /h Displays this help message.\n\n"
        "[DESCRIPTION]\n\n"
        "    Globbing is the operation that expands a wildcard pattern into the list of pathnames\n"
        "    matching the pattern. Matching is defined by:\n"
        "    A '?' (not between brackets) matches any single character.\n"
        "    A '*' (not between brackets) matches any string, including the empty string.\n\n"
        "  Character classes\n\n"
        "    An expression \"[...]\" where the first character after the leading '[' is not an '!'\n"
        "    matches a single character, namely any of the characters enclosed by the brackets.\n"
        "    The string enclosed by the brackets cannot be empty; therefore ']' can be allowed\n"
        "    between the brackets, provided that it is the first character.  (Thus, \"[][!]\"\n"
        "    matches the three characters '[', ']', and '!'.)\n\n"
        "  Ranges\n\n"
        "    There is one special convention: two characters separated by '-' denote a range.\n"
        "    (Thus, \"[A-Fa-f0-9]\" is equivalent to \"[ABCDEFabcdef0123456789]\".)\n"
        "    One may include '-' in its literal meaning by making it the first or last character\n"
        "    between the brackets. (Thus, \"[]-]\" matches just the two characters ']' and '-',\n"
        "    and \"[--0]\" matches the four characters '-', '.', '/', and '0', which is different\n"
        "    from the standard because the standard won't match the character '/'.)\n\n"
        "  Complementation\n\n"
        "    An expression \"[!...]\" matches a single character, namely any character that is not\n"
        "    matched by the expression obtained by removing the first '!' from it. (Thus, \"[!]a-]\"\n"
        "    matches any single character except ']', 'a', and '-'.)\n\n"
        "    One can remove the special meaning of '?', '*', and '[' by preceding them by a\n"
        "    backslash, or, in case this is part of a shell command line, enclosing them in quotes.\n"
        "    Between brackets these characters stand for themselves. Thus, \"[[?*\\]\" matches the\n"
        "    four characters '[', '?', '*', and '\\'.\n\n"
        "  Extensions\n\n"
        "    1. As mentioned above, the character '/' will be matched just fine.\n"
        "    2. An expression \"[a-c-e]\" is equal to \"[a-e]\".\n"
        "    3. A '-' between the brackets can be its literal meaning without being the first or\n"
        "    last character. It just needs to be inside a Range denotation. (Thus, \"[a*--B]\"\n"
        "    matches the six characters 'a', 'b', '*', '+', ',', '-').\n\n"
        "    Based on these 3 rules, \"[*---A]\" is equivalent to \"[*+,-*/0123456789:;<=>?@A]\"\n";
    
    ScratchBegin(scratch);
    
#if READ_ASYNC
    iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    semaphore = CreateSemaphore(0, 0, 1, 0);
    CreateThread(0, 0, ThreadProc, 0, 0, 0);
#endif
    
    u64 ms = OSNowMS();
    u64 desiredSize = MiB(256); // 150MB/s * 5ms
    
    StringList paths = {0};
    StringList errors = {0};
    Flags32 flags = (argc == 1 ? Glob_CLI_Help : 0) | Glob_CLI_Ext;
    
    StringList args = OSSetArgs(argc, argv);
    CmdLine cmd = CmdLineFromList(scratch, &args);
    for (CmdLineOpt* opt = cmd.opts.first; opt; opt = opt->next)
    {
        b32 invalid = 0;
        
        if (opt->name.size == 1)
        {
            switch (opt->name.str[0])
            {
                case 'e':
                {
                    if (opt->values.nodeCount == 0)
                        flags |= Glob_CLI_Ext;
                    else if (!StrIsChr(opt->values.first->string, '-'))
                        invalid = 1;
                }
                
                case 'h':
                case '?': flags |= Glob_CLI_Help; break;
                
                case 'f':
                {
                    invalid = 1;
                    if (opt->values.nodeCount == 1)
                    {
                        u64 num = U64FromStr(opt->values.first->string, 10, 0);
                        if (InRange(num, 1, KiB(8)))
                        {
                            desiredSize = MiB(num);
                            goto FLAG_FILE;
                        }
                    }
                    
                    else if (opt->values.nodeCount == 0)
                    {
                        FLAG_FILE:
                        invalid = 0;
                        flags = (flags|Glob_CLI_File) & ~Glob_CLI_Name;
                    }
                } break;
                
                case 'n': flags = (flags|Glob_CLI_Name) & ~Glob_CLI_File; break;
                case 'd': flags |= Glob_CLI_Debug; break;
                case 'r': flags |= Glob_CLI_Recursive; break;
                
                default: invalid = 1;
            }
        }
        else invalid = 1;
        
        if (invalid)
            StrListPushf(scratch, &errors, "ERROR: Invalid argument or option - '%.*s'.", StrExpand(opt->name));
    }
    
    String currDir = OSGetCurrDir(scratch);
    for (StringNode* input = cmd.inputs.first; input; input = input->next)
    {
        String arg = input->string;
        if (!userPattern.size)
        {
            userPattern = arg;
            if (input == cmd.inputs.last)
                StrListPush(scratch, &cmd.inputs, StrLit("."));
        }
        
        else
        {
            if (arg.size && arg.str[arg.size-1] == '*')
                arg = StrChop(arg, 1);
            if (arg.size && arg.str[arg.size-1] == '\\')
                arg = StrChop(arg, 1);
            
            if (StrCompare(arg, StrLit(".."), 0))
                arg = StrChopLastSlash(currDir);
            if (!arg.size || StrIsChr(arg, '.'))
                arg = currDir;
            
            TIME_BLOCK(duration, getFileMs += duration)
            {
                if (OSFileProperties(arg).flags & FilePropertyFlag_IsFolder)
                {
                    OSFileIterFlags flg = FileIterFlag_SkipFolders;
                    flg |= (flags & Glob_CLI_Recursive) ? FileIterFlag_Recursive : 0;
                    
                    FileIterBlock(scratch, iter, arg, flg)
                    {
                        String str = StrPushf(scratch, "%.*s\\%.*s", StrExpand(iter.path), StrExpand(iter.name));
                        StrListPush(scratch, &paths, str);
                    }
                }
                else StrListPush(scratch, &paths, arg);
            }
        }
    }
    
    if (!errors.nodeCount && !(flags & Glob_CLI_Help) && !userPattern.size)
        StrListPushf(scratch, &errors, "ERROR: Missing pattern string.\n");
    
    if (errors.nodeCount)
    {
        StrListIter(&errors, node)
            Errf("%.*s\n", StrExpand(node->string));
        Errf("Type \"glob /?\" for usage help.\n");
    }
    
    else if (flags & Glob_CLI_Help) Outf("%s", helpMessage);
    
    else
    {
        Flags32 error = GetGlobError(userPattern);
        if (error & GLOB_SYNTAX_ERROR)
            Errf("ERROR: Invalid syntax.\n");
        
        if (error & GLOB_SYNTAX_EXTENSION)
        {
            error = !(flags & Glob_CLI_Ext);
            if (error)
                Errf("ERROR: Extension syntax.\n");
        }
        
        if (!error)
        {
            u64 memoryUsage = 0, smallCount = 0, smallSize = 0, bigCount = 0, bigSize = 0;
            
            if (flags & Glob_CLI_File)
            {
#if READ_ASYNC
                FileEntry* firstEntry = 0;
                StrListIter(&paths, path)
                    PushRead(scratch, &firstEntry, path->string);
                
                if (firstEntry)
                {
                    FileEntry* firstBigEntry = 0,* last = 0;
                    for (FileEntry* entry = firstEntry; entry; last = entry, entry = entry->next)
                    {
                        if (entry->data.size >= desiredSize)
                        {
                            if (last)
                                last->next = entry->next;
                            else
                                firstEntry = entry->next;
                            
                            bigCount++;
                            bigSize += entry->data.size;
                            
                            SLLStackPush(firstBigEntry, entry);
                            entry = last;
                        }
                        
                        else
                        {
                            smallCount++;
                            smallSize += entry->data.size;
                        }
                    }
                    
                    if (firstBigEntry)
                    {
                        if (last)
                            last->next = firstBigEntry;
                        else
                            firstEntry = firstBigEntry;
                    }
                    
                    u64 mb = MiB(1);
                    Outf("Desired: %.2f MiB, Small: %llu Files (%.2f MiB), Big: %llu Files (%.2f MiB)\n",
                         DivF64(desiredSize, mb), smallCount, DivF64(smallSize, mb), bigCount, DivF64(bigSize, mb));
                    memoryUsage += ReadAndProcess(firstEntry, desiredSize);
                }
#else
                StrListIter(&paths, path)
                {
                    String file = path->string;
                    TempBlock(temp, scratch)
                    {
                        String text;
                        TIME_BLOCK(duration, loadFileMs += duration)
                            text = OSReadFile(scratch, file);
                        GlobFile(text, file);
                    }
                }
#endif
            }
            
            else
            {
                StrListIter(&paths, path)
                {
                    String file = path->string;
                    if (flags & Glob_CLI_Name)
                        file = StrSkipLastSlash(file);
                    
                    if (glob(userPattern, file, 0))
                    {
                        matchCount++;
                        Outf("%.*s\n", StrExpand(file));
                    }
                }
            }
            
            if (matchCount)
                Outf("Found %llu matches\n", matchCount);
            else
                Outf("No matches found\n");
            
            memoryUsage += scratch->highWaterMark;
            if (flags & Glob_CLI_Debug)
                Outf("Memory Usage: %.3f MiB, File Count: %llu\n", DivF64(memoryUsage, MiB(1)), paths.nodeCount);
        }
    }
    
    ms = OSNowMS() - ms;
    if (flags & Glob_CLI_Debug)
        Outf("Total: %llums, Get File: %llums, Get Handle: %llums, Load File: %llums, Glob File: %llums\n",
             ms, getFileMs, getHandleMs, loadFileMs, globFileMs);
    
    ScratchEnd(scratch);
    return 0;
}
#endif
