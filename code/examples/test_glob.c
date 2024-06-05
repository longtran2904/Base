#include "Base.h"
#include "LongOS.h"
#include "Base.c"
#include "LongOS_Win32.c"

#define LONG_TEST_IMPLEMENTATION
#define LT_NAME_PADDING 20
#define LT_TEST_PADDING 45
#include "LongTest.h"

#define GLOB_STATIC_LIB 1
#include "glob.c"

function void GlobTestEx(String pattern, String text, b32 expected, Flags32 flags)
{
    Flags32 syntax = 0;
    b32 result = glob(pattern, text, &syntax);
    ALWAYS(TestResult(result == expected && flags == syntax));
}

function void PrintGlobStr(String pattern, String text)
{
    b32 result = glob(pattern, text, 0);
    Outf("%12.*s <=> %-8.*s => %s\n", StrExpand(pattern), StrExpand(text), result ? "true" : "false");
}

#define GlobTest(pattern, text, expected, ...) GlobTestEx(StrLit(pattern), StrLit(text), expected, (__VA_ARGS__+0))
#define PrintGlob(pattern, text) PrintGlobStr(StrLit(pattern), StrLit(text))

int main(void)
{
    TEST("Glob Normal")
    {
        GlobTest("main.?", "main.c", 1);
        GlobTest("index.?", "main.c", 0);
        
        GlobTest("?at", "Cat", 1);
        GlobTest("?at", "cat", 1);
        GlobTest("?at", "Bat", 1);
        GlobTest("?at", "bat", 1);
        GlobTest("?at", "at", 0);
        
        GlobTest("*", "main.c", 1);
        GlobTest("*.c", "main.c", 1);
        GlobTest("*.c", "index.c", 1);
        GlobTest("*.c", "test.c", 1);
        GlobTest("*.js", "main.c", 0);
        
        GlobTest("Law*", "Law", 1);
        GlobTest("Law*", "Laws", 1);
        GlobTest("Law*", "Lawyer", 1);
        GlobTest("Law*", "GrokLaw", 0);
        GlobTest("Law*", "La", 0);
        GlobTest("Law*", "aw", 0);
        
        GlobTest("*Law*", "Law", 1);
        GlobTest("*Law*", "GrokLaw", 1);
        GlobTest("*Law*", "Lawyer", 1);
        GlobTest("*Law*", "La", 0);
        GlobTest("*Law*", "aw", 0);
        
        GlobTest("*.[abc]", "main.a", 1);
        GlobTest("*.[abc]", "main.b", 1);
        GlobTest("*.[abc]", "main.c", 1);
        GlobTest("*.[abc]", "main.d", 0);
        
        GlobTest("[CB]at", "Cat", 1);
        GlobTest("[CB]at", "Bat", 1);
        GlobTest("[CB]at", "cat", 0);
        GlobTest("[CB]at", "bat", 0);
        GlobTest("[CB]at", "CBat", 0);
        
        GlobTest("Letter[0-9]", "Letter0", 1);
        GlobTest("Letter[0-9]", "Letter1", 1);
        GlobTest("Letter[0-9]", "Letter2", 1);
        GlobTest("Letter[0-9]", "Letter9", 1);
        
        GlobTest("Letter[0-9]", "Letters", 0);
        GlobTest("Letter[0-9]", "Letter", 0);
        GlobTest("Letter[0-9]", "Letter10", 0);
        
        GlobTest("[A-Fa-f0-9]", "A", 1);
        GlobTest("[A-Fa-f0-9]", "a", 1);
        GlobTest("[A-Fa-f0-9]", "B", 1);
        GlobTest("[A-Fa-f0-9]", "b", 1);
        GlobTest("[A-Fa-f0-9]", "0", 1);
        GlobTest("[A-Fa-f0-9]", "-", 0);
    }
    
    TEST("Glob Special")
    {
        GlobTest("[][!]", "]", 1);
        GlobTest("[][!]", "[", 1);
        GlobTest("[][!]", "!", 1);
        GlobTest("[][!]", "a", 0);
        
        GlobTest("[]-]", "]", 1);
        GlobTest("[]-]", "-", 1);
        GlobTest("[]-]", "a", 0);
        
        GlobTest("[--0]", "-", 1);
        GlobTest("[--0]", ".", 1);
        GlobTest("[--0]", "0", 1);
        GlobTest("[--0]", "/", 1);
        GlobTest("[--0]", "a", 0);
        
        GlobTest("[!]a-]", "b", 1);
        GlobTest("[!]a-]", "]", 0);
        GlobTest("[!]a-]", "a", 0);
        GlobTest("[!]a-]", "-", 0);
        
        GlobTest("[[?*\\]", "[", 1);
        GlobTest("[[?*\\]", "?", 1);
        GlobTest("[[?*\\]", "*", 1);
        GlobTest("[[?*\\]", "\\", 1);
        GlobTest("[[?*\\]", "a", 0);
        
        GlobTest("\\[", "[", 1);
        GlobTest("\\]", "]", 1);
        GlobTest("\\]", "\\]", 0);
        GlobTest("][\\]", "]\\", 1);
        
        GlobTest("?", "", 0);
        GlobTest("?", "a", 1);
        GlobTest("\\*", "*", 1);
        GlobTest("\\?", "?", 1);
        GlobTest("\\\\", "\\", 1);
    }
    
    TEST("Glob Extension")
    {
        GlobTest("[a-d-g]", "a", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-g]", "b", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-g]", "c", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-g]", "d", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-g]", "e", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-g]", "f", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-g]", "g", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-g]", "h", 0, GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-g]", "z", 0, GLOB_SYNTAX_EXTENSION);
        
        GlobTest("[*--b]", "*", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--b]", "+", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--b]", ",", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--b]", "-", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--b]", "b", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--b]", "a", 0, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--b]", "c", 0, GLOB_SYNTAX_EXTENSION);
        
        GlobTest("[*--a-c]", "*", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--a-c]", "+", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--a-c]", ",", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--a-c]", "-", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--a-c]", "0", 0, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--a-c]", "a", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--a-c]", "b", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--a-c]", "c", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*--a-c]", "d", 0, GLOB_SYNTAX_EXTENSION);
        
        GlobTest("[*---c]", "+", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", ",", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", "-", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", ".", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", "/", 1, GLOB_SYNTAX_EXTENSION);
        
        GlobTest("[*---c]", "0", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", "8", 1, GLOB_SYNTAX_EXTENSION);
        
        GlobTest("[*---c]", ":", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", ";", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", "<", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", "=", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", ">", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", "?", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", "@", 1, GLOB_SYNTAX_EXTENSION);
        
        GlobTest("[*---c]", "A", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", "F", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", "Z", 1, GLOB_SYNTAX_EXTENSION);
        
        GlobTest("[*---c]", "a", 1, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", "d", 0, GLOB_SYNTAX_EXTENSION);
        GlobTest("[*---c]", "f", 0, GLOB_SYNTAX_EXTENSION);
    }
    
    TEST("Glob Error")
    {
        GlobTest("*.[abc", "main.a", 1, GLOB_SYNTAX_ERROR);
        GlobTest("*.[abc", "main.z", 0, GLOB_SYNTAX_ERROR);
        
        GlobTest("[][!", "[", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[][!", "]", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[][!", "!", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[][!", "abc", 0, GLOB_SYNTAX_ERROR);
        
        GlobTest("[", "aaaaa", 0, GLOB_SYNTAX_ERROR);
        GlobTest("[!", "aaaaa", 0, GLOB_SYNTAX_ERROR);
        GlobTest("[!", "", 0); // empty string
        GlobTest("[--", "a", 0, GLOB_SYNTAX_ERROR);
        GlobTest("[--", "-", 1, GLOB_SYNTAX_ERROR);
        
        GlobTest("[ab-", "a", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[ab-", "b", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[ab-", "-", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[ab-", "c", 0, GLOB_SYNTAX_ERROR);
        
        GlobTest("\\", "aa", 0, GLOB_SYNTAX_ERROR);
        GlobTest("\\", "\\", 1, GLOB_SYNTAX_ERROR);
        
        GlobTest("[)---9", "!", 0, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[)---9", "#", 0, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[)---9", "*", 1, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[)---9", "/", 1, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[)---9", "0", 1, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[)---9", "9", 1, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[)---9", "A", 0, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[)---9", "z", 0, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        
        GlobTest("[--a", ".", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[--a", "/", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[--a", "0", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[--a", "9", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[--a", ":", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[--a", "@", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[--a", "A", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[--a", "a", 1, GLOB_SYNTAX_ERROR);
        GlobTest("[--a", "z", 0, GLOB_SYNTAX_ERROR);
        GlobTest("[--a", "}", 0, GLOB_SYNTAX_ERROR);
        
        GlobTest("[a-d-f", "a", 1, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-f", "c", 1, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-f", "d", 1, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-f", "e", 1, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-f", "g", 0, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-f", "0", 0, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-f", "?", 0, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-f", "A", 0, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-f", "Z", 0, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
        GlobTest("[a-d-f", "{", 0, GLOB_SYNTAX_ERROR|GLOB_SYNTAX_EXTENSION);
    }
    
    Outf("\n");
    PrintGlob("Law*", "Law");
    PrintGlob("*Law*", "GrokLaw");
    PrintGlob("Law*", "La");
    PrintGlob("*Law*", "aw");
    
    Outf("\n");
    PrintGlob("main.?", "main.c");
    PrintGlob("?at", "Cat");
    PrintGlob("?at", "Bat");
    PrintGlob("?at", "at");
    
    Outf("\n");
    PrintGlob("*.[abc]", "main.a");
    PrintGlob("*.[abc]", "main.b");
    PrintGlob("[CB]at", "Cat");
    PrintGlob("[CB]at", "Bat");
    
    Outf("\n");
    PrintGlob("Letter[0-9]", "Letter");
    PrintGlob("Letter[0-9]", "Letter1");
    PrintGlob("[A-Fa-f0-9]", "A");
    PrintGlob("[A-Fa-f0-9]", "f");
    PrintGlob("[A-Fa-f0-9]", "5");
    
    return 0;
}
