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

function void MG_MapInsert(Arena* arena, MG_Map* map, String string, void* val);

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
    MG_Expr *parent;
    MG_Expr *left;
    MG_Expr *right;
    MG_ExprOpKind op;
    MD_Node *node;
};

typedef struct MG_ExprResult MG_ExprResult;
struct MG_ExprResult
{
    MG_Expr *root;
    MD_MsgList msgs;
    MD_Node *next_node;
};

readonly global MG_Expr mg_str_expr_nil = {&mg_str_expr_nil, &mg_str_expr_nil, &mg_str_expr_nil};

//~ long: Table Generation Types

typedef struct MG_NodeArray MG_NodeArray;
struct MG_NodeArray
{
    MD_Node** nodes;
    u64 count;
};

typedef struct MG_NodeGrid MG_NodeGrid;
struct MG_NodeGrid
{
    MG_NodeArray cells, row_parents;
};

typedef struct MG_TableExpandTask MG_TableExpandTask;
struct MG_TableExpandTask
{
    MG_TableExpandTask* next;
    String expansion_label;
    MG_NodeGrid* grid;
    StringList column_descs;
    u64 count;
    u64 idx;
};

typedef struct MG_TableExpandInfo MG_TableExpandInfo;
struct MG_TableExpandInfo
{
    MG_TableExpandTask* first_expand_task;
};

#define MG_GridRowCount(grid) ((grid).row_parents.count)
#define MG_GridColCount(grid) ((grid).cells.count / MG_GridRowCount(grid))

function StringList MG_StrListFromTable(Arena* arena, MG_Map grid_name_map, MG_Map grid_column_desc_map, MD_Node* gen);
function StringList   MG_ColDescFromTag(Arena* arena, MD_Node* tag);
function MG_NodeGrid    MG_GridFromNode(Arena* arena, MD_Node* node);

//~ long: C-String Functions

function String MG_StrCEscape(Arena* arena, String string);
function String MG_StrCFromMultiLine(Arena* arena, String string);
function String MG_ArrCFromData(Arena* arena, String data);

//~ long: Main Output Path Types

typedef struct MG_State MG_State;
struct MG_State
{
    StringList enums;
    StringList structs;
    StringList h_functions;
    StringList h_tables;
    StringList c_functions;
    StringList c_tables;
};

#endif //_METAGEN_H
