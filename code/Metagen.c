
//~ long: Map Types

internal void* MG_MapPtrFromStr(MG_Map* map, String string)
{
    void* result = 0;
    u64 hash = Hash64(string.str, string.size);
    MG_MapSlot* slot = &map->slots[hash % map->count];
    
    for (MG_MapNode* n = slot->first; n != 0; n = n->next)
    {
        if (StrCompare(n->key, string, 0))
        {
            result = n->val;
            break;
        }
    }
    
    return result;
}

function void MG_MapInsert(Arena* arena, MG_Map* map, String string, void* val)
{
    u64 hash = Hash64(string.str, string.size);
    MG_MapSlot* slot = &map->slots[hash % map->count];
    MG_MapNode* node = PushStruct(arena, MG_MapNode);
    node->key = StrCopy(arena, string);
    node->val = val;
    SLLQueuePush(slot->first, slot->last,node);
}

//~ long: Table Generation Types

readonly global String mg_str_expr_op_symbol_string_table[MG_ExprOpKind_COUNT] =
{
    StrConst(  ""),
    StrConst( "."), // MG_StrExprOp_Dot
    StrConst("->"), // MG_StrExprOp_ExpandIfTrue
    StrConst(".."), // MG_StrExprOp_Concat
    StrConst("=>"), // MG_StrExprOp_BumpToColumn
    StrConst( "+"), // MG_StrExprOp_Add
    StrConst( "-"), // MG_StrExprOp_Subtract
    StrConst( "*"), // MG_StrExprOp_Multiply
    StrConst( "/"), // MG_StrExprOp_Divide
    StrConst( "%"), // MG_StrExprOp_Modulo
    StrConst("<<"), // MG_StrExprOp_LeftShift
    StrConst(">>"), // MG_StrExprOp_RightShift
    StrConst( "&"), // MG_StrExprOp_BitwiseAnd
    StrConst( "|"), // MG_StrExprOp_BitwiseOr
    StrConst( "^"), // MG_StrExprOp_BitwiseXor
    StrConst( "~"), // MG_StrExprOp_BitwiseNegate
    StrConst("&&"), // MG_StrExprOp_BooleanAnd
    StrConst("||"), // MG_StrExprOp_BooleanOr
    StrConst( "!"), // MG_StrExprOp_BooleanNot
    StrConst("=="), // MG_StrExprOp_Equals
    StrConst("!="), // MG_StrExprOp_DoesNotEqual
};

readonly global i8 mg_str_expr_op_precedence_table[MG_ExprOpKind_COUNT] =
{
    0,
    20, // MG_StrExprOp_Dot
    1,  // MG_StrExprOp_ExpandIfTrue
    2,  // MG_StrExprOp_Concat
    12, // MG_StrExprOp_BumpToColumn
    5,  // MG_StrExprOp_Add
    5,  // MG_StrExprOp_Subtract
    6,  // MG_StrExprOp_Multiply
    6,  // MG_StrExprOp_Divide
    6,  // MG_StrExprOp_Modulo
    7,  // MG_StrExprOp_LeftShift
    7,  // MG_StrExprOp_RightShift
    8,  // MG_StrExprOp_BitwiseAnd
    10, // MG_StrExprOp_BitwiseOr
    9,  // MG_StrExprOp_BitwiseXor
    11, // MG_StrExprOp_BitwiseNegate
    3,  // MG_StrExprOp_BooleanAnd
    3,  // MG_StrExprOp_BooleanOr
    11, // MG_StrExprOp_BooleanNot
    4,  // MG_StrExprOp_Equals
    4,  // MG_StrExprOp_DoesNotEqual
};

readonly global MG_ExprKind mg_str_expr_op_kind_table[MG_ExprOpKind_COUNT] =
{
    MG_ExprKind_Null,
    MG_ExprKind_Binary, // MG_ExprOpKind_Dot
    MG_ExprKind_Binary, // MG_ExprOpKind_ExpandIfTrue
    MG_ExprKind_Binary, // MG_ExprOpKind_Concat
    MG_ExprKind_Prefix, // MG_ExprOpKind_BumpToColumn
    MG_ExprKind_Binary, // MG_ExprOpKind_Add
    MG_ExprKind_Binary, // MG_ExprOpKind_Subtract
    MG_ExprKind_Binary, // MG_ExprOpKind_Multiply
    MG_ExprKind_Binary, // MG_ExprOpKind_Divide
    MG_ExprKind_Binary, // MG_ExprOpKind_Modulo
    MG_ExprKind_Binary, // MG_ExprOpKind_LeftShift
    MG_ExprKind_Binary, // MG_ExprOpKind_RightShift
    MG_ExprKind_Binary, // MG_ExprOpKind_BitwiseAnd
    MG_ExprKind_Binary, // MG_ExprOpKind_BitwiseOr
    MG_ExprKind_Binary, // MG_ExprOpKind_BitwiseXor
    MG_ExprKind_Prefix, // MG_ExprOpKind_BitwiseNegate
    MG_ExprKind_Binary, // MG_ExprOpKind_BooleanAnd
    MG_ExprKind_Binary, // MG_ExprOpKind_BooleanOr
    MG_ExprKind_Prefix, // MG_ExprOpKind_BooleanNot
    MG_ExprKind_Binary, // MG_ExprOpKind_Equals
    MG_ExprKind_Binary, // MG_ExprOpKind_DoesNotEqual
};

function MG_Expr* MG_PushExpr(Arena* arena, MG_ExprOpKind op, MD_Node* node)
{
    MG_Expr* expr = push_array(arena, MG_Expr, 1);
    CopyStruct(expr, &mg_str_expr_nil);
    expr->op = op;
    expr->node = node;
    return expr;
}

function MG_NodeArray MG_PushNodeArray(Arena* arena, u64 count)
{
    MG_NodeArray result = { .count = count, .nodes = PushArray(arena, MD_Node*, count) };
    for (u64 idx = 0; idx < result.count; idx += 1)
        result.nodes[idx] = &md_nil_node;
    return result;
}

function StringList MG_ColDescFromTag(Arena* arena, MD_Node* tag)
{
    StringList result = {0};
    u64 count = md_child_count_from_node(tag);
    for (MD_EachNode(cell, tag->first))
        StrListPush(arena, &result, cell->string); // TODO(long): optimize this because the size is known upfront
    Assert(count == result.nodeCount);
    return result;
}

function MG_NodeGrid MG_GridFromNode(Arena* arena, MD_Node* node)
{
    MG_NodeGrid grid = {0};
    
    // rjf: determine dimensions
    u64 row_count = md_child_count_from_node(node);
    u64 column_count = 0;
    for (MD_EachNode(row, node->first))
    {
        u64 cell_count_this_row = md_child_count_from_node(row);
        column_count = Max(column_count, cell_count_this_row);
    }
    
    // rjf: fill grid
    grid.cells = MG_PushNodeArray(arena, row_count * column_count);
    grid.row_parents = MG_PushNodeArray(arena, row_count);
    
    // rjf: fill nodes
    u64 y = 0, x_stride = 1, y_stride = column_count;
    for (MD_EachNode(row, node->first))
    {
        u64 x = 0;
        grid.row_parents.nodes[y] = row;
        for (MD_EachNode(cell, row->first))
        {
            grid.cells.nodes[x*x_stride + y*y_stride] = cell;
            ++x;
        }
        ++y;
    }
    
    return grid;
}

function MG_ExprResult MG_ParseExpr(Arena* arena, MD_Node* first, MD_Node* opl, i8 min_prec)
{
    MG_ExprResult parse = {&mg_str_expr_nil};
    {
        MD_Node* it = first;
        
        //- rjf: consume prefix operators
        MG_Expr* leafmost_op = &mg_str_expr_nil;
        for (;it != opl && !md_node_is_nil(it);)
        {
            MG_ExprOpKind found_op = MG_ExprOpKind_Null;
            for (MG_ExprOpKind op = (MG_ExprOpKind)(MG_ExprOpKind_Null+1);
                 op < MG_ExprOpKind_COUNT;
                 op = (MG_ExprOpKind)(op+1))
            {
                if (mg_str_expr_op_kind_table[op] == MG_ExprKind_Prefix &&
                    StrCompare(it->string, mg_str_expr_op_symbol_string_table[op], 0) &&
                    mg_str_expr_op_precedence_table[op] >= min_prec)
                {
                    found_op = op;
                    break;
                }
            }
            if (found_op != MG_ExprOpKind_Null)
            {
                MG_Expr* op_expr = MG_PushExpr(arena, found_op, it);
                if (leafmost_op == &mg_str_expr_nil)
                {
                    leafmost_op = op_expr;
                }
                op_expr->left = parse.root;
                parse.root = op_expr;
                it = it->next;
            }
            else
            {
                break;
            }
        }
        
        //- rjf: parse atom
        {
            MG_Expr* atom = &mg_str_expr_nil;
            if (it->flags & (MD_NodeFlag_Identifier|MD_NodeFlag_Numeric|MD_NodeFlag_StringLiteral) &&
                md_node_is_nil(it->first))
            {
                atom = MG_PushExpr(arena, MG_ExprOpKind_Null, it);
                it = it->next;
            }
            else if (!md_node_is_nil(it->first))
            {
                MG_ExprResult subparse = MG_ParseExpr(arena, it->first, &md_nil_node, 0);
                atom = subparse.root;
                md_msg_list_concat_in_place(&parse.msgs, &subparse.msgs);
                it = it->next;
            }
            if (leafmost_op != &mg_str_expr_nil)
            {
                leafmost_op->left = atom;
            }
            else
            {
                parse.root = atom;
            }
        }
        
        //- rjf: parse binary operator extensions at this precedence level
        for (;it != opl && !md_node_is_nil(it);)
        {
            // rjf: find binary op kind of `it`
            MG_ExprOpKind found_op = MG_ExprOpKind_Null;
            for (MG_ExprOpKind op = (MG_ExprOpKind)(MG_ExprOpKind_Null+1);
                 op < MG_ExprOpKind_COUNT;
                 op = (MG_ExprOpKind)(op+1))
            {
                if (mg_str_expr_op_kind_table[op] == MG_ExprKind_Binary &&
                    StrCompare(it->string, mg_str_expr_op_symbol_string_table[op], 0) &&
                    mg_str_expr_op_precedence_table[op] >= min_prec)
                {
                    found_op = op;
                    break;
                }
            }
            
            // rjf: good found_op -> build binary expr
            if (found_op != MG_ExprOpKind_Null)
            {
                MG_Expr* op_expr = MG_PushExpr(arena, found_op, it);
                if (leafmost_op == &mg_str_expr_nil)
                {
                    leafmost_op = op_expr;
                }
                op_expr->left = parse.root;
                parse.root = op_expr;
                it = it->next;
            }
            else
            {
                break;
            }
            
            // rjf: parse right hand side of binary operator
            MG_ExprResult subparse = MG_ParseExpr(arena, it, opl, mg_str_expr_op_precedence_table[found_op]+1);
            parse.root->right = subparse.root;
            md_msg_list_concat_in_place(&parse.msgs, &subparse.msgs);
            if (subparse.root == &mg_str_expr_nil)
            {
                md_msg_list_pushf(arena, &parse.msgs, it, MD_MsgKind_Error, "Missing right-hand-side of '%S'.", mg_str_expr_op_symbol_string_table[found_op]);
            }
            it = subparse.next_node;
        }
        
        // rjf: store next node for more caller-side parsing
        parse.next_node = it;
    }
    return parse;
}

function MG_ExprResult MG_ParseExprFromRoot(Arena* arena, MD_Node* root)
{
    MG_ExprResult parse = MG_ParseExpr(arena, root->first, &md_nil_node, 0);
    return parse;
}

internal String MG_StrFromCol(MD_Node* row_parent, StringList descs, U64 idx)
{
    String result = {0};
    
    if (ALWAYS(0 <= idx && idx < descs.nodeCount))
    {
        MD_Node* node = md_child_from_index(row_parent, idx);
        result = node->string;
    }
    
    return result;
}

internal u64 mg_column_index_from_name(StringList descs, String8 name)
{
    u64 result = 0;
    for (StringNode* node = descs.first; node; node = node->next, ++result)
        if (StrCompare(node->string, name, 0))
            return result;
    return 0;
}

function void mg_eval_table_expand_expr__string(Arena* arena, MG_Expr* expr, MG_TableExpandTask* first_task, String8List* out);

internal S64 mg_eval_table_expand_expr__numeric(MG_Expr *expr, MG_TableExpandTask* task)
{
    S64 result = 0;
    MG_ExprOpKind op = expr->op;
    
    switch(op)
    {
        default:
        {
            if(MG_ExprOpKind_FirstString <= op && op <= MG_ExprOpKind_LastString)
            {
                ScratchBegin(scratch);
                String8List result_strs = {0};
                mg_eval_table_expand_expr__string(scratch, expr, task, &result_strs);
                String8 result_str = StrJoin(scratch, &result_strs);
                result = I64FromStrC(result_str, 0);
                ScratchEnd(scratch);
            }
        }break;
        
        case MG_ExprOpKind_Null:
        {
            result = I64FromStrC(expr->node->string, 0);
        }break;
        
        //- rjf: numeric arithmetic binary ops
        case MG_ExprOpKind_Add:
        case MG_ExprOpKind_Subtract:
        case MG_ExprOpKind_Multiply:
        case MG_ExprOpKind_Divide:
        case MG_ExprOpKind_Modulo:
        case MG_ExprOpKind_LeftShift:
        case MG_ExprOpKind_RightShift:
        case MG_ExprOpKind_BitwiseAnd:
        case MG_ExprOpKind_BitwiseOr:
        case MG_ExprOpKind_BitwiseXor:
        case MG_ExprOpKind_BooleanAnd:
        case MG_ExprOpKind_BooleanOr:
        {
            S64 left_val = mg_eval_table_expand_expr__numeric(expr->left, task);
            S64 right_val = mg_eval_table_expand_expr__numeric(expr->right, task);
            switch(op)
            {
                default:break;
                case MG_ExprOpKind_Add:        result = left_val + right_val;  break;
                case MG_ExprOpKind_Subtract:   result = left_val - right_val;  break;
                case MG_ExprOpKind_Multiply:   result = left_val * right_val;  break;
                case MG_ExprOpKind_Divide:     result = left_val / right_val;  break;
                case MG_ExprOpKind_Modulo:     result = left_val % right_val;  break;
                case MG_ExprOpKind_LeftShift:  result = left_val <<right_val; break;
                case MG_ExprOpKind_RightShift: result = left_val >>right_val; break;
                case MG_ExprOpKind_BitwiseAnd: result = left_val & right_val;  break;
                case MG_ExprOpKind_BitwiseOr:  result = left_val | right_val;  break;
                case MG_ExprOpKind_BitwiseXor: result = left_val ^ right_val;  break;
                case MG_ExprOpKind_BooleanAnd: result = left_val &&right_val; break;
                case MG_ExprOpKind_BooleanOr:  result = left_val ||right_val; break;
            }
        }break;
        
        //- rjf: prefix unary ops
        case MG_ExprOpKind_BitwiseNegate:
        case MG_ExprOpKind_BooleanNot:
        {
            S64 right_val = mg_eval_table_expand_expr__numeric(expr->left, task);
            switch(op)
            {
                default:break;
                case MG_ExprOpKind_BitwiseNegate: result = (S64)(~((U64)right_val)); break;
                case MG_ExprOpKind_BooleanNot:    result = !right_val;
            }
        }break;
        
        //- rjf: comparisons
        case MG_ExprOpKind_Equals:
        case MG_ExprOpKind_DoesNotEqual:
        {
            ScratchBegin(scratch);
            String8List left_strs = {0};
            String8List right_strs = {0};
            mg_eval_table_expand_expr__string(scratch, expr->left, task, &left_strs);
            mg_eval_table_expand_expr__string(scratch, expr->right, task, &right_strs);
            String8 left_str = StrJoin(scratch, &left_strs);
            String8 right_str = StrJoin(scratch, &right_strs);
            B32 match = StrCompare(left_str, right_str, 0);
            result = (op == MG_ExprOpKind_Equals ? match : !match);
            ScratchEnd(scratch);
        }break;
    }
    
    return result;
}

function void mg_eval_table_expand_expr__string(Arena* arena, MG_Expr* expr, MG_TableExpandTask* first_task, String8List* out)
{
    MG_ExprOpKind op = expr->op;
    
    switch (op)
    {
        default:
        {
            if (MG_ExprOpKind_FirstNumeric <= op && op <= MG_ExprOpKind_LastNumeric)
            {
                i64 numeric_eval = mg_eval_table_expand_expr__numeric(expr, first_task);
                String8 numeric_eval_stringized = {0};
                if (md_node_has_tag(md_root_from_node(expr->node), str8_lit("hex"), 0))
                    numeric_eval_stringized = StrPushf(arena, "0x%I64x", numeric_eval);
                else
                    numeric_eval_stringized = StrPushf(arena, "%I64d", numeric_eval);
                StrListPush(arena, out, numeric_eval_stringized);
            }
        } break;
        
        case MG_ExprOpKind_Null:
        {
            StrListPush(arena, out, expr->node->string);
        } break;
        
        case MG_ExprOpKind_Dot:
        {
            // rjf: grab left/right
            MG_Expr* left_expr = expr->left;
            MD_Node* left_node = left_expr->node;
            MG_Expr* right_expr = expr->right;
            MD_Node* right_node = right_expr->node;
            
            // rjf: grab table name (LHS of .) and column lookup string (RHS of .)
            String8 expand_label = left_node->string;
            String8 column_lookup = right_node->string;
            
            // rjf: find which task corresponds to this table
            u64 row_idx = 0;
            MG_NodeGrid* grid = 0;
            StringList column_descs = {0};
            {
                for (MG_TableExpandTask* task = first_task; task != 0; task = task->next)
                {
                    if (StrCompare(expand_label, task->expansion_label, 0))
                    {
                        row_idx = task->idx;
                        grid = task->grid;
                        column_descs = task->column_descs;
                        break;
                    }
                }
            }
            
            // rjf: grab row parent
            MD_Node* row_parent = &md_nil_node;
            if (grid && (0 <= row_idx && row_idx < grid->row_parents.count))
            {
                row_parent = grid->row_parents.nodes[row_idx];
            }
            
            // rjf: get string for this table lookup
            String8 lookup_string = {0};
            {
                u64 column_idx = 0;
                
                if (StrCompare(column_lookup, str8_lit("_it"), 0))
                    lookup_string = StrPushf(arena, "%I64u", row_idx);
                else
                {
                    // NOTE(rjf): numeric column lookup (column index)
                    if (right_node->flags & MD_NodeFlag_Numeric)
                    {
                        column_idx = U64FromStrC(column_lookup, 0);
                    }
                    
                    // NOTE(rjf): string column lookup (column name)
                    if (right_node->flags & (MD_NodeFlag_Identifier|MD_NodeFlag_StringLiteral))
                    {
                        column_idx = mg_column_index_from_name(column_descs, column_lookup);
                    }
                    
                    lookup_string = MG_StrFromCol(row_parent, column_descs, column_idx);
                    Assert(StrCompare(lookup_string, str8_lit("--"), 0));
                }
            }
            
            // rjf: push lookup string
            {
                StrListPush(arena, out, lookup_string);
            }
        } break;
        
        case MG_ExprOpKind_ExpandIfTrue:
        {
            i64 bool_value = mg_eval_table_expand_expr__numeric(expr->left, first_task);
            if (bool_value)
            {
                mg_eval_table_expand_expr__string(arena, expr->right, first_task, out);
            }
        } break;
        
        case MG_ExprOpKind_Concat:
        {
            mg_eval_table_expand_expr__string(arena, expr->left, first_task, out);
            mg_eval_table_expand_expr__string(arena, expr->right, first_task, out);
        } break;
        
        case MG_ExprOpKind_BumpToColumn:
        {
            i64 column = mg_eval_table_expand_expr__numeric(expr->left, first_task);
            i64 current_column = out->totalSize;
            i64 spaces_to_push = column - current_column;
            if (spaces_to_push > 0)
            {
                String8 str = {0};
                str.size = spaces_to_push;
                str.str = push_array(arena, U8, spaces_to_push);
                for (i64 idx = 0; idx < spaces_to_push; idx += 1)
                {
                    str.str[idx] = ' ';
                }
                StrListPush(arena, out, str);
            }
        } break;
    }
}

function void MG_TableColLoopExpansion(Arena* arena, String strexpr, MG_TableExpandTask* task, StringList* out)
{
    ScratchBegin(scratch, arena);
    for (u64 it_idx = 0; it_idx < task->count; it_idx += 1)
    {
        task->idx = it_idx;
        
        //- rjf: iterate all further dimensions, if there's left in the chain
        if (task->next)
            MG_TableColLoopExpansion(arena, strexpr, task->next, out);
        
        //- rjf: if this is the last task in the chain, perform expansion
        else
        {
            StringList expansion_strs = {0};
            u64 start = 0;
            for (u64 char_idx = 0; char_idx <= strexpr.size;)
            {
                // rjf: push plain text parts of strexpr
                if (char_idx == strexpr.size || strexpr.str[char_idx] == '$')
                {
                    String plain_text_substr = Substr(strexpr, start, char_idx);
                    start = char_idx;
                    if (plain_text_substr.size)
                        StrListPush(arena, &expansion_strs, plain_text_substr);
                }
                
                // rjf: handle expansion expression
                if (strexpr.str[char_idx] == '$')
                {
                    String string = StrSkip(strexpr, char_idx+1);
                    r1u64 expr_range = {0};
                    i64 paren_nest = 0;
                    for (u64 idx = 0; idx < string.size; idx += 1)
                    {
                        if (string.str[idx] == '(')
                        {
                            paren_nest += 1;
                            if (paren_nest == 1)
                            {
                                expr_range.min = idx;
                            }
                        }
                        if (string.str[idx] == ')')
                        {
                            paren_nest -= 1;
                            if (paren_nest == 0)
                            {
                                expr_range.max = idx+1;
                                break;
                            }
                        }
                    }
                    String expr_string = Substr(string, expr_range.min, expr_range.max);
                    MD_TokenizeResult expr_tokenize = md_tokenize_from_text(scratch, expr_string);
                    MD_ParseResult expr_base_parse = md_parse_from_text_tokens(scratch, StrLit(""), expr_string, expr_tokenize.tokens);
                    MG_ExprResult expr_parse = MG_ParseExprFromRoot(scratch, expr_base_parse.root->first);
                    mg_eval_table_expand_expr__string(arena, expr_parse.root, task, &expansion_strs);
                    char_idx = start = char_idx + 1 + expr_range.max;
                }
                else
                {
                    char_idx += 1;
                }
            }
            String expansion_str = StrJoin(arena, &expansion_strs);
            if (expansion_str.size != 0)
            {
                StrListPush(arena, out, expansion_str);
            }
        }
    }
    ScratchEnd(scratch);
}

function StringList MG_StrListFromTable(Arena* arena, MG_Map grid_name_map, MG_Map grid_column_desc_map, MD_Node* gen)
{
    StringList result = {0};
    ScratchBegin(scratch, arena);
    
    if (md_node_is_nil(gen->first) && gen->string.size != 0)
    {
        StrListPush(arena, &result, gen->string);
        StrListPush(arena, &result, StrLit("\n"));
    }
    else for (MD_EachNode(strexpr_node, gen->first))
    {
        // rjf: build task list
        MG_TableExpandTask* first_task = 0;
        MG_TableExpandTask* last_task = 0;
        for (MD_EachNode(tag, strexpr_node->first_tag))
        {
            if (StrCompare(tag->string, StrLit("expand"), 0))
            {
                // rjf: grab args for this expansion
                String table_name = md_child_from_index(tag, 0)->string;
                String expand_label = md_child_from_index(tag, 1)->string;
                
                // rjf: lookup table / column descriptions
                MG_NodeGrid* grid = MG_MapPtrFromStr(&grid_name_map, table_name);
                StringList* column_descs = MG_MapPtrFromStr(&grid_column_desc_map, table_name);
                
                // rjf: push task for this expansion
                if (grid)
                {
                    MG_TableExpandTask* task = PushStruct(scratch, MG_TableExpandTask);
                    task->expansion_label = expand_label;
                    task->grid = grid;
                    task->column_descs = *column_descs;
                    task->count = MG_GridRowCount(*grid);
                    task->idx = 0;
                    SLLQueuePush(first_task, last_task, task);
                }
            }
        }
        
        // rjf: do expansion generation, OR just push this string if we have no expansions
        {
            if (first_task)
                MG_TableColLoopExpansion(arena, strexpr_node->string, first_task, &result);
            else
                StrListPush(arena, &result, strexpr_node->string);
        }
    }
    
    ScratchEnd(scratch);
    return result;
}

//~ long: C-String Functions

function String MG_StrCEscape(Arena* arena, String string)
{
    // NOTE(rjf): This doesn't handle hex/octal/unicode escape sequences right now, just the simple stuff
    StringList strs = {0};
    u64 start = 0;
    
    ScratchBlock(scratch, arena)
    {
        for (u64 idx = 0; idx <= string.size; idx += 1)
        {
            // NOTE(long): what about `\n`
            if (idx == string.size || string.str[idx] == '\\' || string.str[idx] == '\r')
            {
                String str = Substr(string, start, idx);
                if (str.size)
                    StrListPush(arena, &strs, str);
                start = idx+1;
            }
            
            if (idx < string.size && string.str[idx] == '\\')
            {
                u8 next_char = string.str[idx+1];
                u8 replace_byte = 0;
                
                switch (next_char)
                {
                    default: break;
                    case 'a': replace_byte = 0x07; break;
                    case 'b': replace_byte = 0x08; break;
                    case 'e': replace_byte = 0x1b; break;
                    case 'f': replace_byte = 0x0c; break;
                    case 'n': replace_byte = 0x0a; break;
                    case 'r': replace_byte = 0x0d; break;
                    case 't': replace_byte = 0x09; break;
                    case 'v': replace_byte = 0x0b; break;
                    case '\\':replace_byte = '\\'; break;
                    case '\'':replace_byte = '\''; break;
                    case '"': replace_byte = '"';  break;
                    case '?': replace_byte = '?';  break;
                }
                
                String replace_string = StrCopy(scratch, Str(&replace_byte, 1));
                StrListPush(scratch, &strs, replace_string);
                
                if (replace_byte == '\\' || replace_byte == '"' || replace_byte == '\'')
                {
                    idx += 1;
                    start += 1;
                }
            }
        }
    }
    
    String result = StrJoin(arena, &strs);
    return result;
}

function String MG_StrCFromMultiLine(Arena* arena, String string)
{
    StringList strings = {0};
    StrListPush(arena, &strings, StrLit("\"\"\n"));
    
    for (u64 off = 0, active_line_start_off = 0; off <= string.size; off += 1)
    {
        b32 is_newline = (off < string.size && (string.str[off] == '\n' || string.str[off] == '\r'));
        b32 is_ender = (off >= string.size || is_newline);
        
        if (is_ender)
        {
            String line = Substr(string, active_line_start_off, off);
            StrListPush(arena, &strings, StrLit("\""));
            StrListPush(arena, &strings, line);
            StrListPush(arena, &strings, is_newline ? StrLit("\\n\"\n") : StrLit("\"\n"));
            active_line_start_off = off+1;
        }
        
        if (is_newline && string.str[off] == '\r')
        {
            active_line_start_off += 1;
            off += 1;
        }
    }
    
    String result = StrJoin(arena, &strings);
    return result;
}

function String MG_ArrCFromData(Arena* arena, String data)
{
    StringList strings = {0};
    ScratchBlock(scratch, arena)
    {
        for (u64 off = 0; off < data.size;)
        {
            u64 chunk_size  = Min(data.size-off, 64);
            u8* chunk_bytes = data.str+off;
            
            String chunk_text_string = {0};
            chunk_text_string.size = chunk_size*5;
            chunk_text_string.str = PushArray(arena, u8, chunk_text_string.size);
            
            for (u64 byte_idx = 0; byte_idx < chunk_size; byte_idx += 1)
            {
                String byte_str = StrPushf(scratch, "0x%02x,", chunk_bytes[byte_idx]);
                CopyMem(chunk_text_string.str+byte_idx*5, byte_str.str, byte_str.size);
            }
            off += chunk_size;
            
            StrListPush(arena, &strings, chunk_text_string);
            StrListPush(arena, &strings, StrLit("\n"));
        }
    }
    
    String result = StrJoin(arena, &strings);
    return result;
}
