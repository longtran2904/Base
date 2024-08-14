/* date = August 4th 2024 2:37 pm */

#ifndef _METAGEN_H
#define _METAGEN_H

//~ long: Parse Types

typedef struct MG_Node MG_Node;
struct MG_Node
{
    MG_Node* next;
    MD_Node* node;
};

typedef struct MG_NodeList MG_NodeList;
struct MG_NodeList
{
    MG_Node* first;
    MG_Node* last;
    u64 count;
};

//~ long: Map Types

typedef struct MG_MapNode MG_MapNode;
struct MG_MapNode
{
    MG_MapNode* next;
    String key;
    void* val;
};

typedef struct MG_MapSlot MG_MapSlot;
struct MG_MapSlot
{
    MG_MapNode* first;
    MG_MapNode* last;
};

typedef struct MG_Map MG_Map;
struct MG_Map
{
    MG_MapSlot* slots;
    u64 count;
};

#define MG_PushMap(arena, cnt) ((MG_Map){ .slots = PushArray((arena), MG_MapSlot, (cnt)), .count = (cnt) })

function void* MG_MapPtrFromStr(MG_Map map, String str);
function void  MG_MapInsert(Arena* arena, MG_Map map, String str, void* val);

//~ long: Expression Types

typedef enum MG_ExprKind
{
    MG_ExprKind_Null,
    MG_ExprKind_Prefix,
    MG_ExprKind_Postfix,
    MG_ExprKind_Binary,
    MG_ExprKind_COUNT
} MG_ExprKind;

typedef enum MG_ExprOpKind
{
    MG_ExprOpKind_Null,
    
#define MG_ExprOpKind_FirstString MG_ExprOpKind_Dot
    MG_ExprOpKind_Dot,
    MG_ExprOpKind_ExpandIfTrue,
    MG_ExprOpKind_Concat,
    MG_ExprOpKind_BumpToColumn,
#define MG_ExprOpKind_LastString MG_ExprOpKind_BumpToColumn
    
#define MG_ExprOpKind_FirstNumeric MG_ExprOpKind_Add
    MG_ExprOpKind_Add,
    MG_ExprOpKind_Subtract,
    MG_ExprOpKind_Multiply,
    MG_ExprOpKind_Divide,
    MG_ExprOpKind_Modulo,
    MG_ExprOpKind_LeftShift,
    MG_ExprOpKind_RightShift,
    MG_ExprOpKind_BitwiseAnd,
    MG_ExprOpKind_BitwiseOr,
    MG_ExprOpKind_BitwiseXor,
    MG_ExprOpKind_BitwiseNegate,
    MG_ExprOpKind_BooleanAnd,
    MG_ExprOpKind_BooleanOr,
    MG_ExprOpKind_BooleanNot,
    MG_ExprOpKind_Equals,
    MG_ExprOpKind_DoesNotEqual,
#define MG_ExprOpKind_LastNumeric MG_ExprOpKind_DoesNotEqual
    
    MG_ExprOpKind_COUNT,
} MG_ExprOpKind;

typedef struct MG_Expr MG_Expr;
struct MG_Expr
{
    MG_Expr* parent;
    MG_Expr* left;
    MG_Expr* right;
    MG_ExprOpKind op;
    MD_Node* node;
};

typedef struct MG_ExprResult MG_ExprResult;
struct MG_ExprResult
{
    MG_Expr* root;
    MD_MsgList msgs;
    MD_Node* next;
};

function MG_Expr* MG_PushExpr(Arena* arena, MG_ExprOpKind op, MD_Node* node);
function MG_ExprOpKind MG_ExprLookupOp(MD_Node* expr, MG_ExprKind lookup, i8 minPrec);
function MG_ExprResult MG_ParseExpr(Arena* arena, MD_Node* first, MD_Node* opl, i8 minPrec);
function MG_ExprResult MG_ParseExprFromRoot(Arena* arena, MD_Node* root);

//~ long: Table Generation Types

typedef struct MG_NodeArray MG_NodeArray;
struct MG_NodeArray
{
    MD_Node** nodes;
    u64 count;
};

typedef struct MG_Table MG_Table;
struct MG_Table
{
    MG_NodeArray cells, rowParents;
    StringList members;
};

typedef struct MG_TableExpandTask MG_TableExpandTask;
struct MG_TableExpandTask
{
    MG_TableExpandTask* next;
    String label;
    MG_Table* table;
    u64 count;
    u64 idx;
};

#define MG_TableRowCount(table) ((table).rowParents.count)
#define MG_TableColCount(table) ((table).cells.count / MG_TableRowCount(table))

function MG_NodeArray MG_PushNodeArray(Arena* arena, u64 count);
function MG_Table*        MG_PushTable(Arena* arena, MD_Node* node, MD_Node* tag);

function void         MG_TableExpandStr(Arena* arena, MG_Expr* expr, MG_TableExpandTask* task, StringList* out);
function String     MG_StrFromExpansion(Arena* arena, MG_Expr* expr, MG_TableExpandTask* task);
function i64      MG_TableExpandNumeric(MG_Expr* expr, MG_TableExpandTask* task);
function void     MG_TableLoopExpansion(Arena* arena, String strexpr, MG_TableExpandTask* task, StringList* out);
function StringList MG_StrListFromTable(Arena* arena, MG_Map tableMap, MD_Node* gen, String expandTag);

//~ long: C-String Functions

function String MG_StrCFromMultiLine(Arena* arena, String str);
function String MG_ArrCFromData(Arena* arena, String data);

//~ long: Main Output Path Types

typedef enum MG_GenType MG_GenType;
enum MG_GenType
{
    MG_Gen_Null,
    
    MG_Gen_Enum,
    MG_Gen_Union,
    MG_Gen_Struct,
    
    MG_Gen_Text,
    MG_Gen_Embed,
    MG_Gen_Table,
    
    MG_Gen_Function,
    MG_Gen_All,
    
    MG_Gen_COUNT,
};

global String mg_tagNames[] =
{
    StrConst(""),
    
    StrConst("enum"),
    StrConst("union"),
    StrConst("struct"),
    
    StrConst("text"),
    StrConst("embed"),
    StrConst("data"),
    
    StrConst(""),
    StrConst("gen"),
};

global String mg_genArg[] =
{
    StrConst(""),
    
    StrConst("enums"),
    StrConst("unions"),
    StrConst("structs"),
    
    StrConst(""),
    StrConst(""),
    StrConst("tables"),
    
    StrConst("functions"),
    StrConst(""),
};

typedef struct MG_State MG_State;
struct MG_State
{
    StringList enums;
    StringList unions;
    StringList structs;
    StringList h_functions;
    StringList h_tables;
    StringList h_catchall;
    StringList c_functions;
    StringList c_tables;
    StringList c_catchall;
};

#endif //_METAGEN_H
