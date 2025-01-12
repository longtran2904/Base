
//~ long: Lexing Functions

function TokenArray CL_TokenArrayFromStr(Arena* arena, String text)
{
    Scanner* scanner = &ScannerFromStr(arena, text);
    
    MarkerPushRange(scanner, CL_TokenFlag_Comment, StrLit("/*"), MarkerFlag_InverseStr);
    MarkerPushLine (scanner, CL_TokenFlag_Comment, StrLit("//"), '\\', 0);
    MarkerPushLine (scanner, CL_TokenFlag_Preproc, StrLit( "#"), '\\', 0);
    MarkerPushArr  (scanner, CL_TokenFlag_Whitespace, StrLit(SpaceStr"\r\n"), 0);
    
    MarkerPushLine(scanner, CL_TokenFlag_String, StrLit("\""), '\\', 1);
    MarkerPushLine(scanner,   CL_TokenFlag_Char,  StrLit("'"), '\\', 1);
    
    MarkerPushIdent (scanner, CL_TokenFlag_Identifier);
    MarkerPushNumber(scanner, CL_TokenFlag_Numeric, StrLit("eEpP"));
    
    MarkerPushOpsCombine(scanner, CL_TokenFlag_Symbol, StrLit("<>"), '=', 1); // <= >= <<= >>=
    MarkerPushOpsCombine(scanner, CL_TokenFlag_Symbol, StrLit("&|*/+-!^%*/"), '=', 0); // &= |= *= /= += -= != etc
    ScannerPushMark(scanner, CL_TokenFlag_Symbol, StrLit("..."), 0);
    ScannerPushMark(scanner, CL_TokenFlag_Symbol, StrLit("->"), 0);
    MarkerPushOps(scanner, CL_TokenFlag_Symbol, StrLit("<>&|+-="), 1);
    MarkerPushOps(scanner, CL_TokenFlag_Symbol, StrLit("[](){0},;:?~!^%."), 0);
    
    // C
    // 1 - 2 - 1+Equal - 2+Equal: < >
    // 1 - 2 - 1+Equal: & | + -
    // 1 - 1+Equal: * / ! ^ % =
    // 1 - 3: .
    // 1: [ ] ( ) { } , ; : ? ~
    // 1+1: ->
    
    // C#
    // 1 - 2: . :
    // 1 - 2 - 2+Equal: ?
    // 1+1: ?. ?[ !. =>
    
    // GCC: 1+1: ({ }) ?:
    
    ScratchBegin(scratch, arena);
    TokenChunkList tokens = {0};
    
    while (scanner->pos < scanner->source.size)
    {
        Token token = ScannerNext(scanner);
        if (token.flags & ScanResultFlag_TokenUnclosed)
            token.user |= CL_TokenFlag_Broken;
        
        TokenChunkListPush(scratch, &tokens, 4096, token);
    }
    
    TokenArray result = TokenArrayFromChunkList(arena, &tokens);
    ScratchEnd(scratch);
    return result;
}

//#define Long_CS_ParseStr(ctx, str)             Long_Index_ParseStr    (ctx, str)
//#define Long_CS_ParseToken(ctx, str)           Long_Index_ParseStr    (ctx, S8Lit(str))
//#define Long_CS_ParseKind(ctx, kind, range)    Long_Index_ParseKind   (ctx, kind, range)
//#define Long_CS_ParseSubKind(ctx, kind, range) Long_Index_ParseSubKind(ctx, kind, range)
//#define Long_CS_PeekToken(ctx, str)            Long_Index_PeekStr     (ctx, S8Lit(str))
//#define Long_CS_PeekTwo(ctx, strA, strB)       Long_CS_PeekTwoStr     (ctx, S8Lit(strA), S8Lit(strB))
// Token string, token kind, soft token, op token

global String CL_Keywords[] = {
    StrConst("alignas"),      StrConst("alignof"),  StrConst("auto"),          StrConst("bool"),      StrConst("break"),
    StrConst("case"),         StrConst("char"),     StrConst("const"),         StrConst("constexpr"), StrConst("continue"),
    StrConst("default"),      StrConst("do"),       StrConst("double"),        StrConst("else"),      StrConst("enum"),
    StrConst("extern"),       StrConst("false"),    StrConst("float"),         StrConst("for"),       StrConst("goto"),
    StrConst("if"),           StrConst("inline"),   StrConst("int"),           StrConst("long"),      StrConst("nullptr"),
    StrConst("register"),     StrConst("restrict"), StrConst("return"),        StrConst("short"),     StrConst("signed"),
    StrConst("sizeof"),       StrConst("static"),   StrConst("static_assert"), StrConst("struct"),    StrConst("switch"),
    StrConst("thread_local"), StrConst("true"),     StrConst("typedef"),       StrConst("typeof"),    StrConst("typeof_unqual"),
    StrConst("union"),        StrConst("unsigned"), StrConst("void"),          StrConst("volatile"),  StrConst("while"),
};

global String CL_TypeKewords[] = { StrConst("struct"), StrConst("union"), StrConst("enum") };

global String CL_Qualifiers[] = {
    StrConst("const"), StrConst("volatile"), StrConst("restrict"), StrConst("_Atomic"),
};

global String CL_Specifiers[] = {
    StrConst("typedef"), StrConst("constexpr"), StrConst("auto"), StrConst("register"), StrConst("static"),
    StrConst("extern"), StrConst("_Thread_local"), StrConst("inline"), StrConst("_Noreturn"), StrConst("_Alignas"),
};

#define MD_NodeMatch(node, str) StrCompare((node)->string, StrLit(str), 0)

function b32 MD_NodeCompareArr(MD_Node* node, String* array, u64 count)
{
    for (u64 i = 0; i < count; ++i)
        if (StrCompare(node->string, array[i], 0))
            return 1;
    return 0;
}

function u64 CL_NodePtrCount(CL_Node* node)
{
    u64 ptrCount = 0;
    MD_Node* iter = node->base->next;
    String name = node->name->string;
    
    REPEAT:
    while (iter->flags & MD_NodeFlag_Symbol)
    {
        for (u64 i = 0; i < iter->string.size; ++i)
        {
            if (iter->string.str[i] == '*')
                ptrCount++;
            else
                goto DONE;
        }
        iter = iter->next;
    }
    
    if (MD_NodeCompareArr(iter, CL_Qualifiers, ArrayCount(CL_Qualifiers)))
    {
        iter = iter->next;
        goto REPEAT;
    }
    
    if (!StrCompare(iter->string, name, 0))
    {
        ptrCount = 0;
        do {
            iter = iter->next;
            if (md_node_is_nil(iter))
                goto DONE;
        } while (!(iter->flags & MD_NodeFlag_IsAfterComma));
        
        goto REPEAT;
    }
    
    DONE:
    return ptrCount;
}

// TODO(long): The base node can be a qualifiers
function MD_Node* CL_NodeSkipBase(MD_Node* node)
{
    while (1)
    {
        b32 repeat = 0;
        if (node->flags & MD_NodeFlag_Symbol)
        {
            for (u64 i = 0; i < node->string.size; ++i)
                if (node->string.str[i] != '*')
                    goto DONE;
            repeat = 1;
        }
        else if (MD_NodeCompareArr(node, CL_Qualifiers, ArrayCount(CL_Qualifiers)))
            repeat = 1;
        
        if (HasAnyFlags(node->flags, MD_NodeFlag_IsAfterSemicolon|MD_NodeFlag_IsAfterComma))
            break;
        if (md_node_is_nil(node->next))
            break;
        if (!repeat)
            break;
        node = node->next;
    }
    
    DONE:
    return node;
}

#define MD_PushNode(arena, flags, str, off) md_push_node(arena, MD_NodeKind_Main, (flags), str, str, off)

function void MD_DebugTree(MD_Node* root)
{
    for (MD_EachNode(node, root->first))
    {
        Assert(node->next      != node);
        Assert(node->prev      != node);
        Assert(node->parent    != node);
        Assert(node->first     != node);
        Assert(node->last      != node);
        Assert(node->first_tag != node);
        Assert(node->last_tag  != node);
        
        Assert(md_node_is_nil(node->first->prev));
        Assert(md_node_is_nil(node->last->next));
        if (node->first == node->last)
            Assert(md_node_is_nil(node->first->next));
        
        // For some stupid reason, MD forces first/last tag to be null
        if (node->first_tag == 0)
        {
            Assert(node->last_tag == 0);
            node->first_tag = node->last_tag = &md_nil_node;
        }
        
        Assert(md_node_is_nil(node->first_tag->prev));
        Assert(md_node_is_nil(node->last_tag->next));
        if (node->first_tag == node->last_tag)
            Assert(md_node_is_nil(node->first_tag->next));
        
        if (!md_node_is_nil(node->next))
        {
            Assert(node->next->prev == node);
            Assert(node->next->parent == node->parent);
            
            if (!md_node_is_nil(node->first))
            {
                Assert(node->first != node->next->first && node->first != node->next->last);
                Assert(node-> next != node->next->first && node-> next != node->next->last);
            }
        }
        
        MD_DebugTree(node->first);
        MD_DebugTree(node->first_tag);
    }
}

#if 0
function MD_Node* MD_ParseFromTokens(Arena* arena, String text, TokenArray array)
{
    ScratchBegin(scratch, arena);
    
    //- rjf: set up outputs
    MD_MsgList msgs = {0};
    MD_Node* root = md_push_node(arena, MD_NodeKind_File, 0, , text, 0);
    
    //- rjf: set up parse rule stack
    typedef enum MD_ParseWorkKind
    {
        MD_ParseWorkKind_Main,
        MD_ParseWorkKind_MainImplicit,
        MD_ParseWorkKind_NodeOptionalFollowUp,
        MD_ParseWorkKind_NodeChildrenStyleScan,
    }
    MD_ParseWorkKind;
    typedef struct MD_ParseWorkNode MD_ParseWorkNode;
    struct MD_ParseWorkNode
    {
        MD_ParseWorkNode* next;
        MD_ParseWorkKind kind;
        MD_Node* parent;
        MD_Node* first_gathered_tag;
        MD_Node* last_gathered_tag;
        MD_NodeFlags gathered_node_flags;
        S32 counted_newlines;
    };
    MD_ParseWorkNode first_work =
    {
        0,
        MD_ParseWorkKind_Main,
        root,
    };
    MD_ParseWorkNode broken_work = { 0, MD_ParseWorkKind_Main, root,};
    MD_ParseWorkNode* work_top = &first_work;
    MD_ParseWorkNode* work_free = 0;
#define MD_ParseWorkPush(work_kind, work_parent) do\
    {\
        MD_ParseWorkNode* work_node = work_free;\
        if(work_node == 0) {work_node = push_array(scratch.arena, MD_ParseWorkNode, 1);}\
        else { SLLStackPop(work_free); }\
        work_node->kind = (work_kind);\
        work_node->parent = (work_parent);\
        SLLStackPush(work_top, work_node);\
    }while(0)
#define MD_ParseWorkPop() do\
    {\
        SLLStackPop(work_top);\
        if(work_top == 0) {work_top = &broken_work;}\
    }while(0)
    
    //- rjf: parse
    MD_Token* tokens_first = tokens.v;
    MD_Token* tokens_opl = tokens_first + tokens.count;
    MD_Token* token = tokens_first;
    for(;token < tokens_opl;)
    {
        //- rjf: unpack token
        String8 token_string = str8_substr(text, token[0].range);
        
        //- rjf: whitespace -> always no-op & inc
        if(token->flags & MD_TokenFlag_Whitespace)
        {
            token += 1;
            goto end_consume;
        }
        
        //- rjf: comments -> always no-op & inc
        if(token->flags & MD_TokenGroup_Comment)
        {
            token += 1;
            goto end_consume;
        }
        
        //- rjf: [node follow up] : following label -> work top parent has children. we need
        // to scan for explicit delimiters, else parse an implicitly delimited set of children
        if(work_top->kind == MD_ParseWorkKind_NodeOptionalFollowUp && str8_match(token_string, str8_lit(":"), 0))
        {
            MD_Node* parent = work_top->parent;
            MD_ParseWorkPop();
            MD_ParseWorkPush(MD_ParseWorkKind_NodeChildrenStyleScan, parent);
            token += 1;
            goto end_consume;
        }
        
        //- rjf: [node follow up] anything but : following label -> node has no children. just
        // pop & move on
        if(work_top->kind == MD_ParseWorkKind_NodeOptionalFollowUp)
        {
            MD_ParseWorkPop();
            goto end_consume;
        }
        
        //- rjf: [main] separators -> mark & inc
        if(work_top->kind == MD_ParseWorkKind_Main && token->flags & MD_TokenFlag_Reserved &&
           (str8_match(token_string, str8_lit(","), 0) ||
            str8_match(token_string, str8_lit(";"), 0)))
        {
            MD_Node* parent = work_top->parent;
            if(!md_node_is_nil(parent->last))
            {
                parent->last->flags |=     MD_NodeFlag_IsBeforeComma*!!str8_match(token_string, str8_lit(","), 0);
                parent->last->flags |= MD_NodeFlag_IsBeforeSemicolon*!!str8_match(token_string, str8_lit(";"), 0);
                work_top->gathered_node_flags |=     MD_NodeFlag_IsAfterComma*!!str8_match(token_string, str8_lit(","), 0);
                work_top->gathered_node_flags |= MD_NodeFlag_IsAfterSemicolon*!!str8_match(token_string, str8_lit(";"), 0);
            }
            token += 1;
            goto end_consume;
        }
        
        //- rjf: [main_implicit] separators -> pop
        if(work_top->kind == MD_ParseWorkKind_MainImplicit && token->flags & MD_TokenFlag_Reserved &&
           (str8_match(token_string, str8_lit(","), 0) ||
            str8_match(token_string, str8_lit(";"), 0)))
        {
            MD_ParseWorkPop();
            goto end_consume;
        }
        
        //- rjf: [main, main_implicit] unexpected reserved tokens
        if((work_top->kind == MD_ParseWorkKind_Main || work_top->kind == MD_ParseWorkKind_MainImplicit) &&
           token->flags & MD_TokenFlag_Reserved &&
           (str8_match(token_string, str8_lit("#"), 0) ||
            str8_match(token_string, str8_lit("\\"), 0) ||
            str8_match(token_string, str8_lit(":"), 0)))
        {
            MD_Node* error = md_push_node(arena, MD_NodeKind_ErrorMarker, 0, token_string, token_string, token->range.min);
            String8 error_string = push_str8f(arena, "Unexpected reserved symbol \"%.*s\".", StrExpand(token_string));
            md_msg_list_push(arena, &msgs, error, MD_MsgKind_Error, error_string);
            token += 1;
            goto end_consume;
        }
        
        //- rjf: [main, main_implicit] tag signifier -> create new tag
        if((work_top->kind == MD_ParseWorkKind_Main || work_top->kind == MD_ParseWorkKind_MainImplicit) &&
           token[0].flags & MD_TokenFlag_Reserved && str8_match(token_string, str8_lit("@"), 0))
        {
            if(token+1 >= tokens_opl ||
               !(token[1].flags & MD_TokenGroup_Label))
            {
                MD_Node* error = md_push_node(arena, MD_NodeKind_ErrorMarker, 0, token_string, token_string, token->range.min);
                String8 error_string = str8_lit("Tag label expected after @ symbol.");
                md_msg_list_push(arena, &msgs, error, MD_MsgKind_Error, error_string);
                token += 1;
                goto end_consume;
            }
            else
            {
                String8 tag_name_raw = str8_substr(text, token[1].range);
                String8 tag_name = md_content_string_from_token_flags_str8(token[1].flags, tag_name_raw);
                MD_Node* node = md_push_node(arena, MD_NodeKind_Tag, md_node_flags_from_token_flags(token[1].flags), tag_name, tag_name_raw, token[0].range.min);
                DLLPushBack_NPZ(&md_nil_node, work_top->first_gathered_tag, work_top->last_gathered_tag, node, next, prev);
                if(token+2 < tokens_opl && token[2].flags & MD_TokenFlag_Reserved && str8_match(str8_substr(text, token[2].range), str8_lit("("), 0))
                {
                    token += 3;
                    MD_ParseWorkPush(MD_ParseWorkKind_Main, node);
                }
                else
                {
                    token += 2;
                }
                goto end_consume;
            }
        }
        
        //- rjf: [main, main_implicit] label -> create new main
        if((work_top->kind == MD_ParseWorkKind_Main || work_top->kind == MD_ParseWorkKind_MainImplicit) &&
           token->flags & MD_TokenGroup_Label)
        {
            String8 node_string_raw = token_string;
            String8 node_string = md_content_string_from_token_flags_str8(token->flags, node_string_raw);
            MD_NodeFlags flags = md_node_flags_from_token_flags(token->flags)|work_top->gathered_node_flags;
            work_top->gathered_node_flags = 0;
            MD_Node* node = md_push_node(arena, MD_NodeKind_Main, flags, node_string, node_string_raw, token[0].range.min);
            node->first_tag = work_top->first_gathered_tag;
            node->last_tag = work_top->last_gathered_tag;
            for(MD_Node* tag = work_top->first_gathered_tag; !md_node_is_nil(tag); tag = tag->next)
            {
                tag->parent = node;
            }
            work_top->first_gathered_tag = work_top->last_gathered_tag = &md_nil_node;
            md_node_push_child(work_top->parent, node);
            MD_ParseWorkPush(MD_ParseWorkKind_NodeOptionalFollowUp, node);
            token += 1;
            goto end_consume;
        }
        
        //- rjf: [main] {s, [s, and (s -> create new main
        if(work_top->kind == MD_ParseWorkKind_Main && token->flags & MD_TokenFlag_Reserved &&
           (str8_match(token_string, str8_lit("{"), 0) ||
            str8_match(token_string, str8_lit("["), 0) ||
            str8_match(token_string, str8_lit("("), 0)))
        {
            MD_NodeFlags flags = md_node_flags_from_token_flags(token->flags)|work_top->gathered_node_flags;
            flags |=   MD_NodeFlag_HasBraceLeft*!!str8_match(token_string, str8_lit("{"), 0);
            flags |= MD_NodeFlag_HasBracketLeft*!!str8_match(token_string, str8_lit("["), 0);
            flags |=   MD_NodeFlag_HasParenLeft*!!str8_match(token_string, str8_lit("("), 0);
            work_top->gathered_node_flags = 0;
            MD_Node* node = md_push_node(arena, MD_NodeKind_Main, flags, str8_lit(""), str8_lit(""), token[0].range.min);
            node->first_tag = work_top->first_gathered_tag;
            node->last_tag = work_top->last_gathered_tag;
            for(MD_Node* tag = work_top->first_gathered_tag; !md_node_is_nil(tag); tag = tag->next)
            {
                tag->parent = node;
            }
            work_top->first_gathered_tag = work_top->last_gathered_tag = &md_nil_node;
            md_node_push_child(work_top->parent, node);
            MD_ParseWorkPush(MD_ParseWorkKind_Main, node);
            token += 1;
            goto end_consume;
        }
        
        //- rjf: [node children style scan] {s, [s, and (s -> explicitly delimited children
        if(work_top->kind == MD_ParseWorkKind_NodeChildrenStyleScan && token->flags & MD_TokenFlag_Reserved &&
           (str8_match(token_string, str8_lit("{"), 0) ||
            str8_match(token_string, str8_lit("["), 0) ||
            str8_match(token_string, str8_lit("("), 0)))
        {
            MD_Node* parent = work_top->parent;
            parent->flags |=   MD_NodeFlag_HasBraceLeft*!!str8_match(token_string, str8_lit("{"), 0);
            parent->flags |= MD_NodeFlag_HasBracketLeft*!!str8_match(token_string, str8_lit("["), 0);
            parent->flags |=   MD_NodeFlag_HasParenLeft*!!str8_match(token_string, str8_lit("("), 0);
            MD_ParseWorkPop();
            MD_ParseWorkPush(MD_ParseWorkKind_Main, parent);
            token += 1;
            goto end_consume;
        }
        
        //- rjf: [node children style scan] count newlines
        if(work_top->kind == MD_ParseWorkKind_NodeChildrenStyleScan && token->flags & MD_TokenFlag_Newline)
        {
            work_top->counted_newlines += 1;
            token += 1;
            goto end_consume;
        }
        
        //- rjf: [main_implicit] newline -> pop
        if(work_top->kind == MD_ParseWorkKind_MainImplicit && token->flags & MD_TokenFlag_Newline)
        {
            MD_ParseWorkPop();
            token += 1;
            goto end_consume;
        }
        
        //- rjf: [all but main_implicit] newline -> no-op & inc
        if(work_top->kind != MD_ParseWorkKind_MainImplicit && token->flags & MD_TokenFlag_Newline)
        {
            token += 1;
            goto end_consume;
        }
        
        //- rjf: [node children style scan] anything causing implicit set -> <2 newlines, all good,
        // >=2 newlines, houston we have a problem
        if(work_top->kind == MD_ParseWorkKind_NodeChildrenStyleScan)
        {
            if(work_top->counted_newlines >= 2)
            {
                MD_Node* node = work_top->parent;
                MD_Node* error = md_push_node(arena, MD_NodeKind_ErrorMarker, 0, token_string, token_string, token->range.min);
                String8 error_string = push_str8f(arena, "More than two newlines following \"%.*s\", which has implicitly-delimited children, resulting in an empty list of children.", StrExpand(node->string));
                md_msg_list_push(arena, &msgs, error, MD_MsgKind_Warning, error_string);
                MD_ParseWorkPop();
            }
            else
            {
                MD_Node* parent = work_top->parent;
                MD_ParseWorkPop();
                MD_ParseWorkPush(MD_ParseWorkKind_MainImplicit, parent);
            }
            goto end_consume;
        }
        
        //- rjf: [main] }s, ]s, and )s -> pop
        if(work_top->kind == MD_ParseWorkKind_Main && token->flags & MD_TokenFlag_Reserved &&
           (str8_match(token_string, str8_lit("}"), 0) ||
            str8_match(token_string, str8_lit("]"), 0) ||
            str8_match(token_string, str8_lit(")"), 0)))
        {
            MD_Node* parent = work_top->parent;
            parent->flags |=   MD_NodeFlag_HasBraceRight*!!str8_match(token_string, str8_lit("}"), 0);
            parent->flags |= MD_NodeFlag_HasBracketRight*!!str8_match(token_string, str8_lit("]"), 0);
            parent->flags |=   MD_NodeFlag_HasParenRight*!!str8_match(token_string, str8_lit(")"), 0);
            MD_ParseWorkPop();
            token += 1;
            goto end_consume;
        }
        
        //- rjf: [main implicit] }s, ]s, and )s -> pop without advancing
        if(work_top->kind == MD_ParseWorkKind_MainImplicit && token->flags & MD_TokenFlag_Reserved &&
           (str8_match(token_string, str8_lit("}"), 0) ||
            str8_match(token_string, str8_lit("]"), 0) ||
            str8_match(token_string, str8_lit(")"), 0)))
        {
            MD_ParseWorkPop();
            goto end_consume;
        }
        
        //- rjf: no consumption -> unexpected token! we don't know what to do with this.
        {
            MD_Node* error = md_push_node(arena, MD_NodeKind_ErrorMarker, 0, token_string, token_string, token->range.min);
            String8 error_string = push_str8f(arena, "Unexpected \"%.*s\" token.", StrExpand(token_string));
            md_msg_list_push(arena, &msgs, error, MD_MsgKind_Error, error_string);
            token += 1;
        }
        
        end_consume:;
    }
    
    //- rjf: fill & return
    MD_ParseResult result = {0};
    result.root = root;
    result.msgs = msgs;
    
    ScratchEnd(scratch);
    return result;
}
MD_ParseWorkNode* nil;
#endif

function b32 MD_NodeIsPtr(MD_Node* node)
{
    b32 result = (node->flags & MD_NodeFlag_Symbol) && md_node_is_nil(node->first);
    for (u64 i = 0; i < node->string.size && result; ++i)
        result = node->string.str[i] != '*';
    return result;
}

function void MD_ReinsertNode(MD_Node* node, MD_Node* parent)
{
    DLLRemove_NPZ(&md_nil_node, node->parent->first, node->parent->last, node, next, prev);
    md_node_push_child(parent, node);
}

function void MD_NodePushTag(MD_Node* first, MD_Node* last, MD_Node* parent)
{
    Assert(md_node_is_nil(parent->first_tag));
    parent->first_tag = first;
    parent-> last_tag = last;
    
    if (!md_node_is_nil(first->prev)) first->prev->next = last->next;
    if (!md_node_is_nil(last->next)) last->next->prev = first->prev;
    first->prev = &md_nil_node;
    last->next = &md_nil_node;
    
    for (MD_EachNode(tag, first))
    {
        Assert(md_node_is_nil(tag->first));
        tag->parent = parent;
    }
}

function void MD_SkipPreproc(MD_Node* root, String text)
{
    MD_Node* node = root->first;
    b32 begin = 1;
    
    for (u64 i = 0; i < text.size && !md_node_is_nil(node); ++i)
    {
        char c = text.str[i];
        
        if (c == '\n')
        {
            begin = 1;
            
            REPEAT:
            while (!md_node_is_nil(node->next) && node->next->src_offset < i)
                node = node->next;
            
            if (node->src_offset < i)
            {
                if (!md_node_is_nil(node->first))
                {
                    node = node->first;
                    goto REPEAT;
                }
                
                else if (md_node_is_nil(node->next))
                {
                    while (md_node_is_nil(node->next) && !md_node_is_nil(node->parent))
                        node = node->parent;
                    node = node->next;
                    if (!md_node_is_nil(node))
                        goto REPEAT;
                }
                
                else node = node->next;
            }
        }
        
        else if (c == '#' && begin)
        {
            for (u64 j = i + 1; j < text.size; ++j)
            {
                b32 wasSlash = text.str[j-1] == '\\' || (text.str[j-1] == '\r' && text.str[j-2] == '\\');
                if (text.str[j] == '\n' && !wasSlash)
                {
                    MD_Node* first = node;
                    while (!md_node_is_nil(node->next) && node->next->src_offset < j)
                        node = node->next;
                    
                    MD_Node* parent = node->parent;
                    if (parent->first == first)
                    {
                        parent->first = node->next;
                        parent->first->prev = &md_nil_node;
                    }
                    
                    else if (parent->last == node)
                    {
                        parent->last = first->prev;
                        parent->last->next = &md_nil_node;
                    }
                    
                    else
                    {
                        first->prev->next = node->next;
                        node->next->prev = first->prev;
                        
                        MD_NodeFlags flags = MD_NodeFlag_IsBeforeComma|MD_NodeFlag_IsBeforeSemicolon;
                        node->next->flags |= MD_NodeFlag_AfterFromBefore(first->prev->flags & flags);
                        // TODO(long): The opposite way (after comma/semicolon) is harder to handler
                    }
                    
                    i = j - 1;
                    break;
                }
            }
        }
        
        else if (!IsWspace(c))
            begin = 0;
    }
}

function MD_Node* MD_ParseStrC(Arena* arena, String text)
{
    MD_ParseResult result = MD_ParseText(arena, StrLit(""), text);
    MD_SkipPreproc(result.root, text);
    MD_DebugTree(result.root);
    
    local const String CL_declTags[] = {
        StrConst("struct"), StrConst("union"), StrConst("enum"),
        StrConst("const"), StrConst("volatile"), StrConst("restrict"), StrConst("_Atomic"),
        StrConst("typedef"), StrConst("constexpr"), StrConst("auto"), StrConst("register"), StrConst("static"),
        StrConst("extern"), StrConst("_Thread_local"), StrConst("inline"), StrConst("_Noreturn"), StrConst("_Alignas"),
    };
    
    for (MD_EachNode(node, result.root->first))
    {
        MD_Node* firstTag = 0;
        MD_Node*  lastTag = 0;
        MD_NodeFlags beforeFlags = MD_NodeFlag_IsBeforeComma|MD_NodeFlag_IsBeforeSemicolon;
        MD_NodeFlags  afterFlags =  MD_NodeFlag_IsAfterComma| MD_NodeFlag_IsAfterSemicolon;
        
        while (MD_NodeCompareArr(node, (String*)CL_declTags, ArrayCount(CL_declTags)) && !(node->flags & beforeFlags))
        {
            if (!firstTag)
                firstTag = lastTag = node;
            else
                lastTag = node;
            node = node->next;
        }
        
        if (firstTag)
        {
            MD_Node* parent = 0;
            b32 beg = md_node_is_nil(firstTag->prev) || (firstTag->flags & afterFlags);
            b32 end = md_node_is_nil(lastTag->next) || (lastTag->flags & beforeFlags);
            if (!beg && (MD_NodeMatch(firstTag->prev, "*") || (firstTag->prev->flags & MD_NodeFlag_Identifier)))
                MD_NodePushTag(firstTag, lastTag, firstTag->prev);
            else if (!end && (lastTag->next->flags & MD_NodeFlag_Identifier))
                MD_NodePushTag(firstTag, lastTag, lastTag->next);
        }
        
        if (MD_NodeIsPtr(node))
        {
            MD_Node* lastPtr = node;
            for (u64 i = 0; i < node->string.size - 1; ++i)
            {
                MD_Node* ptr = MD_PushNode(arena, MD_NodeFlag_Symbol, StrLit("*"), node->src_offset + i);
                if (lastPtr == node)
                    lastPtr = ptr;
                
                Assert(node->first == node->last);
                if (!md_node_is_nil(node->first))
                    md_node_push_child(ptr, node->first);
                md_node_push_child(node, ptr);
            }
            
            node->src_offset += node->string.size - 1;
            node->string = StrPostfix(node->string, 1);
            
            MD_Node* prev = node->prev;
            // TODO(long): Handle: int a, *** b
            if ((prev->flags & MD_NodeFlag_Identifier) && NoFlags(node->flags, afterFlags))
                MD_ReinsertNode(prev, lastPtr);
        }
        
        else if ((node->flags & MD_NodeFlag_MaskSetDelimiters) && !(node->flags & afterFlags) &&
                 (node->prev->flags & MD_NodeFlag_Identifier))
        {
            node->prev->flags |= (node->flags & beforeFlags);
            MD_ReinsertNode(node, node->prev);
            if (!md_node_is_nil(node->first))
                node = node->first;
        }
        
        else if (((node->flags & beforeFlags) || md_node_is_nil(node->next)) && !(node->flags & afterFlags) &&
                 ((node->prev->flags & MD_NodeFlag_Identifier) || MD_NodeMatch(node->prev, "*")))
            MD_ReinsertNode(node->prev, node);
        
        while (md_node_is_nil(node->next) && !md_node_is_nil(node->parent))
            node = node->parent;
    }
    
    return result.root;
}

function CL_Node* CL_PushNode(Arena* arena, CL_NodeType type, MD_Node* name, MD_Node* base, CL_Node* parent)
{
    CL_Node* result = PushStruct(arena, CL_Node);
    result->next = &cl_nilNode;
    result->prev = &cl_nilNode;
    result->parent = &cl_nilNode;
    result->first = &cl_nilNode;
    result->last = &cl_nilNode;
    
    result->name = name;
    result->base = base;
    result->body = &md_nil_node;
    result->type = type;
    
    if (!CheckNil(&cl_nilNode, parent))
    {
        result->parent = parent;
        DLLPushBack_NPZ(&cl_nilNode, parent->first, parent->last, result, next, prev);
    }
    
    return result;
}

function CL_Node* CL_PushType(Arena* arena, CL_NodeType type, MD_NodeFlags flags, MD_Node* node, CL_Node* parent)
{
    CL_Node* result = &cl_nilNode;
    
    b32 isTypedef = MD_NodeMatch(node->prev, "typedef");
    MD_Node* typeName = node->next;
    MD_Node* typeBody = node->next->next;
    if ((typeName->flags & flags) && (typeBody->flags & MD_NodeFlag_Identifier) && isTypedef)
        Swap(MD_Node*, typeName, typeBody);
    
    if ((typeName->flags & MD_NodeFlag_Identifier) && (typeBody->flags & flags))
    {
        result = CL_PushNode(arena, type, typeName, node, parent);
        result->body = typeBody;
        
        if (isTypedef)
        {
            MD_Node* typedefName = typeBody->next;
            if (!StrCompare(typeName->string, typedefName->string, 0))
                CL_PushNode(arena, CL_NodeType_Typedef, typedefName, typeName, parent);
        }
    }
    
    return result;
}

function MD_Node* CL_ParseDecl(Arena* arena, MD_Node* body, CL_Node* parent, b32 terminate)
{
    MD_Node* node = body->first;
    b32 oldTerminate = terminate;
    
    for (MD_EachNode(member, (node->flags & MD_NodeFlag_Identifier) ? node : &md_nil_node))
    {
        REPEAT:
        MD_NodeFlags flags = terminate ? MD_NodeFlag_IsBeforeSemicolon : MD_NodeFlag_IsBeforeComma;
        MD_Node* memberType = member;
        MD_Node* memberName = member->next;
        
        if (md_node_is_nil(memberName))
            memberName = memberType;
        
        else if (MD_NodeCompareArr(memberType, ArrayExpand(String, StrLit("struct"), StrLit("union"))))
        {
            if (memberName->flags & MD_NodeFlag_HasBraceLeft)
            {
                if (memberName->flags & flags)
                {
                    member = memberName->first;
                    terminate = 1;
                    goto REPEAT;
                }
                //else TODO(long): Inline struct
            }
            
            else if (memberName->flags & MD_NodeFlag_Identifier)
            {
                memberType = memberType->next;
                memberName = memberName->next;
            }
            //else TODO(long): Error
        }
        
        NEXT_MEMBER:
        memberName = CL_NodeSkipBase(memberName);
        if (memberName->flags & MD_NodeFlag_Identifier)
        {
            CL_PushNode(arena, CL_NodeType_Decl, memberName, memberType, parent);
            PARSE_POSTFIX:
            u64 arrayCount = 0;
            while (memberName->next->flags & MD_NodeFlag_HasBracketLeft)
            {
                arrayCount++;
                memberName = memberName->next;
            }
            
            if (arrayCount > 1)
            {
                // TODO(long): Multi-dimensional array
            }
            
            if (terminate && (memberName->flags & MD_NodeFlag_IsBeforeComma))
            {
                memberName = memberName->next;
                goto NEXT_MEMBER;
            }
            
            member = memberName;
            while (!(member->next->flags & MD_NodeFlag_AfterFromBefore(flags)) && !md_node_is_nil(member->next))
                member = member->next;
            
            while (member->parent != body)
            {
                if (md_node_is_nil(member->next))
                    member = member->parent;
                else
                    break;
            }
            
            if (member->parent == body)
                terminate = oldTerminate;
        }
        
        else if (!terminate && ((memberName->flags & MD_NodeFlag_IsAfterComma) || md_node_is_nil(memberName->next)))
        {
            CL_PushNode(arena, CL_NodeType_Decl, &md_nil_node, memberType, parent);
            goto PARSE_POSTFIX;
        }
        
        else
        {
            // TODO(long): Error if false
            // NOTE(long): MD_NodeFlag_HasParenLeft means this is a complex declaration
        }
    }
    
    return node;
}

function CL_Node* CL_MDParseText(Arena* arena, String filename, String text)
{
    CL_Node* root = CL_PushNode(arena, 0, &md_nil_node, &md_nil_node, 0);
    MD_ParseResult parse = MD_ParseText(arena, filename, text);
    String typeKeywords[] = { StrLit("struct"), StrLit("union"), StrLit("enum") };
    
    for (MD_EachNode(node, parse.root->first))
    {
        CL_Node* typeNode = 0;
        
        if (MD_NodeMatch(node, "enum"))
        {
            typeNode = CL_PushType(arena, CL_NodeType_Enum, MD_NodeFlag_HasBraceLeft, node, root);
            MD_Node* firstMember = typeNode->body->first;
            
            if (firstMember->flags & MD_NodeFlag_Identifier)
            {
                CL_PushNode(arena, CL_NodeType_Value, firstMember, &md_nil_node, typeNode);
                for (MD_EachNode(member, firstMember->next))
                    if (HasAllFlags(member->flags, MD_NodeFlag_Identifier|MD_NodeFlag_IsAfterComma))
                        CL_PushNode(arena, CL_NodeType_Value, member, &md_nil_node, typeNode);
            }
        }
        
        else if (MD_NodeMatch(node, "union"))
        {
            typeNode = CL_PushType(arena, CL_NodeType_Union, MD_NodeFlag_HasBraceLeft, node, root);
            CL_ParseDecl(arena, typeNode->body, typeNode, 1);
        }
        
        else if (MD_NodeMatch(node, "struct"))
        {
            typeNode = CL_PushType(arena, CL_NodeType_Struct, MD_NodeFlag_HasBraceLeft, node, root);
            CL_ParseDecl(arena, typeNode->body, typeNode, 1);
        }
        
        else if (node->flags & MD_NodeFlag_Identifier)
        {
            MD_Node* base = node;
            b32 isTypedef = 0;
            if (MD_NodeMatch(node, "typedef"))
            {
                isTypedef = 1;
                base = node->next;
            }
            
            if (MD_NodeCompareArr(base, typeKeywords, ArrayCount(typeKeywords)))
                continue;
            
            MD_Node* name = CL_NodeSkipBase(base->next);
            if (name->flags & MD_NodeFlag_Identifier)
            {
                MD_Node* body = name->next;
                if ((body->flags & MD_NodeFlag_HasParenLeft) &&
                    HasAnyFlags(body->next->flags, MD_NodeFlag_IsAfterSemicolon|MD_NodeFlag_HasBraceLeft))
                {
                    typeNode = CL_PushNode(arena, CL_NodeType_Proc, name, base, root);
                    typeNode->body = body;
                    if (!md_node_is_nil(body->first->next))
                        CL_ParseDecl(arena, body, typeNode, 0);
                }
                
                else if (MD_NodeMatch(name->next, "=") || (name->flags & MD_NodeFlag_IsBeforeSemicolon))
                {
                    typeNode = CL_PushNode(arena, isTypedef ? CL_NodeType_Typedef : CL_NodeType_Decl, name, base, root);
                    while (!(name->flags & MD_NodeFlag_IsBeforeSemicolon) && !md_node_is_nil(name->next))
                        name = name->next;
                    node = name;
                }
            }
        }
        
        //- long: Skip if success
        if (typeNode && !md_node_is_nil(typeNode->body))
            node = typeNode->body;
    }
    
    return root;
}

function MetaTable CL_TableFromTokens(Arena* arena, String text, TokenArray array)
{
    MetaTable result = {0};
    u64 cap = array.count / 3;
    
#define SkipNextNoWhitespace(token) Stmnt(token++; \
                                          while ((token->user & CL_TokenFlags_Ignorable) && token < opl) token++;)
    for (Token* token = array.tokens,* opl = array.tokens+array.count; token < opl; ++token)
    {
        MetaFlags flags = 0;
        String name = {0};
        String path = {0};
        
        if (token->user & CL_TokenFlag_Identifier)
        {
            if (TokenMatch(text, *token, StrLit("EmbedFile")))
                flags = MetaFlag_EmbedFile;
            else if (TokenMatch(text, *token, StrLit("TweakB32")))
                flags = MetaFlag_TweakB32;
            else if (TokenMatch(text, *token, StrLit("TweakF32")))
                flags = MetaFlag_TweakF32;
            
            if (flags)
            {
                SkipNextNoWhitespace(token);
                if (token && TokenMatch(text, *token, StrLit("(")))
                {
                    SkipNextNoWhitespace(token);
                    if (token && (token->user & CL_TokenFlag_Identifier))
                    {
                        name = StrFromToken(text, *token);
                        
                        if (flags & MetaFlag_EmbedFile)
                        {
                            SkipNextNoWhitespace(token);
                            if (token && TokenMatch(text, *token, StrLit(",")))
                            {
                                SkipNextNoWhitespace(token);
                                if (token && (token->user & CL_TokenFlag_String))
                                    path = StrFromToken(text, *token);
                            }
                        }
                    }
                }
            }
        }
        
        if (name.size)
        {
            Assert(result.count <= cap);
            if (!result.v)
                result.v = PushArray(arena, MetaInfo, cap);
            result.v[result.count++] = (MetaInfo){
                .parent = &metaNil, .base = &metaNil,
                .flags = flags, .name = name,
            };
            
            if (flags & MetaFlag_EmbedFile)
            {
                MetaInfo* parent = &result.v[result.count-1];
                MetaInfo* val = PushStruct(arena, MetaInfo);
                *val = (MetaInfo){ .base = &metaNil, .flags = MetaFlag_Value, .name = path };
                
                val->parent = parent;
                parent->childs.v = val;
                parent->childs.count = 1;
            }
        }
    }
#undef SkipNextNoWhitespace
    
    if (result.v)
        ArenaPop(arena, cap - result.count);
    return result;
}

//~ long: Type Info Lookups

function MetaInfo* MemberFromName(MetaInfo* type, String name)
{
    MetaInfo* result = &metaNil;
    MetaTable members = type->childs;
    ForEach(idx, members.count)
    {
        if (StrCompare(members.v[idx].name, name, 0))
        {
            result = members.v + idx;
            break;
        }
    }
    return result;
}
