
global String MetaTokenType_names[] =
{
#define ENUM_NAME(n) StrConst(Stringify(n)),
    TOKEN_TYPE(ENUM_NAME)
#define NAME_KEYWORD(n) StrConst(Stringify(Keyword_##n)),
    C_KEYWORD(NAME_KEYWORD)
        
#undef ENUM_NAME
#undef NAME_KEYWORD
};

global String MetaInfoKind_names[] =
{
#define ENUM_NAME(e, n) StrConst(Stringify(n)),
    META_INFO_KIND(ENUM_NAME)
#undef ENUM_NAME
};

global String MetaInfoFlag_names[] =
{
#define ENUM_NAME(e, v, n) StrConst(n),
    META_INFO_FLAG(ENUM_NAME)
#undef ENUM_NAME
};

function void Refill(Lexer* lexer, String str)
{
    if(lexer->input.size == 0)
    {
        lexer->at[0] = 0;
        lexer->at[1] = 0;
    }
    else if(lexer->input.size == 1)
    {
        lexer->at[0] = lexer->input.str[0];
        lexer->at[1] = 0;
    }
    else
    {
        lexer->at[0] = lexer->input.str[0];
        lexer->at[1] = lexer->input.str[1];
    }
    
    String advanceStr = SubstrSplit(str, lexer->input).pre;
    for (u64 i = 0; i < advanceStr.size; ++i)
    {
        lexer->colNumber++;
        if (advanceStr.str[i] == '\n')
        {
            lexer->lineNumber++;
            lexer->colNumber = 1;
        }
    }
}

function void AdvanceChars(Lexer* lexer, u32 advance)
{
    String str = lexer->input;
    lexer->input = StrSkip(lexer->input, advance);
    Refill(lexer, str);
}

enum
{
    StringMatchFlags_NoCase = 1 << 0,
    StringMatchFlags_NotEqual = 1 << 1,
    StringMatchFlags_Inclusive = 1 << 2,
};

function String SkipStrUntil(String str, String characters, StringMatchFlags flags)
{
    b32 result = false;
    b32 noCase = flags & StringMatchFlags_NoCase;
    b32 equal = !!!(flags & StringMatchFlags_NotEqual);
    b32 inclusive = flags & StringMatchFlags_Inclusive;
    
    u64 size;
    for (size = 0; size < str.size; ++size)
    {
        if (ChrCompareArr(str.str[size], characters, noCase) == equal)
        {
            result = 1;
            break;
        }
    }
    
    if (!result)
    {
        size = 0;
        inclusive = true;
    }
    
    return StrSkip(str, size + (inclusive ? 0 : 1));
}

function void AdvanceUntil(Lexer* lexer, String characters, StringMatchFlags flags)
{
    String str = lexer->input;
    lexer->input = SkipStrUntil(lexer->input, characters, flags);
    Refill(lexer, str);
}

function MetaTokenType ParseKeyword(Lexer* lexer)
{
#define DEF_KEYWORD(n) \
    do \
    { \
        String str = StrSkip(StrLit(Stringify(n)), 1); \
        if (StrCompare(StrPrefix(lexer->input, str.size), str, 0)) return MetaTokenType_Keyword_##n; \
    } while (0);
    C_KEYWORD(DEF_KEYWORD)
#undef DEF_KEYWORD
    return MetaTokenType_Identifier;
}

function Token LastToken(Lexer* lexer)
{
    return lexer->lastToken;
}

function Token PeekToken(Lexer* lexer)
{
    Lexer temp = *lexer;
    Token token = GetToken(lexer);
    *lexer = temp;
    return token;
}

function Token GetToken(Lexer* lexer)
{
    AdvanceUntil(lexer, StrLit(" \t\n\r\f\v"), StringMatchFlags_NotEqual|StringMatchFlags_Inclusive);
    
    Token result = {
        .type = MetaTokenType_Unknow,
        .lineNumber = lexer->lineNumber,
        .colNumber = lexer->colNumber,
        .fileName = lexer->fileName,
        .str = lexer->input
    };
    
    char c = lexer->at[0];
    AdvanceChars(lexer, 1);
    switch (c)
    {
        case  0 : { result.type = MetaTokenType_EndOfStream;  } break;
        case '(': { result.type = MetaTokenType_OpenParen;    } break;
        case ')': { result.type = MetaTokenType_CloseParen;   } break;
        case ':': { result.type = MetaTokenType_Colon;        } break; // goto label, switch case, bitfield, or ? operator
        case ';': { result.type = MetaTokenType_Semicolon;    } break;
        case '*': { result.type = MetaTokenType_Asterisk;     } break; // pointer or operator
        case '[': { result.type = MetaTokenType_OpenBracket;  } break;
        case ']': { result.type = MetaTokenType_CloseBracket; } break;
        case '{': { result.type = MetaTokenType_OpenBrace;    } break;
        case '}': { result.type = MetaTokenType_CloseBrace;   } break;
        case '=': { result.type = MetaTokenType_Equals;       } break;
        case ',': { result.type = MetaTokenType_Comma;        } break;
        case '#': { result.type = MetaTokenType_Pound;        } break;
        case '&': { result.type = MetaTokenType_Ampersand;    } break;
        case '!': { result.type = MetaTokenType_Exclamation;  } break;
        case '|': { result.type = MetaTokenType_Or;           } break;
        case '~': { result.type = MetaTokenType_Tilde;        } break;
        case '+': { result.type = MetaTokenType_Plus;         } break;
        case '%': { result.type = MetaTokenType_Mod;          } break;
        case '?': { result.type = MetaTokenType_Question;     } break;
        case '<': { result.type = MetaTokenType_LeftArrow;    } break;
        case '>': { result.type = MetaTokenType_RightArrow;   } break;
        
        case '-':
        {
            result.type = MetaTokenType_Minus;
            if (lexer->at[0] == '>')
            {
                result.type = MetaTokenType_Arrow;
                AdvanceChars(lexer, 1);
            }
        } break;
        
        case '.':
        {
            result.type = MetaTokenType_Dot;
            if (IsDigit(lexer->at[0]))
                goto FLOATING_CONSTANT;
        } break;
        
        case '\'':
        {
            result.type = MetaTokenType_Constant_Char;
            
            while(lexer->at[0] && lexer->at[0] != '\'')
            {
                if((lexer->at[0] == '\\') && lexer->at[1])
                    AdvanceChars(lexer, 1);
                AdvanceChars(lexer, 1);
            }
            
            if(lexer->at[0] == '\'')
                AdvanceChars(lexer, 1);
        } break;
        
        case '"':
        {
            result.type = MetaTokenType_Constant_String;
            
            while(lexer->at[0] && lexer->at[0] != '"')
            {
                if((lexer->at[0] == '\\') && lexer->at[1])
                    AdvanceChars(lexer, 1);
                AdvanceChars(lexer, 1);
            }
            
            if(lexer->at[0] == '"')
                AdvanceChars(lexer, 1);
        } break;
        
        case '/':
        {
            if (lexer->at[0] == '/')
            {
                result.type = MetaTokenType_Comment;
                AdvanceUntil(lexer, StrLit("\n"), StringMatchFlags_Inclusive);
            }
            else if (lexer->at[0] == '*')
            {
                result.type = MetaTokenType_Comment;
                AdvanceChars(lexer, 1);
                while (!(lexer->at[0] == '*' && lexer->at[1] == '/') && lexer->at[0])
                    AdvanceChars(lexer, 1);
                if (lexer->at[0] == '*' && lexer->at[1] == '/')
                    AdvanceChars(lexer, 2);
            }
            else
            {
                result.type = MetaTokenType_Divide;
            }
        } break;
        
        case '//': { Error(lexer, "The input file contains a standalone backslash!"); } break;
        
        default:
        {
            if (IsDigit(c))
            {
                result.type = MetaTokenType_Constant_Integer;
                if (c == '0')
                {
                    if (ChrCompareNoCase(lexer->at[0], 'x'))
                    {
                        // NOTE: hexadecimal constant
                        AdvanceChars(lexer, 1);
                        AdvanceUntil(lexer, StrLit(Stringify(HexadecimalDigits)), StringMatchFlags_NotEqual|StringMatchFlags_Inclusive);
                    }
#if COMPILER_CL
                    else if (ChrCompareNoCase(lexer->at[0], 'b'))
                    {
                        // NOTE: MSVC's binary extension
                        AdvanceChars(lexer, 1);
                        AdvanceUntil(lexer, StrLit(Stringify(Binary)), StringMatchFlags_NotEqual|StringMatchFlags_Inclusive);
                    }
#endif
                    else
                    {
                        // NOTE: octal constant
                        AdvanceUntil(lexer, StrLit(Stringify(OctalDigits)), StringMatchFlags_NotEqual|StringMatchFlags_Inclusive);
                    }
                    goto INTEGER_SUFFIX;
                }
                
                else
                {
                    // NOTE: decimal constant
                    AdvanceUntil(lexer, StrLit(Stringify(Digits)), StringMatchFlags_NotEqual|StringMatchFlags_Inclusive);
                    
                    b32 isFloat = false;
                    if (ChrCompare(lexer->at[0], '.', 0))
                    {
                        FLOATING_CONSTANT:
                        isFloat = true;
                        AdvanceChars(lexer, 1);
                        AdvanceUntil(lexer, StrLit(Stringify(Digits)), StringMatchFlags_NotEqual|StringMatchFlags_Inclusive);
                    }
                    
                    if (ChrCompareNoCase(lexer->at[0], 'e'))
                    {
                        isFloat = true;
                        if (ChrCompareArr(lexer->at[0], StrLit("+-"), 0))
                            AdvanceChars(lexer, 1);
                    }
                    
                    if (isFloat)
                    {
                        result.type = MetaTokenType_Constant_Float;
                        AdvanceUntil(lexer, StrLit(Stringify(Concat(Digits, fl))), StringMatchFlags_NoCase|StringMatchFlags_NotEqual|StringMatchFlags_Inclusive);
                    }
                    else
                    {
                        INTEGER_SUFFIX:
                        AdvanceUntil(lexer, StrLit("uli64"), StringMatchFlags_NoCase|StringMatchFlags_NotEqual|StringMatchFlags_Inclusive);
                    }
                }
                
            }
            else if (IsCharacter(c) || c == '_')
            {
                // TODO: Handle keyword
                result.type = ParseKeyword(lexer);
                AdvanceUntil(lexer, StrLit(Digits Characters "_"), StringMatchFlags_NotEqual|StringMatchFlags_Inclusive);
            }
            else
            {
                result.type = MetaTokenType_Unknow;
            }
        } break;
    }
    
    result.str.size = lexer->input.str - result.str.str;
    lexer->lastToken = result;
    return result;
}

function Token RequireToken(Lexer* lexer, MetaTokenType desiredType)
{
    Token token = GetToken(lexer);
    if (token.type != desiredType)
        ErrorToken(lexer, token, "Unexpected token type, required type is %s", GetEnumCStr(MetaTokenType, desiredType));
    return token;
}

function Token RequireIdentifier(Lexer* lexer, String match)
{
    Token token = RequireToken(lexer, MetaTokenType_Identifier);
    if (!StrCompare(token.str, match, 0))
        ErrorToken(lexer, token, "Unexpected Identifier");
    return token;
}

function b32 OptionalToken(Lexer* lexer, MetaTokenType desiredType)
{
    Token token = PeekToken(lexer);
    b32 result = token.type == desiredType;
    if (result)
        GetToken(lexer);
    return result;
}

function Lexer Lexing(String input, String fileName, Arena* arena)
{
    Lexer result = {
        .arena = arena,
        .fileName = fileName,
        .colNumber = 1,
        .lineNumber = 1,
        .input = input,
        .errorList = PushStruct(arena, StringList),
    };
    Refill(&result, result.input);
    
    return result;
}

function Parser Parsing(Lexer* lexer, Arena* arena)
{
    Parser result = {
        .lexer = lexer,
        .table = PushStruct(arena, TypeTable),
    };
    result.table->arena = arena;
    return result;
}

function void ErrorArgList(Lexer* lexer, Token errorToken, char* format, va_list argList)
{
    String tokenError = StrPushf(lexer->arena, "%.*s (%u, %u) on \"%.*s\": Error:",
                                 StrExpand(errorToken.fileName), errorToken.lineNumber, errorToken.colNumber, StrExpand(errorToken.str));
    String formatError = StrPushfv(lexer->arena, format, argList);
    String error = StrPushf(lexer->arena, "%.*s %.*s\n", StrExpand(tokenError), StrExpand(formatError));
    
    StrListPush(lexer->arena, lexer->errorList, error);
    
    lexer->error = true;
}

function void ErrorToken(Lexer* lexer, Token errorToken, char* message, ...)
{
    va_list argList;
    va_start(argList, message);
    
    ErrorArgList(lexer, errorToken, message, argList);
    
    va_end(argList);
}

function void Error(Lexer* lexer, char* message, ...)
{
    va_list argList;
    va_start(argList, message);
    
    Token errorToken = {
        .lineNumber = lexer->lineNumber,
        .colNumber = lexer->colNumber,
        .fileName = lexer->fileName,
        .str = LastToken(lexer).str
    };
    ErrorArgList(lexer, errorToken, message, argList);
    
    va_end(argList);
}

function b32 IsType(MetaTokenType type)
{
    return ((type == MetaTokenType_Keyword_union) ||
            //(type == MetaTokenType_Keyword_enum) ||
            (type == MetaTokenType_Keyword_struct)); 
}

function b32 IsRef(MetaInfoKind kind)
{
    return ((kind == MetaInfoKind_Pointer) ||
            (kind == MetaInfoKind_Alias  ) ||
            (kind == MetaInfoKind_Member ));
}

function MetaInfoKind GetTypeKind(MetaTokenType type)
{
    if (type == MetaTokenType_Keyword_struct)
        return MetaInfoKind_Struct;
    if (type == MetaTokenType_Keyword_enum)
        return MetaInfoKind_Enum;
    if (type == MetaTokenType_Keyword_union)
        return MetaInfoKind_Union;
    return 0;
}

function MetaInfo* PushType(TypeTable* table, MetaInfo* info)
{
    MetaInfo* type = PushStruct(table->arena, MetaInfo);
    *type = *info;
    SLLQueuePush(table->first, table->last, type);
    type->indexWithinTypeTable = table->typeCount++;
    return type;
}

function MetaInfo* PushMember(TypeTable* table, MetaInfo* type, MetaInfo* info)
{
    MetaInfo* member = PushStruct(table->arena, MetaInfo);
    *member = *info;
    if (!type)
        type = table->last;
    if (type)
    {
        member->indexWithinTypeTable = (u32)type->count++;
        SLLQueuePush(type->first, type->last, member);
    }
    return member;
}

function MetaInfo* AddForwardType(TypeTable* table, MetaInfoKind kind, String name)
{
    MetaInfo* type = PushType(table, &(MetaInfo){ .name = name, .kind = kind });
    return type;
}

function MetaInfo* BeginTypeInfo(TypeTable* table, MetaInfoKind kind, String name)
{
    MetaInfo* typeInfo = FindMetaType(table, name);
    if (!typeInfo)
        typeInfo = PushType(table, &(MetaInfo){0});
    
    typeInfo->kind = kind;
    typeInfo->name = name,
    typeInfo->indexWithinTypeTable = typeInfo->indexWithinTypeTable;
    
    return typeInfo;
}

function void EndTypeInfo(MetaInfo* info)
{
    u32 maxSize = 0;
    for (MetaInfo* member = info->first; member; member = member->next)
    {
        if (member->flags & ~MetaInfoFlag_DoneCompiling)
            return;
        u32 runtimeSize = (member->count > 0) ? POINTER_SIZE : member->first->runtimeSize;
        //Assert(member->first == member->last);
        if (maxSize < runtimeSize)
            maxSize = runtimeSize;
    }
    
    u32 cursor = 0;
    for (MetaInfo* member = info->first; member; member = member->next)
    {
        member->runtimeSize = cursor;
        u32 runtimeSize = (member->count > 0) ? POINTER_SIZE : member->first->runtimeSize;
        u32 remainingSize = maxSize - (cursor % maxSize);
        if (runtimeSize > remainingSize)
            cursor += remainingSize;
        cursor += runtimeSize;
    }
    
    info->runtimeSize = cursor;
    info->flags |= MetaInfoFlag_DoneCompiling;
}

function MetaInfo* FindMetaType(TypeTable* table, String name)
{
    if (name.size != 0)
        for (MetaInfo* info = table->first; info; info = info->next)
            if (StrCompare(info->name, name, 0))
                return info;
    return 0;
}

function void AddMetaInfo(TypeTable* table, MetaInfo* currentType, MetaInfo* info)
{
    if (info->kind == MetaInfoKind_Alias && StrCompare(info->name, info->first->name, 0))
        return;
    
    if (info->kind == MetaInfoKind_Member)
        PushMember(table, currentType, info);
    else
        PushType(table, info);
}

// EX:
// nameList = *(>= 0) name [n](>= 0, n >= 0 and is compile time value), ...
// nameInitList = *(>= 0) name (= {value})(opt), ...
// typeDecl = [struct/union/enum typeName(opt) { typeDecl }]/[typeName] nameList ;
// typeInit = [struct/union/enum typeName(opt) { typeDecl }]/[typeName] nameInitList ;
// auto/register/static/extern(opt) typeInit
// typedef typeDecl

function void ParseDeclaration(Parser* parser, DeclarationType declType, MetaInfo* currentType)
{
    Token token = LastToken(parser->lexer);
    MetaInfo* typeInfo = 0;
    
    switch (declType)
    {
        case Declaration_Member:
        case Declaration_Typedef: token = GetToken(parser->lexer);
        case Declaration_Type:
        {
            String typeName = {0};
            MetaInfoKind kind = GetTypeKind(token.type);
            if (kind)
            {
                if (OptionalToken(parser->lexer, MetaTokenType_Identifier))
                    typeName = LastToken(parser->lexer).str;
                
                if (OptionalToken(parser->lexer, MetaTokenType_OpenBrace))
                    typeInfo = ParseType(parser, kind, typeName);
                else if (declType == Declaration_Member)
                    typeInfo = FindMetaType(parser->table, typeName);
                else
                    typeInfo = AddForwardType(parser->table, kind, typeName);
            }
            else if (token.type == MetaTokenType_Identifier)
            {
                typeName = token.str;
                typeInfo = FindMetaType(parser->table, typeName);
            }
        } break;
    }
    
    if (typeInfo)
    {
        MetaInfo type = *typeInfo;
        if (declType == Declaration_Member)
            type.kind = MetaInfoKind_Member;
        else if (declType == Declaration_Typedef)
            type.kind = MetaInfoKind_Alias;
        type.first = type.last = typeInfo;
        type.count = 0;
        type.name = Str(0);
        
        do
        {
            while (OptionalToken(parser->lexer, MetaTokenType_Asterisk))
                type.count++;
            
            if (type.count > 0)
            {
                type.flags = MetaInfoFlag_DoneCompiling;
                if (declType != Declaration_Member) // member's runtimSize is the offset from base
                    type.runtimeSize = POINTER_SIZE;
            }
            
            if (declType == Declaration_Typedef)
            {
                type.name = RequireToken(parser->lexer, MetaTokenType_Identifier).str;
            }
            else
            {
                if (OptionalToken(parser->lexer, MetaTokenType_Identifier))
                {
                    type.name = LastToken(parser->lexer).str;
                    
                    if (OptionalToken(parser->lexer, MetaTokenType_Equals))
                    {
                        // TODO: Parse expression
                        Error(parser->lexer, "Doesn't support expression parsing yet!");
                    }
                }
                else if (!(typeInfo->name.size == 0 && declType == Declaration_Member)) // not anonymous member
                    continue;
            }
            
            if (!parser->lexer->error)
                AddMetaInfo(parser->table, currentType, &type);
        } while (OptionalToken(parser->lexer, MetaTokenType_Comma) && !parser->lexer->error);
        
        RequireToken(parser->lexer, MetaTokenType_Semicolon);
    }
    else
    {
        ErrorToken(parser->lexer, token, "Undefined type: \"%.*s\"", StrExpand(token.str));
    }
}

function MetaInfo* ParseType(Parser* parser, MetaInfoKind kind, String typeName)
{
    MetaInfo* typeInfo = BeginTypeInfo(parser->table, kind, typeName);
    if (!typeInfo)
        Error(parser->lexer, "Can't add new struct type");
    while (!parser->lexer->error)
    {
        ParseDeclaration(parser, Declaration_Member, typeInfo);
        if (OptionalToken(parser->lexer, MetaTokenType_CloseBrace))
            break; // TODO: Handle declaring global variables
    }
    
    EndTypeInfo(typeInfo);
    return typeInfo;
}
