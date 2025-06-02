#include "Base.h"
#include "Base.c"

#include "LongMD.h"
#include "LongScanner.h"
#include "LongScanner.c"
#include "CLex.h"
#include "CLex.c"

internal MD_TokenizeResult MD_TokenizeFromText(Arena *arena, String text)
{
    ScratchBegin(scratch, arena);
    MD_TokenChunkList tokens = {0};
    u8 *byte_first = text.str;
    u8 *byte_opl = byte_first + text.size;
    u8 *byte = byte_first;
    
    //- rjf: scan string & produce tokens
    for (; byte < byte_opl;)
    {
        MD_TokenFlags flags = 0;
        u64 start = byte - byte_first;
        
#define TokenInit(_flags, size) Stmnt(flags = (_flags); byte += (size))
#define Check2Bytes(str) (byte+1 < byte_opl && byte[0] == (str)[0] && byte[1] == (str)[1])
#define CheckTriplet(c) (byte[0] == (c) && byte[1] == (c) && byte[2] == (c))
        
        //- rjf: whitespace
        if (*byte == ' ' || *byte == '\t' || *byte == '\v')
        {
            TokenInit(MD_TokenFlag_Whitespace, 1);
            for (; byte < byte_opl; ++byte)
                if (*byte != ' ' && *byte != '\t' && *byte != '\v' && *byte != '\r')
                    break;
        }
        
        //- rjf: newlines
        else if (*byte == '\n')
            TokenInit(MD_TokenFlag_Newline, 1);
        else if (Check2Bytes("\r\n"))
            TokenInit(MD_TokenFlag_Newline, 2);
        
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
        else if (IsDigit(*byte) || (*byte == '.' && byte+1 < byte_opl && IsDigit(byte[1])) ||
                 (*byte == '-' && byte+1 < byte_opl && IsDigit(byte[1])))
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
            u8 literal_style = byte[0];
            TokenInit(MD_TokenFlag_StringLiteral|MD_TokenFlag_StringTriplet, 3);
            switch (literal_style)
            {
                case '\'': flags |= MD_TokenFlag_StringSingleQuote; break;
                case '"':  flags |= MD_TokenFlag_StringDoubleQuote; break;
                case '`':  flags |= MD_TokenFlag_StringTick; break;
            }
            
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
            u8 literal_style = byte[0];
            TokenInit(MD_TokenFlag_StringLiteral, 1);
            switch (literal_style)
            {
                case '\'': flags |= MD_TokenFlag_StringSingleQuote; break;
                case '"':  flags |= MD_TokenFlag_StringDoubleQuote; break;
                case '`':  flags |= MD_TokenFlag_StringTick; break;
            }
            
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

internal TokenFlags MD_ScanRawString(Scanner* scanner)
{
    TokenFlags result = 0;
    u64 pos = scanner->pos;
    
    if (ScannerParse(scanner, StrLit("\"\"\"")) ||
        ScannerParse(scanner, StrLit("'''")) ||
        ScannerParse(scanner, StrLit("```")))
    {
        result |= TokenFlag_String;
        String style = ScannerRange(scanner, pos, scanner->pos);
        if (!ScannerAdvancePast(scanner, style, 0))
            result |= TokenFlag_Unterminated;
    }
    
    return result;
}

internal TokenArray MD_TokenArrayFromStr(Arena* arena, String text)
{
    ScratchBegin(scratch, arena);
    Scanner scanner = {
        .hook = MD_ScanRawString,
        .source = text,
        .flags = (CL_Scan_Whitespace|CL_Scan_Newline|CL_Scan_Comments|CL_Nest_Comments|
                  CL_Scan_SingleQuotes|CL_Scan_DoubleQuotes|CL_Scan_Ticks|
                  CL_Scan_Idents|CL_Scan_Nums|CL_Scan_Symbols),
        
        .commentDelim = StrLit("//"),
        .commentMulti = StrLit("/*"),
        
        .escapeChar = '\\',
        
        .preIdentSymbols = StrLit("_"),
        .midIdentSymbols = StrLit("_"),
        
        .preNumSymbols = StrLit(".-"),
        .midNumSymbols = StrLit("._"),
        .exponents = StrLit("eE"),
        
        .symbols = StrList(scratch, ArrayExpand(String, StrLit("{"), StrLit("}"), StrLit("("), StrLit(")"),
                                                StrLit("["), StrLit("]"), StrLit("#"), StrLit(","),
                                                StrLit("\\"), StrLit(":"), StrLit(";"), StrLit("@"))),
        .joinSymbols = StrLit("~!$%^&*-=+<.>/?|"),
    };
    
    TokenChunkList tokens = {0};
    while (scanner.pos < scanner.source.size)
    {
        Token token = ScannerNext(&scanner);
        u64 pos = token.range.min, size = token.range.max - pos;
        String lexeme = SubstrRange(scanner.source, pos, size);
        
        if (token.flags & TokenFlag_Unterminated)
            token.user |= token.flags & TokenFlag_String ? MD_TokenFlag_BrokenStringLiteral : MD_TokenFlag_BrokenComment;
        
        if (token.flags & TokenFlag_Comment)
            token.user |= MD_TokenFlag_Comment;
        
        if (token.flags & TokenFlag_Whitespace)
            token.user |= MD_TokenFlag_Whitespace;
        
        if (token.flags & TokenFlag_Newline)
            token.user |= MD_TokenFlag_Newline;
        
        if (token.flags & TokenFlag_Identifier)
            token.user |= MD_TokenFlag_Identifier;
        
        if (token.flags & TokenFlag_Numeric)
            token.user |= MD_TokenFlag_Numeric;
        
        if (token.flags & TokenFlag_String)
        {
            token.user |= MD_TokenFlag_StringLiteral;
            u8 style = lexeme.str[0];
            
            switch (style)
            {
                case '\'': token.user |= MD_TokenFlag_StringSingleQuote; break;
                case '"':  token.user |= MD_TokenFlag_StringDoubleQuote; break;
                case '`':  token.user |= MD_TokenFlag_StringTick; break;
            }
            
            if (size >= 3 && lexeme.str[1] == style && lexeme.str[2] == style)
                token.user |= MD_TokenFlag_StringTriplet;
        }
        
        if (token.flags & TokenFlag_Symbol)
        {
            b32 reserved = StrCompareList(lexeme, &scanner.symbols, 0);
            token.user |= reserved ? MD_TokenFlag_Reserved : MD_TokenFlag_Symbol;
        }
        
        if (token.flags == 0)
        {
            Assert(size == 1);
            token.user |= MD_TokenFlag_BadCharacter;
        }
        
        TokenChunkListPush(scratch, &tokens, 4096, token);
    }
    
    TokenArray result = TokenArrayFromChunkList(arena, &tokens);
    ScratchEnd(scratch);
    return result;
}

internal TokenArray CL_TokenizeFromText(Arena* arena, String text)
{
    ScratchBegin(scratch, arena);
    TokenChunkList tokens = {0};
    u8* firstByte = text.str;
    u8* oplByte = firstByte + text.size;
    u8* byte = firstByte;
    
    //- long: Scanning
    b32 isPreproc = 0;
    b32 blankLine = 1;
    while (byte < oplByte)
    {
        TokenFlags flags = 0;
        u64 start = byte - firstByte;
        
#define TokenInit(flg, size) Stmnt(flags = (flg); byte += (size))
#define Check2Bytes(str) (byte+1 < oplByte && byte[0] == (str)[0] && byte[1] == (str)[1])
        if (0) { }
        
        //- long: Whitespaces/Newlines
        else if (*byte == ' ' || *byte == '\t' || *byte == '\v' || *byte == '\r' || *byte == '\n')
        {
            if (*byte == '\n' || *byte == '\r')
            {
                blankLine = 1;
                if (isPreproc)
                    isPreproc = byte[-1] == '\\';
            }
            
            TokenInit(TokenFlag_Whitespace, 1);
            for (; byte < oplByte; ++byte)
            {
                if (*byte == '\n')
                    blankLine = 1;
                
                if (*byte != ' ' && *byte != '\t' && *byte != '\v' && *byte != '\r' && *byte != '\n')
                    break;
            }
        }
        
        //- long: Single-line Comments
        else if (Check2Bytes("//"))
        {
            TokenInit(TokenFlag_Comment, 2);
            for (; byte < oplByte; ++byte)
                if (*byte == '\n')
                    break;
        }
        
        //- long: Multi-line Comments
        else if (Check2Bytes("/*"))
        {
            TokenInit(TokenFlag_Comment, 2);
            b32 closed = 0;
            while (!(closed = Check2Bytes("*/")))
                ++byte;
            
            if (closed)
                byte += 2;
            else
                flags |= TokenFlag_Unterminated;
        }
        
        //- long: Preproc
        else if (*byte == '#' && !isPreproc)
        {
            isPreproc = 1;
            Assert(blankLine); // TODO(long): Error
            // NOTE(long): I could merge the next identifier into this
            TokenInit(TokenFlag_Symbol, 1);
        }
        
        else if (*byte == '#') // @COPYPASTA(long): SYMBOL_TWO
        {
            if (byte+1 < oplByte && byte[1] == byte[0])
                TokenInit(TokenFlag_Symbol, 2);
            else
                goto SYMBOL_ONE;
        }
        
        //- long: Identifiers
        else if (IsCharacter(*byte) || *byte == '_' || utf8_class[*byte>>3] >= 2)
        {
            TokenInit(TokenFlag_Identifier, 1);
            for (; byte < oplByte; ++byte)
                if (!IsAlphaNumeric(*byte) && *byte != '_' && utf8_class[*byte>>3] < 2)
                    break;
        }
        
        //- long: Numerics
        else if (IsDigit(*byte) || (*byte == '.' && byte+1 < oplByte && IsDigit(byte[1])))
        {
            TokenInit(TokenFlag_Numeric, 1);
            for (u8 prev = 0; byte < oplByte; prev = *byte++)
            {
                if (*byte == '+' || *byte == '-')
                {
                    u8 c = ChrToLower(prev);
                    if (c == 'e' || c == 'p')
                        continue;
                    break;
                }
                
                if (!IsAlphaNumeric(*byte) && *byte != '.')
                    break;
            }
        }
        
        //- long: String/Char literals
        else if (byte[0] == '"' || byte[0] == '\'')
        {
            u8 lit = byte[0];
            TokenInit(TokenFlag_String, 1);
            
            for (b32 escaped = 0, done = 0; byte <= oplByte && !done; ++byte)
            {
                if (byte == oplByte || *byte == '\n')
                {
                    flags |= TokenFlag_Unterminated;
                    break;
                }
                
                if (escaped)            escaped = 0;
                else if (*byte == '\\') escaped = 1;
                else if (*byte ==  lit)    done = 1;
            }
        }
        
        //- long: Special Symbols
        else if (byte+2 < oplByte && byte[0] == '.' && byte[1] == '.' && byte[2] == '.')
            TokenInit(TokenFlag_Symbol, 3);
        else if (Check2Bytes("->"))
            TokenInit(TokenFlag_Symbol, 2);
        
        //- long: 3-character Symbols
        else if (*byte == '<' || *byte == '>')
        {
            if (byte+2 < oplByte && byte[1] == byte[0] && byte[2] == '=')
                TokenInit(TokenFlag_Symbol, 3); // <<= >>=
            else // < > << >>
                goto SYMBOL_TWO;
        }
        
        //- long: 2-character Symbols
        else if (*byte == '+' || *byte == '-' || *byte == '&' || *byte == '|')
        {
            SYMBOL_TWO:
            if (byte+1 < oplByte && byte[1] == byte[0])
                TokenInit(TokenFlag_Symbol, 2); // ++ -- && ||
            else
                goto SYMBOL_EQUAL;
        }
        
        else if (*byte == '*' || *byte == '/' || *byte == '!' || *byte == '^' || *byte == '%' || *byte == '=')
        {
            SYMBOL_EQUAL:
            if (byte+1 < oplByte && byte[1] == '=')
                TokenInit(TokenFlag_Symbol, 2);
            else
                goto SYMBOL_ONE;
        }
        
        //- long: 1-character Symbols
        else if (*byte == '[' || *byte == ']' || *byte == '(' || *byte == ')' || *byte == '{' || *byte == '}' ||
                 *byte == ',' || *byte == ';' || *byte == ':' || *byte == '?' || *byte == '~')
        {
            SYMBOL_ONE:
            TokenInit(TokenFlag_Symbol, 1);
        }
        
        //- long: Invalid characters
        else byte++;
        
#undef TokenInit
#undef Check2Bytes
        
        //- long: Push Token
        u64 opl = byte - firstByte;
        Assert(opl > start);
        
        if (!(flags & CL_TokenFlags_Ignorable))
            blankLine = 0;
        if (isPreproc)
            flags |= TokenFlag_Preproc;
        
        Token token = { .flags = flags, .range = {start, opl} };
        TokenChunkListPush(scratch, &tokens, 4096, token);
    }
    
    TokenArray result = TokenArrayFromChunkList(arena, &tokens);
    ScratchEnd(scratch);
    
    for (u64 i = 1; i < result.count; ++i)
    {
        Token token1 = result.tokens[i-1];
        Token token2 = result.tokens[i];
        Assert(token2.range.min == token1.range.max);
        
        if (i == 1)
            Assert(token1.range.min == 0);
        if (i == result.count - 1)
            Assert(token2.range.max == text.size);
    }
    
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
    
    if (0)
    {
        String file = StrLit("code/test.json");
        MD_ParseResult parse = MD_ParseText(scratch, file, OSReadFile(scratch, file));
        String dump = MD_DumpJSON(scratch, parse.root->first, 0);
        OSWriteFile(StrLit("code/out_test.mdesk"), dump);
        Assert(parse.root->first == parse.root->last);
    }
    
    String paths[] = { StrLit("code/test.json"), StrLit("code/test.mdesk"), StrLit("code/retired/syntax.mdesk") };
    ForEach(pathIdx, ArrayCount(paths))
    {
        String text = OSReadFile(scratch, paths[pathIdx]);
        MD_TokenArray tokens1 = MD_TokenizeFromText(scratch, text).tokens;
        TokenArray tokens2 = MD_TokenArrayFromStr(scratch, text);
        
        Assert(tokens1.count == tokens2.count);
        u64 count = Min(tokens1.count, tokens2.count);
        
        ForEach(i, count)
        {
            MD_Token t1 = tokens1.v[i];
            Token t2 = tokens2.tokens[i];
            Assert(t1.flags == t2.user);
            Assert(t1.range.min == t2.range.min && t1.range.max == t2.range.max);
        }
    }
    
    {
        String data = OSReadFile(scratch, StrLit("code/examples/test_parser.c"));
        TokenArray array = CL_TokenizeFromText(scratch, data);
        
        TokenArray array_ = CL_TokenArrayFromStr(scratch, data);
        Assert(array.count == array_.count);
        for (u64 i = 0; i < array.count; ++i)
        {
            Token t1 = array_.tokens[i];
            Token t2 = array .tokens[i];
            Assert(t1.flags == t2.flags && t1.user == t2.user);
            Assert(t1.range.min != t1.range.max);
            Assert(t1.range.min == t2.range.min && t1.range.max == t2.range.max);
        }
        
        for (u64 i = 0; i < array.count; ++i)
        {
            Token token = array.tokens[i];
            Assert(NoFlags(token.flags, TokenFlag_Unterminated|TokenFlag_Preproc));
            Assert(token.flags);
        }
        
        CL_ParseResult parse = CL_ParseFromTokens(scratch, data, array);
        CL_Node* node = parse.root->body.first;
        u32 indent = 0;
        
#define NodeExpand(node) StrExpand((node)->string.size ? (node)->string : StrLit("<unnamed>"))
        
        REP:
        Outf("%*s%.*s", indent * 2, "", NodeExpand(node));
        if (node->flags & CL_NodeFlag_Symbol)
        {
            if (node->offset < node->body.first->offset)
                Assert(node->body.first == node->body.last);
            if (!StrCompare(node->string, StrLit("("), 0))
                Assert(!CL_IsNil(node->body.first));
            
            CL_ExprOpKind op = CL_ExprOpFromNode(node);
            Outf(" (%s)", GetEnumCStr(CL_ExprOpKind, op));
        }
        
        for (CL_Node* base = node->reference; !CL_IsNil(base); base = base->reference)
        {
            if (base == node->reference)
                Outf(": ");
            Outf("%.*s", NodeExpand(base));
            
            for (CL_Node* tag = base->tags.first; !CL_IsNil(tag); tag = tag->next)
            {
                if (CL_IsNil(tag->prev))
                    Outf(" (");
                Outf("%.*s", NodeExpand(tag));
                Outf(CL_IsNil(tag->next) ? ")" : ", ");
            }
            
            if (!CL_IsNil(base->reference))
                Outf(" -> ");
        }
        Outf("\n");
        
        CL_Node* old = node;
        if (!CL_IsNil(node->args.first))
        {
            indent++;
            Outf("%*s<args>:\n", indent * 2, "");
            node = node->args.first;
            indent++;
        }
        
        else if (!CL_IsNil(node->body.first))
        {
            node = node->body.first;
            indent++;
        }
        
        else if (!CL_IsNil(node->next))
            node = node->next;
        
        else for (CL_Node* parent = node->parent,* child = node; !CL_IsNil(parent); parent = parent->parent, child = child->parent)
        {
            indent--;
            b32 isArg = child->parent->args.last == child;
            if (isArg)
            {
                if (!CL_IsNil(parent->body.first))
                {
                    node = parent->body.first;
                    break;
                }
                indent--;
            }
            
            if (!CL_IsNil(parent->next))
            {
                node = parent->next;
                break;
            }
        }
        
        if (indent == 0)
            Outf("\n");
        if (node != old)
            goto REP;
    }
    
#if 0
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
                if (token->flags & TokenFlag_Symbol)
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
    
    Outf("\nDone");
    
    ScratchEnd(scratch);
    return 0;
}
