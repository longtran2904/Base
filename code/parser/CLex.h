/* date = September 25th 2024 1:13 pm */

#ifndef _C_LEX_H
#define _C_LEX_H

//~ long: Node Types

enum
{
    CL_TokenFlags_Ignorable = TokenFlag_Preproc|TokenFlag_Comment|TokenFlag_Whitespace|TokenFlag_Newline,
    CL_TokenFlags_Literal = TokenFlag_Numeric|TokenFlag_String,
};

typedef Flags64 CL_NodeFlags;

#define CL_NodeFlag_File  (1ULL << 0)
#define CL_NodeFlag_Error (1ULL << 1)
#define CL_NodeFlag_Decl (1ULL << 3)

#define CL_NodeFlag_Brack (1ULL <<  8)
#define CL_NodeFlag_Brace (1ULL <<  9)
#define CL_NodeFlag_Paren (1ULL << 10)

#define CL_NodeFlags_Atom      (0x7 << 11)
#define CL_NodeFlag_Identifier (1ULL << 11)
#define CL_NodeFlag_Numeric    (1ULL << 12)
#define CL_NodeFlag_String     (1ULL << 14)
#define CL_NodeFlag_Symbol     (1ULL << 15)

#define CL_NodeFlags_TypeKeyword (0x7 << 16)
#define CL_NodeFlag_Struct (1ULL << 16)
#define CL_NodeFlag_Union  (1ULL << 17)
#define CL_NodeFlag_Enum   (1ULL << 18)

#define CL_NodeFlags_Leaf    (0x7F << 19)
#define CL_NodeFlag_Void     (1ULL << 19)
#define CL_NodeFlag_Char     (1ULL << 20)
#define CL_NodeFlag_Short    (1ULL << 21)
#define CL_NodeFlag_Int      (1ULL << 22)
#define CL_NodeFlag_Long     (1ULL << 23)
#define CL_NodeFlag_Float    (1ULL << 24)
#define CL_NodeFlag_Double   (1ULL << 25)

#define CL_NodeFlag_Signed   (1ULL << 26)
#define CL_NodeFlag_Unsigned (1ULL << 27)
//#define CL_NodeFlag_Complex (1ULL << 28)
//#define CL_NodeFlag_Imaginary (1ULL << 29)

#define CL_NodeFlags_Qualifier (0xF << 32)
#define CL_NodeFlag_Const    (1ULL << 32)
#define CL_NodeFlag_Volatile (1ULL << 33)
#define CL_NodeFlag_Restrict (1ULL << 34)
#define CL_NodeFlag_Atomic   (1ULL << 35)

#define CL_NodeFlags_Specifier (0x2FF << 36)
#define CL_NodeFlag_Typedef     (1ULL << 36)
#define CL_NodeFlag_Constexpr   (1ULL << 37)
#define CL_NodeFlag_Auto        (1ULL << 38)
#define CL_NodeFlag_Register    (1ULL << 39)
#define CL_NodeFlag_Static      (1ULL << 40)
#define CL_NodeFlag_Extern      (1ULL << 41)
#define CL_NodeFlag_Inline      (1ULL << 42)
#define CL_NodeFlag_ThreadLocal (1ULL << 43)
#define CL_NodeFlag_Noreturn    (1ULL << 44)
#define CL_NodeFlag_Alignas     (1ULL << 45)

typedef struct CL_Node CL_Node;
typedef struct CL_NodeList CL_NodeList;

struct CL_NodeList
{
    CL_Node* first;
    CL_Node* last;
};

struct CL_Node
{
    CL_Node* next;
    CL_Node* prev;
    CL_Node* parent;
    CL_Node* first;
    CL_Node* last;
    
    CL_Node* ref;
    CL_Node* firstTag;
    CL_Node* lastTag;
    CL_Node* firstArg;
    CL_Node* lastArg;
    
    String string;
    CL_NodeFlags flags;
    u64 value;
    u64 offset;
};

global readonly CL_Node cl_nilNode = {
    &cl_nilNode, &cl_nilNode, &cl_nilNode, &cl_nilNode, &cl_nilNode,
    &cl_nilNode, &cl_nilNode, &cl_nilNode, &cl_nilNode, &cl_nilNode,
};

typedef struct CL_ParseResult CL_ParseResult;
struct CL_ParseResult
{
    CL_Node* root;
    CL_NodeList errors;
};

//~ @REF(long): Data Desk Types

typedef enum
{
    DataDeskNodeType_Invalid,
    DataDeskNodeType_Identifier,
    DataDeskNodeType_NumericConstant,
    DataDeskNodeType_StringConstant,
    DataDeskNodeType_CharConstant,
    DataDeskNodeType_BinaryOperator,
    DataDeskNodeType_StructDeclaration,
    DataDeskNodeType_UnionDeclaration,
    DataDeskNodeType_EnumDeclaration,
    DataDeskNodeType_FlagsDeclaration,
    DataDeskNodeType_Declaration,
    DataDeskNodeType_TypeUsage,
    DataDeskNodeType_Tag,
    DataDeskNodeType_ConstantDefinition,
    DataDeskNodeType_ProcedureHeader,
} DataDeskNodeType;

typedef struct DataDeskNode DataDeskNode;
struct DataDeskNode
{
    //- NOTE(rjf): AST Relationship Data
    DataDeskNode* next;
    DataDeskNode* prev;
    DataDeskNode* parent;
    DataDeskNodeType type;
    //DataDeskNodeSubType sub_type;
    
    //- NOTE(rjf): String
    union
    {
        int string_length;
        int name_length;
    };
    union
    {
        char* string;
        char* name;
    };
    
    //- NOTE(rjf): File/Line Source Information
    char* file;
    int line;
    
    //- NOTE(rjf): Tag List
    union
    {
        DataDeskNode* tag_list_head;
        DataDeskNode* tag_list;
        DataDeskNode* first_tag;
    };
    
    //- NOTE(rjf): Children and node-specific children aliases
    union
    {
        struct
        {
            DataDeskNode* children_list_head;
            DataDeskNode* children_list_tail;
        };
        
        struct
        {
            DataDeskNode* left;
            DataDeskNode* right;
        };
        
        DataDeskNode* operand;
        DataDeskNode* reference;
        DataDeskNode* expression;
    };
    
    //- NOTE(rjf): Node-specific data shortcuts
    union
    {
        struct
        {
            DataDeskNode* return_type;
            DataDeskNode* first_parameter;
        } procedure_header;
        
        struct
        {
            DataDeskNode* type;
            DataDeskNode* initialization;
        } declaration;
        
        struct
        {
            DataDeskNode* first_tag_parameter;
        } tag;
    };
};

//~ long: Meta Info Types

typedef Flags32 MetaFlags;
enum
{
    MetaFlags_Leaf    = 0x1F,
    MetaFlags_Numeric = 0x7 << 1,
    MetaFlag_Void  = 1 << 0,
    MetaFlag_Int   = 1 << 1,
    MetaFlag_UInt  = 1 << 2,
    MetaFlag_Float = 1 << 3,
    MetaFlag_Bool  = 1 << 4,
    MetaFlag_Flag  = 1 << 5,
    MetaFlag_Ext   = 1 << 6,
    
    MetaFlags_UserType = 0xF << 5,
    MetaFlag_Struct = 1 << 8,
    MetaFlag_Union  = 1 << 9,
    MetaFlag_Enum   = 1 << 10,
    MetaFlag_Proc   = 1 << 11,
    MetaFlag_Alias  = 1 << 12, // Typedef
    
    MetaFlag_Decl  = 1 << 13,
    MetaFlag_Value = 1 << 14,
    MetaFlag_Array = 1 << 15,
    
    MetaFlag_TweakB32  = 1 << 16,
    MetaFlag_TweakF32  = 1 << 17,
    MetaFlag_EmbedFile = 1 << 14,
    MetaFlag_Tag = 1 << 18,
};

typedef struct MetaInfo MetaInfo;
typedef struct MetaTable MetaTable;

struct MetaTable
{
    MetaInfo* v;
    u64 count;
};

struct MetaInfo
{
    MetaInfo* parent;
    MetaInfo* base;
    MetaTable childs;
    
    MetaFlags flags;
    String name;
    u64 value;
    u64 pointerCount;
};

//~ TODO(long): Type Serialization

//~ long: Meta Info Helpers

#define TypeOf(type) type##__type

#define LeafType(type, flag) \
    readonly global MetaInfo TypeOf(type) = { \
        .parent = &metaNil, .base = &metaNil, \
        .flags = (flag), .name = StrConst(#type), .value = sizeof(type) \
    }
#define TypedefLit(type, _name, ...) readonly global MetaInfo TypeOf(_name) = { \
        .parent = &metaNil, .base = &TypeOf(type), .flags = MetaFlag_Alias, \
        .name = StrConst(#_name), .value = sizeof(type), __VA_ARGS__ \
    }
#define EmptyProc(Proc, returnType, ...) readonly global MetaInfo TypeOf(Proc) = { \
        .parent = &metaNil, .base = &TypeOf(returnType), .flags = MetaFlag_Proc, \
        .name = StrConst(#Proc), .value = sizeof(Proc*), __VA_ARGS__ \
    }

#define DeclLit(Parent, Type, declName, flg, val, ...) { \
        .parent = &TypeOf(Parent), .base = &Type, .flags = flg, \
        .name = StrConst(#declName), .value = val, __VA_ARGS__ \
    }
#define ArrayMember(Struct, Type, member) DeclLit(Struct, TypeOf(Type), member, MetaFlag_Decl|MetaFlag_Array, \
                                                  OffsetOf(Struct, member), ArrayCount(MemberOf(Struct, member)))
#define MemberLit(Struct, Type, member, ...) DeclLit(Struct, TypeOf(Type), member, MetaFlag_Decl, \
                                                     OffsetOf(Struct, member), __VA_ARGS__)
#define EnumValue(Enum, val) DeclLit(Enum, metaNil, val, MetaFlag_Value, val)
#define ArgLit(Proc, Type, name, ...) DeclLit(Proc, TypeOf(Type), name, MetaFlag_Decl, 0, __VA_ARGS__)

#define _TypeLit(Type, flg, val, count, ...) \
    readonly global MetaInfo Type##__members[count]; \
    readonly global MetaInfo Type##__type = { \
        .parent = &metaNil, .base = &metaNil, .childs = {Type##__members, count}, \
        .flags = flg, .name = StrConst(#Type), .value = val, __VA_ARGS__ \
    }; \
    readonly global MetaInfo Type##__members[count] =

#define StructLit(Struct, count) _TypeLit(Struct, MetaFlag_Struct, sizeof(Struct), count)
#define UnionLit(Union, count) _TypeLit(Union, MetaFlag_Union, sizeof(Union), count)
#define EnumLit(Enum, count) _TypeLit(Enum, MetaFlag_Enum, sizeof(Enum), count)
#define ProcLit(Proc, returnType, count, ...) _TypeLit(Proc, MetaFlag_Proc, sizeof(Proc*), count, \
                                                       .base = &TypeOf(returnType), __VA_ARGS__)

//~ rjf: Built-In Types

readonly global MetaInfo metaNil = { .parent = &metaNil, .base = &metaNil };
readonly global MetaInfo void__type = {
    .parent = &metaNil, .base = &metaNil,
    .flags = MetaFlag_Void, .name = StrConst("void")
};

//- long: Leaves
LeafType( u8, MetaFlag_UInt);
LeafType(u16, MetaFlag_UInt);
LeafType(u32, MetaFlag_UInt);
LeafType(u64, MetaFlag_UInt);
LeafType( i8, MetaFlag_Int);
LeafType(i16, MetaFlag_Int);
LeafType(i32, MetaFlag_Int);
LeafType(i64, MetaFlag_Int);
LeafType( b8, MetaFlag_Bool);
LeafType(b16, MetaFlag_Bool);
LeafType(b32, MetaFlag_Bool);
LeafType(b64, MetaFlag_Bool);
LeafType(f32, MetaFlag_Float);
LeafType(f64, MetaFlag_Float);

LeafType(Flags8, MetaFlag_Flag);
LeafType(Flags16, MetaFlag_Flag);
LeafType(Flags32, MetaFlag_Flag);
LeafType(Flags64, MetaFlag_Flag);

// @CONSIDER(long): MetaFlag_Char
LeafType(char, MetaFlag_Int);

LeafType(va_list, MetaFlag_Ext);

StructLit(u128, 1)
{
    ArrayMember(u128, u64, v),
};

//- long: User
EmptyProc(VoidFunc, void);

ProcLit(VoidFuncVoid, void, 1)
{
    ArgLit(VoidFuncVoid, void, , 1),
};

UnionLit(v2f32, 3)
{
    MemberLit(v2f32, f32, x, 0),
    MemberLit(v2f32, f32, y, 0),
    ArrayMember(v2f32, f32, v),
};

UnionLit(v2i64, 3)
{
    MemberLit(v2i64, i64, x, 0),
    MemberLit(v2i64, i64, y, 0),
    ArrayMember(v2i64, i64, v),
};

UnionLit(v2i32, 3)
{
    MemberLit(v2i32, i32, x, 0),
    MemberLit(v2i32, i32, y, 0),
    ArrayMember(v2i32, i32, v),
};

UnionLit(v2i16, 3)
{
    MemberLit(v2i16, i16, x, 0),
    MemberLit(v2i16, i16, y, 0),
    ArrayMember(v2i16, i16, v),
};

UnionLit(v3f32, 8)
{
    MemberLit(v3f32, f32, x, 0),
    MemberLit(v3f32, f32, y, 0),
    MemberLit(v3f32, f32, z, 0),
    MemberLit(v3f32, v2f32, xy, 0),
    MemberLit(v3f32, f32, _z0, 0),
    MemberLit(v3f32, f32, _x0, 0),
    MemberLit(v3f32, v2f32, yz, 0),
    ArrayMember(v3f32, f32, v),
};

UnionLit(v3i32, 8)
{
    MemberLit(v3i32, i32, x, 0),
    MemberLit(v3i32, i32, y, 0),
    MemberLit(v3i32, i32, z, 0),
    MemberLit(v3i32, v2i32, xy, 0),
    MemberLit(v3i32, i32, _z0, 0),
    MemberLit(v3i32, i32, _x0, 0),
    MemberLit(v3i32, v2i32, yz, 0),
    ArrayMember(v3i32, i32, v),
};

UnionLit(r1i32, 3)
{
    MemberLit(r1i32, i32, min, 0),
    MemberLit(r1i32, i32, max, 0),
    ArrayMember(r1i32, i32, v),
};

UnionLit(r1u64, 3)
{
    MemberLit(r1u64, u64, min, 0),
    MemberLit(r1u64, u64, max, 0),
    ArrayMember(r1u64, u64, v),
};

UnionLit(r1f32, 3)
{
    MemberLit(r1f32, f32, min, 0),
    MemberLit(r1f32, f32, max, 0),
    ArrayMember(r1f32, f32, v),
};

UnionLit(r2i32, 9)
{
    MemberLit(r2i32, v2i32, min, 0),
    MemberLit(r2i32, v2i32, max, 0),
    MemberLit(r2i32, v2i32, p0, 0),
    MemberLit(r2i32, v2i32, p1, 0),
    MemberLit(r2i32, i32, x0, 0),
    MemberLit(r2i32, i32, y0, 0),
    MemberLit(r2i32, i32, x1, 0),
    MemberLit(r2i32, i32, y1, 0),
    ArrayMember(r2i32, v2i32, v),
};

UnionLit(r2f32, 9)
{
    MemberLit(r2f32, v2f32, min, 0),
    MemberLit(r2f32, v2f32, max, 0),
    MemberLit(r2f32, v2f32, p0, 0),
    MemberLit(r2f32, v2f32, p1, 0),
    MemberLit(r2f32, f32, x0, 0),
    MemberLit(r2f32, f32, y0, 0),
    MemberLit(r2f32, f32, x1, 0),
    MemberLit(r2f32, f32, y1, 0),
    ArrayMember(r2f32, v2f32, v),
};

StructLit(TextLoc, 2)
{
    MemberLit(TextLoc, u32, line, 0),
    MemberLit(TextLoc, u32, col, 0),
};

EnumLit(Axis, 4)
{
    EnumValue(Axis, Axis_X),
    EnumValue(Axis, Axis_Y),
    EnumValue(Axis, Axis_Z),
    EnumValue(Axis, Axis_W),
};

EnumLit(Side, 2)
{
    EnumValue(Side, Side_Min),
    EnumValue(Side, Side_Max),
};

EnumLit(Compiler, 5)
{
    EnumValue(Compiler, Compiler_None),
    EnumValue(Compiler, Compiler_CLANG),
    EnumValue(Compiler, Compiler_CL),
    EnumValue(Compiler, Compiler_GCC),
    EnumValue(Compiler, Compiler_Count),
};

EnumLit(Arch, 6)
{
    EnumValue(Arch, Arch_None),
    EnumValue(Arch, Arch_X64),
    EnumValue(Arch, Arch_X86),
    EnumValue(Arch, Arch_ARM),
    EnumValue(Arch, Arch_ARM64),
    EnumValue(Arch, Arch_Count),
};

EnumLit(OS, 5)
{
    EnumValue(OS, OS_None),
    EnumValue(OS, OS_Win),
    EnumValue(OS, OS_Linux),
    EnumValue(OS, OS_Mac),
    EnumValue(OS, OS_Count),
};

EnumLit(Month, 13)
{
    EnumValue(Month, Month_Jan),
    EnumValue(Month, Month_Feb),
    EnumValue(Month, Month_Mar),
    EnumValue(Month, Month_Apr),
    EnumValue(Month, Month_May),
    EnumValue(Month, Month_Jun),
    EnumValue(Month, Month_Jul),
    EnumValue(Month, Month_Aug),
    EnumValue(Month, Month_Sep),
    EnumValue(Month, Month_Oct),
    EnumValue(Month, Month_Nov),
    EnumValue(Month, Month_Dec),
    EnumValue(Month, Month_Count),
};

EnumLit(Day, 12)
{
    EnumValue(Day, Day_Sunday),
    EnumValue(Day, Day_Monday),
    EnumValue(Day, Day_Tuesday),
    EnumValue(Day, Day_Wednesday),
    EnumValue(Day, Day_Thursday),
    EnumValue(Day, Day_Friday),
    EnumValue(Day, Day_Saturday),
    EnumValue(Day, Day_Count),
};

TypedefLit(u64, DenseTime);

StructLit(DateTime, 7)
{
    MemberLit(DateTime, u16, msec, 0),
    MemberLit(DateTime, u8, sec, 0),
    MemberLit(DateTime, u8, min, 0),
    MemberLit(DateTime, u8, hour, 0),
    MemberLit(DateTime, u8, day, 0),
    MemberLit(DateTime, u8, mon, 0),
    MemberLit(DateTime, i16, year, 0),
};

TypedefLit(Flags32, DataAccessFlags);

TypedefLit(Flags32, FilePropertyFlags);

StructLit(FileProperties, 5)
{
    MemberLit(FileProperties, u64, size, 0),
    MemberLit(FileProperties, FilePropertyFlags, flags, 0),
    MemberLit(FileProperties, DataAccessFlags, access, 0),
    MemberLit(FileProperties, DenseTime, createTime, 0),
    MemberLit(FileProperties, DenseTime, modifyTime, 0),
};

TypedefLit(Flags32, ArenaFlags);

StructLit(Arena, 9)
{
    MemberLit(Arena, Arena, prev, 1),
    MemberLit(Arena, Arena, curr, 1),
    MemberLit(Arena, u32, alignment, 0),
    MemberLit(Arena, ArenaFlags, flags, 0),
    MemberLit(Arena, u64, basePos, 0),
    MemberLit(Arena, u64, pos, 0),
    MemberLit(Arena, u64, commitPos, 0),
    MemberLit(Arena, u64, cap, 0),
    MemberLit(Arena, u64, highWaterMark, 0),
};

StructLit(TempArena, 2)
{
    MemberLit(TempArena, Arena, arena, 1),
    MemberLit(TempArena, u64, pos, 0),
};

StructLit(String, 2)
{
    MemberLit(String, u8, str, 1),
    MemberLit(String, u64, size, 0),
};

StructLit(String32, 2)
{
    MemberLit(String32, u32, str, 1),
    MemberLit(String32, u64, size, 0),
};

StructLit(String16, 2)
{
    MemberLit(String16, u16, str, 1),
    MemberLit(String16, u64, size, 0),
};

StructLit(StringNode, 2)
{
    MemberLit(StringNode, StringNode, next, 1),
    MemberLit(StringNode, String, string, 0),
};

StructLit(StringMetaNode, 2)
{
    MemberLit(StringMetaNode, StringMetaNode, next, 1),
    MemberLit(StringMetaNode, StringNode, node, 1),
};

StructLit(StringList, 4)
{
    MemberLit(StringList, StringNode, first, 1),
    MemberLit(StringList, StringNode, last, 1),
    MemberLit(StringList, u64, nodeCount, 0),
    MemberLit(StringList, u64, totalSize, 0),
};

StructLit(StringJoin, 3)
{
    MemberLit(StringJoin, String, pre, 0),
    MemberLit(StringJoin, String, mid, 0),
    MemberLit(StringJoin, String, post, 0),
};

TypedefLit(Flags32, StringMatchFlags);

EnumLit(PathStyle, 4)
{
    EnumValue(PathStyle, PathStyle_Relative),
    EnumValue(PathStyle, PathStyle_WindowsAbsolute),
    EnumValue(PathStyle, PathStyle_UnixAbsolute),
};

EnumLit(DecodeError, 5)
{
    EnumValue(DecodeError, DecodeError_EOF),
    EnumValue(DecodeError, DecodeError_Overlong),
    EnumValue(DecodeError, DecodeError_Surrogate),
    EnumValue(DecodeError, DecodeError_OutOfRange),
    EnumValue(DecodeError, DecodeError_InvalidBits),
};

StructLit(StringDecode, 3)
{
    MemberLit(StringDecode, u32, codepoint, 0),
    MemberLit(StringDecode, u32, size, 0),
    MemberLit(StringDecode, u32, error, 0),
};

StructLit(CmdLineOpt, 3)
{
    MemberLit(CmdLineOpt, CmdLineOpt, next, 1),
    MemberLit(CmdLineOpt, String, name, 0),
    MemberLit(CmdLineOpt, StringList, values, 0),
};

StructLit(CmdLineOptList, 3)
{
    MemberLit(CmdLineOptList, CmdLineOpt, first, 1),
    MemberLit(CmdLineOptList, CmdLineOpt, last, 1),
    MemberLit(CmdLineOptList, u64, count, 0),
};

StructLit(CmdLine, 3)
{
    MemberLit(CmdLine, String, programName, 0),
    MemberLit(CmdLine, StringList, inputs, 0),
    MemberLit(CmdLine, CmdLineOptList, opts, 0),
};

StructLit(Record, 5)
{
    MemberLit(Record, String, log, 0),
    MemberLit(Record, char, file, 1),
    MemberLit(Record, i32, line, 0),
    MemberLit(Record, i32, level, 0),
    MemberLit(Record, DenseTime, time, 0),
};

EnumLit(LogType, 7)
{
    EnumValue(LogType, LOG_TRACE),
    EnumValue(LogType, LOG_DEBUG),
    EnumValue(LogType, LOG_INFO),
    EnumValue(LogType, LOG_WARN),
    EnumValue(LogType, LOG_ERROR),
    EnumValue(LogType, LOG_FATAL),
    EnumValue(LogType, LogType_Count),
};

ProcLit(LogHandler, void, 4)
{
    ArgLit(LogHandler, Arena, arena, 1),
    ArgLit(LogHandler, Record, record, 1),
    ArgLit(LogHandler, char, fmt, 1),
    ArgLit(LogHandler, va_list, args, 0),
};

StructLit(LogInfo, 2)
{
    MemberLit(LogInfo, i64, level, 0),
    MemberLit(LogInfo, LogHandler, callback, 1),
};

StructLit(Logger, 3)
{
    MemberLit(Logger, Record, records, 1),
    MemberLit(Logger, u64, count, 0),
    MemberLit(Logger, LogInfo, info, 0),
};

StructLit(RNG, 2)
{
    MemberLit(RNG, u32, seed, 0),
    MemberLit(RNG, u32, pos, 0),
};

TypedefLit(Flags32, OSFileIterFlags);

StructLit(OSFileIter, 5)
{
    MemberLit(OSFileIter, FileProperties, props, 0),
    MemberLit(OSFileIter, String, name, 0),
    MemberLit(OSFileIter, String, path, 0),
    MemberLit(OSFileIter, OSFileIterFlags, flags, 0),
    ArrayMember(OSFileIter, u8, v),
};

StructLit(OS_Handle, 1)
{
    ArrayMember(OS_Handle, u64, v),
};

//~ long: Lexing Functions

function TokenArray CL_TokenArrayFromStr(Arena* arena, String text);
function MetaTable    CL_TableFromTokens(Arena* arena, String text, TokenArray array);

//~ long: Type Info Lookups

function MetaInfo* MemberFromName(MetaInfo* type, String name);
#define EachMember(T, it) (MetaInfo* it = (TypeOf(T))->childs; it != 0 && it < (TypeOf(T))->childs + (TypeOf(T))->count; it++)

#endif //_C_LEX_H
