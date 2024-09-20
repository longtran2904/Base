#include "Base.h"
#include "Base.c"

#include "LongScanner.h"
#include "LongScanner.c"
#include "LongMD.h"

function MD_TokenizeResult Long_MD_TokenizeFromScanner(Arena* arena, String text)
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
    MarkerPushArr(scanner, MD_TokenFlag_Reserved, StrLit("{}()[]#,\\:;@"), 1);
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

function MD_TokenizeResult Long_MD_TokenizeFromLexer(Arena* arena, String8 text)
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
        else if (ScannerParse(scanner, StrLit("{}()[]#,\\:;@"), ScannerMatchFlag_IsArray))
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

internal MD_TokenizeResult Long_MD_TokenizeFromText(Arena *arena, String8 text)
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

internal String MD_DumpJSON(Arena* arena, MD_Node* node, u64 indent)
{
    String result = {0};
    if (!md_node_is_nil(node) && node->kind == MD_NodeKind_Main)
    {
        if (node->flags & MD_NodeFlag_MaskLabelKind)
            result = node->raw_string;
        
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
    }
    
    return result;
}

int main(void)
{
    ScratchBegin(scratch);
    String file = StrLit("code/test.json");
    MD_ParseResult parse = MD_ParseText(scratch, file, OSReadFile(scratch, file));
    String dump = MD_DumpJSON(scratch, parse.root->first, 0);
    OSWriteFile(StrLit("code/out_test.mdesk"), dump);
    Assert(parse.root->first == parse.root->last);
    
    String text = OSReadFile(scratch, StrLit("code/test.json"));
    TokenArray array = JSON_TokenizeFromText(scratch, text);
    JSON_Value value = JSON_ValueFromTokens (scratch, text, array);
    OSWriteFile(StrLit("code/out_test.json"), JSON_StrFromValue(scratch, value, 0));
    
    text = OSReadFile(scratch, StrLit("code/test.csv"));
    StringTable table = CSV_TableFromStr(scratch, text);
    OSWriteFile(StrLit("code/out_test.csv"), CSV_StrFromTable(scratch, table));
    
    Outf("Done");
    
    ScratchEnd(scratch);
    return 0;
}
