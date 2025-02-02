#include "Base.h"
#include "Base.c"

#include "LongMD.h"
#include "LongScanner.h"
#include "LongScanner.c"
#include "CLex.h"
#include "CLex.c"

function MD_TokenizeResult MD_TokenizeFromScanner(Arena* arena, String text)
{
    Scanner* scanner = &ScannerFromStr(arena, text);
    
    MarkerPushLine (scanner, MD_TokenFlag_Comment, StrLit("//"), '\\', 0);
    MarkerPushRange(scanner, MD_TokenFlag_Comment, StrLit("/*"), MarkerFlag_MatchAll|MarkerFlag_InverseStr);
    
    Flags64 flags = MD_TokenFlag_StringLiteral|MD_TokenFlag_StringTriplet;
    MarkerPushRange(scanner, flags|MD_TokenFlag_StringTick       , StrLit(   "```"), 0);
    MarkerPushRange(scanner, flags|MD_TokenFlag_StringSingleQuote, StrLit(   "'''"), 0);
    MarkerPushRange(scanner, flags|MD_TokenFlag_StringDoubleQuote, StrLit("\"\"\""), 0);
    
    flags ^= MD_TokenFlag_StringTriplet;
    MarkerPushLine(scanner, flags|MD_TokenFlag_StringTick       , StrLit( "`"), '\\', 1);
    MarkerPushLine(scanner, flags|MD_TokenFlag_StringSingleQuote, StrLit( "'"), '\\', 1);
    MarkerPushLine(scanner, flags|MD_TokenFlag_StringDoubleQuote, StrLit("\""), '\\', 1);
    
    MarkerPushIdent (scanner, MD_TokenFlag_Identifier);
    MarkerPushNumber(scanner, MD_TokenFlag_Numeric, StrLit("eEpP"));
    
    MarkerPushArr(scanner, MD_TokenFlag_Whitespace, StrLit(SpaceStr"\r"), 0);
    MarkerPushArr(scanner, MD_TokenFlag_Newline, StrLit("\n"), 1);
    MarkerPushArr(scanner, MD_TokenFlag_Reserved, StrLit("{0}()[]#,\\:;@"), 1);
    MarkerPushArr(scanner, MD_TokenFlag_Symbol, StrLit("~!$%^&*-=+<.>/?|"), 0);
    
    scanner->fallback = MD_TokenFlag_BadCharacter;
    
    ScratchBegin(scratch, arena);
    MD_TokenChunkList tokens = {0};
    
    while (scanner->pos < scanner->source.size)
    {
        Token token = ScannerNext(scanner);
        if (token.flags & ScanResultFlag_TokenUnclosed)
        {
            if (token.user & MD_TokenFlag_StringLiteral)
                token.user |= MD_TokenFlag_BrokenStringLiteral;
            if (token.user & MD_TokenFlag_Comment)
                token.user |= MD_TokenFlag_BrokenComment;
        }
        
        md_token_chunk_list_push(scratch, &tokens, 4096, (MD_Token){ token.range, (MD_TokenFlags)token.user });
    }
    
    MD_TokenizeResult result = { .tokens = md_token_array_from_chunk_list(arena, &tokens) };
    ScratchEnd(scratch);
    return result;
}

function MD_TokenizeResult MD_TokenizeFromLexer(Arena* arena, String8 text)
{
    Scanner* scanner = &ScannerFromStr(arena, text);
    MD_TokenChunkList tokens = {0};
    
    while (scanner->pos < scanner->source.size)
    {
        MD_TokenFlags flags = 0;
        u64 start = scanner->pos;
        
        //- rjf: whitespace
        if (ScannerParse(scanner, StrLit(SpaceStr"\r"), ScannerMatchFlag_IsArray))
        {
            flags = MD_TokenFlag_Whitespace;
            ScannerAdvanceUntil(scanner, StrLit(SpaceStr"\r"), ScannerMatchFlag_UntilNot);
        }
        
        //- rjf: newline
        else if (ScannerParse(scanner, StrLit("\n"), 0))
        {
            flags = MD_TokenFlag_Newline;
        }
        
        //- rjf: single-line comments
        else if (ScannerParse(scanner, StrLit("//"), 0))
        {
            flags = MD_TokenFlag_Comment;
            while (ScannerAdvanceUntil(scanner, StrLit("\\"), ScannerMatchFlag_LineExit))
                ScannerParse(scanner, StrLit("\n"), 0);
        }
        
        //- rjf: multi-line comments
        else if (ScannerParse(scanner, StrLit("/*"), 0))
        {
            flags = MD_TokenFlag_Comment;
            i64 nest = 1;
            
            while (scanner->pos < scanner->source.size)
            {
                if (nest == 0)
                    break;
                else if (ScannerParse(scanner, StrLit("/*"), 0))
                    nest++;
                else if (ScannerParse(scanner, StrLit("*/"), 0))
                    nest--;
                else
                    ScannerAdvance(scanner, 1);
            }
            
            if (nest != 0)
                flags |= MD_TokenFlag_BrokenComment;
        }
        
        //- rjf: identifiers
        else if (ScannerParse(scanner, StrLit("_"), ScannerMatchFlag_Characters|ScannerMatchFlag_IsArray))
        {
            flags = MD_TokenFlag_Identifier;
            ScannerAdvanceUntil(scanner, StrLit("_"), ScannerMatchFlag_UntilNot|ScannerMatchFlag_AlphaNumerics);
        }
        
        //- rjf: numerics
        else if (ScannerParse(scanner, StrLit("."), ScannerMatchFlag_Numbers) ||
                 ScannerParse(scanner, ZeroStr, ScannerMatchFlag_Numbers|ScannerMatchFlag_IsArray))
        {
            flags = MD_TokenFlag_Numeric;
            REPEAT:
            ScannerAdvanceUntil(scanner, StrLit("._"), ScannerMatchFlag_UntilNot|ScannerMatchFlag_AlphaNumerics);
            u8 prev = ScannerPeekByte(scanner, -1);
            if (ChrCompareArr(prev, StrLit("ep"), MatchStr_IgnoreCase))
                if (ScannerParse(scanner, StrLit("+-"), ScannerMatchFlag_IsArray))
                    goto REPEAT;
        }
        
        //- rjf: triplet string literals
        else if (ScannerParse(scanner, StrLit("```"), 0) ||
                 ScannerParse(scanner, StrLit("'''"), 0) ||
                 ScannerParse(scanner, StrLit("\"\"\""), 0))
        {
            String style = ScannerStrFromRange(scanner, R1U64(start, scanner->pos));
            flags = MD_TokenFlag_StringLiteral;
            
            if (!ScannerAdvanceUntil(scanner, style, 0))
                flags |= MD_TokenFlag_BrokenStringLiteral;
        }
        
        //- rjf: singlet string literals
        else if (ScannerParse(scanner, StrLit("\"'`"), ScannerMatchFlag_IsArray))
        {
            flags = MD_TokenFlag_StringLiteral;
            u8 styles[2] = { ScannerPeekByte(scanner, -1), '\\' };
            if (!ScannerAdvanceUntil(scanner, Str(styles, ArrayCount(styles)), ScannerMatchFlag_LineAndEscape))
                flags |= MD_TokenFlag_BrokenStringLiteral;
        }
        
        //- rjf: symbols
        else if (ScannerParse(scanner, StrLit("~!$%^&*-=+<.>/?|"), ScannerMatchFlag_IsArray))
        {
            flags = MD_TokenFlag_Symbol;
            ScannerAdvanceUntil(scanner, StrLit("~!$%^&*-=+<.>/?|"), ScannerMatchFlag_UntilNot);
        }
        
        //- rjf: resvered symbols
        else if (ScannerParse(scanner, StrLit("{0}()[]#,\\:;@"), ScannerMatchFlag_IsArray))
        {
            flags = MD_TokenFlag_Reserved;
        }
        
        //- rjf: bad characters in all other cases
        else
        {
            flags = MD_TokenFlag_BadCharacter;
            ScannerAdvance(scanner, 1);
        }
        
        //- rjf: push token if formed
        u64 opl = scanner->pos;
        if (flags && opl > start)
        {
            MD_Token token = {{start, opl}, flags};
            md_token_chunk_list_push(arena, &tokens, 4096, token);
        }
        
    }
    
    //- rjf: bake, fill & return
    MD_TokenizeResult result = {0};
    {
        result.tokens = md_token_array_from_chunk_list(arena, &tokens);
    }
    return result;
}

internal MD_TokenizeResult MD_TokenizeFromText(Arena *arena, String8 text)
{
    ScratchBegin(scratch, arena);
    MD_TokenChunkList tokens = {0};
    U8 *byte_first = text.str;
    U8 *byte_opl = byte_first + text.size;
    U8 *byte = byte_first;
    
    //- rjf: scan string & produce tokens
    for (; byte < byte_opl;)
    {
        MD_TokenFlags flags = 0;
        u64 start = byte - byte_first;
        
#define TokenInit(_flags, size) Stmnt(flags = (_flags); byte += (size))
#define Check2Bytes(str) (byte+1 < byte_opl && byte[0] == (str)[0] && byte[1] == (str)[1])
#define CheckTriplet(c) (byte[0] == (c) && byte[1] == (c) && byte[2] == (c))
        
        //- rjf: whitespace
        if (*byte == ' ' || *byte == '\t' || *byte == '\v' || *byte == '\r')
        {
            TokenInit(MD_TokenFlag_Whitespace, 1);
            for (; byte < byte_opl; ++byte)
                if (*byte != ' ' && *byte != '\t' && *byte != '\v' && *byte != '\r')
                    break;
        }
        
        //- rjf: newlines
        else if (*byte == '\n')
            TokenInit(MD_TokenFlag_Newline, 1);
        
        //- rjf: single-line comments
        else if (Check2Bytes("//"))
        {
            TokenInit(MD_TokenFlag_Comment, 2);
            B32 escaped = 0;
            for (; byte < byte_opl; ++byte)
            {
                if (escaped)
                    escaped = 0;
                else if (*byte == '\n')
                    break;
                else if (*byte == '\\')
                    escaped = 1;
            }
        }
        
        //- rjf: multi-line comments
        else if (Check2Bytes("/*"))
        {
            TokenInit(MD_TokenFlag_Comment, 2);
            for (i64 nest = 1; byte < byte_opl;)
            {
                if (nest == 0) break;
                else if (Check2Bytes("*/"))
                {
                    byte += 2;
                    nest--;
                }
                else if (Check2Bytes("/*"))
                {
                    byte += 2;
                    nest++;
                }
                else byte += 1;
            }
            
            if (byte == byte_opl)
                flags |= MD_TokenFlag_BrokenComment;
        }
        
        //- rjf: identifiers
        else if (IsCharacter(*byte) || *byte == '_' || utf8_class[*byte>>3] >= 2)
        {
            TokenInit(MD_TokenFlag_Identifier, 1);
            for (; byte < byte_opl; ++byte)
                if (!IsAlphaNumeric(*byte) && *byte != '_' && utf8_class[*byte>>3] < 2)
                    break;
        }
        
        //- rjf: numerics
        else if (IsDigit(*byte) || (*byte == '.' && byte+1 < byte_opl && IsDigit(byte[1])))
        {
            TokenInit(MD_TokenFlag_Numeric, 1);
            for (; byte < byte_opl; ++byte)
            {
                if ((*byte == '+' || *byte == '-') && (byte[-1] == 'e' || byte[-1] == 'E'))
                    continue;
                if (!IsAlphaNumeric(*byte) && *byte != '_' && *byte != '.')
                    break;
            }
        }
        
        //- rjf: triplet string literals
        else if (byte+2 < byte_opl && (CheckTriplet('"') || CheckTriplet('\'') || CheckTriplet('`')))
        {
            U8 literal_style = byte[0];
            TokenInit(MD_TokenFlag_StringLiteral, 3);
            
            for (; byte+2 < byte_opl; ++byte)
            {
                if (CheckTriplet(literal_style))
                {
                    byte += 3;
                    break;
                }
            }
            
            if (byte+2 >= byte_opl)
                flags |= MD_TokenFlag_BrokenStringLiteral;
        }
        
        //- rjf: singlet string literals
        else if (byte[0] == '"' || byte[0] == '\'' || byte[0] == '`')
        {
            U8 literal_style = byte[0];
            TokenInit(MD_TokenFlag_StringLiteral, 1);
            
            B32 escaped = 0;
            for (; byte <= byte_opl; ++byte)
            {
                if (byte == byte_opl || *byte == '\n')
                {
                    flags |= MD_TokenFlag_BrokenStringLiteral;
                    break;
                }
                
                if (escaped) escaped = 0;
                else if (byte[0] == '\\') escaped = 1;
                else if (byte[0] == literal_style)
                {
                    ++byte;
                    break;
                }
            }
        }
        
        //- rjf: non-reserved symbols
#define MD_IsNonReservedSymbols(byte) (*byte == '~' || *byte == '!' || *byte == '$' || *byte == '%' || *byte == '^' || \
                                       *byte == '&' || *byte == '*' || *byte == '-' || *byte == '=' || *byte == '+' || \
                                       *byte == '<' || *byte == '.' || *byte == '>' || *byte == '/' || *byte == '?' || \
                                       *byte == '|')
        else if (MD_IsNonReservedSymbols(byte))
        {
            TokenInit(MD_TokenFlag_Symbol, 1);
            for (; byte < byte_opl; ++byte)
                if (!MD_IsNonReservedSymbols(byte))
                    break;
        }
        
        //- rjf: reserved symbols
        else if (*byte == '{' || *byte == '}' || *byte == '(' || *byte == ')' ||
                 *byte == '[' || *byte == ']' || *byte == '#' || *byte == ',' ||
                 *byte == '\\'|| *byte == ':' || *byte == ';' || *byte == '@')
            TokenInit(MD_TokenFlag_Reserved, 1);
        
        //- rjf: bad characters in all other cases
        else
            TokenInit(MD_TokenFlag_BadCharacter, 1);
        
#undef TokenInit
#undef Check2Bytes
#undef CheckTriplet
        
        //- rjf: push token if formed
        u64 opl = byte - byte_first;
        if (flags && opl > start)
        {
            MD_Token token = {{start, opl}, flags};
            md_token_chunk_list_push(scratch, &tokens, 4096, token);
        }
        
        //- TODO(long): Report Error
        if (flags & MD_TokenGroup_Error)
        {
            
        }
    }
    
    //- rjf: bake, fill & return
    MD_TokenizeResult result = {0};
    {
        result.tokens = md_token_array_from_chunk_list(arena, &tokens);
    }
    ScratchEnd(scratch);
    return result;
}

internal String MD_DumpJSON(Arena* arena, MD_Node* node, i32 indent)
{
    String result = {0};
    if (md_node_is_nil(node) || node->kind != MD_NodeKind_Main) result = StrLit("<invalid>");
    
    // TODO(long): Indentation for key and array
    
    else if (node->flags & MD_NodeFlag_MaskLabelKind) result = node->raw_string;
    
    else if (node->flags & MD_NodeFlag_HasBracketLeft)
    {
        ScratchBlock(scratch, arena)
        {
            StringList list = {0};
            for (MD_EachNode(child, node->first))
            {
                String str = MD_DumpJSON(scratch, child, indent + 2);
                StrListPush(scratch, &list, str);
            }
            result = StrJoin(arena, &list, .pre = StrLit("[\n"), .mid = StrLit(",\n"), .post = StrLit("\n]"));
        }
    }
    
    else if (node->flags & MD_NodeFlag_HasBraceLeft)
    {
        ScratchBlock(scratch, arena)
        {
            StringList list = {0};
            StrListPushf(scratch, &list, "%*s{\n", indent, "");
            for (MD_EachNode(child, node->first))
            {
                String str = MD_DumpJSON(scratch, child->first, indent + 2);
                StrListPushf(scratch, &list, "%*s\"%.*s\": %.*s%s", indent + 2, "",
                             StrExpand(child->string), StrExpand(str),
                             md_node_is_nil(child->next) ? "\n" : ",\n");
            }
            StrListPushf(scratch, &list, "%*s}", indent, "");
            result = StrJoin(arena, &list);
        }
    }
    
    return result;
}

int main(void)
{
    ScratchBegin(scratch);
    {
        String file = StrLit("code/test.json");
        MD_ParseResult parse = MD_ParseText(scratch, file, OSReadFile(scratch, file));
        String dump = MD_DumpJSON(scratch, parse.root->first, 0);
        OSWriteFile(StrLit("code/out_test.mdesk"), dump);
        Assert(parse.root->first == parse.root->last);
    }
    
    {
        String data = OSReadFile(scratch, StrLit("code/examples/test_parser.c"));
        TokenArray array = CL_TokenArrayFromStr(scratch, data);
        CL_ParseResult parse = CL_ParseFromTokens(scratch, data, array);
        
        CL_Node* node = parse.root->body.first;
        u64 indent = 0;
        
        REP:
        Outf("%*s%.*s\n", indent * 2, "", StrExpand(node->string));
        for (CL_Node* base = node->reference; !CL_IsNil(base); base = base->reference)
        {
            Outf("%.*s", StrExpand(base->string));
            
            for (CL_Node* tag = base->tags.first; !CL_IsNil(tag); tag = tag->next)
            {
                if (CL_IsNil(tag->prev))
                    Outf(" (");
                Outf("%.*s", StrExpand(tag->string));
                Outf(CL_IsNil(tag->next) ? ")" : ", ");
            }
            
            Outf(CL_IsNil(base->reference) ? "\n" : " -> ");
        }
        
        CL_Node* old = node;
        if (!CL_IsNil(node->body.first))
        {
            node = node->body.first;
            indent++;
        }
        else if (!CL_IsNil(node->next))
        {
            node = node->next;
        }
        else for (CL_Node* parent = node->parent; !CL_IsNil(parent); parent = parent->parent)
        {
            indent--;
            if (!CL_IsNil(parent->next))
            {
                node = parent->next;
                break;
            }
        }
        
        if (node != old)
            goto REP;
    }
    
    {
        // int**** a[1][2][3];
        // float** (**(*b[1])[2][3])[4];
        // char (*(*c[3])())[5];
        // char * const (*(* const d)[5])(int);
        // int (*(*e)(const void *))[3];
        // int* f1,** f2, f3[10], ** f4[20], ** (**(*f5[1])[2][3])[4];
        // void func1( int ), * (func2)( int ), (*funcPtr)(void);
        // int (*(*foo1)(void))[3], (*foo2(void))[3];
        // int bar1(int[]), bar3(char arg(double)), bar4(int(void)), bar5(const int[10]);
        
        // MSVC and cdecl.org complained about these, while Clang doesn't
        // int pointerInArray(int[*]);
        // int specifier1InArray(int [static 10]);
        // int specifier2InArray(int[const volatile]);
        // _Atomic unsigned long long int const volatile *restrict const foo[10];
        
        // NOTE(long): Ambiguous
        // foo * bar;
        // foo(bar); foo(*bar); foo(*bar[5]);
        // foo (*(*bar)[5])(baz);
        
        String text = StrLit("struct A { int a; int b; };\n"
                             "struct B"
                             "{\n"
                             "int array[10];\n"
                             "char ** const * const ptr;"
                             "int a1, a2;\n"
                             "int* b1, b2, **** *** ** * b3;\n"
                             "};\n"
                             "typedef struct C C;\n"
                             "struct C { char c[1024]; };\n"
                             "typedef struct D { int a; } D;\n"
                             "struct E { int a; };\n"
                             "typedef struct { int a; } F;\n"
                             "struct { int a; } G;\n"
                             "struct H { int a; } myH;\n"
                             "typedef struct X { int x; } Y;\n"
                             "struct Z { struct { i32 a, ** * b; }; struct { float f; float f2[10]; }; struct H h; }\n"
                             "struct W { struct { i32 a, ** * b; }; struct { float f; struct { char c[1024]; }; float f2[10]; }; b32 b; }\n"
                             
                             "union A { int a; int b; };\n"
                             "union B"
                             "{\n"
                             "int array[10];\n"
                             "char ** const * const ptr;"
                             "int a1, a2;\n"
                             "int* b1, b2, **** *** ** * b3;\n"
                             "};\n"
                             "typedef union C C;\n"
                             "union C { char c[1024]; };\n"
                             "typedef union D { int a; } D;\n"
                             "union E { int a; };\n"
                             "typedef union { int a; } F;\n"
                             "union { int a; } G;\n"
                             "union H { int a; } myH;\n"
                             "typedef union X { int x; } Y;\n"
                             "union Z { union { i32 a, ** * b; }; union { float f; float f2[10]; }; union H h; }\n"
                             "union W { union { i32 a, ** * b; }; union { float f; union { char c[1024]; }; float f2[10]; }; b32 b; }\n"
                             
                             "enum A { FooA, BarA };\n"
                             "typedef enum B { FooB } B;\n"
                             "typedef enum C C; enum C { FooC };\n"
                             "typedef enum { FooD, BarD = 10, } D;\n"
                             "enum { FooE } E\n"
                             "enum F { F1 = foo * bar / blah, F2 = {}, F3 = Func(&(Type){ .a = 100*100, .b = 0, }, baz), F4 = 100 }"
                             );
        text = OSReadFile(scratch, StrLit("code/Base.h"));
        
#if 0
        MD_Node* root = MD_ParseStrC(scratch, text);
        MD_DebugTree(root);
        
        for (MD_EachNode(node, root->first))
        {
            if (md_node_has_tag(node, StrLit("struct"), 0) || md_node_has_tag(node, StrLit("union"), 0))
            {
                StringList members = {0};
                if (!md_node_is_nil(node->first))
                {
                    Assert(node->first->flags & MD_NodeFlag_HasBraceLeft);
                    for (MD_EachNode(child, node->first->first))
                    {
                        
                    }
                }
                
                String body = StrJoin(scratch, &members);
                Outf("StructLit(%.*s, %llu)\n{\n%.*s};\n\n", StrExpand(node->string), members.nodeCount, StrExpand(body));
            }
        }
#endif
        
#if 0
        CL_Node* root = CL_MDParseText(scratch, StrLit("test"), text);
        for (CL_Node* node = root->first; node != &cl_nilNode; node = node->next)
        {
            String name = node->name->string;
            String base = node->base->string;
            
            char* typeLit = 0;
            switch (node->type)
            {
                case CL_NodeType_Typedef:
                {
                    u64 ptrCount = CL_NodePtrCount(node);
                    String ptr = ptrCount ? StrPushf(scratch, ", %llu", ptrCount) : (String){0};
                    Outf("TypedefLit(%.*s, %.*s%.*s);\n\n", StrExpand(base), StrExpand(name), StrExpand(ptr));
                } break;
                
                case CL_NodeType_Struct: typeLit = "Struct"; break;
                case CL_NodeType_Union:  typeLit = "Union"; break;
                case CL_NodeType_Enum:   typeLit = "Enum"; break;
                case CL_NodeType_Proc:   typeLit = "Proc"; break;
                
                default: break;
            }
            
            if (typeLit)
            {
                StringList members = {0};
                b32 isProc = node->type == CL_NodeType_Proc;
                
                for (CL_Node* member = node->first; member != &cl_nilNode; member = member->next)
                {
                    String memberName = member->name->string;
                    String memberType = member->base->string;
                    u64 ptrCount = CL_NodePtrCount(member);
                    
                    if (node->type == CL_NodeType_Enum)
                        StrListPushf(scratch, &members, "    EnumValue(%.*s, %.*s),\n",
                                     StrExpand(name), StrExpand(memberName));
                    else if (isProc)
                        StrListPushf(scratch, &members, "    ArgLit(%.*s, %.*s, %.*s, %llu),\n",
                                     StrExpand(name), StrExpand(memberType), StrExpand(memberName), ptrCount);
                    else if (member->name->next->flags & MD_NodeFlag_HasBracketLeft)
                        StrListPushf(scratch, &members, "    ArrayMember(%.*s, %.*s, %.*s),\n",
                                     StrExpand(name), StrExpand(memberType), StrExpand(memberName));
                    else
                        StrListPushf(scratch, &members, "    MemberLit(%.*s, %.*s, %.*s, %llu),\n",
                                     StrExpand(name), StrExpand(memberType), StrExpand(memberName), ptrCount);
                }
                
                String body = StrJoin(scratch, &members);
                if (isProc)
                {
                    if (members.nodeCount)
                        Outf("ProcLit(%.*s, %.*s, %llu)\n{\n%.*s};\n\n",
                             StrExpand(name), StrExpand(base), members.nodeCount, StrExpand(body));
                    else
                    {
                        u64 ptrCount = CL_NodePtrCount(node);
                        String ptr = ptrCount ? StrPushf(scratch, ", %llu", ptrCount) : (String){0};
                        Outf("EmptyProc(%.*s, %.*s%.*s);\n\n", StrExpand(name), StrExpand(base), StrExpand(ptr));
                    }
                }
                else
                    Outf("%sLit(%.*s, %llu)\n{\n%.*s};\n\n", typeLit, StrExpand(name), members.nodeCount, StrExpand(body));
            }
        }
#endif
    }
    
    if (0)
    {
        String inputs[] = {
            OSReadFile(scratch, StrLit("code/Base.h")),
            StrLit("if (TweakB32(draw_ur_mom)) {\n"
                   "f32 height = TweakF32(mom_height, 1f, 2f);\n"
                   "EmbedFile(ur_mom_sprite, \"D:\\\\Documents\\\\Important\\\\my_mom.png\");\n"
                   "DrawUrMom(height, ur_mom_sprite);\n"
                   "}\n"),
            StrLit("u64 a = 0, b = 0, c = a+++b; a+=+b;\n"
                   "u64* ptr =&a; u64** dptr =&ptr;\n"
                   "*ptr; **dptr; a&=*&a\n"),
            StrLit("><<==++=>>->=.../**/?::%=%%"),
        };
        
        StringList operators = StrList(scratch, ArrayExpand(String,
                                                            StrLit("<"), StrLit("<<"), StrLit("<="), StrLit("<<="),
                                                            StrLit(">"), StrLit(">>"), StrLit(">="), StrLit(">>="),
                                                            StrLit("&"), StrLit("&&"), StrLit("&="),
                                                            StrLit("|"), StrLit("||"), StrLit("|="),
                                                            StrLit("+"), StrLit("++"), StrLit("+="),
                                                            StrLit("-"), StrLit("--"), StrLit("-="),
                                                            StrLit("*"), StrLit("*="), StrLit("/"), StrLit("/="),
                                                            StrLit("!"), StrLit("!="), StrLit("^"), StrLit("^="),
                                                            StrLit("%"), StrLit("%="), StrLit("="), StrLit("=="),
                                                            StrLit("["), StrLit("]"),  StrLit("("), StrLit(")"),
                                                            StrLit("{"), StrLit("}"),  StrLit(","), StrLit(";"),
                                                            StrLit(":"), StrLit("?"),  StrLit("~"),
                                                            StrLit("."), StrLit("->"), StrLit("...")));
        
        ForEach(i, ArrayCount(inputs))
        {
            String in = inputs[i];
            TokenArray array = CL_TokenArrayFromStr(scratch, in);
            MetaTable table = CL_TableFromTokens(scratch, in, array);
            
            ForEach(slotIdx, table.count)
            {
                MetaInfo info = table.v[slotIdx];
                switch (info.flags)
                {
                    case MetaFlag_TweakB32:  Outf("TweakB32: %.*s\n", StrExpand(info.name)); break;
                    case MetaFlag_TweakF32:  Outf("TweakF32: %.*s\n", StrExpand(info.name)); break;
                    case MetaFlag_EmbedFile:
                    {
                        String path = {0};
                        if (info.childs.v)
                            path = info.childs.v[0].name;
                        Outf("Embed: %.*s, Path: %.*s\n", StrExpand(info.name), StrExpand(path));
                    } break;
                }
            }
            
            for (Token* token = array.tokens; token < array.tokens + array.count; ++token)
            {
                if (token->user & CL_TokenFlag_Symbol)
                {
                    String lexeme = Substr(in, token->range.min, token->range.max);
                    Assert(StrCompareList(lexeme, &operators, 0));
                }
            }
        }
    }
    
    if (0)
    {
        String text = OSReadFile(scratch, StrLit("code/test.json"));
        TokenArray array = JSON_TokenizeFromText(scratch, text);
        JSON_Value value = JSON_ValueFromTokens (scratch, text, array);
        OSWriteFile(StrLit("code/out_test.json"), JSON_StrFromValue(scratch, value, 0));
    }
    
    if (0)
    {
        String text = OSReadFile(scratch, StrLit("code/test.csv"));
        StringTable table = CSV_TableFromStr(scratch, text);
        OSWriteFile(StrLit("code/out_test.csv"), CSV_StrFromTable(scratch, table));
    }
    
    Outf("Done");
    
    ScratchEnd(scratch);
    return 0;
}
