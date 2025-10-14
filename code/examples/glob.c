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

global String userPattern;

function u64 GlobFile(String text, String file)
{
    u64 matchCount = 0;
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
    
    return matchCount;
}

// NOTE(long): Async reads are ~1.5x faster on my HDD drive, while ~3-5x faster on my SSD drive (~3GB tests)

global OS_Handle iocp;
global u64 pushFileMs;
global u64 fileCount;
global u64 totalSize;
global u64 memoryUsage;

#include <stdlib.h>

typedef struct ThreadResult ThreadResult;
struct ThreadResult
{
    u64 waitMs;
    u64 workMs;
    u64 matchCount;
};

#define CHUNK_SIZE KiB(64)
#define MAX_REQUESTS 32

//- DiskMark
// 1024: 59.09 61.23 61.47 61.43 61.56
//  512: 58.83 59.04 59.58 59.93 59.87
//  256: 60.68 60.87 60.97 60.80 60.82
//  128: 59.49 60.69 60.87 60.93 60.28
//   64: 59.08 58.75 59.53 59.28 58.01

//- CHUNKING BUFFER

// 32 KiB
//c (55.31 + 56.15 + 55.07 + 55.51 + 55.76) / 5
//c (114.95 + 115.89 + 115.89 + 116.20 + 115.51) / 5

// 64 KiB
//c (58.40 + 59.06 + 58.06 + 58.78 + 58.31) / 5
//c (116.39 + 117.25 + 116.36 + 117.13 + 115.76) / 5

// 128 KiB
//c (58.47 + 58.58 + 58.38 + 59.01 + 59.10) / 5
//c (116.20 + 116.45 + 115.98 + 116.80 + 116.52) / 5

// 256 KiB
//c (59.00 + 58.89 + 58.62 + 58.38 + 58.56) / 5
//c (116.71 + 116.93 + 117.90 + 116.33 + 117.70) / 5

// 512 KiB
//c (55.80 + 56.24 + 55.72 + 56.19 + 56.02) / 5
//c (116.14 + 116.84 + 116.33 + 116.23 + 115.39) / 5

// 1024 KiB
//c (56.37 + 56.88 + 57.16 + 56.19 + 56.68) / 5
//c (116.61 + 116.23 + 116.04 + 116.39 + 116.74) / 5

// 2048 KiB
//c (57.21 + 57.30 + 56.88 + 57.23 + 57.05) / 5
//c (117.16 + 116.91 + 116.55 + 116.49 + 117.35) / 5

// 4096 KiB
//c (57.63 + 57.57 + 57.78 + 57.44 + 57.76) / 5
//c (116.14 + 117.22 + 116.90 + 116.68 + 117.38) / 5

// 8192 KiB
//c (57.44 + 57.96 + 57.49 + 57.33 + 57.90) / 5
//c (116.74 + 116.11 + 116.52 + 117.83 + 116.67) / 5

//- OUTSTANDING BUFFER

// 32 KiB
//c (57.64 + 58.65 + 58.06 + 58.46 + 57.90) / 5
//c (117.15 + 117.93 + 117.03 + 117.00 + 116.83) / 5

// 64 KiB
//c (57.74 + 57.28 + 57.20 + 57.61 + 57.66) / 5
//c (116.81 + 116.39 + 116.93 + 116.93 + 116.90) / 5

// 128 KiB
//c (58.07 + 54.46 + 58.38 + 58.15 + 57.65) / 5
//c (117.73 + 117.64 + 116.97 + 117.64 + 116.80) / 5

// 256 KiB
//c (57.73 + 57.83 + 57.46 + 57.43 + 57.82) / 5
//c (117.48 + 117.51 + 116.72 + 117.54 + 117.19) / 5

// 512 KiB
//c (58.00 + 57.56 + 57.50 + 57.63 + 57.95) / 5
//c (117.45 + 117.48 + 116.71 + 116.77 + 117.28) / 5

// 1024 KiB
//c (58.23 + 58.00 + 57.79 + 57.07 + 57.73) / 5
//c (115.29 + 116.55 + 115.82 + 115.17 + 114.95) / 5

// 2048 KiB
//c (56.49 + 57.94 + 57.58 + 57.80 + 57.45) / 5
//c (112.59 + 113.04 + 112.86 + 112.66 + 113.49) / 5

#if !CHUNK_SIZE
typedef struct FileEntry FileEntry;
struct FileEntry
{
    String path;
    String data;
    OS_Handle file;
};

// TODO(long): https://youtu.be/xbvaeBCbDDA?t=498
// workSize is only used for 2 things:
// 1. Signal that all works are done
// 2. Keep track of the maximum allocation size
// The first one can be replaced with a single bool where one thread read and the other thread write
// The second one can be replaced with each thread having a separate int
global volatile u64 workSize;
global u64 desiredSize = MiB(512);

function void ThreadProc(void* arg)
{
    volatile ThreadResult* info = arg;
    
    while (1)
    {
        FileEntry* entry = 0;
        TIME_BLOCK(duration, info->waitMs += duration)
        {
            u64 bytesRead = OS_FileWaitAsync(iocp, &entry, INFINITE);
            if (bytesRead == 0)
                Errf("ERROR: Failed to wait for file\n");
        }
        
        if (ALWAYS(entry))
        {
#if 0
            TIME_BLOCK(duration, info->workMs += duration)
                info->matchCount += GlobFile(entry->data, entry->path);
#endif
            
            OS_FileClose(entry->file);
            _aligned_free(entry->data.str);
            AtomicSub64(&workSize, entry->data.size);
        }
    }
}

function u64 ReadFilesAsync(Arena* arena, StringList* paths)
{
    u64 maxSize = 0;
    u64 loadMs = OSNowMS();
    
    StrListIter(paths, pathNode)
    {
        String path = pathNode->string;
        
        TIME_BLOCK(duration, pushFileMs += duration)
        {
            OS_Handle file = OS_FileOpen(path, AccessFlag_Read|AccessFlag_NoCache, iocp);
            u64 size = OS_FileProp(file).size;
            
            if (size)
            {
                FileEntry* entry = PushStruct(arena, FileEntry);
                entry->path = path;
                entry->file = file;
                entry->data.size = size;
                
                void* buffer = entry->data.str = _aligned_malloc(AlignUpPow2(size, 4096), 4096);
                if (!buffer)
                    Errf("ERROR: \"%.*s\" is too big (%.2f MiB)\n", StrExpand(path), DivF64(size, MiB(1)));
                else if (!OS_FileReadAsync(file, (r1u64){0}, buffer, entry))
                    Errf("ERROR: \"%.*s\"\n", StrExpand(path));
                
                else
                {
                    fileCount++;
                    totalSize += size;
                    
                    size = AtomicAdd64(&workSize, size);
                    maxSize = Max(maxSize, size);
                }
            }
        }
        
        while (workSize >= desiredSize)
            _mm_pause();
    }
    
    while (workSize)
        _mm_pause();
    
    loadMs = OSNowMS() - loadMs;
    Outf("Bandwidth: %.2f MiB/s, Loaded Files: %llu (%.2f MiB)\n",
         DivF64(totalSize, 1000 * loadMs), fileCount, DivF64(totalSize, MiB(1)));
    return maxSize;
}

#else
typedef struct FileState FileState;
struct FileState
{
    Arena* arena;
    String path;
    StringList lines;
    OS_Handle file;
    u64 size;
    volatile u64 readCount;
};

typedef struct FileEntry FileEntry;
struct FileEntry
{
    FileState* state;
    String data;
    u64 offset;
};

global volatile u64 requestCount;
global u64 chunkSize = CHUNK_SIZE;

function void ThreadProc(void* arg)
{
    volatile ThreadResult* info = arg;
    
    while (1)
    {
        FileEntry* entry = 0;
        TIME_BLOCK(duration, info->waitMs += duration)
        {
            u64 bytesRead = OS_FileWaitAsync(iocp, &entry, INFINITE);
            if (bytesRead == 0)
                Errf("ERROR: Failed to wait for file\n");
        }
        
        if (ALWAYS(entry))
        {
            FileState* state = entry->state;
            if (AtomicDec64(&state->readCount) == 0)
                OS_FileClose(state->file);
            AtomicDec64(&requestCount);
            
#if 0
            TIME_BLOCK(duration, info->workMs += duration)
            {
                String data = entry->data;
                if (entry->offset + chunkSize < state->size)
                {
                    i64 lastLine = StrFindArr(data, StrLit("\r\n"), MatchStr_LastMatch);
                    if (lastLine != data.size - 1)
                    {
#if 0
                        String leftover = StrCopy(state->arena, StrSkip(data, lastLine + 1));
                        StrListPush(state->arena, &state->lines, leftover);
#endif
                        
                        data = StrPrefix(data, lastLine);
                    }
                }
                
                if (data.size)
                    info->matchCount += GlobFile(data, state->path);
            }
#endif
            
            _aligned_free(entry->data.str);
        }
    }
}

function u64 ReadFilesAsync(Arena* arena, StringList* paths)
{
    u64 maxSize = 0;
    u64 stallMs = 0;
    u64  loadMs = OSNowMS();
    
    StrListIter(paths, pathNode)
    {
        TIME_BLOCK(duration, pushFileMs += duration)
        {
            String path = pathNode->string;
            OS_Handle file = OS_FileOpen(path, AccessFlag_Read|AccessFlag_NoCache, iocp);
            u64 size = OS_FileProp(file).size;
            
            FileState* state = PushStruct(arena, FileState);
            state->arena = ArenaMake();
            state->path = path;
            state->file = file;
            state->size = size;
            state->readCount = (u64)Ceil_f32((f32)size / chunkSize);
            
            for (u64 offset = 0; offset < size; offset += chunkSize)
            {
                FileEntry* entry = PushStruct(arena, FileEntry);
                entry->state = state;
                entry->offset = offset;
                
                entry->data.size = Min(size - entry->offset, chunkSize);
                u64 entrySize = AlignUpPow2(entry->data.size, 4096);
                entry->data.str = _aligned_malloc(entrySize, 4096);
                
                if (OS_FileReadAsync(file, R1U64Size(entry->offset, entrySize), entry->data.str, entry))
                {
                    if (offset == 0)
                    {
                        fileCount++;
                        totalSize += size;
                        maxSize = Max(size, maxSize);
                    }
                    
                    AtomicInc64(&requestCount);
                    TIME_BLOCK(stallDuration, stallMs += stallDuration)
                        while (requestCount >= MAX_REQUESTS)
                            _mm_pause();
                }
                
                else Errf("ERROR: \"%.*s\"\n", StrExpand(path));
            }
        }
    }
    
    while (requestCount)
        _mm_pause();
    pushFileMs -= stallMs;
    loadMs = OSNowMS() - loadMs;
    Outf("Total Stall: %llu ms, Bandwidth: %.2f MiB/s, Loaded Files: %llu (%.2f MiB)\n",
         stallMs, DivF64(totalSize, 1000 * loadMs), fileCount, DivF64(totalSize, MiB(1)));
    return maxSize;
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
        "           You can also specify the outstanding buffer size (in MiBs) here.\n"
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
    
#define THREAD_COUNT 1
    iocp = OS_PushFileQueue();
    ThreadResult results[THREAD_COUNT] = {0};
    for (u64 i = 0; i < THREAD_COUNT; ++i)
        OS_ThreadLaunch(ThreadProc, results + i);
    
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
                
                case 'f':
                {
                    invalid = 1;
                    if (opt->values.nodeCount == 1)
                    {
                        u64 num = U64FromStr(opt->values.first->string, 10, 0);
                        if (InRange(num, 1, KiB(8)))
                        {
#if CHUNK_SIZE
                            chunkSize = KiB(num);
                            Assert(chunkSize % 4096 == 0);
#else
                            desiredSize = MiB(num);
#endif
                            
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
    
    u64 iterFileMs = 0;
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
            
            TIME_BLOCK(duration, iterFileMs += duration)
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
    
    else if (flags & Glob_CLI_Help)
        Outf("%s", helpMessage);
    
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
            u64 matchCount = 0, waitFileMs = 0, globFileMs = 0;
            UNUSED(matchCount); UNUSED(waitFileMs);
            b32 globFile = flags & Glob_CLI_File;
            
            if (globFile)
            {
                if (paths.nodeCount)
                {
                    if (flags & Glob_CLI_Debug)
                    {
#if CHUNK_SIZE
                        Outf("Chunk size: %llu KiB, Threads: %d\n", chunkSize/KiB(1), THREAD_COUNT);
#else
                        Outf("Outstanding Buffer: %.2f MiB, Threads: %d\n", DivF64(desiredSize, MiB(1)), THREAD_COUNT);
#endif
                    }
                    
                    u64 maxSize = ReadFilesAsync(scratch, &paths);
                    memoryUsage += maxSize;
                    
                    for (u64 i = 0; i < THREAD_COUNT; ++i)
                    {
                        matchCount += results[i].matchCount;
                        waitFileMs += results[i].waitMs;
                        globFileMs += results[i].workMs;
                    }
                }
            }
            
            else
            {
                StrListIter(&paths, path)
                {
                    String file = path->string;
                    if (flags & Glob_CLI_Name)
                        file = StrSkipLastSlash(file);
                    
                    b32 match = 0;
                    TIME_BLOCK(duration, globFileMs += duration)
                        match = glob(userPattern, file, 0);
                    
                    if (match)
                    {
                        matchCount++;
                        Outf("%.*s\n", StrExpand(file));
                    }
                }
            }
            
#if 0
            if (matchCount)
                Outf("\n");
            Outf("Found %llu match(es) in %llu ", matchCount, paths.nodeCount);
            if (globFile)
                Outf("files (%.2f MiB)\n", DivF64(totalSize, MiB(1)));
            else
                Outf("file names\n");
#endif
            
            ms = OSNowMS() - ms;
            if (flags & Glob_CLI_Debug)
            {
#if 0
                memoryUsage += scratch->highWaterMark;
                Outf("Total: %llums, Memory Usage: %.3f MiB, Loaded Files: %llu\n",
                     ms, DivF64(memoryUsage, MiB(1)), fileCount);
                
                Outf("Iter File: %llums, ", iterFileMs);
                if (globFile)
                    Outf("Push File: %llums, Wait File: %.2f ms/Thread, Glob File: %.2f ms/Thread\n",
                         pushFileMs, DivF64(waitFileMs, THREAD_COUNT), DivF64(globFileMs, THREAD_COUNT));
                else
                    Outf("Glob: %llums\n", globFileMs);
                
#elif 0
                Outf("Total: %llums, Push File: %llums, Wait File: %.2f ms/Thread\n",
                     ms, pushFileMs, DivF64(waitFileMs, THREAD_COUNT));
#endif
            }
        }
    }
    
    ScratchEnd(scratch);
    return 0;
}
#endif
