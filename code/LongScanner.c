
//~ long: Scanner Functions

// AdvanceUntilNot
// MatchLine
// MatchStr -> AdvanceUntilStr (escape)
// MatchArr -> AdvanceUntilNot
// Exponent and Unary plus/minus

//- long: Lexing Functions
internal String ScannerInverseStr(Arena* arena, String str)
{
    String result = PushBuffer(arena, str.size);
    ForEach(i, result.size)
        result.str[i] = str.str[str.size - 1 - i];
    
    ForEach(i, result.size)
    {
        u8 c = result.str[i];
        switch (result.str[i])
        {
            case '{': c = '}'; break;
            case '}': c = '{'; break;
            case '[': c = ']'; break;
            case ']': c = '['; break;
            case '(': c = ')'; break;
            case ')': c = '('; break;
            case '+': c = '-'; break;
            case '-': c = '+'; break;
        }
        result.str[i] = c;
    }
    
    return result;
}

function b32 ScannerAdvance(Scanner* scanner, i64 advance)
{
    i64 pos = scanner->pos;
    scanner->pos = Clamp(pos + advance, 0, (i64)scanner->source.size);
    return scanner->pos != pos;
}

function b32 ScannerCompare(Scanner* scanner, String str)
{
    b32 result = 0;
    if (str.size)
    {
        String lexeme = ScannerStr(scanner, str.size);
        result = StrCompare(lexeme, str, 0);
    }
    return result;
}

function b32 ScannerCompareArr(Scanner* scanner, String arr)
{
    u8 byte = ScannerByte(scanner, 0);
    b32 result = 0;
    for (u64 i = 0; i < arr.size && !result; ++i)
        if (byte == arr.str[i])
            result = 1;
    return result;
}

// NOTE(long): This currently doesn't handle when you actually want to escape with \0
function b32 ScannerAdvanceUntil(Scanner* scanner, String str, u8 escapeChar)
{
    for (b32 escaped = 0; scanner->pos < scanner->source.size; scanner->pos++)
    {
        if (escaped)
            escaped = 0;
        else if (scanner->source.str[scanner->pos] == escapeChar)
            escaped = 1;
        else if (ScannerCompare(scanner, str))
            return 1;
    }
    return 0;
}

function b32 ScannerAdvanceUntilArr(Scanner* scanner, String arr)
{
    b32 result = 0;
    for (; scanner->pos < scanner->source.size && !result; scanner->pos++)
        if (ScannerCompareArr(scanner, arr))
            result = 1;
    return result;
}

function b32 ScannerAdvancePast(Scanner* scanner, String str, u8 escapeChar)
{
    b32 result = ScannerAdvanceUntil(scanner, str, escapeChar);
    if (result)
        ScannerAdvance(scanner, str.size);
    return result;
}

function b32 ScannerParse(Scanner* scanner, String str)
{
    b32 result = ScannerCompare(scanner, str);
    if (result)
        ScannerAdvance(scanner, str.size);
    return result;
}

function b32 ScannerParseArr(Scanner* scanner, String arr)
{
    b32 result = 0;
    while (ScannerCompareArr(scanner, arr) && ScannerAdvance(scanner, 1))
        result = 1;
    return result;
}

function b32 ScannerParseList(Scanner* scanner, StringList* list)
{
    b32 result = 0;
    StrListIter(list, node)
    {
        if (ScannerParse(scanner, node->string))
        {
            result = 1;
            break;
        }
    }
    return result;
}

function void ScannerParseLine(Scanner* scanner, u8 escapeChar)
{
    REPEAT:
    u64 oldPos = scanner->pos;
    if (ScannerAdvanceUntil(scanner, StrLit("\n"), escapeChar))
    {
        if (scanner->pos - oldPos > 1)
        {
            u8* str = scanner->source.str;
            if (str[scanner->pos-1] == '\r' && str[scanner->pos-2] == escapeChar)
                goto REPEAT;
        }
    }
}

internal b32 ScannerParseStrDelim(Scanner* scanner)
{
    b32 result = 0;
    Scanner restore = *scanner;
    
    ScannerParseList(scanner, &scanner->strPrefixes);
    
    if (0) { }
    else if ((scanner->flags & CL_Scan_Ticks       ) && ScannerParse(scanner, StrLit( "`"))) result = 1;
    else if ((scanner->flags & CL_Scan_SingleQuotes) && ScannerParse(scanner, StrLit( "'"))) result = 1;
    else if ((scanner->flags & CL_Scan_DoubleQuotes) && ScannerParse(scanner, StrLit("\""))) result = 1;
    
    if (!result)
        *scanner = restore;
    return result;
}

function b32 ScannerParseLineDelim(Scanner* scanner, String delim, u8 escapeChar)
{
    b32 result = 0;
    
    for (b32 escaped = 0; scanner->pos < scanner->source.size && !result; scanner->pos++)
    {
        if (ScannerCompare(scanner, StrLit("\r\n")))
            scanner->pos++;
        
        if (ScannerByte(scanner, 0) == '\n')
            if (!(escaped && (scanner->flags & CL_Line_Cont_Strings)))
                break;
        
        if (escaped)
            escaped = 0;
        else if (scanner->source.str[scanner->pos] == escapeChar)
            escaped = 1;
        else if (ScannerCompare(scanner, delim))
            result = 1;
    }
    
    return result;
}

function Token ScannerNext(Scanner* scanner)
{
    ScratchBegin(scratch);
    Token token = {0};
    Token prevToken = scanner->token;
    
    b32 isPreproc = prevToken.flags & TokenFlag_Preproc;
    b32 blankLine = scanner->pos == 0;
    if (HasAnyFlags(prevToken.flags, TokenFlag_Whitespace|TokenFlag_Newline))
    {
        Assert(prevToken.range.min != prevToken.range.max);
        String lexeme = ScannerRange(scanner, prevToken.range.min, prevToken.range.max);
        if (lexeme.str[0] == '\n' || (lexeme.size >= 2 && lexeme.str[0] == '\r' && lexeme.str[1] == '\n'))
            blankLine = 1;
    }
    
    String commentMultiEnd = ScannerInverseStr(scratch, scanner->commentMulti);
    String alphaNumeric = StrLit(Characters Digits);
    String identDelim = StrJoin3(scratch, scanner->flags & CL_Skip_Ident_Nums ?
                                 StrLit(Characters) : alphaNumeric, scanner->midIdentSymbols);
    String numDelim = StrJoin3(scratch, scanner->flags & CL_Skip_Num_Idents ?
                               StrLit(Digits) : alphaNumeric, scanner->midNumSymbols);
    
    b32 scanNewline = scanner->flags & CL_Scan_Newline;
    String whitespace = StrLit(WspaceStr);
    if (isPreproc || scanNewline)
        whitespace = StrLit(SpaceStr);
    
    String lineCont1 = StrPushf(scratch, "%c\n", scanner->lineContinuation);
    String lineCont2 = StrPushf(scratch, "%c\r\n", scanner->lineContinuation);
    
    //- long: Scanning
    if (scanner->pos < scanner->source.size)
    {
        TokenFlags flags = 0;
        u64 start = scanner->pos;
        
        if (0) { }
        
        //- long: Whitespace
        else if (ScannerParseArr(scanner, whitespace) || ScannerCompare(scanner, lineCont1) || ScannerCompare(scanner, lineCont2))
        {
            flags |= TokenFlag_Whitespace;
            while (ScannerParse(scanner, lineCont1) || ScannerParse(scanner, lineCont2))
                if (!ScannerParseArr(scanner, whitespace))
                    break;
        }
        
        // NOTE(long): This only runs when in preproc or CL_Scan_Newline is set
        else if (ScannerParse(scanner, StrLit("\n")) || ScannerParse(scanner, StrLit("\r\n")))
        {
            flags |= scanNewline ? TokenFlag_Newline : TokenFlag_Whitespace;
            isPreproc = 0;
        }
        
        //- long: Hook
        else if (scanner->hook && (flags = scanner->hook(scanner)));
        
        //- long: Single-line Comments
        else if (ScannerParse(scanner, scanner->commentDelim))
        {
            flags |= TokenFlag_Comment;
            u8 escape = scanner->flags & CL_Line_Cont_Comments ? scanner->lineContinuation : 0;
            ScannerParseLine(scanner, escape);
        }
        
        //- long: Multi-line Comments
        else if (ScannerParse(scanner, scanner->commentMulti))
        {
            flags |= TokenFlag_Comment;
            if (scanner->flags & CL_Nest_Comments)
            {
                i64 nest = 1;
                while (scanner->pos < scanner->source.size)
                {
                    if (nest == 0)
                        break;
                    
                    if (ScannerParse(scanner, scanner->commentMulti))
                        nest++;
                    else if (ScannerParse(scanner, commentMultiEnd))
                        nest--;
                    else scanner->pos++;
                }
                
                if (nest)
                    flags |= TokenFlag_Unterminated;
            }
            
            else if (!ScannerAdvancePast(scanner, commentMultiEnd, 0))
                flags |= TokenFlag_Unterminated;
        }
        
        //- long: Preproc
        else if (!isPreproc && blankLine && ScannerParse(scanner, scanner->preproc))
        {
            if (scanner->flags & CL_Scan_Preproc)
            {
                isPreproc = 1;
                flags |= TokenFlag_Symbol;
            }
            
            else
            {
                flags |= TokenFlag_Preproc;
                ScannerParseLine(scanner, scanner->flags & CL_Line_Cont_Preprocs ? scanner->lineContinuation : 0);
            }
        }
        
        //- long: String Literals
        else if (ScannerParseStrDelim(scanner))
        {
            flags |= TokenFlag_String;
            String style = StrFromChr(scanner->source.str[scanner->pos-1]);
            ScannerParseLineDelim(scanner, style, scanner->escapeChar);
        }
        
        //- long: Identifiers
        // TODO(long): UTF8
        else if (ScannerParseArr(scanner, StrLit(Characters)) || ScannerParseArr(scanner, scanner->preIdentSymbols))
        {
            flags |= TokenFlag_Identifier;
            ScannerParseArr(scanner, identDelim);
        }
        
        //- long: Numerics
        else if (ScannerParseArr(scanner, StrLit(Digits)))
        {
            SCAN_NUMERIC:
            flags |= TokenFlag_Numeric;
            
            ScannerParseArr(scanner, numDelim);
            while (ScannerCompareArr(scanner, StrLit("+-")) && ChrCompareArr(ScannerByte(scanner, -1), scanner->exponents, 0))
            {
                ScannerAdvance(scanner, 1);
                ScannerParseArr(scanner, numDelim);
            }
            
            // TODO(long): handle symbols after numerics (e.g. 0..<10 in Odin)
        }
        
        else if (ScannerCompareArr(scanner, scanner->preNumSymbols) && IsDigit(ScannerByte(scanner, 1)))
        {
            ScannerAdvance(scanner, 2);
            goto SCAN_NUMERIC;
        }
        
        //- long: Symbols
        else if (ScannerParseList(scanner, &scanner->symbols))
            flags |= TokenFlag_Symbol;
        else if (ScannerParseArr(scanner, scanner->joinSymbols))
            flags |= TokenFlag_Symbol;
        
        //- long: Invalid characters
        else scanner->pos++;
        
        //- long: Push Token
        u64 opl = scanner->pos;
        Assert(opl > start);
        if (isPreproc)
            flags |= TokenFlag_Preproc;
        
        token = (Token){ .flags = flags, .range = {start, opl} };
        scanner->token = prevToken;
    }
    
    ScratchEnd(scratch);
    return token;
}

function Token ScannerPeek(Scanner* scanner)
{
    u64 pos = scanner->pos;
    Token result = ScannerNext(scanner);
    scanner->pos = pos;
    return result;
}

function Token ScannerPeekAhead(Scanner* scanner, i64 tokenCount)
{
    u64 pos = scanner->pos;
    Token result = {0};
    ForEach(_, tokenCount)
    {
        result = ScannerNext(scanner);
        if (scanner->pos >= scanner->source.size)
            break;
    }
    scanner->pos = pos;
    return result;
}

//~ long: Token Functions

function String StrFromToken(String text, Token token)
{
    String result = Substr(text, token.range.min, token.range.max);
    return result;
}

function b32 TokenMatch(String text, Token token, String match)
{
    String lexeme = StrFromToken(text, token);
    b32 result = StrCompare(lexeme, match, 0);
    return result;
}

function Token* TokenChunkListPush(Arena* arena, TokenChunkList* list, u64 cap, Token token)
{
    TokenChunkNode* node = list->last;
    if (!node || node->count >= node->cap)
    {
        node = PushStruct(arena, TokenChunkNode);
        node->cap = cap;
        node->tokens = PushArrayNZ(arena, Token, cap);
        SLLQueuePush(list->first, list->last, node);
        list->chunkCount++;
    }
    
    Token* result = node->tokens + node->count;
    node->count++;
    list->totalTokenCount++;
    
    *result = token;
    return result;
}

function TokenArray TokenArrayFromChunkList(Arena* arena, TokenChunkList* chunks)
{
    TokenArray result = {0};
    result.count = chunks->totalTokenCount;
    result.tokens = PushArrayNZ(arena, Token, result.count);
    u64 index = 0;
    ForList(TokenChunkNode, n, chunks->first)
    {
        CopyMem(result.tokens + index, n->tokens, sizeof(Token)*n->count);
        index += n->count;
    }
    return result;
}

//~ long: CSV Parser

function u64 CSV_StrListPushRow(Arena* arena, StringList* list, String text)
{
    Scanner* scanner = &ScannerFromStr(text);
    r1u64 range = {0};
    
    while (1)
    {
        b32 eof = !ScannerAdvanceUntilArr(scanner, StrLit(",\n\""));
        range.max = scanner->pos;
        
        b32 isComma = ScannerParse(scanner, StrLit(","));
        if (eof || isComma || ScannerParse(scanner, StrLit("\n")))
        {
            String str = ScannerRange(scanner, range.min, range.max);
            if (StrStartsWith(str, StrLit("\""), 0) && StrEndsWith(str, StrLit("\""), 0))
            {
                str = Substr(str, 1, str.size - 1);
                str = StrReplace(arena, str, StrLit("\"\""), StrLit("\""), 0);
            }
            StrListPush(arena, list, str);
            
            if (isComma)
                range.min = scanner->pos;
            else // newline or eof
                break;
        }
        
        if (ScannerParse(scanner, StrLit("\"")))
            if (ScannerAdvanceUntil(scanner, StrLit("\""), '"'))
                ScannerAdvance(scanner, 1);
    }
    
    return scanner->pos;
}

function StringTable CSV_TableFromStr(Arena* arena, String str)
{
    StringTable result = {0};
    
    u64 lineCount = 1;
    ForEach(i, str.size-1)
    {
        if (str.str[i] == '\n')
            lineCount++;
    }
    
    result.rowCount = lineCount;
    result.rows = PushArray(arena, StringList, lineCount);
    
    ForEach(i, result.rowCount)
    {
        u64 next = CSV_StrListPushRow(arena, result.rows + i, str);
        result.cellCount += result.rows[i].nodeCount;
        result.totalSize += result.rows[i].totalSize;
        str = StrSkip(str, next);
        Assert(next != 0);
    }
    
    return result;
}

function String CSV_StrFromTable(Arena* arena, StringTable table)
{
    u64 cellCount = ClampBot(table.cellCount, 1);
    String result = StrPush(arena, cellCount-1 + cellCount*2 + table.totalSize * 2);
    for (u64 row = 0, off = 0; row < table.rowCount; ++row)
    {
        ForList(StringNode, node, table.rows[row].first)
        {
            String str = node->string;
            ScratchBlock(scratch, arena)
            {
                if (StrContainsChr(str, "\",\n"))
                {
                    String quote = StrLit("\"");
                    StringList list = StrSplit(scratch, str, quote, SplitStr_KeepEmpties);
                    str = StrJoin(scratch, &list, .mid = StrLit("\"\""), .pre = quote, .post = quote);
                }
                
                StrWriteToStr(str, 0, result, off);
                off += str.size;
            }
            
            b32 last = row == table.rowCount - 1 && !node->next;
            if (last)
            {
                ArenaPop(arena, result.size - off);
                result.size = off;
            }
            else
                result.str[off++] = node->next ? ',' : '\n';
            
            Assert(off <= result.size);
        }
    }
    
    return result;
}

//~ long: JSON Parser
function TokenArray JSON_TokenizeFromText(Arena* arena, String text)
{
    ScratchBegin(scratch, arena);
    TokenChunkList tokens = {0};
    u8 *byte_first = text.str;
    u8 *byte_opl = byte_first + text.size;
    u8 *byte = byte_first;
    
    //- long: Scan string & produce tokens
    while (byte < byte_opl)
    {
        Token token = { .range.min = byte - byte_first };
        
#define TokenInit(type, size) Stmnt(token.user = (type); byte += (size))
#define Check2Bytes(str) (byte+1 < byte_opl && byte[0] == (str)[0] && byte[1] == (str)[1])
        if (0) { }
        
#if USE_LJSON_SPEC
        // long: Terminator
        else if (*byte == ';' || *byte == '\n') TokenInit(JSON_TokenType_Terminator, 1);
        
        // long: Assignment
        else if (*byte == '=') TokenInit(JSON_TokenType_Assignment, 1);
        
        //- long: Identifiers
        else if (IsCharacter(*byte) || *byte == '_' || *byte > MAX_I8)
        {
            TokenInit(JSON_TokenType_String, 1);
            for (; byte < byte_opl; ++byte)
                if (!IsAlphaNumeric(*byte) && *byte != '_' && *byte > MAX_I8)
                    break;
        }
        
        //- long: Number
        else if (IsDigit(byte[0]) ||
                 (byte+1 < byte_opl && (byte[0] == '-' || byte[0] == '+' || byte[0] == '.') && IsDigit(byte[1])) ||
                 (byte+2 < byte_opl && (byte[0] == '-' || byte[0] == '+') && byte[1] == '.' && IsDigit(byte[2])))
        {
            TokenInit(JSON_TokenType_Number, 1);
            for (; byte < byte_opl; ++byte)
            {
                if ((*byte == '+' || *byte == '-') && (byte[-1] == 'e' || byte[-1] == 'E'))
                    continue;
                if (!IsAlphaNumeric(*byte) && *byte != '.')
                    break;
            }
        }
        
        //- long: Single-line comments
        else if (Check2Bytes("//"))
        {
            TokenInit(JSON_TokenType_Whitespace, 2);
            b32 escaped = 0;
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
        
        //- long: Multi-line comments
        else if (Check2Bytes("/*"))
        {
            TokenInit(JSON_TokenType_Whitespace, 2);
            for (i64 nest = 1; byte < byte_opl;)
            {
                if (nest == 0)
                    break;
                
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
                token.flags |= TokenFlag_Unterminated;
        }
        
        //- long: Single-quote string
        else if (*byte == '\'')
        {
            u8 style = byte[0];
            TokenInit(JSON_TokenType_String, 1);
            
            b32 escaped = 0;
            for (; byte <= byte_opl; ++byte)
            {
                if (byte == byte_opl)
                {
                    ENDSTR:
                    token.flags |= TokenFlag_Unterminated;
                    break;
                }
                
                if (escaped) escaped = 0;
                else if (*byte == '\n') goto ENDSTR;
                else if (*byte == '\\') escaped = 1;
                else if (*byte == style)
                {
                    ++byte;
                    break;
                }
            }
        }
#endif
        
        //- long: Whitespace
        else if (*byte == ' ' || *byte == '\t' || *byte == '\v' || *byte == '\r' || *byte == '\n')
        {
            TokenInit(JSON_TokenType_Whitespace, 1);
            for (; byte < byte_opl; ++byte)
                if (*byte != ' ' && *byte != '\t' && *byte != '\v' && *byte != '\r' && *byte != '\n')
                    break;
        }
        
        //- long: Number
        else if (IsDigit(byte[0]) || (byte+1 < byte_opl && byte[0] == '-' && IsDigit(byte[1])))
        {
            TokenInit(JSON_TokenType_Number, 1);
            for (; byte < byte_opl; ++byte)
            {
                if (*byte == 'e' || *byte == 'E')
                {
                    if (byte+1 < byte_opl && (byte[1] == '+' || byte[1] == '-'))
                        ++byte;
                    continue;
                }
                
                if (!IsDigit(*byte) && *byte != '.')
                    break;
            }
        }
        
        //- long: Double-quote String
        else if (byte[0] == '"')
        {
            TokenInit(JSON_TokenType_String, 1);
            
            b32 escaped = 0;
            for (; byte <= byte_opl; ++byte)
            {
                if (byte == byte_opl
#if !USE_LJSON_SPEC
                    || *byte < ' ' || *byte == 127
#endif
                    )
                {
                    token.flags |= TokenFlag_Unterminated;
                    break;
                }
                
                if (escaped) escaped = 0;
                else if (byte[0] == '\\') escaped = 1;
                else if (byte[0] == '"')
                {
                    ++byte;
                    break;
                }
            }
        }
        
        // long: Comma/Colon
        else if (*byte == ',')
            TokenInit(JSON_TokenType_Terminator, 1);
        else if (*byte == ':')
            TokenInit(JSON_TokenType_Assignment, 1);
        
        //- long: Braces/Brackets
        else if (*byte == '{')
            TokenInit(JSON_TokenType_OpenObj, 1);
        else if (*byte == '}')
            TokenInit(JSON_TokenType_CloseObj, 1);
        else if (*byte == '[')
            TokenInit(JSON_TokenType_OpenArr, 1);
        else if (*byte == ']')
            TokenInit(JSON_TokenType_CloseArr, 1);
        
        //- long: Value
        else if (StrStartsWith(Str(byte, byte_opl - byte), StrLit("true"), 0))
            TokenInit(JSON_TokenType_True, 1);
        else if (StrStartsWith(Str(byte, byte_opl - byte), StrLit("false"), 0))
            TokenInit(JSON_TokenType_False, 1);
        else if (StrStartsWith(Str(byte, byte_opl - byte), StrLit("null"), 0))
            TokenInit(JSON_TokenType_Null, 1);
        
        //- long: Invalid characters in all other cases
        else TokenInit(JSON_TokenType_Invalid, 1);
        
#undef TokenInit
#undef Check2Bytes
        
        //- NOTE(long): This is optional
        switch (token.user)
        {
            case JSON_TokenType_OpenObj:
            case JSON_TokenType_CloseObj:
            case JSON_TokenType_OpenArr:
            case JSON_TokenType_CloseArr:
            case JSON_TokenType_Terminator:
            case JSON_TokenType_Assignment: token.flags |= TokenFlag_Symbol; break;
            
            case JSON_TokenType_Null:
            case JSON_TokenType_True:
            case JSON_TokenType_False:      token.flags |= TokenFlag_Identifier; break;
            
            case JSON_TokenType_Whitespace: token.flags |= TokenFlag_Whitespace; break;
            case JSON_TokenType_String:     token.flags |= TokenFlag_String; break;
            case JSON_TokenType_Number:     token.flags |= TokenFlag_Numeric; break;
        }
        
        //- long: push token if formed
        token.range.max = byte - byte_first;
        if (ALWAYS(token.range.max > token.range.min))
            TokenChunkListPush(scratch, &tokens, 4096, token);
    }
    
    //- rjf: bake, fill & return
    TokenArray result = TokenArrayFromChunkList(arena, &tokens);
    ScratchEnd(scratch);
    return result;
}

function JSON_Value JSON_ValueFromStr(JSON_Object obj, String str)
{
    JSON_Value result = {0};
    u64 hash = Hash64(str.str, str.size);
    
    JSON_MapSlot slot = obj.slots[hash % obj.count];
    ForList(JSON_MapNode, node, slot.first)
    {
        if (StrCompare(node->key, str, 0))
        {
            result = node->value;
            break;
        }
    }
    
    return result;
}

function void JSON_ObjInsertValue(Arena* arena, JSON_Object obj, String key, JSON_Value value)
{
    u64 hash = Hash64(key.str, key.size);
    JSON_MapNode* node = PushStruct(arena, JSON_MapNode);
    node->key = StrCopy(arena, key);
    node->value = value;
    
    JSON_MapSlot* slot = &obj.slots[hash % obj.count];
    SLLQueuePush(slot->first, slot->last, node);
}

internal JSON_Node* JSON_PopNode(JSON_Node** parent, JSON_ValueType type, Token* token)
{
    JSON_Node* result = 0;
    if (type == 0 || (*parent)->value.type == type)
    {
        result = *parent;
        *parent = (*parent)->parent;
        (*parent)->range.max = token->range.max;
    }
    return result;
}

internal u64 JSON_ChildCount(JSON_Node* node)
{
    u64 result = 0;
    ForList(JSON_Node, _, node->first)
        result++;
    return result;
}

function JSON_Node* JSON_ParseFromTokens(Arena* arena, String text, TokenArray array)
{
    JSON_Node* result = PushStruct(arena, JSON_Node);
    
    JSON_Node* parent = result;
    for (Token* token = array.tokens,* tokenOpl = array.tokens + array.count; token < tokenOpl; ++token)
    {
        JSON_Value value = {0};
        String str = Substr(text, token->range.min, token->range.max);
        b32 error = 0; // TODO(long): Report errors
        
        switch (token->user)
        {
            case JSON_TokenType_OpenObj: value.type = JSON_ValueType_Object; break;
            case JSON_TokenType_OpenArr: value.type = JSON_ValueType_Array;  break;
            
            case JSON_TokenType_CloseObj:
            {
                JSON_Node* objNode = JSON_PopNode(&parent,  JSON_ValueType_Object, token);
                if (objNode)
                {
                    u64 count = JSON_ChildCount(objNode);
                    objNode->value.obj = JSON_PushObject(arena, count);
                    
                    ForList(JSON_Node, node, objNode->first)
                    {
                        if (node->first)
                            JSON_ObjInsertValue(arena, objNode->value.obj, node->value.str, node->first->value);
                    }
                }
            } break;
            
            case JSON_TokenType_CloseArr:
            {
                JSON_Node* arrNode = JSON_PopNode(&parent, JSON_ValueType_Array, token);
                if (arrNode)
                {
                    u64 count = JSON_ChildCount(arrNode);
                    arrNode->value.array = JSON_PushArray(arena, count);
                    
                    u64 i = 0;
                    ForList(JSON_Node, node, arrNode->first)
                        arrNode->value.array.values[i++] = node->value;
                }
            } break;
            
            case JSON_TokenType_String:
            {
                u8 start = str.str[0];
                if (start == '\'' || start == '"')
                    str = StrSkip(str, 1);
                
                if (str.size)
                {
                    u8 end = str.str[str.size-1];
                    if (end == '\'' || end == '"')
                        str = StrChop(str, 1);
                }
                
                value = JSON_StrValue(StrCEscape(arena, str));
            } break;
            
            case JSON_TokenType_Number:
            {
                if (StrContainsChr(str, ".eE"))
                    value = JSON_F64Value( F64FromStr(str, &error));
                else
                    value = JSON_I64Value(I64FromStrC(str, &error));
            } break;
            
            case JSON_TokenType_True:   JSON_B32Value(1); break;
            case JSON_TokenType_False:  JSON_B32Value(0); break;
        }
        
        if (value.type != JSON_ValueType_Null || token->user == JSON_TokenType_Null)
        {
            JSON_Node* node = PushStruct(arena, JSON_Node);
            node->value = value;
            node->range = token->range;
            node->parent = parent;
            DLLPushBack(parent->first, parent->last, node);
            
            b32 pushParent = value.type == JSON_ValueType_Object || value.type == JSON_ValueType_Array;
            if ((parent->value.type == JSON_ValueType_Object || parent == result) &&
                value.type == JSON_ValueType_String)
            {
                for (++token; token < tokenOpl - 1 && token->user == JSON_TokenType_Whitespace; ++token); // Skip whitespaces
                if (token->user == JSON_TokenType_Assignment)
                    pushParent = 1;
            }
            
            if (pushParent)
                parent = node;
        }
        
        if (parent->value.type == JSON_ValueType_String && parent->first)
            JSON_PopNode(&parent, 0, token);
    }
    
    Assert(result == parent);
    return result;
}

function JSON_Value JSON_ValueFromTokens(Arena* arena, String text, TokenArray array)
{
    JSON_Node* node = JSON_ParseFromTokens(arena, text, array);
    JSON_Value result = node->first ? node->first->value : (JSON_Value){0};
    return result;
}

function String JSON_StrFromValue(Arena* arena, JSON_Value value, u32 indent)
{
    String result = {0};
    
    switch (value.type)
    {
        case JSON_ValueType_Null:    result = StrLit("null"); break;
        case JSON_ValueType_Integer: result = StrFromI64(arena, value.ivalue, 10); break;
        case JSON_ValueType_Float:   result = StrFromF64(arena, value.fvalue, 2); break;
        case JSON_ValueType_Boolean: result = value.bvalue ? StrLit("true") : StrLit("false"); break;
        
        case JSON_ValueType_String:
        {
            u64 size = value.str.size + 2;
            ForEach(i, value.str.size)
            {
                char c = value.str.str[i];
                
#if USE_LJSON_SPEC
                if (c == '\n')
                    size++;
#else
                if (c == '"' || c == '\\' || c == '\b' || c == '\f' || c == '\n' || c == '\r' || c == '\t')
                    size++;
                else if (IsControl(c))
                    size += 5;
#endif
            }
            
            String str = { PushArrayNZ(arena, u8, size) };
            str.str[str.size++] = '"';
            
            ForEach(i, value.str.size)
            {
                char c = value.str.str[i];
                
#if USE_LJSON_SPEC
                if (c == '\n')
                    str.str[str.size++] = '\\';
                str.str[str.size++] = c;
#else
                if (IsControl(c))
                {
                    str.str[str.size++] = '\\';
                    switch (c)
                    {
                        case  '"': c = '"'; break;
                        case '\\': c ='\\'; break;
                        case '\b': c = 'b'; break;
                        case '\f': c = 'f'; break;
                        case '\n': c = 'n'; break;
                        case '\r': c = 'r'; break;
                        case '\t': c = 't'; break;
                        
                        default:
                        {
                            str.str[str.size++] = 'u';
                            str.str[str.size++] = '0';
                            str.str[str.size++] = '0';
                            const u8 symbols[] = {
                                '0', '1', '2', '3', '4', '5', '6', '7',
                                '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
                            };
                            str.str[str.size++] = (c/ 1) % symbols;
                            str.str[str.size++] = (c/16) % symbols;
                        } break;
                    }
                }
#endif
            }
            
            str.str[str.size++] = '"';
            Assert(str.size == size);
            result = str;
        } break;
        
        case JSON_ValueType_Array:
        {
            ScratchBlock(scratch, arena)
            {
                StringList list = {0};
                ForEach(idx, value.array.count)
                {
                    String str = JSON_StrFromValue(scratch, value.array.values[idx], indent + 2);
                    StrListPush(scratch, &list, str);
                }
                result = StrJoin(arena, &list, .pre = StrLit("[\n"), .mid = StrLit(",\n"), .post = StrLit("\n]"));
            }
        } break;
        
        case JSON_ValueType_Object:
        {
            ScratchBlock(scratch, arena)
            {
                StringList list = {0};
                StrListPushf(scratch, &list, "%*s{\n", indent, "");
                ForEach(objIdx, value.obj.count)
                {
                    ForList(JSON_MapNode, node, value.obj.slots[objIdx].first)
                    {
                        String str = JSON_StrFromValue(scratch, node->value, indent + 2);
                        StrListPushf(scratch, &list, "%*s\"%.*s\": %.*s%s", indent + 2, "",
                                     StrExpand(node->key), StrExpand(str), node->next ? ",\n" : "\n");
                    }
                }
                StrListPushf(scratch, &list, "%*s}", indent, "");
                result = StrJoin(arena, &list);
            }
        } break;
    }
    
    return result;
}
