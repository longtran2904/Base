/* date = August 13th 2022 5:27 pm */

#ifndef _META_PARSER_H
#define _META_PARSER_H

#define TOKEN_TYPE(X) \
X(Unknow) \
\
X(OpenParen) \
X(CloseParen) \
X(Colon) \
X(Semicolon) \
X(Asterisk) \
X(OpenBracket) \
X(CloseBracket) \
X(OpenBrace) \
X(CloseBrace) \
X(Equals) \
X(Comma) \
X(Pound) \
\
X(Ampersand) \
X(Exclamation) \
X(Or) \
X(Tilde) \
\
X(Plus) \
X(Minus) \
X(Divide) \
X(Mod) \
\
X(Question) \
X(RightArrow) \
X(LeftArrow) \
X(Arrow) \
X(Dot) \
\
X(Identifier) \
\
X(Constant_Integer) \
X(Constant_Float) \
X(Constant_Enumeration) \
X(Constant_Char) \
X(Constant_String) \
\
X(Comment) \
X(EndOfStream)

#define C_KEYWORD(X) \
X(break) \
X(case) \
X(continue) \
X(else) \
X(for) \
X(goto) \
X(if) \
X(struct) \
X(enum) \
X(union) \
X(switch) \
X(typedef) \
X(while) \

typedef enum MetaTokenType
{
#define DEF_ENUM(n) MetaTokenType_##n,
    TOKEN_TYPE(DEF_ENUM)
#define DEF_KEYWORD(n) MetaTokenType_Keyword_##n,
    C_KEYWORD(DEF_KEYWORD)
#undef DEF_KEYWORD
#undef DEF_ENUM
    MetaTokenType_Count
} MetaTokenType;

typedef enum MetaInfoKind MetaInfoKind;
enum MetaInfoKind
{
#define META_INFO_KIND(X) \
X(MetaInfoKind_None, "None") \
X(MetaInfoKind_Int, "Int") \
X(MetaInfoKind_UInt, "UInt") \
X(MetaInfoKind_Float, "Float") \
X(MetaInfoKind_Bool, "Bool") \
X(MetaInfoKind_String, "String") \
X(MetaInfoKind_Void, "Void") \
    \
X(MetaInfoKind_Alias, "Alias") \
X(MetaInfoKind_Pointer, "Pointer") \
    \
X(MetaInfoKind_Struct, "Struct") \
X(MetaInfoKind_Union, "Union") \
X(MetaInfoKind_Enum, "Enum") \
X(MetaInfoKind_Procedure, "Procedure") \
    \
X(MetaInfoKind_Member, "Member") \
X(MetaInfoKind_Argument, "Argument") \
X(MetaInfoKind_Return, "Return")
    //MetaInfoKind_Array,
    //MetaInfoKind_Code,
    //MetaInfoKind_Type, // First class type
    
#define ENUM_DEF(n, ...) n,
    META_INFO_KIND(ENUM_DEF)
        
        MetaInfoKind_Count
#undef ENUM_DEF
};

typedef struct Token
{
    MetaTokenType type;
    u32 lineNumber, colNumber;
    String fileName, str;
    union
    {
        f64 floatValue;
        u64 intValue;
        String stringValue; // For string, char and macro, comments
    };
} Token;

typedef struct Lexer
{
    MemArena* arena;
    String fileName, input;
    u32 lineNumber, colNumber;
    b32 error;
    char at[2];
    StringList* errorList;
    Token lastToken;
} Lexer;

typedef u32 MetaInfoFlags;
typedef enum MetaInfoFlag
{
#define META_INFO_FLAG(X) \
X(MetaInfoFlag_Polymorphic, (1 << 0), "Polymorphic") \
X(MetaInfoFlag_CompileTimeValue, (1 << 1), "Comptime Value") \
X(MetaInfoFlag_CompileTimeType, (1 << 2), "Comptime Type") \
X(MetaInfoFlag_DoneCompiling, (1 << 3), "Done Compiling") \
    
#define ENUM_DEF(n, v, c) n = v,
    META_INFO_FLAG(ENUM_DEF)
#undef ENUM_DEF
} MetaInfoFlag;

typedef enum DeclarationType
{
    Declaration_Typedef,
    Declaration_Type,
    Declaration_Member,
    //Declaration_Enum,
    
    Declaration_Count
} DeclarationType;

typedef struct MetaInfo MetaInfo;
struct MetaInfo
{
    MetaInfo* next;
    MetaInfoKind kind;
    MetaInfoFlags flags;
    String name;
    u32 indexWithinTypeTable;
    i32 runtimeSize; // is offset if kind is member, argument, or return. -1 when unknow
    MetaInfo* first; // is array if kind is struct, enum, or procedure
    MetaInfo* last;
    u64 count; // is pointer level if kind is pointer or alias
};

#define MAX_TYPE_COUNT 1024
#define MAX_MEMBER_COUNT MAX_TYPE_COUNT * 10
typedef struct TypeTable
{
    MemArena* arena;
    MetaInfo* first;
    MetaInfo* last;
    u32 typeCount;
} TypeTable;

typedef struct Parser
{
    Lexer* lexer;
    TypeTable* table;
} Parser;

#define POINTER_SIZE 8

function Lexer Lexing(String input, String fileName, MemArena* arena);
function Parser Parsing(Lexer* lexer, MemArena* arena);
function void ErrorToken(Lexer* lexer, Token errorToken, char* message, ...);
function void Error(Lexer* lexer, char* message, ...);

function Token GetToken(Lexer* lexer);
function Token PeekToken(Lexer* lexer);
function Token LastToken(Lexer* lexer);
function Token RequireToken(Lexer* lexer, MetaTokenType desiredType);
function Token RequireIdentifier(Lexer* lexer, String match);
function b32   OptionalToken(Lexer* lexer, MetaTokenType desiredType);

function MetaInfo* FindMetaType(TypeTable* table, String name);
function MetaInfo* AddForwardType(TypeTable* table, MetaInfoKind kind, String name);
function void      AddMetaInfo(TypeTable* table, MetaInfo* currentType, MetaInfo* info);
function MetaInfo* BeginTypeInfo(TypeTable* table, MetaInfoKind kind, String name);
function void      EndTypeInfo(TypeTable* table, MetaInfo* info);

function MetaInfo* PushType(TypeTable* table, MetaInfo* info);
function MetaInfo* PushMember(TypeTable* table, MetaInfo* type, MetaInfo* info);

function b32 IsType(MetaTokenType type);
function b32 IsRef(MetaInfoKind kind);
function MetaInfoKind GetTypeKind(MetaTokenType type);

function MetaInfo* ParseType(Parser* parser, MetaInfoKind type, String name);
function void      ParseDeclaration(Parser* parser, DeclarationType declType, MetaInfo* currentType);

#endif //_META_PARSER_H
