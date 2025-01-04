
//~ long: Scanner Functions

//- long: Construction Functions
function Marker* ScannerPushMark(Scanner* scanner, i64 user, String str, MarkerFlags flags)
{
    MarkerNode* node = PushStruct(scanner->arena, MarkerNode);
    node->user = user;
    node->marker = (Marker){ .flags = flags, .str = str };
    SLLQueuePush(scanner->first, scanner->last, node);
    scanner->count++;
    return &node->marker;
}

function Marker* MarkerPushArr(Scanner* scanner, i64 user, String array, b32 matchOnce)
{
    Marker* result = ScannerPushMark(scanner, user, array, MarkerFlag_MatchArray);
    if (!matchOnce)
        result->flags |= MarkerFlag_MatchAll;
    return result;
}

function Marker* MarkerPushLine(Scanner* scanner, i64 user, String start, u8 escape, b32 matchRange)
{
    Marker* result = ScannerPushMark(scanner, user, start, MarkerFlag_MatchLine);
    if (matchRange)
        result->flags |= MarkerFlag_MatchRange;
    
    result->escapes[0] = escape;
    return result;
}

function Marker* MarkerPushRange(Scanner* scanner, i64 user, String str, MarkerFlags flags)
{
    Marker* result = ScannerPushMark(scanner, user, str, flags|MarkerFlag_MatchRange);
    return result;
}

function Marker* MarkerPushIdent(Scanner* scanner, i64 user)
{
    Marker* result = ScannerPushMark(scanner, user, StrLit(""), MarkerFlag_MatchIdentifier);
    return result;
}

function Marker* MarkerPushNumber(Scanner* scanner, i64 user, String exponents)
{
    Marker* result = ScannerPushMark(scanner, user, exponents, MarkerFlag_MatchNumeric);
    return result;
}

function Marker* MarkerPushOpsCombine(Scanner* scanner, i64 user, String symbols, u8 postfixSymbol, b32 matchTwice)
{
    Marker* result = ScannerPushMark(scanner, user, symbols, MarkerFlag_MatchArray|MarkerFlag_PostfixStr);
    result->escapes[0] = postfixSymbol;
    if (matchTwice)
        result->flags |= MarkerFlag_MatchTwice;
    return result;
}

function Marker* MarkerPushOps(Scanner* scanner, i64 user, String symbols, b32 matchTwice)
{
    Marker* result = ScannerPushMark(scanner, user, symbols, MarkerFlag_MatchArray);
    if (matchTwice)
        result->flags |= MarkerFlag_MatchTwice;
    return result;
}

//- long: Lexing Functions

function b32 ScannerAdvance(Scanner* scanner, i64 advance)
{
    u64 pos = scanner->pos + advance;
    b32 result = pos <= scanner->source.size;
    if (result)
        scanner->pos = pos;
    return result;
}

function b32 ScannerCompare(Scanner* scanner, String val, ScannerMatchFlags flags)
{
    b32 result = 0;
    if (/*ALWAYS*/(scanner->pos < scanner->source.size))
    {
        if (flags & ScannerMatchFlag_IsArray)
        {
            char c = scanner->source.str[scanner->pos];
            result = ChrCompareArr(c, val, 0);
            if (flags & ScannerMatchFlag_Characters)
                result |= IsCharacter(c);
            if (flags & ScannerMatchFlag_Numbers)
                result |= IsDigit(c);
        }
        
        else
        {
            char c = ScannerPeekByte(scanner, val.size);
            result = StrCompare(SubstrRange(scanner->source, scanner->pos, val.size), val, 0);
            if (flags & ScannerMatchFlag_Characters)
                result &= IsCharacter(c);
            if (flags & ScannerMatchFlag_Numbers)
                result &= IsDigit(c);
        }
    }
    
    if (flags & ScannerMatchFlag_Negate)
        result = !result;
    
    return result;
}

function b32 ScannerParse(Scanner* scanner, String val, ScannerMatchFlags flags)
{
    b32 result = ScannerCompare(scanner, val, flags);
    if (result)
        ScannerAdvance(scanner, (flags & ScannerMatchFlag_IsArray) ? 1 : val.size);
    return result;
}

function b32 ScannerAdvanceUntil(Scanner* scanner, String val, ScannerMatchFlags flags)
{
    b32 result = 0;
    u8 escapeChar = 0;
    if (flags & ScannerMatchFlag_LastIsEscape)
    {
        if (val.size)
        {
            escapeChar = val.str[val.size - 1];
            val = StrChop(val, 1);
        }
    }
    
    b32 (*check)(Scanner*, String, ScannerMatchFlags) = ScannerParse;
    if (flags & ScannerMatchFlag_Negate)
        check = ScannerCompare;
    
    for (b32 escaped = 0; scanner->pos < scanner->source.size; )
    {
        if ((flags & ScannerMatchFlag_LineExit) && ScannerCompare(scanner, StrLit(NlineStr), ScannerMatchFlag_IsArray))
        {
            result = 0;
            break;
        }
        
        if (escaped) escaped = 0;
        else if (scanner->source.str[scanner->pos] == escapeChar) escaped = 1;
        else if (check(scanner, val, flags))
        {
            result = 1;
            break;
        }
        
        ScannerAdvance(scanner, 1);
    }
    
    return result;
}

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

internal b32 MarkerMatchBeg(Scanner* scanner, Marker* marker, u64* outSize)
{
    b32 result = 0;
    u8 byte = ScannerPeekByte(scanner, 0);
    MarkerFlags flags = marker->flags;
    String str = marker->str;
    
    ScratchBegin(scratch);
    u64 size = 1;
    
    if (HasAnyFlags(flags, MarkerFlag_MatchRange|MarkerFlag_MatchLine) || flags == 0)
    {
        result |= ScannerCompare(scanner, str, 0);
        if (result)
        {
            size = str.size;
            if ((flags & MarkerFlag_MatchTwice) && ScannerCompare(scanner, StrRepeat(scratch, str, 2), 0))
                size = str.size * 2;
        }
    }
    
    if (flags & MarkerFlag_MatchArray)
    {
        result |= ScannerCompare(scanner, str, ScannerMatchFlag_IsArray);
        if (result && (flags & MarkerFlag_MatchTwice))
        {
            String twice = ChrRepeat(scratch, ScannerPeekByte(scanner, 0), 2);
            if (ScannerCompare(scanner, twice, 0))
                size = 2;
        }
    }
    
    if (flags & MarkerFlag_MatchAlpha)
        result |= IsCharacter(byte);
    if (flags & MarkerFlag_MatchDigit)
        result |= IsDigit(byte);
    if (HasAllFlags(flags, MarkerFlag_MatchIdentifier))
        result |= byte == '_' /*|| byte > MAX_I8*/;
    if (HasAllFlags(flags, MarkerFlag_MatchNumeric))
    {
        u8 next = ScannerPeekByte(scanner, 1);
        result |= (byte == '.' && IsDigit(next));
    }
    
    ScratchEnd(scratch);
    if (result && outSize)
        *outSize = size;
    return result;
}

internal b32 MarkerMatchEnd(Scanner* scanner, Marker* marker, u64* outSize)
{
    b32 result = 0;
    u8 byte = ScannerPeekByte(scanner, 0);
    MarkerFlags flags = marker->flags;
    String str = marker->str;
    
    ScratchBegin(scratch);
    String inverseStr = str;
    if (flags & MarkerFlag_InverseStr)
        inverseStr = ScannerInverseStr(scratch, str);
    
    b32 matchStr = flags & MarkerFlag_MatchRange;
    if (matchStr)
        result |= ScannerCompare(scanner, inverseStr, 0);
    if (flags & MarkerFlag_MatchArray)
        result |= ScannerCompare(scanner, inverseStr, ScannerMatchFlag_UntilNot);
    
    if (!result && HasAnyFlags(flags, MarkerFlag_MatchAlpha|MarkerFlag_MatchDigit))
    {
        if (flags & MarkerFlag_MatchAlpha)
            result |= IsCharacter(byte);
        if (flags & MarkerFlag_MatchDigit)
            result |= IsDigit(byte);
        if (HasAllFlags(flags, MarkerFlag_MatchIdentifier))
            result |= IsDigit(byte) || byte == '_' /*|| byte > MAX_I8*/;
        if (HasAllFlags(flags, MarkerFlag_MatchNumeric))
        {
            result |= IsCharacter(byte) || byte == '.' || byte == '_';
            if (byte == '+' || byte == '-')
                result |= ChrCompareArr(ScannerPeekByte(scanner, -1), marker->str, 0);
        }
        result = !result;
    }
    
    ScratchEnd(scratch);
    if (result && outSize)
        *outSize = matchStr ? str.size : 0;
    return result;
}

function Token ScannerNext(Scanner* scanner)
{
    Token result = {0};
    
    if (scanner->pos < scanner->source.size)
    {
        Marker* marker = 0;
        i64 user = 0;
        u64 start = scanner->pos;
        ForList(MarkerNode, node, scanner->first)
        {
            if (node->marker.flags & MarkerFlag_PrefixStr)
                if (!ScannerParse(scanner, StrFromChr(node->marker.escapes[0]), 0))
                    goto CLEANUP;
            
            u64 size = 0;
            if (MarkerMatchBeg(scanner, &node->marker, &size))
            {
                ScannerAdvance(scanner, size);
                if (node->marker.flags & MarkerFlag_PostfixStr)
                    if (!ScannerParse(scanner, StrFromChr(node->marker.escapes[0]), 0))
                        goto CLEANUP;
                
                marker = &node->marker;
                user = node->user;
                break;
            }
            
            CLEANUP:
            scanner->pos = start;
        }
        
        if (marker)
        {
            b32 nested = HasAllFlags(marker->flags, MarkerFlag_MatchRange|MarkerFlag_MatchAll);
            b32 matchOnce =  NoFlags(marker->flags, MarkerFlag_MatchRange|MarkerFlag_MatchAll|MarkerFlag_MatchLine);
            b32 match = matchOnce;
            
            for (i64 escaped = 0, nestCount = 1, advance = 1;
                 !match && scanner->pos < scanner->source.size;
                 ScannerAdvance(scanner, advance))
            {
                u64 size = 1;
                
                if (ScannerCompare(scanner, StrLit(NlineStr), ScannerMatchFlag_IsArray) && (marker->flags & MarkerFlag_MatchLine))
                    if (marker->flags != MarkerFlag_MatchLine || !escaped)
                        break;
                
                if (escaped)
                    escaped = 0;
                else if (ScannerCompare(scanner, Str(marker->escapes, ArrayCount(marker->escapes)), ScannerMatchFlag_IsArray))
                    escaped = 1;
                else if (MarkerMatchEnd(scanner, marker, &size))
                    match = (--nestCount) <= 0;
                else if (nested && MarkerMatchBeg(scanner, marker, &size))
                    nestCount++;
                
                advance = size;
            }
            
            result.range = R1U64(start, scanner->pos);
            result.user = user;
            if (!match && (marker->flags & MarkerFlag_MatchRange))
                result.flags |= ScanResultFlag_TokenUnclosed;
        }
        else
        {
            result.flags |= ScanResultFlag_NoMatches;
            result.range = R1U64(scanner->pos, scanner->pos + 1);
            result.user = scanner->fallback;
            ScannerAdvance(scanner, 1);
        }
    }
    
    if (scanner->pos >= scanner->source.size)
        result.flags |= ScanResultFlag_EOF;
    
    return result;
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
        if (result.flags & ScanResultFlag_EOF)
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

function void TokenChunkListPush(Arena* arena, TokenChunkList* list, u64 cap, Token token)
{
    TokenChunkNode* node = list->last;
    if (!node || node->count >= node->cap)
    {
        node = PushStruct(arena, TokenChunkNode);
        node->cap = cap;
        node->tokens = PushArrayNZ(arena, Token, cap);
        SLLQueuePush(list->first, list->last, node);
        list->chunkCount += 1;
    }
    node->tokens[node->count] = token;
    node->count++;
    list->totalTokenCount++;
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
    Scanner* scanner = &ScannerFromStr(arena, text);
    r1u64 range = {0};
    
    while (1)
    {
        b32 eof = !ScannerAdvanceUntil(scanner, StrLit(",\n\""), ScannerMatchFlag_IsArray);
        if (!eof)
            ScannerAdvance(scanner, -1);
        range.max = scanner->pos;
        
        b32 isComma = ScannerParse(scanner, StrLit(","), 0);
        if (eof || isComma || ScannerParse(scanner, StrLit("\n"), 0))
        {
            String str = ScannerStrFromRange(scanner, range);
            if (StrStartsWith(str, StrLit("\""), 0) && StrEndsWith(str, StrLit("\""), 0))
            {
                str = Substr(str, 1, str.size - 1);
                str = StrReplace(arena, str, StrLit("\"\""), StrLit("\""), 0);
            }
            
            StrListPush(arena, list, str);
            
            if (isComma)
                range.min = scanner->pos;
            else
                break;
        }
        
        while (ScannerParse(scanner, StrLit("\""), 0))
            ScannerAdvanceUntil(scanner, StrLit("\""), 0);
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
                token.flags |= ScanResultFlag_TokenUnclosed;
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
                    token.flags |= ScanResultFlag_TokenUnclosed;
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
        else if (IsDigit(byte[0]) ||
                 (byte+1 < byte_opl && byte[0] == '-' && IsDigit(byte[1])))
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
                    token.flags |= ScanResultFlag_TokenUnclosed;
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
        else
        {
            TokenInit(JSON_TokenType_Invalid, 1);
            token.flags |= ScanResultFlag_NoMatches;
        }
        
#undef TokenInit
#undef Check2Bytes
        
        //- rjf: push token if formed
        token.range.max = byte - byte_first;
        if (token.range.max > token.range.min)
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
