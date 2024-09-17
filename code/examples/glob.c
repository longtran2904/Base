#ifndef GLOB_STATIC_LIB
#define GLOB_STATIC_LIB 0
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
                                         pattern.str[patternIndex+1] == '-' && pattern.str[patternIndex+2] != ']')
                                        )
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
    Glob_CLI_Help  = 1 << 0,
    Glob_CLI_File  = 1 << 1,
    Glob_CLI_Ext   = 1 << 2,
    Glob_CLI_Debug = 1 << 3,
};

int main(i32 argc, char** argv)
{
    char* helpMessage =
        "[SYNTAX]\n\n"
        "    glob [options...] \"pattern\" [paths...]\n\n"
        "[OPTIONS]\n\n"
        "    /e[-] Turns on all the extensions. By default, /e is on. /e- disables all the extensions.\n"
        "    /f    Rather than match the file's name, match the file's content instead.\n"
        "    /d    Displays the internal memory usage. This is for debugging only.\n"
        "    /?    Displays this help message.\n\n"
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
    
    ScratchBlock(scratch)
    {
        String pattern = {0};
        StringList paths = {0};
        StringList errors = {0};
        Flags32 flags = (argc == 1 ? Glob_CLI_Help : 0) | Glob_CLI_Ext;
        
        StringList args = OSSetArgs(argc, argv);
        CmdLine cmd = CmdLineFromStrList(scratch, &args);
        for (CmdLineOpt* opt = cmd.opts.first; opt; opt = opt->next)
        {
            StringList nameList = StrList(scratch, ArrayExpand(String, StrLit("?"), StrLit("f"), StrLit("e"), StrLit("d")));
            u64 optType = 0;
            if (StrCompareListEx(opt->name, &nameList, 0, 0, &optType))
            {
                flags |= 1 << optType;
                if (opt->values.nodeCount)
                {
                    if ((1 << optType) == Glob_CLI_Ext)
                    {
                        if (StrIsChr(opt->values.first->string, '-'))
                            flags &= ~Glob_CLI_Ext;
                        else
                            goto INVALID;
                    }
                    else goto INVALID;
                }
            }
            else
                INVALID: StrListPushf(scratch, &errors, "ERROR: Invalid argument or option - '/%.*s'.", StrExpand(opt->name));
        }
        
        for (StringNode* input = cmd.inputs.first; input; input = input->next)
        {
            String arg = input->string;
            if (!pattern.size)
            {
                pattern = arg;
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
                    arg = StrChopAfter(OSGetCurrDir(scratch), StrLit("\\"), MatchStr_LastMatch);
                if (!arg.size || StrIsChr(arg, '.'))
                    arg = OSGetCurrDir(scratch);
                
                if (OSFileProperties(arg).flags & FilePropertyFlag_IsFolder)
                {
                    FileIterBlock(scratch, iter, arg)
                    {
                        String str = StrJoin3(scratch, arg, StrLit("\\"), iter.name);
                        str = PathNormString(scratch, str);
                        StrListPush(scratch, &paths, str);
                    }
                }
                else
                    StrListPush(scratch, &paths, arg);
            }
        }
        
        if (!errors.nodeCount && !(flags & Glob_CLI_Help) && !pattern.size)
            StrListPushf(scratch, &errors, "ERROR: Missing pattern string.\n");
        
        if (errors.nodeCount)
        {
            StrListIter(&errors, node)
                Errf("%.*s\n", StrExpand(node->string));
            Errf("Type \"glob /?\" for usage help.\n");
        }
        else
        {
            if (flags & Glob_CLI_Help) Outf("%s", helpMessage);
            else
            {
                Flags32 error = 0;
                {
                    error = GetGlobError(pattern);
                    
                    if (error & GLOB_SYNTAX_ERROR)
                        Errf("ERROR: Invalid syntax.\n");
                    if ((error & GLOB_SYNTAX_EXTENSION))
                    {
                        if (flags & Glob_CLI_Ext)
                            error = 0;
                        else
                            Errf("ERROR: Extension syntax.\n");
                    }
                }
                
                if (!error)
                {
                    b32 noMatches = 1;
                    StrListIter(&paths, path)
                    {
                        TempBlock(temp, scratch)
                        {
                            String text = path->string;
                            if (flags & Glob_CLI_File)
                                text = OSReadFile(scratch, path->string, 0);
                            
                            b32 matched = glob(pattern, text, 0);
                            if (matched)
                            {
                                noMatches = 0;
                                Outf("%.*s\n", StrExpand(path->string));
                            }
                        }
                    }
                    
                    if (noMatches)
                        Outf("No matches found\n");
                    
                    if (flags & Glob_CLI_Debug)
                        Outf("Memory Usage: %llu Bytes\n", scratch->highWaterMark);
                }
            }
        }
    }
    
    return 0;
}
#endif
