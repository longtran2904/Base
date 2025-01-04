/* date = August 17th 2024 10:40 pm */

#ifndef _LONG_SCANNER_H
#define _LONG_SCANNER_H

//~ long: Scanner Types

typedef Flags32 ScanResultFlags;
enum
{
    ScanResultFlag_EOF = 1 << 0,
    ScanResultFlag_NoMatches = 1 << 1,
    
    ScanResultFlag_TokenUnclosed = 1 << 2,
    ScanResultFlag_TokenHasPrefix = 1 << 3,
    ScanResultFlag_TokenHasPostfix = 1 << 4,
};

typedef Flags16 MarkerFlags;
enum
{
    MarkerFlag_MatchLine = 1 << 0, // end before newline
    MarkerFlag_MatchAll = 1 << 1,
    MarkerFlag_MatchTwice = 1 << 2,
    
    MarkerFlag_MatchRange = 1 << 4,
    MarkerFlag_MatchArray = 1 << 5,
    
    MarkerFlag_MatchAlpha = 1 << 6, // a-z A-Z _
    MarkerFlag_MatchDigit = 1 << 7, // 0-9
    MarkerFlag_MatchAlphaNumeric = MarkerFlag_MatchAlpha|MarkerFlag_MatchDigit,
    
    // Starts with Alpha or underscore and ends when not AlphaNumeric or underscore
    MarkerFlag_MatchIdentifier = MarkerFlag_MatchAlpha|MarkerFlag_MatchAll|(1 << 8),
    // Starts with Numeric or dot and ends when not Identifier, or dot
    MarkerFlag_MatchNumeric = MarkerFlag_MatchDigit|MarkerFlag_MatchAll|(1 << 9),
    
    MarkerFlag_InverseStr = 1 << 11, // `{` -> `}`, `/*` -> `*/`, `({` -> `})`
    MarkerFlag_PrefixStr = 1 << 12,
    MarkerFlag_PostfixStr = 1 << 13,
};

typedef struct Marker Marker;
struct Marker
{
    MarkerFlags flags;
    u8 escapes[6];
    String str;
};

typedef struct MarkerNode MarkerNode;
struct MarkerNode
{
    MarkerNode* next;
    i64 user;
    Marker marker;
};

typedef struct Scanner Scanner;
struct Scanner
{
    Arena* arena;
    String source;
    u64 pos;
    
    i64 fallback;
    MarkerNode* first;
    MarkerNode* last;
    u64 count;
};

typedef Flags32 ScannerMatchFlags;
enum
{
    ScannerMatchFlag_IsArray  = 1 << 0,
    ScannerMatchFlag_Negate   = 1 << 1,
    ScannerMatchFlag_UntilNot = ScannerMatchFlag_IsArray|ScannerMatchFlag_Negate,
    
    ScannerMatchFlag_Characters = 1 << 2,
    ScannerMatchFlag_Numbers = 1 << 3,
    ScannerMatchFlag_AlphaNumerics = ScannerMatchFlag_Characters|ScannerMatchFlag_Numbers,
    
    ScannerMatchFlag_LastIsEscape  = 1 << 6,
    ScannerMatchFlag_LineExit      = 1 << 7,
    ScannerMatchFlag_LineAndEscape = ScannerMatchFlag_LastIsEscape|ScannerMatchFlag_LineExit,
};

//~ long: Token Types

typedef struct Token Token;
struct Token
{
    ScanResultFlags flags;
    i64 user;
    r1u64 range;
};

typedef struct TokenArray TokenArray;
struct TokenArray
{
    Token* tokens;
    u64 count;
};

typedef struct TokenChunkNode TokenChunkNode;
struct TokenChunkNode
{
    TokenChunkNode* next;
    union
    {
        struct { Token* tokens; u64 count; };
        TokenArray array;
    };
    u64 cap;
};

typedef struct TokenChunkList TokenChunkList;
struct TokenChunkList
{
    TokenChunkNode* first;
    TokenChunkNode* last;
    u64 chunkCount;
    u64 totalTokenCount;
};

typedef struct TokenIter TokenIter;
struct TokenIter
{
    TokenArray array;
    Token* current;
};

//~ long: Scanner Functions

//- long: Construction Functions
function Marker* ScannerPushMark(Scanner* scanner, i64 user, String str, MarkerFlags flags);

function Marker*   MarkerPushArr(Scanner* scanner, i64 user, String array, b32 matchOnce);
function Marker*  MarkerPushLine(Scanner* scanner, i64 user, String start, u8 escape, b32 matchRange);
function Marker* MarkerPushRange(Scanner* scanner, i64 user, String str, MarkerFlags flags);

function Marker*  MarkerPushIdent(Scanner* scanner, i64 user);
function Marker* MarkerPushNumber(Scanner* scanner, i64 user, String prefixes);

#define  ScannerFromStr(_arena, str) ((Scanner){ .arena = (_arena), .source = (str) })
#define ScannerFromFile(_arena, file) ScannerFromStr((_arena), OSReadFile((_arena), (file), 0))
#define ScannerPushFlags(scanner, user, flags) ScannerPushMark((scanner), (user), ZeroStr, ZeroStr, (flags));

#define ScannerStrFromRange(scanner, range) Substr((scanner)->source, (range).min, (range).max)
#define ScannerCurrStr(scanner) StrSkip((scanner)->source, (scanner)->pos)
#define ScannerPrevStr(scanner) StrPrefix((scanner)->source, (scanner)->pos)

//- long: Lexing Functions
function b32 ScannerAdvance(Scanner* scanner, i64 advance);
function b32 ScannerCompare(Scanner* scanner, String val, ScannerMatchFlags flags);
function b32   ScannerParse(Scanner* scanner, String val, ScannerMatchFlags flags);

function b32 ScannerAdvanceUntil(Scanner* scanner, String val, ScannerMatchFlags flags);

// @UB(long): if offset is negative will this wrap around?
#define ScannerPeekByte(scanner, offset) ChrFromStr((scanner)->source, (scanner)->pos + (offset))

function Token ScannerNext(Scanner* scanner);
function Token ScannerPeek(Scanner* scanner);
function Token ScannerPeekAhead(Scanner* scanner, i64 tokenCount);

//~ long: Token Functions

function String StrFromToken(String text, Token token);
function b32 TokenMatch(String text, Token token, String match);

function void TokenChunkListPush(Arena* arena, TokenChunkList* list, u64 cap, Token token);
function TokenArray TokenArrayFromChunkList(Arena* arena, TokenChunkList* chunks);

#define TokenItFromArray(array) ((TokenIter){ (array), (array).tokens })
#define TokenItIsDone(it) ((it)->current && (it)->current < (it)->array.tokens + (it)->array.count)
function Token* TokenItNext(TokenIter* it);
function Token* TokenItNextIgnore(TokenIter* it, i64 ignore);

//~ long: CSV Parser

//- long: String Table
typedef struct StringTable StringTable;
struct StringTable
{
    StringList* rows;
    u64 rowCount, cellCount, totalSize;
};

function StringTable StrTableFromStr(Arena* arena, String str, u8 seperator, u8 terminator);
function StringTable StrTablePushRow(Arena* arena, String row, u8 seperator);

//- long: CSV
function u64 CSV_StrListPushRow(Arena* arena, StringList* list, String text);
function StringTable CSV_TableFromStr(Arena* arena, String str);
#define StrListIterCSVRow(arena, list, text) for (String UNIQUE(str) = (text); UNIQUE(str).size; \
                                                  StrSkip(UNIQUE(str), CSV_StrListPushRow((arena), (list), UNIQUE(str))))

//~ long: (L)JSON Parser

// LJSON is a superset of SJSON + JSON5. It has all the same features plus extras.
// - A value may be an identifier
// - Semicolons and commas are the same
// - Commas and newlines are optional
// - Numbers can have C prefixes (0x for hex and 0b for bin)
// Examples
// - abc = [ 1 23 "abc" def ]
// - foo = { "bar" : 0x123 baz : "456"; foobar : [ abc def, ghi 124; 564 ]; }
// https://json5.org
// https://bitsquid.blogspot.com/2009/10/simplified-json-notation.html

#ifndef USE_LJSON_SPEC
#define USE_LJSON_SPEC 1
#endif

//- long: JSON Types
typedef enum JSON_TokenType JSON_TokenType;
enum JSON_TokenType
{
    JSON_TokenType_Invalid,
    JSON_TokenType_Whitespace,
    
    JSON_TokenType_Terminator,
    JSON_TokenType_Assignment,
    
    JSON_TokenType_OpenObj,
    JSON_TokenType_CloseObj,
    JSON_TokenType_OpenArr,
    JSON_TokenType_CloseArr,
    
    JSON_TokenType_String,
    JSON_TokenType_Number,
    JSON_TokenType_True,
    JSON_TokenType_False,
    JSON_TokenType_Null,
};

typedef enum JSON_ValueType JSON_ValueType;
enum JSON_ValueType
{
    JSON_ValueType_Null,
	JSON_ValueType_Integer,
	JSON_ValueType_Float,
	JSON_ValueType_Boolean,
	JSON_ValueType_String,
	JSON_ValueType_Array,
	JSON_ValueType_Object,
};

typedef struct JSON_MapNode JSON_MapNode;
typedef struct JSON_MapSlot JSON_MapSlot;
typedef struct JSON_Object JSON_Object;
typedef struct JSON_Array JSON_Array;
typedef struct JSON_Value JSON_Value;

struct JSON_MapSlot
{
    JSON_MapNode* first;
    JSON_MapNode* last;
};

struct JSON_Object
{
    JSON_MapSlot* slots;
    u64 count;
};

struct JSON_Array
{
    JSON_Value* values;
    u64 count;
};

struct JSON_Value
{
    JSON_ValueType type;
    union
    {
        i64 ivalue;
        f64 fvalue;
        b32 bvalue;
        String str;
        JSON_Array array;
        JSON_Object obj;
    };
};

struct JSON_MapNode
{
    JSON_MapNode* next;
    String key;
    JSON_Value value;
};

typedef struct JSON_Node JSON_Node;
struct JSON_Node
{
    JSON_Node* parent;
    JSON_Node* next;
    JSON_Node* prev;
    JSON_Node* first;
    JSON_Node* last;
    
    b32 error;
    JSON_Value value;
    r1u64 range;
};

//- long: JSON Functions
#define JSON_I64Value(value) ((JSON_Value){.type = JSON_ValueType_Integer, .ivalue = (value)})
#define JSON_F64Value(value) ((JSON_Value){.type = JSON_ValueType_Float,   .fvalue = (value)})
#define JSON_B32Value(value) ((JSON_Value){.type = JSON_ValueType_Boolean, .bvalue = (value)})
#define JSON_StrValue(value) ((JSON_Value){.type = JSON_ValueType_String,  .str    = (value)})

#define  JSON_PushArray(arena, count) ( (JSON_Array){ PushArrayNZ(arena,   JSON_Value, (count)), (count) })
#define JSON_PushObject(arena, count) ((JSON_Object){   PushArray(arena, JSON_MapSlot, (count)), (count) })

function JSON_Value JSON_ValueFromStr(JSON_Object obj, String str);
function void JSON_ObjInsertValue(Arena* arena, JSON_Object obj, String key, JSON_Value value);

function TokenArray JSON_TokenizeFromText(Arena* arena, String text);
function JSON_Node* JSON_ParseFromTokens (Arena* arena, String text, TokenArray array);
function JSON_Value JSON_ValueFromTokens (Arena* arena, String text, TokenArray array);

function String JSON_StrFromValue(Arena* arena, JSON_Value value, u32 indent);

#endif //_LONG_SCANNER_H
