/* date = August 17th 2024 10:40 pm */

#ifndef _LONG_SCANNER_H
#define _LONG_SCANNER_H

//~ long: Token Types

typedef Flags32 TokenFlags;
enum
{
    TokenFlag_Unterminated  = 1 << 0,
    TokenFlag_BrokenUTF8    = 1 << 1,
    
    TokenFlag_Preproc    = 1 << 2,
    TokenFlag_Comment    = 1 << 3,
    TokenFlag_Whitespace = 1 << 4,
    TokenFlag_Newline    = 1 << 5,
    
    TokenFlag_Identifier = 1 << 6,
    TokenFlag_Symbol     = 1 << 7,
    TokenFlag_Numeric    = 1 << 8,
    TokenFlag_String     = 1 << 9,
};

typedef struct Token Token;
struct Token
{
    Flags32 flags;
    u32 user;
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

//~ long: Scanner Types

typedef Flags32 CL_Scan_Flags;
enum
{
    // If CL_Scan_XXX is not set then they are skipped and don't get reported
    CL_Scan_Whitespace = 1 << 0,
    CL_Scan_Newline    = 1 << 1,
    
    CL_Line_Cont_Comments = 1 << 2,
    CL_Line_Cont_Preprocs = 1 << 3,
    CL_Line_Cont_Strings  = 1 << 4,
    
    CL_Scan_Preproc  = 1 << 5,
    CL_Scan_Comments = 1 << 6,
    CL_Nest_Comments = 1 << 7,
    
    CL_Scan_SingleQuotes = 1 <<  8,
    CL_Scan_DoubleQuotes = 1 <<  9,
    CL_Scan_Ticks        = 1 << 10,
    
    CL_Scan_UTF8       = 1 << 11,
    CL_Scan_Idents     = 1 << 12,
    CL_Skip_Ident_Nums = 1 << 13,
    
    CL_Scan_Nums       = 1 << 14,
    CL_Skip_Num_Idents = 1 << 15,
    
    CL_Scan_Symbols = 1 << 16,
};

typedef struct Scanner Scanner;
typedef TokenFlags ScannerHook(Scanner* scanner);

struct Scanner
{
    String source;
    u64 pos;
    u32 line, col;
    r1u64 prevLine;
    Token token;
    
    ScannerHook* hook;
    CL_Scan_Flags flags;
    u8 lineContinuation;
    
    // preproc
    String preproc;
    
    // Comment
    String commentDelim;
    String commentMulti;
    
    // String
    u8 escapeChar;
    StringList strPrefixes; // L"...", u8"...", U"...", etc
    
    // Identifier
    String preIdentSymbols;
    String midIdentSymbols;
    
    // Numeric
    String preNumSymbols; // unary +- (JSON and expressionless languages), `.` seperator
    String midNumSymbols; // . _ ` '
    String exponents; // e/p/x
    
    // Symbols
    StringList symbols;
    String joinSymbols;
};

//~ long: Scanner Functions

//- long: Construction Functions
#define  ScannerFromStr(str) ((Scanner){ .source = (str) })
#define ScannerFromFile(arena, file) ScannerFromStr(OSReadFile((arena), (file), 0))

#define ScannerRange(scanner, min, max) Substr((scanner)->source, (min), (max))
#define ScannerStr(scanner, size) SubstrRange((scanner)->source, (scanner)->pos, (size))

//- long: Lexing Functions
function b32 ScannerAdvance(Scanner* scanner, i64 advance);
function b32 ScannerCompare(Scanner* scanner, String str);
function b32 ScannerCompareArr(Scanner* scanner, String arr);
function b32 ScannerParse(Scanner* scanner, String str);

function b32 ScannerAdvanceUntil(Scanner* scanner, String str, u8 escapeChar);
function void ScannerAdvanceLine(Scanner* scanner, u8 escapeChar);

// @UB(long): if offset is negative will this wrap around?
#define ScannerByte(scanner, offset) ChrFromStr((scanner)->source, (scanner)->pos + (offset))

function Token ScannerNext(Scanner* scanner);
function Token ScannerPeek(Scanner* scanner);
function Token ScannerPeekAhead(Scanner* scanner, i64 tokenCount);

//~ long: Token Functions

function String StrFromToken(String text, Token token);
function b32 TokenMatch(String text, Token token, String match);

function Token*     TokenChunkListPush(Arena* arena, TokenChunkList* list, u64 cap, Token token);
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
