#ifndef GLOB_STATIC_LIB
#define GLOB_STATIC_LIB 0
#endif

#if !GLOB_STATIC_LIB
#define ENABLE_ASSERT 1
#endif

#include "Base.h"
#if !GLOB_STATIC_LIB
#include "Base.c"
#endif

//~ long: Glob Library

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

//~ long: Glob Program

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

global OS_Handle iocp;
global String userPattern;

global u64 fileCount;
global u64 totalSize;
global u64 loadFileMs;

#define MAX_REQUESTS 32
global volatile u64 requestCount;
global u64 chunkSize = KiB(64);

//- long: Block
typedef struct MatchLine MatchLine;
struct MatchLine
{
    MatchLine* next;
    u64 line;
    String text;
};

typedef struct FileBlock FileBlock;
struct FileBlock
{
    String prefix;
    String suffix;
    u64 lineCount;
    
    MatchLine* firstMatch;
    MatchLine*  lastMatch;
    u64 matchCount;
};

function void PushMatch(Arena* arena, FileBlock* block, String line)
{
    MatchLine* match = PushStruct(arena, MatchLine);
    match->line = block->lineCount + 1;
    match->text = StrCopy(arena, line);
    
    SLLQueuePush(block->firstMatch, block->lastMatch, match);
    block->matchCount++;
}

//- long: File
typedef struct FileState FileState;
struct FileState
{
    Arena* arena;
    FileBlock* blocks;
    String path;
    OS_Handle handle;
    u64 size;
    volatile u64 readCount;
};

typedef struct FileEntry FileEntry;
struct FileEntry
{
    FileState* file;
    String data;
    u64 offset;
};

function void GlobBlock(FileEntry* entry, FileState* file)
{
    FileBlock* block = file->blocks + (entry->offset/chunkSize);
    String text = entry->data;
    u64 offset = entry->offset;
    
    // NOTE(long): This doesn't handle long lines that span multiple chunks
    // In the future, I should rewrite glob from a recursive to an iterative style
    
    if (offset > 0)
    {
        i64 firstLine = StrFindChr(text, "\n");
        if (firstLine > 0)
        {
            block->prefix = StrCopy(file->arena, StrPrefix(text, firstLine));
            text = StrSkip(text, firstLine + 1);
        }
    }
    
    if (offset + chunkSize < file->size)
    {
        i64 lastLine = StrFindArr(text, StrLit("\n"), MatchStr_LastMatch);
        if (lastLine != text.size - 1 && lastLine > 0)
        {
            block->suffix = StrCopy(file->arena, StrSkip(text, lastLine + 1));
            text = StrPrefix(text, lastLine);
        }
    }
    
    for (i64 index; text.size; text = StrSkip(text, index + 1), ++block->lineCount)
    {
        index = StrFindChr(text, "\n");
        if (index < 0)
            index = text.size;
        
        if (index > 0 && text.str[index-1] == '\r')
            index--;
        
        if (index > 0)
        {
            String line = StrPrefix(text, index);
            if (glob(userPattern, line, 0))
                PushMatch(file->arena, block, line);
        }
    }
}

//- long: Thread
typedef struct ThreadResult ThreadResult;
struct ThreadResult
{
    u64 waitMs;
    u64 workMs;
    u64 matchCount;
};

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
            FileState* file = entry->file;
            TIME_BLOCK(duration, info->workMs += duration)
                GlobBlock(entry, file);
            
            if (AtomicDec64(&file->readCount) == 0)
            {
                String path = file->path;
                u64 line = 1;
                u64 blockCount = (u64)Ceil_f32(DivF32(file->size, chunkSize));
                FileBlock* block = file->blocks;
                
                // Print matches
                for (u32 blockIdx = 0; blockIdx < blockCount; ++blockIdx, line += block->lineCount, ++block)
                {
                    if (blockIdx < blockCount - 1)
                    {
                        String text = StrJoin3(file->arena, block->suffix, (block+1)->prefix);
                        if (glob(userPattern, text, 0))
                            PushMatch(file->arena, block, text);
                    }
                    
                    for (MatchLine* match = block->firstMatch; match; match = match->next)
                        Outf("%.*s(%lld): %.*s\n", StrExpand(path), match->line + line, StrExpand(match->text));
                    info->matchCount += block->matchCount;
                }
                
                // Free file
                OS_FileClose(file->handle);
                ArenaRelease(file->arena);
            }
            
            // NOTE(long): Pop after globbing to ensure the program
            // exits only after all files have been fully globbed
            AtomicDec64(&requestCount);
            OSRelease(entry->data.str);
        }
    }
}

//- long: IO
function void ReadFilesAsync(Arena* arena, StringList* paths)
{
    u64 maxSize = 0;
    loadFileMs = OSNowMS();
    
    StrListIter(paths, pathNode)
    {
        String path = pathNode->string;
        OS_Handle handle = OS_FileOpen(path, AccessFlag_Read|AccessFlag_NoCache, iocp);
        u64 size = OS_FileProp(handle).size;
        
        Arena* fileArena = ArenaMake();
        FileState* file = PushStruct(fileArena, FileState);
        file->arena = fileArena;
        file->path = path;
        file->handle = handle;
        file->size = size;
        
        u64 readCount = (u64)Ceil_f32(DivF32(size, chunkSize));
        file->blocks = PushArray(file->arena, FileBlock, readCount);
        file->readCount = readCount;
        
        for (u64 offset = 0; offset < size; offset += chunkSize)
        {
            // NOTE(long): Can't use file->arena because ThreadProc might still be using it (it's not thread-safe)
            FileEntry* entry = PushStruct(arena, FileEntry);
            entry->file = file;
            entry->offset = offset;
            
            u64 entrySize = Min(size - entry->offset, chunkSize);
            entrySize = AlignUpPow2(entrySize, 4096);
            entry->data.size = entrySize;
            entry->data.str = OSCommit(0, entrySize);
            
            if (ALWAYS(OS_FileReadAsync(handle, R1U64Size(entry->offset, entrySize), entry->data.str, entry)))
            {
                if (offset == 0)
                {
                    fileCount++;
                    totalSize += size;
                    maxSize = Max(size, maxSize);
                }
                
                AtomicInc64(&requestCount);
                while (requestCount >= MAX_REQUESTS)
                    _mm_pause();
            }
            
            else Errf("ERROR: \"%.*s\"\n", StrExpand(path));
        }
    }
    
    while (requestCount)
        _mm_pause();
    loadFileMs = OSNowMS() - loadFileMs;
}

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
    
#define THREAD_COUNT 2
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
                            chunkSize = KiB(num);
                            Assert(chunkSize % 4096 == 0);
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
                        Outf("Chunk size: %llu KiB, Threads: %d\n", chunkSize/KiB(1), THREAD_COUNT);
                    
                    ReadFilesAsync(scratch, &paths);
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
            
            if (matchCount)
                Outf("\n");
            Outf("Found %llu match(es) in %llu file%s\n", matchCount, paths.nodeCount, globFile ? "s" : " names");
            
            ms = OSNowMS() - ms;
            if (flags & Glob_CLI_Debug)
            {
                Outf("Total: %llums, Bandwidth: %.2f MiB/s, Loaded Files: %llu (%.2f MiB)\n",
                     ms, DivF64(totalSize, 1000 * loadFileMs), fileCount, DivF64(totalSize, MiB(1)));
                
                Outf("Iter File: %llums, ", iterFileMs);
                if (globFile)
                    Outf("Wait File: %.2f ms/Thread, Glob File: %.2f ms/Thread\n",
                         DivF64(waitFileMs, THREAD_COUNT), DivF64(globFileMs, THREAD_COUNT));
                else
                    Outf("Glob: %llums\n", globFileMs);
            }
        }
    }
    
    ScratchEnd(scratch);
    return 0;
}
#endif
