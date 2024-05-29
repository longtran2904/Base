#include "DefaultMemory.h"
#include "Base.h"
#include "LongOS.h"
#include "Base.c"
#include "LongOS_Win32.c"
#include <stdio.h>

#pragma WarnPush()
#pragma WarnDisable(28182)
#pragma WarnDisable(6250)
#define MD_DEFAULT_SPRINTF 0
#define MD_IMPL_Vsnprintf stbsp_vsnprintf
#include "md\md.h"
#include "md\md.c"
#pragma WarnPop()

#define C_LIKE_OPS_NO_SIDE_EFFECTS(X) \
    X(ArraySubscript,      "[]",        Postfix,                18) \
    X(Member,              ".",         Binary,                 18) \
    X(PointerMember,       "->",        Binary,                 18) \
    X(UnaryPlus,           "+",         Prefix,                 17) \
    X(UnaryMinus,          "-",         Prefix,                 17) \
    X(LogicalNot,          "!",         Prefix,                 17) \
    X(BitwiseNot,          "~",         Prefix,                 17) \
    X(Dereference,         "*",         Prefix,                 17) \
    X(AddressOf,           "&",         Prefix,                 17) \
    X(SizeOf,              "sizeof",    Prefix,                 17) \
    X(Multiplication,      "*",         Binary,                 15) \
    X(Division,            "/",         Binary,                 15) \
    X(Modulo,              "%",         Binary,                 15) \
    X(Addition,            "+",         Binary,                 14) \
    X(Subtraction,         "-",         Binary,                 14) \
    X(LeftShift,           "<<",        Binary,                 13) \
    X(RightShift,          ">>",        Binary,                 13) \
    X(LessThan,            "<",         Binary,                 11) \
    X(LessThanOrEqual,     "<=",        Binary,                 11) \
    X(GreaterThan,         ">",         Binary,                 11) \
    X(GreaterThanOrEqual,  ">=",        Binary,                 11) \
    X(Equal,               "==",        Binary,                 10) \
    X(NotEqual,            "!=",        Binary,                 10) \
    X(BitwiseAnd,          "&",         Binary,                  9) \
    X(BitwiseXor,          "^",         Binary,                  8) \
    X(BitwiseOr,           "|",         Binary,                  7) \
    X(LogicalAnd,          "&&",        Binary,                  6) \
    X(LogicalOr,           "||",        Binary,                  5)

#define C_LIKE_OPS_CALLS(X) \
    X(Call,                "()",        Postfix,                18)

#define C_LIKE_OPS_WITH_SIDE_EFFECTS(X) \
    X(PostFixIncrement,    "++",        Postfix,                18) \
    X(PostFixDecrement,    "--",        Postfix,                18) \
    X(PreFixIncrement,     "++",        Prefix,                 17) \
    X(PreFixDecrement,     "--",        Prefix,                 17) \
    X(Assign,              "=",         BinaryRightAssociative,  3) \
    X(AssignAddition,      "+=",        BinaryRightAssociative,  3) \
    X(AssignSubtraction,   "-=",        BinaryRightAssociative,  3) \
    X(AssignMultiplication,"*=",        BinaryRightAssociative,  3) \
    X(AssignDivision,      "/=",        BinaryRightAssociative,  3) \
    X(AssignModulo,        "%=",        BinaryRightAssociative,  3) \
    X(AssignLeftShift,     "<<=",       BinaryRightAssociative,  3) \
    X(AssignRightShift,    ">>=",       BinaryRightAssociative,  3) \
    X(AssignBitwiseAnd,    "&=",        BinaryRightAssociative,  3) \
    X(AssignBitwiseXor,    "^=",        BinaryRightAssociative,  3) \
    X(AssignBitwiseOr,     "|=",        BinaryRightAssociative,  3)

enum Op
{
#define DEF_ENUM(e,t,k,p) Op##e,
    C_LIKE_OPS_NO_SIDE_EFFECTS(DEF_ENUM)
        C_LIKE_OPS_CALLS(DEF_ENUM)
        C_LIKE_OPS_WITH_SIDE_EFFECTS(DEF_ENUM)
#undef DEF_ENUM
};

void print_expression(Arena* arena, StringList* list, MD_Expr *expr)
{
    MD_ExprOpr *op = expr->op;
    if (op == 0)
    {
        MD_Node *node = expr->md_node;
        if (node->raw_string.size != 0)
        {
            StrListPushf(arena, list, "%.*s", MD_S8VArg(node->raw_string));
        }
        else if (!MD_NodeIsNil(node->first_child))
        {
            char c1 = 0;
            char c2 = 0;
            
            if (node->flags & MD_NodeFlag_HasParenLeft  ) c1 = '(';
            if (node->flags & MD_NodeFlag_HasBraceLeft  ) c1 = '{';
            if (node->flags & MD_NodeFlag_HasBracketLeft) c1 = '[';
            
            if (node->flags & MD_NodeFlag_HasParenRight  ) c2 = ')';
            if (node->flags & MD_NodeFlag_HasBraceRight  ) c2 = '}';
            if (node->flags & MD_NodeFlag_HasBracketRight) c2 = ']';
            
            StrListPushf(arena, list, "%c...%c", c1, c2);
        }
        else
        {
            MD_CodeLoc loc = MD_CodeLocFromNode(node);
            MD_PrintMessage(stderr, loc, MD_MessageKind_Error,
                            MD_S8Lit("the expression system does not expect this kind of node"));
        }
    }
    else
    {
        switch (op->kind)
        {
            default:
            {
                MD_Node *node = expr->md_node;
                MD_CodeLoc loc = MD_CodeLocFromNode(node);
                MD_PrintMessage(stderr, loc, MD_MessageKind_FatalError,
                                MD_S8Lit("this is an unknown kind of operator"));
            } break;
            
            case MD_ExprOprKind_Prefix:
            {
                StrListPushf(arena, list, "%.*s(", MD_S8VArg(op->string));
                print_expression(arena, list, expr->unary_operand);
                StrListPushf(arena, list, ")");
            } break;
            
            case MD_ExprOprKind_Postfix:
            {
                StrListPushf(arena, list, "(");
                print_expression(arena, list, expr->unary_operand);
                MD_String8 op_string = op->string;
                if ((expr->md_node->flags & MD_NodeFlag_MaskSetDelimiters) != 0)
                    StrListPushf(arena, list, ")%c...%c", op_string.str[0], op_string.str[1]);
                else
                    StrListPushf(arena, list, ")%.*s", MD_S8VArg(op_string));
            } break;
            
            case MD_ExprOprKind_Binary:
            case MD_ExprOprKind_BinaryRightAssociative:
            {
                StrListPushf(arena, list, "(");
                print_expression(arena, list, expr->left);
                StrListPushf(arena, list, " %.*s ", MD_S8VArg(op->string));
                print_expression(arena, list, expr->right);
                StrListPushf(arena, list, ")");
            } break;
        }
    }
}

typedef struct Parser Parser;
struct Parser
{
    MD_Arena* arena;
    
    MD_Node* current;
    MD_Node* parent;
    
    MD_Message* error;
    MD_Node* list;
};

function b32 IsPreProcError(MD_Message* error)
{
    b32 result = 0;
    if (error)
        result = MD_S8Match(error->string, MD_S8Lit("Unexpected reserved symbol \"#\""), 0);
    return result;
}

function b32 ParserInc(Parser* parser, b32 skipPreProc)
{
    b32 result = 0;
    
    if (!MD_NodeIsNil(parser->current))
    {
        parser->current = parser->current->next;
        result = 1;
        
        if (skipPreProc)
        {
            REPEAT_1:
            if (parser->error)
            {
                if (!IsPreProcError(parser->error))
                {
                    parser->error = parser->error->next;
                    goto REPEAT_1;
                }
                else
                {
                    MD_CodeLoc procLoc = MD_CodeLocFromNode(parser->error->node);
                    
                    REPEAT_2:
                    if (!MD_NodeIsNil(parser->current))
                    {
                        MD_CodeLoc currLoc = MD_CodeLocFromNode(parser->current);
                        if (procLoc.line == currLoc.line)
                        {
                            parser->current = parser->current->next;
                            goto REPEAT_2;
                        }
                        else if (procLoc.line < currLoc.line)
                        {
                            parser->error = parser->error->next;
                            goto REPEAT_1;
                        }
                    }
                }
            }
        }
    }
    
    return result;
}

function MD_Node* GetNodeBody(MD_Node* node)
{
    MD_Node* result = 0;
    for (i32 i = 0; i < 2 && !MD_NodeIsNil(node) && !result; ++i, node = node->next)
        if (HasAllFlags(node->flags, MD_NodeFlag_HasBraceLeft|MD_NodeFlag_HasBraceRight))
            result = node;
    return result;
}

function MD_Node* GetNodeBase(MD_Node* node)
{
    return node->first_tag;
}

function MD_String8 GetNodeType(MD_Node* node, MD_Node* base)
{
    UNUSED(node);
    MD_String8 string = base->string;
    return string;
}

function void PrintNode(MD_Node* node)
{
    MD_ArenaTemp temp = MD_GetScratch(0, 0);
    MD_String8List stream = {0};
    MD_Node* name = node->ref_target;
    MD_DebugDumpFromNode(temp.arena, &stream, name, 0, MD_S8Lit(" "), MD_GenerateFlags_Tree|MD_GenerateFlag_NodeFlags);
    MD_String8 str = MD_S8ListJoin(temp.arena, stream, 0);
    MD_Node* base = GetNodeBase(node);
    MD_String8 type = GetNodeType(name, base);
    ScratchBegin(scratch);
    String pointers = ChrRepeat(scratch, '*', base->offset);
    fprintf(stdout, "// Base Type: %.*s%.*s\n%.*s\n\n", MD_S8VArg(type), StrExpand(pointers), MD_S8VArg(str));
    MD_ReleaseScratch(temp);
    ScratchEnd(scratch);
}

function MD_Node* PushParent(Parser* parser, MD_Node* parent)
{
    MD_Node* result = parser->parent;
    parser->parent = parent;
    return result;
}

function void PopParent(Parser* parser)
{
    if (!MD_NodeIsNil(parser->parent))
        parser->parent = parser->parent->parent;
}

function MD_Node* PushNode(Parser* parser, MD_Node* name, MD_Node* base, u32 pointerLevel)
{
    MD_Node* result = MD_PushNewReference(parser->arena, parser->list, name);
    result->parent = parser->parent;
    
    MD_Node* tag = MD_MakeNode(parser->arena, MD_NodeKind_Tag, base->string, base->raw_string, base->offset);
    tag->ref_target = base;
    tag->offset = pointerLevel;
    
    MD_PushTag(result, tag);
    PrintNode(result);
    
    return result;
}

function b32 ParseToken(Parser* parser, MD_String8 token)
{
    b32 result = 0;
    if (!MD_NodeIsNil(parser->current) && MD_S8Match(parser->current->string, token, 0))
    {
        result = 1;
        ParserInc(parser, 1);
    }
    return result;
}

function MD_Node* ParseFlag(Parser* parser, MD_NodeFlags flags)
{
    MD_Node* result = 0;
    if (!MD_NodeIsNil(parser->current) && ((parser->current->flags & flags)/* == flags*/))
    {
        result = parser->current;
        ParserInc(parser, 1);
    }
    return result;
}

function u32 ParsePointer(Parser* parser, b32* error)
{
    u32 result = 0;
    b32 _err_ = 0;
    if (!error) error = &_err_;
    
    REPEAT:
    if (!MD_NodeIsNil(parser->current))
    {
        MD_String8 str = parser->current->string;
        u32 i = 0;
        for (; i < str.size; ++i)
            if (str.str[i] != '*')
                break;
        
        if (i)
        {
            result += i;
            ParserInc(parser, 1);
            
            if (i == str.size)
                goto REPEAT;
            else
                *error = 1;
        }
    }
    return result;
}

function u32 ParseArray(Parser* parser)
{
    u32 result = 0;
    while (!MD_NodeIsNil(parser->current))
    {
        if (HasAllFlags(parser->current->flags, MD_NodeFlag_HasBracketLeft|MD_NodeFlag_HasBracketRight))
            result++;
        else
            break;
        ParserInc(parser, 1);
    }
    return result;
}

function MD_Node* ParseIdentifier(Parser* parser, b32 ignoreFlags)
{
    MD_Node* result = 0;
    if (!MD_NodeIsNil(parser->current))
    {
        MD_NodeFlags flags = MD_NodeFlag_IsBeforeSemicolon|MD_NodeFlag_IsBeforeComma;
        if (HasAnyFlags(parser->current->flags, MD_NodeFlag_Identifier) && (ignoreFlags || NoFlags(parser->current->flags, flags)))
        {
            result = parser->current;
            ParserInc(parser, 1);
        }
    }
    return result;
}

#pragma WarnDisable(4706)
int main(int argc, char** argv)
{
    OSInit(0, 0);
    
    // setup the global arena
    MD_Arena* arena = MD_ArenaAlloc();
    
    // parse all files passed to the command line
    MD_Node *list = MD_MakeList(arena);
    for (int i = 1; i < argc; i += 1)
    {
        MD_String8 file_name = MD_S8CString(argv[i]);
        MD_ParseResult parse_result = MD_ParseWholeFile(arena, file_name);
        
        for (MD_Message *message = parse_result.errors.first; message != 0; message = message->next)
        {
            if (IsPreProcError(message))
            {
                MD_CodeLoc code_loc = MD_CodeLocFromNode(message->node);
                MD_PrintMessage(stdout, code_loc, message->kind, message->string);
                MD_String8List stream = {0};
                MD_DebugDumpFromNode(arena, &stream, message->node, 0, MD_S8Lit(" "), MD_GenerateFlags_Tree|MD_GenerateFlag_NodeFlags|MD_GenerateFlag_NodeKind);
                MD_String8 str = MD_S8ListJoin(arena, stream, 0);
                fprintf(stdout, "%.*s\n\n", MD_S8VArg(str));
            }
        }
        
        //if (parse_result.errors.max_message_kind < MD_MessageKind_Error)
        {
            MD_PushNewReference(arena, list, parse_result.node);
        }
        
        Parser* parser = &(Parser){
            .arena = arena,
            .current = parse_result.node->first_child,
            .error = parse_result.errors.first,
            .list = MD_MakeList(arena),
        };
        
        while (!MD_NodeIsNil(parser->current))
        {
            Parser restore = *parser;
            b32 handled = 0;
            
            MD_Node* name = 0;
            MD_Node* base = 0;
            
            if (base = ParseIdentifier(parser, 0))
            {
                REPEAT:
                b32 error = 0;
                u32 pointerLevel = ParsePointer(parser, &error);
                if (!error && (name = ParseIdentifier(parser, 1)))
                {
                    //- NOTE(long): Functions
                    if (ParseFlag(parser, MD_NodeFlag_HasParenLeft|MD_NodeFlag_HasParenRight))
                    {
                        PushNode(parser, name, base, pointerLevel);
                        handled = 1;
                    }
                    
                    //- NOTE(long): Types
                    else if (ParseFlag(parser, MD_NodeFlag_HasBraceLeft|MD_NodeFlag_HasBraceRight))
                    {
                        PushNode(parser, name, base, pointerLevel);
                        handled = 1;
                    }
                    
                    //- NOTE(long): Decls
                    else
                    {
                        u32 arrayCount = ParseArray(parser);
                        MD_NodeFlags flags = MD_NodeFlag_IsAfterSemicolon|MD_NodeFlag_IsAfterComma;
                        if (HasAnyFlags(parser->current->flags, flags) || MD_S8Match(parser->current->string, MD_S8Lit("="), 0))
                        {
                            MD_Node* decl = PushNode(parser, name, base, pointerLevel);
                            handled = 1;
                            
                            if (HasAnyFlags(parser->current->flags, MD_NodeFlag_IsAfterComma))
                                goto REPEAT;
                        }
                        
                        // TODO(long): Handle parens inside a declaration
                    }
                }
            }
            
            if (!handled)
            {
                *parser = restore;
                ParserInc(parser, 1);
            }
        }
    }
    
    // setup the expression system
    MD_ExprOprTable table = {0};
    {
        MD_ExprOprList exprList = {0};
        
#define PUSH_OP(e,t,k,p) \
    MD_ExprOprPush(arena, &exprList, MD_ExprOprKind_##k, p, MD_S8Lit(t), Op##e, 0);
        C_LIKE_OPS_NO_SIDE_EFFECTS(PUSH_OP);
        C_LIKE_OPS_CALLS(PUSH_OP);
        C_LIKE_OPS_WITH_SIDE_EFFECTS(PUSH_OP);
#undef PUSH_OP
        
        table = MD_ExprBakeOprTableFromList(arena, &exprList);
    }
    
#if 0
    for (MD_EachNode(root_it, list->first_child))
    {
        MD_Node *root = MD_ResolveNodeFromReference(root_it);
        for (MD_EachNode(node, root->first_child))
        {
            MD_String8List stream = {0};
            MD_DebugDumpFromNode(arena, &stream, node, 0, MD_S8Lit(" "),
                                 MD_GenerateFlags_Tree|MD_GenerateFlag_NodeFlags|MD_GenerateFlag_NodeKind);
            MD_String8 str = MD_S8ListJoin(arena, stream, 0);
            fprintf(stdout, "%.*s\n\n", MD_S8VArg(str));
        }
    }
#endif
}
#pragma WarnEnable(4706)
