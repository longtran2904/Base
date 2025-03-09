#ifndef GLOB_STATIC_LIB
#define GLOB_STATIC_LIB 0
#endif

#if !GLOB_STATIC_LIB
#define ENABLE_ASSERT 0
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

typedef struct FileEntry FileEntry;
struct FileEntry
{
    FileEntry* next;
    String path;
    String data;
    HANDLE file;
};

global u64 getFileMs = 0;
global u64 loadFileMs = 0;

global String userPattern;
global u64 matchCount;

#define ENTRY_COUNT 64
#define READ_ASYNC ENTRY_COUNT > 0

// NOTE(long): Async reads are ~1.5x faster on my HDD drive, while ~3-5x faster on my SSD drive (~3GB tests)

#if READ_ASYNC
global FileEntry* fileEntries[ENTRY_COUNT];
global Arena*     readsArena[ENTRY_COUNT];
global OVERLAPPED readsOverlapped[ENTRY_COUNT];
global u64        readsPending;
StaticAssert(ENTRY_COUNT <= 64);

b32 StartRead(FileEntry* entry)
{
    u64 free = ~readsPending;
    if (!free) // back pressure the requestor if we already have our max reads pending
        return 0;
    
    u64 i = __ctz64(free);
    if (i >= ENTRY_COUNT)
        return 0;
    
    OVERLAPPED* overlapped = readsOverlapped + i;
    overlapped->Internal = 0;
    overlapped->InternalHigh = 0;
    overlapped->Pointer = 0;
    
    u64 now = OSNowMS();
    u64 len = entry->data.size;
    u8* buffer = ArenaPush(readsArena[i], len);
    
    DWORD bytes_read;
    if (ReadFile(entry->file, buffer, (DWORD)len, &bytes_read, overlapped))
    {
        Assert(bytes_read == len);
        
        // even though we did everything async, we completed synchronously
        // this can happen in Via if the data is already cached in memory
        goto PUSH;
    }
    else
    {
        // either we are running async or we failed, so check the error code
        Assert(GetLastError() == ERROR_IO_PENDING);
        
        PUSH:
        entry->data.str = buffer;
        readsPending |= 1ULL << i;
        fileEntries[i] = entry;
    }
    
    loadFileMs += OSNowMS() - now;
    //Outf("--%.*s--: %llu Bytes\n", StrExpand(path), len);
    return 1;
}

function void PushRead(Arena* arena, FileEntry** first, String path)
{
    u64 now = OSNowMS();
    HANDLE file = INVALID_HANDLE_VALUE;
    W32WidePath(wpath, path, file = CreateFileW(wpath.str, GENERIC_READ, FILE_SHARE_READ, 0,
                                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, 0));
    if (file == INVALID_HANDLE_VALUE)
        return;
    
    u64 len = 0;
    if (!(GetFileSizeEx(file, (PLARGE_INTEGER)&len) && InRange(len, 0, MAX_U32)))
        return;
    loadFileMs += OSNowMS() - now;
    
    FileEntry* entry = PushStruct(arena, FileEntry);
    SLLStackPush(*first, entry);
    entry->path = path;
    entry->file = file;
    entry->data.size = len;
}
#endif

function void GlobFile(String text, String file)
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
    HANDLE events[ENTRY_COUNT];
    for (u64 i = 0; i < ENTRY_COUNT; ++i)
    {
        readsArena[i] = ArenaReserve(GB(4ULL), 1, 0);
        events[i] = CreateEvent(0, 0, 0, 0);
        readsOverlapped[i].hEvent = events[i];
    }
#endif
    
    u64 ms = OSNowMS();
    
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
                
                case 'f': flags = (flags|Glob_CLI_File) & ~Glob_CLI_Name; break;
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
            
            u64 now;
            DeferBlock(now = OSNowMS(), getFileMs += OSNowMS() - now)
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
            u64 fileCount = paths.nodeCount;
            
            if (flags & Glob_CLI_File)
            {
#if READ_ASYNC
                FileEntry* firstEntry = 0;
                StrListIter(&paths, path)
                    PushRead(scratch, &firstEntry, path->string);
                
                READ:
                for (FileEntry* entry = firstEntry; entry; entry = entry->next)
                {
                    if (StartRead(entry))
                        firstEntry = entry->next;
                    else
                        break;
                }
                
                while (readsPending)
                {
                    u64 now = OSNowMS();
                    DWORD start = WaitForMultipleObjects(ENTRY_COUNT, events, 0, INFINITE);
                    loadFileMs += OSNowMS() - now;
                    
                    for (u64 i = start; i < ENTRY_COUNT; ++i)
                    {
                        OVERLAPPED* overlapped = readsOverlapped + i;
                        if (!(readsPending & (1ULL << i))) continue;
                        if (!HasOverlappedIoCompleted(overlapped)) continue;
                        
                        FileEntry* entry = fileEntries[i];
                        DWORD bytes_read;
                        ALWAYS(GetOverlappedResult(entry->file, overlapped, &bytes_read, 0));
                        Assert(bytes_read == entry->data.size);
                        
                        // do whatever you want with the data
                        GlobFile(entry->data, entry->path);
                        ArenaPopTo(readsArena[i], 0);
                        CloseHandle(entry->file);
                        
                        readsPending &= ~(1ULL << i);
                        if (firstEntry)
                            goto READ;
                    }
                }
#else
                StrListIter(&paths, path)
                {
                    String file = path->string;
                    TempBlock(temp, scratch)
                    {
                        u64 now = OSNowMS();
                        String text = OSReadFile(scratch, file);
                        loadFileMs += OSNowMS() - now;
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
            
            if (flags & Glob_CLI_Debug)
                Outf("Memory Usage: %llu Bytes, File Count: %llu\n", scratch->highWaterMark, fileCount);
        }
    }
    
    ms = OSNowMS() - ms;
    if (flags & Glob_CLI_Debug)
        Outf("Total: %llums, Get File: %llums, Load File: %llums\n", ms, getFileMs, loadFileMs);
    
    ScratchEnd(scratch);
    return 0;
}
#endif
