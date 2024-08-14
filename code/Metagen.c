
//~ long: Map Types

function void* MG_MapPtrFromStr(MG_Map map, String str)
{
    void* result = 0;
    u64 hash = Hash64(str.str, str.size);
    MG_MapSlot* slot = &map.slots[hash % map.count];
    
    for (MG_MapNode* n = slot->first; n; n = n->next)
    {
        if (StrCompare(n->key, str, 0))
        {
            result = n->val;
            break;
        }
    }
    
    return result;
}

function void MG_MapInsert(Arena* arena, MG_Map map, String str, void* val)
{
    u64 hash = Hash64(str.str, str.size);
    MG_MapSlot* slot = &map.slots[hash % map.count];
    MG_MapNode* node = PushStruct(arena, MG_MapNode);
    node->key = StrCopy(arena, str);
    node->val = val;
    SLLQueuePush(slot->first, slot->last,node);
}

//~ long: Expression Parsing Functions

readonly global MG_Expr mg_exprNil = {&mg_exprNil, &mg_exprNil, &mg_exprNil};

readonly global String mg_exprSymbolTable[MG_ExprOpKind_COUNT] =
{
    StrConst(  ""),
    StrConst( "."), // MG_ExprOpKind_Dot
    StrConst("->"), // MG_ExprOpKind_ExpandIfTrue
    StrConst(".."), // MG_ExprOpKind_Concat
    StrConst("=>"), // MG_ExprOpKind_BumpToColumn
    StrConst( "+"), // MG_ExprOpKind_Add
    StrConst( "-"), // MG_ExprOpKind_Subtract
    StrConst( "*"), // MG_ExprOpKind_Multiply
    StrConst( "/"), // MG_ExprOpKind_Divide
    StrConst( "%"), // MG_ExprOpKind_Modulo
    StrConst("<<"), // MG_ExprOpKind_LeftShift
    StrConst(">>"), // MG_ExprOpKind_RightShift
    StrConst( "&"), // MG_ExprOpKind_BitwiseAnd
    StrConst( "|"), // MG_ExprOpKind_BitwiseOr
    StrConst( "^"), // MG_ExprOpKind_BitwiseXor
    StrConst( "~"), // MG_ExprOpKind_BitwiseNegate
    StrConst("&&"), // MG_ExprOpKind_BooleanAnd
    StrConst("||"), // MG_ExprOpKind_BooleanOr
    StrConst( "!"), // MG_ExprOpKind_BooleanNot
    StrConst("=="), // MG_ExprOpKind_Equals
    StrConst("!="), // MG_ExprOpKind_DoesNotEqual
};

readonly global i8 mg_exprPrecedenceTable[MG_ExprOpKind_COUNT] =
{
    0,
    20, // MG_ExprOpKind_Dot
    1,  // MG_ExprOpKind_ExpandIfTrue
    2,  // MG_ExprOpKind_Concat
    12, // MG_ExprOpKind_BumpToColumn
    5,  // MG_ExprOpKind_Add
    5,  // MG_ExprOpKind_Subtract
    6,  // MG_ExprOpKind_Multiply
    6,  // MG_ExprOpKind_Divide
    6,  // MG_ExprOpKind_Modulo
    7,  // MG_ExprOpKind_LeftShift
    7,  // MG_ExprOpKind_RightShift
    8,  // MG_ExprOpKind_BitwiseAnd
    10, // MG_ExprOpKind_BitwiseOr
    9,  // MG_ExprOpKind_BitwiseXor
    11, // MG_ExprOpKind_BitwiseNegate
    3,  // MG_ExprOpKind_BooleanAnd
    3,  // MG_ExprOpKind_BooleanOr
    11, // MG_ExprOpKind_BooleanNot
    4,  // MG_ExprOpKind_Equals
    4,  // MG_ExprOpKind_DoesNotEqual
};

readonly global MG_ExprKind mg_exprOpKindTable[MG_ExprOpKind_COUNT] =
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
    MG_Expr* expr = PushStruct(arena, MG_Expr);
    CopyStruct(expr, &mg_exprNil);
    expr->op = op;
    expr->node = node;
    return expr;
}

function MG_ExprOpKind MG_ExprLookupOp(MD_Node* expr, MG_ExprKind lookup, i8 minPrec)
{
    MG_ExprOpKind result = MG_ExprOpKind_Null;
    for (MG_ExprOpKind op = MG_ExprOpKind_Null + 1; op < MG_ExprOpKind_COUNT; ++op)
    {
        if (mg_exprOpKindTable[op] == lookup &&
            StrCompare(expr->string, mg_exprSymbolTable[op], 0) &&
            mg_exprPrecedenceTable[op] >= minPrec)
        {
            result = op;
            break;
        }
    }
    return result;
}

function MG_ExprResult MG_ParseExpr(Arena* arena, MD_Node* first, MD_Node* opl, i8 minPrec)
{
    MG_ExprResult parse = {&mg_exprNil};
    {
        MD_Node* it = first;
        
        //- rjf: consume prefix operators
        MG_Expr* leafmostOp = &mg_exprNil;
        while (it != opl && !md_node_is_nil(it))
        {
            MG_ExprOpKind op = MG_ExprLookupOp(it, MG_ExprKind_Prefix, minPrec);
            if (op)
            {
                MG_Expr* expr = MG_PushExpr(arena, op, it);
                if (leafmostOp == &mg_exprNil)
                    leafmostOp = expr;
                
                expr->left = parse.root;
                parse.root = expr;
                it = it->next;
            }
            else break;
        }
        
        //- rjf: parse atom
        {
            MG_Expr* atom = &mg_exprNil;
            MD_NodeFlags flags = MD_NodeFlag_Identifier|MD_NodeFlag_Numeric|MD_NodeFlag_StringLiteral;
            if ((it->flags & flags) && md_node_is_nil(it->first))
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
            
            if (leafmostOp != &mg_exprNil)
                leafmostOp->left = atom;
            else
                parse.root = atom;
        }
        
        //- rjf: parse binary operator extensions at this precedence level
        while (it != opl && !md_node_is_nil(it))
        {
            MG_ExprOpKind op = MG_ExprLookupOp(it, MG_ExprKind_Binary, minPrec);
            if (op)
            {
                MG_Expr* expr = MG_PushExpr(arena, op, it);
                if (leafmostOp == &mg_exprNil)
                    leafmostOp = expr;
                
                expr->left = parse.root;
                parse.root = expr;
                it = it->next;
            }
            else break;
            
            // rjf: parse right hand side of binary operator
            MG_ExprResult subparse = MG_ParseExpr(arena, it, opl, mg_exprPrecedenceTable[op]+1);
            parse.root->right = subparse.root;
            md_msg_list_concat_in_place(&parse.msgs, &subparse.msgs);
            if (subparse.root == &mg_exprNil)
                md_msg_list_pushf(arena, &parse.msgs, it, MD_MsgKind_Error, "Missing right-hand-side of '%S'.", mg_exprSymbolTable[op]);
            it = subparse.next;
        }
        
        // rjf: store next node for more caller-side parsing
        parse.next = it;
    }
    
    return parse;
}

function MG_ExprResult MG_ParseExprFromRoot(Arena* arena, MD_Node* root)
{
    MG_ExprResult parse = MG_ParseExpr(arena, root->first, &md_nil_node, 0);
    return parse;
}

//~ long: Table Generation Types

function MG_NodeArray MG_PushNodeArray(Arena* arena, u64 count)
{
    MG_NodeArray result = { .count = count, .nodes = PushArray(arena, MD_Node*, count) };
    for (u64 idx = 0; idx < result.count; ++idx)
        result.nodes[idx] = &md_nil_node;
    return result;
}

function MG_Table* MG_PushTable(Arena* arena, MD_Node* node, MD_Node* tag)
{
    MG_Table* result = PushStruct(arena, MG_Table);
    
    // long: setup the table's grid
    {
        // rjf: determine dimensions
        u64 rowCount = md_child_count_from_node(node);
        u64 colCount = 0;
        for (MD_EachNode(row, node->first))
        {
            u64 cellCount = md_child_count_from_node(row);
            colCount = Max(colCount, cellCount);
        }
        
        // rjf: fill grid
        result->cells = MG_PushNodeArray(arena, rowCount * colCount);
        result->rowParents = MG_PushNodeArray(arena, rowCount);
        
        // rjf: fill nodes
        u64 y = 0, xStride = 1, yStride = colCount;
        for (MD_EachNode(row, node->first))
        {
            u64 x = 0;
            result->rowParents.nodes[y] = row;
            for (MD_EachNode(cell, row->first))
            {
                result->cells.nodes[x*xStride + y*yStride] = cell;
                ++x;
            }
            ++y;
        }
        
    }
    
    // long: setup the table's description
    {
        u64 count = md_child_count_from_node(tag);
        result->members = StrList(arena, 0, count);
        StringNode* strNode = result->members.first;
        for (MD_EachNode(member, tag->first), strNode = strNode->next)
            strNode->string = member->string;
        Assert(count == result->members.nodeCount);
    }
    
    return result;
}

function String MG_StrFromExpansion(Arena* arena, MG_Expr* expr, MG_TableExpandTask* task)
{
    StringList list = {0};
    MG_TableExpandStr(arena, expr, task, &list);
    String result = StrJoin(arena, &list);
    return result;
}

function i64 MG_TableExpandNumeric(MG_Expr* expr, MG_TableExpandTask* task)
{
    i64 result = 0;
    MG_ExprOpKind op = expr->op;
    
    switch (op)
    {
        default:
        {
            if (MG_ExprOpKind_FirstString <= op && op <= MG_ExprOpKind_LastString)
            {
                ScratchBlock(scratch)
                {
                    String str = MG_StrFromExpansion(scratch, expr, task);
                    result = I64FromStrC(str, 0);
                }
            }
        } break;
        
        case MG_ExprOpKind_Null:
        {
            result = I64FromStrC(expr->node->string, 0);
        } break;
        
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
            i64 left = MG_TableExpandNumeric(expr->left, task);
            i64 right = MG_TableExpandNumeric(expr->right, task);
            switch (op)
            {
                default: break;
                case MG_ExprOpKind_Add:        result = left + right;  break;
                case MG_ExprOpKind_Subtract:   result = left - right;  break;
                case MG_ExprOpKind_Multiply:   result = left * right;  break;
                case MG_ExprOpKind_Divide:     result = left / right;  break;
                case MG_ExprOpKind_Modulo:     result = left % right;  break;
                case MG_ExprOpKind_LeftShift:  result = left <<right; break;
                case MG_ExprOpKind_RightShift: result = left >>right; break;
                case MG_ExprOpKind_BitwiseAnd: result = left & right;  break;
                case MG_ExprOpKind_BitwiseOr:  result = left | right;  break;
                case MG_ExprOpKind_BitwiseXor: result = left ^ right;  break;
                case MG_ExprOpKind_BooleanAnd: result = left &&right; break;
                case MG_ExprOpKind_BooleanOr:  result = left ||right; break;
            }
        } break;
        
        //- rjf: prefix unary ops
        case MG_ExprOpKind_BitwiseNegate:
        case MG_ExprOpKind_BooleanNot:
        {
            i64 right = MG_TableExpandNumeric(expr->left, task);
            switch (op)
            {
                default: break;
                case MG_ExprOpKind_BitwiseNegate: result = (i64)(~((u64)right)); break;
                case MG_ExprOpKind_BooleanNot:    result = !right;
            }
        } break;
        
        //- rjf: comparisons
        case MG_ExprOpKind_Equals:
        case MG_ExprOpKind_DoesNotEqual:
        {
            ScratchBlock(scratch)
            {
                String left  = MG_StrFromExpansion(scratch, expr-> left, task);
                String right = MG_StrFromExpansion(scratch, expr->right, task);
                b32 match = StrCompare(left, right, 0);
                result = (op == MG_ExprOpKind_Equals) == match;
            }
        } break;
    }
    
    return result;
}

function void MG_TableExpandStr(Arena* arena, MG_Expr* expr, MG_TableExpandTask* task, StringList* out)
{
    MG_ExprOpKind op = expr->op;
    
    switch (op)
    {
        default:
        {
            if (MG_ExprOpKind_FirstNumeric <= op && op <= MG_ExprOpKind_LastNumeric)
            {
                i64 numeric = MG_TableExpandNumeric(expr, task);
                String numericStr = {0};
                b32 hex = md_node_has_tag(md_root_from_node(expr->node), StrLit("hex"), 0);
                numericStr = StrPushf(arena, hex ? "0x%I64x" : "%I64d", numeric);
                StrListPush(arena, out, numericStr);
            }
        } break;
        
        case MG_ExprOpKind_Null:
        {
            if (expr->node)
                StrListPush(arena, out, expr->node->string);
        } break;
        
        case MG_ExprOpKind_Dot:
        {
            // rjf: grab left/right
            MD_Node* left = expr->left->node;
            MD_Node* right = expr->right->node;
            
            // long: grab the table name/label (LHS of .) and the lookup member (RHS of .)
            // e.g expand(Foo foo) foo.bar
            // foo is the label of the table Foo while bar is a member of the table Foo
            String label = left->string;
            String member = right->string;
            
            // rjf: find which task corresponds to this table
            u64 row = 0;
            MG_Table* table = 0;
            {
                for (MG_TableExpandTask* t = task; t; t = t->next)
                {
                    if (StrCompare(label, t->label, 0))
                    {
                        row = t->idx;
                        table = t->table;
                        break;
                    }
                }
            }
            
            // rjf: get string for this table lookup
            String result = {0};
            {
                u64 memberPos = 0;
                if (StrCompare(member, StrLit("_it"), 0))
                    result = StrPushf(arena, "%I64u", row);
                else
                {
                    // long: From what I can tell, this is for lookup using the member's index
                    // But MG_ParseExpr would never parse `$(a.5)` properly, and RADDBG also never uses it
                    if (NEVER(right->flags & MD_NodeFlag_Numeric))
                        memberPos = U64FromStrC(member, 0);
                    
                    if (table)
                    {
                        // long: lookup using the member's identifier
                        if (right->flags & (MD_NodeFlag_Identifier|MD_NodeFlag_StringLiteral))
                            StrFindList(member, &table->members, 0, &memberPos);
                        
                        if (ALWAYS(0 <= memberPos && memberPos < table->members.nodeCount))
                        {
                            // rjf: grab row parent
                            MD_Node* rowParent = &md_nil_node;
                            if (0 <= row && row < table->rowParents.count)
                                rowParent = table->rowParents.nodes[row];
                            result = md_child_from_index(rowParent, memberPos)->string;
                            
                            // NOTE(long): I don't know what this is for but Ryan was checking for this
                            // After a quick search, it doesn't seem like RADDBG uses this or hits this check
                            // So I'm just gonna assert this for now.
                            NEVER(StrCompare(result, StrLit("--"), 0));
                        }
                    }
                }
            }
            
            // rjf: push lookup string
            ScratchBlock(scratch, arena)
            {
                for (MD_EachNode(tag, left->first_tag))
                {
                    if (StrCompare(tag->string, StrLit("replace"), 0))
                    {
                        String oldStr = StrCEscape(scratch, md_child_from_index(tag, 0)->string);
                        String newStr = StrCEscape(scratch, md_child_from_index(tag, 1)->string);
                        result = StrReplace(scratch, result, oldStr, newStr, 0);
                    }
                    
                    else if (StrCompare(tag->string, StrLit("lower"), 0))
                    {
                        if (md_node_is_nil(tag->first))
                            result = StrToLower(scratch, result);
                        else
                        {
                            StringList* matches = &(StringList){0};
                            for (MD_EachNode(tagArg, tag->first))
                                StrListPush(scratch, matches, tagArg->string);
                            
                            u8* mem = (u8*)scratch;
                            if (!InRange(result.str, mem, mem + scratch->pos))
                                result = StrCopy(scratch, result);
                            
                            for (u64 i = 0; i < result.size; ++i)
                            {
                                StrListIter(matches, node)
                                {
                                    String match  = node->string;
                                    String substr = SubstrRange(result, i, match.size);
                                    if (StrCompare(substr, match, 0))
                                    {
                                        StrToLowerIP(substr);
                                        i += match.size;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    
                    else if (StrCompare(tag->string, StrLit("upper"), 0))
                    {
                        if (md_node_is_nil(tag->first))
                            result = StrToUpper(scratch, result);
                        else
                        {
                            StringList* matches = &(StringList){0};
                            for (MD_EachNode(tagArg, tag->first))
                                StrListPush(scratch, matches, tagArg->string);
                            
                            u8* mem = (u8*)scratch;
                            if (!InRange(result.str, mem, mem + scratch->pos))
                                result = StrCopy(scratch, result);
                            
                            for (u64 i = 0; i < result.size; ++i)
                            {
                                StrListIter(matches, node)
                                {
                                    String match  = node->string;
                                    String substr = SubstrRange(result, i, match.size);
                                    if (StrCompare(substr, match, 0))
                                    {
                                        StrToUpperIP(substr);
                                        i += match.size;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    
                    if (md_node_is_nil(tag->next))
                        result = StrCopy(arena, result);
                }
            }
            
            StrListPush(arena, out, result);
        } break;
        
        case MG_ExprOpKind_ExpandIfTrue:
        {
            b64 val = MG_TableExpandNumeric(expr->left, task);
            if (val)
                MG_TableExpandStr(arena, expr->right, task, out);
        } break;
        
        case MG_ExprOpKind_Concat:
        {
            MG_TableExpandStr(arena, expr-> left, task, out);
            MG_TableExpandStr(arena, expr->right, task, out);
        } break;
        
        case MG_ExprOpKind_BumpToColumn:
        {
            i64 col = MG_TableExpandNumeric(expr->left, task);
            i64 spaces = col - out->totalSize; // totalSize represents the current column count
            if (spaces > 0)
            {
                String result = ChrRepeat(arena, ' ' , spaces);
                StrListPush(arena, out, result);
            }
        } break;
    }
}

function void MG_TableLoopExpansion(Arena* arena, String strexpr, MG_TableExpandTask* task, StringList* out)
{
    ScratchBegin(scratch, arena);
    for (u64 taskIndex = 0; taskIndex < task->count; ++taskIndex)
    {
        task->idx = taskIndex;
        
        //- rjf: iterate all further dimensions, if there's left in the chain
        if (task->next)
            MG_TableLoopExpansion(arena, strexpr, task->next, out);
        
        //- rjf: if this is the last task in the chain, perform expansion
        else
        {
            StringList list = {0};
            u64 start = 0;
            for (u64 strIndex = 0; strIndex <= strexpr.size; )
            {
                // rjf: push plain text parts of strexpr
                if (strIndex == strexpr.size || strexpr.str[strIndex] == '$')
                {
                    String plainText = Substr(strexpr, start, strIndex);
                    start = strIndex;
                    if (plainText.size)
                        StrListPush(arena, &list, plainText);
                }
                
                // rjf: handle expansion expression
                if (strexpr.str[strIndex] == '$')
                {
                    String str = StrSkip(strexpr, strIndex + 1);
                    r1u64 range = {0};
                    for (i64 i = 0, nest = 0; i < (i64)str.size; ++i)
                    {
                        if (str.str[i] == '(')
                        {
                            ++nest;
                            if (nest == 1)
                                range.min = i;
                        }
                        
                        if (str.str[i] == ')')
                        {
                            --nest;
                            if (nest == 0)
                            {
                                range.max = i+1;
                                break;
                            }
                        }
                    }
                    
                    String expr = Substr(str, range.min, range.max);
                    MD_ParseResult parse = MD_ParseText(scratch, StrLit(""), expr);
                    MG_ExprResult exprResult = MG_ParseExprFromRoot(scratch, parse.root->first);
                    MG_TableExpandStr(arena, exprResult.root, task, &list);
                    strIndex = start = strIndex + 1 + range.max;
                }
                else
                    strIndex++;
            }
            
            String result = StrJoin(arena, &list);
            if (result.size)
                StrListPush(arena, out, result);
        }
    }
    ScratchEnd(scratch);
}

function StringList MG_StrListFromTable(Arena* arena, MG_Map tableMap, MD_Node* gen, String expandTag)
{
    StringList result = {0};
    
    ScratchBlock(scratch, arena)
    {
        if (md_node_is_nil(gen->first) && gen->string.size)
        {
            StrListPush(arena, &result, gen->string);
            StrListPush(arena, &result, StrLit("\n"));
        }
        
        else for (MD_EachNode(strexpr_node, gen->first))
        {
            // rjf: build task list
            MG_TableExpandTask* first = 0;
            MG_TableExpandTask* last = 0;
            
            for (MD_EachNode(tag, strexpr_node->first_tag))
            {
                if (StrCompare(tag->string, expandTag, 0))
                {
                    // long: grab args and lookup table for this expansion
                    String name  = md_child_from_index(tag, 0)->string;
                    String label = md_child_from_index(tag, 1)->string;
                    MG_Table* table = MG_MapPtrFromStr(tableMap, name);
                    
                    // rjf: push task for this expansion
                    if (table)
                    {
                        MG_TableExpandTask* task = PushStruct(scratch, MG_TableExpandTask);
                        task->label = label;
                        task->table = table;
                        task->count = MG_TableRowCount(*table);
                        task->idx = 0;
                        SLLQueuePush(first, last, task);
                    }
                }
            }
            
            // rjf: do expansion generation, OR just push this string if we have no expansions
            {
                if (first)
                    MG_TableLoopExpansion(arena, strexpr_node->string, first, &result);
                else
                    StrListPush(arena, &result, strexpr_node->string);
            }
        }
    }
    
    return result;
}

//~ long: C-String Functions

function String MG_StrCFromMultiLine(Arena* arena, String str)
{
    StringList list = {0};
    
    for (u64 off = 0, lineOff = 0; off <= str.size; ++off)
    {
        b32 newline = (off < str.size && (str.str[off] == '\n' || str.str[off] == '\r'));
        
        if (off >= str.size || newline)
        {
            String line = Substr(str, lineOff, off);
            StrListPush(arena, &list, StrLit("\""));
            StrListPush(arena, &list, line);
            StrListPush(arena, &list, newline ? StrLit("\\n\"\n") : StrLit("\""));
            lineOff = off+1;
        }
        
        if (newline && str.str[off] == '\r')
        {
            ++lineOff;
            ++off;
        }
    }
    
    String result = StrJoin(arena, &list);
    return result;
}

function String MG_ArrCFromData(Arena* arena, String data)
{
    StringList list = {0};
    ScratchBlock(scratch, arena)
    {
        for (u64 off = 0; off < data.size; )
        {
            u64 chunkSize  = Min(data.size - off, 64);
            u8* chunkBytes = data.str + off;
            
            String str = StrPush(arena, chunkSize * 5, 0);
            
            for (u64 i = 0; i < chunkSize; ++i)
            {
                String bytes = StrPushf(scratch, "0x%02x,", chunkBytes[i]);
                CopyMem(str.str + i * 5, bytes.str, bytes.size);
            }
            off += chunkSize;
            
            StrListPush(arena, &list, str);
            StrListPush(arena, &list, StrLit("\n"));
        }
    }
    
    String result = StrJoin(arena, &list);
    return result;
}
