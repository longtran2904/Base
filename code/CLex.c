
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

function CL_Node* CL_PushNode(Arena* arena, CL_NodeFlags flags, String string, u64 offset)
{
    CL_Node* result = PushStruct(arena, CL_Node);
    *result = cl_nilNode;
    result->flags = flags;
    result->string = string;
    result->offset = offset;
    return result;
}

function CL_Node* CL_PushRangeNode(Arena* arena, String text, CL_NodeFlags flags, r1u64 range)
{
    String string = Substr(text, range.min, range.max);
    CL_Node* result = CL_PushNode(arena, flags, string, range.min);
    return result;
}

function CL_Node* CL_PushErrorNode(Arena* arena, String error, u64 offset)
{
    CL_Node* result = CL_PushNode(arena, CL_NodeFlag_Error, error, offset);
    return result;
}

#define CL_IsNil(node) CheckNil(&cl_nilNode, node)

function void CL_ErrorListPushf(Arena* arena, CL_NodeList* list, u64 offset, char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    String error = StrPushfv(arena, fmt, args);
    va_end(args);
    
    CL_Node* node = CL_PushErrorNode(arena, error, offset);
    SLLQueuePush(list->first, list->last, node);
    list->count++;
}

#define CL_NodePushChild(p, list, c) Stmnt((c)->parent = (p); (p)->list.count++;\
                                           DLLPushBack_NPZ(&cl_nilNode, (p)->list.first, (p)->list.last, (c), next, prev))

typedef enum CL_ParseWorkKind
{
    CL_ParseWorkKind_Stmt,
    CL_ParseWorkKind_TypeExpr,
    CL_ParseWorkKind_Expr,
    CL_ParseWorkKind_Args,
    CL_ParseWorkKind_TypeBody,
} CL_ParseWorkKind;

typedef struct CL_ParseWorkNode CL_ParseWorkNode;
struct CL_ParseWorkNode
{
    CL_ParseWorkNode* next;
    CL_ParseWorkKind kind;
    CL_Node* parent;
    CL_Node* first_tag;
    CL_Node* last_tag;
    Token* token;
};

function b32 CL_WorkHasFlags(CL_ParseWorkNode* work, CL_NodeFlags flags)
{
    for (CL_Node* node = work->first_tag; !CL_IsNil(node); node = node->next)
        if (node->flags & flags)
            return 1;
    return 0;
}

#define CL_ParseWorkPush(work_kind, work_parent)                             \
    Stmnt(CL_ParseWorkNode* work_node = work_free;                             \
          if (!work_node) work_node = PushArray(scratch, CL_ParseWorkNode, 1); \
          else SLLStackPop(work_free);                                         \
          work_node->kind = (work_kind);                                       \
          work_node->parent = (work_parent);                                   \
          work_node->first_tag = work_node->last_tag = &cl_nilNode;            \
          work_node->token = token;                                            \
          SLLStackPush(work_top, work_node))

#define CL_ParseWorkPop() Stmnt(SLLStackPop(work_top); if (!work_top) work_top = &broken_work;)

function CL_NodeFlags CL_NodeFlagsFromToken(CL_TokenFlags flags, String lexeme)
{
    CL_NodeFlags result = 0;
    
    if (flags & CL_TokenFlag_Symbol ) result |= CL_NodeFlag_Symbol;
    if (flags & CL_TokenFlag_Numeric) result |= CL_NodeFlag_Numeric;
    if (flags & CL_TokenFlag_Char   ) result |= CL_NodeFlag_CharLit;
    if (flags & CL_TokenFlag_String ) result |= CL_NodeFlag_String;
    
    if (flags & CL_TokenFlag_Identifier)
    {
        if (0) {} 
        
        else if (StrCompare(lexeme, StrLit("struct"), 0)) result |= CL_NodeFlag_Struct;
        else if (StrCompare(lexeme, StrLit( "union"), 0)) result |= CL_NodeFlag_Union;
        else if (StrCompare(lexeme, StrLit(  "enum"), 0)) result |= CL_NodeFlag_Enum;
        
        else if (StrCompare(lexeme, StrLit(    "void"), 0)) result |= CL_NodeFlag_Void;
        else if (StrCompare(lexeme, StrLit(    "char"), 0)) result |= CL_NodeFlag_Char;
        else if (StrCompare(lexeme, StrLit(   "short"), 0)) result |= CL_NodeFlag_Short;
        else if (StrCompare(lexeme, StrLit(     "int"), 0)) result |= CL_NodeFlag_Int;
        else if (StrCompare(lexeme, StrLit(    "long"), 0)) result |= CL_NodeFlag_Long;
        else if (StrCompare(lexeme, StrLit(  "signed"), 0)) result |= CL_NodeFlag_Signed;
        else if (StrCompare(lexeme, StrLit("unsigned"), 0)) result |= CL_NodeFlag_Unsigned;
        else if (StrCompare(lexeme, StrLit(   "float"), 0)) result |= CL_NodeFlag_Float;
        else if (StrCompare(lexeme, StrLit(  "double"), 0)) result |= CL_NodeFlag_Double;
        
        else if (StrCompare(lexeme, StrLit(   "const"), 0)) result |= CL_NodeFlag_Const;
        else if (StrCompare(lexeme, StrLit("volatile"), 0)) result |= CL_NodeFlag_Volatile;
        else if (StrCompare(lexeme, StrLit("restrict"), 0)) result |= CL_NodeFlag_Restrict;
        else if (StrCompare(lexeme, StrLit( "_Atomic"), 0)) result |= CL_NodeFlag_Atomic;
        
        else if (StrCompare(lexeme, StrLit(      "typedef"), 0)) result |= CL_NodeFlag_Typedef;
        else if (StrCompare(lexeme, StrLit(    "constexpr"), 0)) result |= CL_NodeFlag_Constexpr;
        else if (StrCompare(lexeme, StrLit(         "auto"), 0)) result |= CL_NodeFlag_Auto;
        else if (StrCompare(lexeme, StrLit(     "register"), 0)) result |= CL_NodeFlag_Register;
        else if (StrCompare(lexeme, StrLit(       "static"), 0)) result |= CL_NodeFlag_Static;
        else if (StrCompare(lexeme, StrLit(       "extern"), 0)) result |= CL_NodeFlag_Extern;
        else if (StrCompare(lexeme, StrLit(       "inline"), 0)) result |= CL_NodeFlag_Inline;
        else if (StrCompare(lexeme, StrLit("_Thread_local"), 0)) result |= CL_NodeFlag_ThreadLocal;
        else if (StrCompare(lexeme, StrLit(    "_Noreturn"), 0)) result |= CL_NodeFlag_Noreturn;
        else if (StrCompare(lexeme, StrLit(     "_Alignas"), 0)) result |= CL_NodeFlag_Alignas;
        
        else result |= CL_NodeFlag_Identifier;
    }
    
    return result;
}

function CL_Node* CL_BaseTypeFromTag(CL_ParseWorkNode* work_top)
{
    CL_Node* baseType = work_top->first_tag;
    if (!CL_IsNil(baseType))
    {
        if (!StrCompare(baseType->string, StrLit("*"), 0))
            for (CL_Node* tag = baseType; !CL_IsNil(tag); tag = tag->next)
                if (HasAnyFlags(tag->flags, CL_NodeFlag_Identifier|CL_NodeFlags_Leaf|
                                CL_NodeFlags_TypeKeyword|CL_NodeFlag_Decl))
                    baseType = tag;
        
        DLLRemove_NPZ(&cl_nilNode, work_top->first_tag, work_top->last_tag, baseType, next, prev);
        baseType->tags.first = work_top->first_tag;
        baseType->tags.last  = work_top-> last_tag;
        
        for (CL_Node* tag = baseType->tags.first; !CL_IsNil(tag); tag = tag->next)
        {
            tag->parent = baseType;
            baseType->tags.count++;
        }
        work_top->first_tag = work_top->last_tag = &cl_nilNode;
    }
    return baseType;
}

function CL_ParseResult CL_ParseFromTokens(Arena* arena, String text, TokenArray array)
{
    ScratchBegin(scratch, arena);
    
    //- rjf: set up outputs
    CL_NodeList msgs = {0};
    CL_Node* root = CL_PushNode(arena, CL_NodeFlag_File, text, 0);
    
    //- rjf: set up parse rule stack
    CL_ParseWorkNode  first_work = { 0, CL_ParseWorkKind_Stmt, root, &cl_nilNode, &cl_nilNode };
    CL_ParseWorkNode broken_work = { 0, CL_ParseWorkKind_Stmt, root, &cl_nilNode, &cl_nilNode };
    CL_ParseWorkNode* work_top = &first_work;
    CL_ParseWorkNode* work_free = 0;
    
    //- rjf: parse
    Token* tokens_first = array.tokens;
    Token* tokens_opl = tokens_first + array.count;
    Token* token = tokens_first;
    Token* prevToken = tokens_first;
    
    while (token < tokens_opl)
    {
#define LexMatch(str) StrCompare(lexeme, StrLit(str), 0)
#define TokMatch(tok, str) StrCompare(Substr(text, (tok)->range.min, (tok)->range.max), StrLit(str), 0)
#define HasFlags(flgs) (token->user & (flgs))
#define LeftNode(node) ((node)->body.first)
#define RightNode(node) ((node)->body.last)
        
#define CL_IsTypeDecl(node) (((node)->flags & CL_NodeFlag_Decl) && \
                             ((node)->reference->flags & CL_NodeFlags_TypeKeyword))
        
        //- rjf: unpack token
        String lexeme = Substr(text, token->range.min, token->range.max);
        b32 inc = 1;
        
        //- long: whitespace/preproc/comments -> always no-op & inc
        if (HasFlags(CL_TokenFlags_Ignorable)) { }
        
#if 0
        else if (work_top->kind == CL_ParseWorkKind_Expr && HasFlags(CL_TokenFlag_Symbol))
        {
            CL_Node* expr = CL_ExprFromStr(arena, lexeme);
            CL_Node* parent = work_top->parent;
            i32 prec = CL_ExprPrec(expr->flags);
            
            for (CL_Node* tail = parent; CL_IsNil(tail); parent = tail, tail = tail->parent)
            {
                i32 parentPrec = CL_ExprPrec(parent->flags);
                if (parentPrec < prec && !(parentPrec == prec && CL_ExprIsLR(parent->flags)))
                    break;
                if (!CL_ExprIsClosed(parent))
                    break;
            }
            
            if (CL_IsNil(parent))
            {
                //Assert(CL_IsNil(parent->body.last) || CL_ExprIsPrefix(op));
                //Assert(CL_IsNil(parent->body.first) || CL_ExprIsUnary(parent->op));
                
                expr->parent = parent;
                LeftNode(expr) = RightNode(parent);
                LeftNode(expr)->parent = expr;
            }
            //else Assert(it == first);
            
            work_top->parent = expr;
        }
#endif
        
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && TokMatch(prevToken, "}") && LexMatch(";"))
        {
            // TODO(long): This is a stupid hack
            goto CLEAN_UP;
        }
        
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && work_top->parent == work_top->next->parent &&
                 (HasAnyFlags(work_top->last_tag->flags, CL_NodeFlag_Identifier|CL_NodeFlags_TypeKeyword) ||
                  (work_top->next->kind == CL_ParseWorkKind_Args || work_top->next->kind == CL_ParseWorkKind_Expr)) &&
                 (LexMatch("(") || LexMatch(")") || LexMatch("[") || // can be nested in other parens
                  LexMatch(",") || LexMatch("=") || LexMatch(";") || // can't be nested in other parens
                  LexMatch("{") || LexMatch("}")))
        {
            CL_Node* decl = work_top->last_tag;
            b32 hasName = 1;
            
            if (work_top->last_tag->flags & CL_NodeFlags_TypeKeyword)
                hasName = 0;
            
            else if (work_top->next->kind == CL_ParseWorkKind_Args || work_top->next->kind == CL_ParseWorkKind_Expr)
            {
                hasName = decl->flags & CL_NodeFlag_Identifier;
                if (hasName)
                {
                    if (!StrCompare(work_top->first_tag->string, StrLit("*"), 0))
                        for (CL_Node* tag = work_top->first_tag; tag != decl && !hasName; tag = tag->next)
                            if (HasAnyFlags(tag->flags, CL_NodeFlag_Identifier|CL_NodeFlags_Leaf))
                                hasName = 1;
                }
            }
            
            if (hasName)
                DLLRemove_NPZ(&cl_nilNode, work_top->first_tag, work_top->last_tag, work_top->last_tag, next, prev);
            else
                decl = CL_PushNode(arena, CL_NodeFlag_Decl, StrPrefix(lexeme, 0), token->range.min);
            
            if ((work_top->parent->flags & CL_NodeFlag_Decl) && TokMatch(work_top->next->token, "("))
                CL_NodePushChild(work_top->parent, args, decl);
            else
                CL_NodePushChild(work_top->parent, body, decl);
            
            work_top->parent = decl;
            decl->flags |= CL_NodeFlag_Decl;
            decl->reference = CL_BaseTypeFromTag(work_top);
            if (!CL_IsNil(decl->reference)) // @useless_paren
                decl->reference->parent = decl;
            inc = 0;
        }
        
        //- long: Type's Body
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && LexMatch("{") &&
                 (CL_WorkHasFlags(work_top, CL_NodeFlags_TypeKeyword|CL_NodeFlag_Typedef) ||
                  HasAnyFlags(work_top->parent->reference->flags, CL_NodeFlags_TypeKeyword)))
        {
            CL_ParseWorkPush(CL_ParseWorkKind_Stmt, work_top->parent);
        }
        
        else if (work_top->kind == CL_ParseWorkKind_Stmt && LexMatch("}"))
        {
            if (ALWAYS(work_top->next->kind == CL_ParseWorkKind_TypeExpr))
            {
                CL_ParseWorkPop();
                
                if (ALWAYS(CL_IsTypeDecl(work_top->parent)))
                {
                    Assert(work_top->first_tag == &cl_nilNode);
                    // TODO(long): prefix qualifiers before struct keyword
                    CL_Node* type = work_top->parent;
                    CL_Node* base = CL_PushNode(arena, type->flags, type->string, type->offset);
                    work_top->first_tag = work_top->last_tag = base;
                    work_top->parent = work_top->parent->parent;
                }
            }
        }
        
        //- long: Clean up
        else if (LexMatch(";"))
        {
            CLEAN_UP:
            if (work_top->kind != CL_ParseWorkKind_Stmt && work_top->kind != CL_ParseWorkKind_Args)
            {
                CL_ParseWorkPop();
                inc = 0;
            }
            else
            {
                // TODO(long): Memory leaked
                work_top->first_tag = work_top->last_tag = &cl_nilNode;
            }
        }
        
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && LexMatch(","))
        {
            if (work_top->next->kind == CL_ParseWorkKind_Args)
            {
                CL_ParseWorkPop();
                work_top->first_tag = work_top->last_tag = &cl_nilNode;
            }
            else if (ALWAYS(work_top->next->kind != CL_ParseWorkKind_TypeExpr))
            {
                CL_Node* base = work_top->parent->reference;
                while (!CL_IsNil(base->reference))
                    base = base->reference;
                
                if (!CL_IsNil(base))
                    base = CL_PushNode(arena, base->flags, base->string, base->offset);
                
                work_top->first_tag = work_top->last_tag = base;
                if (ALWAYS((work_top->parent->flags & CL_NodeFlag_Decl) ||
                           work_top->parent->parent->reference == work_top->parent))
                    work_top->parent = work_top->next->parent;
            }
        }
        
        //- long: Type Expression
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && HasFlags(CL_TokenFlag_Identifier))
        {
            CL_NodeFlags flags = CL_NodeFlagsFromToken((CL_TokenFlags)token->user, lexeme);
            /*if (HasAnyFlags(flags, CL_NodeFlags_TypeKeyword|CL_NodeFlag_Typedef))
            {
            }
            else*/ if (ALWAYS(flags))
            {
                CL_Node* node = CL_PushNode(arena, flags, lexeme, token->range.min);
                DLLPushBack_NPZ(&cl_nilNode, work_top->first_tag, work_top->last_tag, node, next, prev);
            }
        }
        
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && LexMatch("*"))
        {
            CL_Node* base = CL_BaseTypeFromTag(work_top);
            CL_Node* ptr = CL_PushNode(arena, CL_NodeFlag_Symbol, lexeme, token->range.min);
            
            if (!CL_IsNil(base))
            {
                ptr->reference = base;
                base->parent = ptr;
            }
            work_top->first_tag = work_top->last_tag = ptr;
        }
        
        //- long: Push/Pop Paren
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && LexMatch("("))
        {
            if (!CL_IsNil(work_top->parent->reference))
            {
                CL_ParseWorkPush(CL_ParseWorkKind_Args, work_top->parent);
            }
            
            else
            {
                CL_Node* base = CL_BaseTypeFromTag(work_top);
                work_top->first_tag = work_top->last_tag = base;
                CL_ParseWorkPush(CL_ParseWorkKind_TypeExpr, work_top->parent);
            }
        }
        
        else if (work_top->kind == CL_ParseWorkKind_Args && LexMatch(")"))
        {
            CL_ParseWorkPop();
        }
        
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && LexMatch(")"))
        {
            CL_Node* base = work_top->parent->reference;
            if (CL_IsNil(base)) // @useless_paren
                base = work_top->parent;
            
            CL_ParseWorkPop();
            
            if (work_top->kind == CL_ParseWorkKind_TypeExpr)
            {
                while (!CL_IsNil(base->reference))
                    base = base->reference;
                work_top->parent = base;
                base->reference = work_top->first_tag;
                base->reference->parent = base;
            }
            else if (ALWAYS(work_top->kind == CL_ParseWorkKind_Args))
                CL_ParseWorkPop(); // NOTE(long): This can also be `inc = 0;`
        }
        
        //- long: Push/Pop Bracket
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && LexMatch("["))
        {
            CL_Node* node = CL_PushNode(arena, CL_NodeFlag_Brack, lexeme, token->range.min);
            CL_Node* parent = work_top->parent;
            node->reference = parent->reference;
            
            if (parent != root)
            {
                while (parent->reference->flags & CL_NodeFlag_Brack)
                    parent = parent->reference;
                
                if (!CL_IsNil(parent->reference))
                {
                    parent->reference->parent = node;
                    node->reference = parent->reference;
                }
                
                parent->reference = node;
                node->parent = parent;
            }
            else CL_NodePushChild(parent, body, node);
            
            CL_ParseWorkPush(CL_ParseWorkKind_Expr, node);
        }
        else if ((work_top->parent->flags & CL_NodeFlag_Brack) && LexMatch("]"))
            CL_ParseWorkPop();
        
        //- long: Expr
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && LexMatch("="))
        {
            CL_Node* node = CL_PushNode(arena, CL_NodeFlag_Symbol, lexeme, token->range.min);
            CL_NodePushChild(work_top->parent, body, node);
            CL_ParseWorkPush(CL_ParseWorkKind_Expr, node);
        }
        
        else if (work_top->kind == CL_ParseWorkKind_Expr && work_top->next->kind == CL_ParseWorkKind_TypeExpr && LexMatch(","))
        {
            CL_ParseWorkPop();
            inc = 0;
        }
        
        else if (work_top->kind == CL_ParseWorkKind_Expr && HasFlags(CL_TokenFlags_Literal|CL_TokenFlag_Identifier))
        {
            CL_Node* atom = CL_PushNode(arena, CL_NodeFlag_Expr, lexeme, token->range.min);
            CL_Node* expr = work_top->parent;
            CL_NodePushChild(expr, body, atom);
        }
        
        else if (work_top->kind == CL_ParseWorkKind_Expr && LexMatch("*"))
        {
            
        }
        
        else if (work_top->kind == CL_ParseWorkKind_Expr && LexMatch(")"))
        {
            CL_ParseWorkPop();
            inc = 0;
        }
        
        // TODO(long): Parse operator precedence
        else if (NEVER(work_top->kind == CL_ParseWorkKind_Expr && HasFlags(CL_TokenFlag_Symbol)));
        
        //- long: Identifier
        else if ((work_top->kind == CL_ParseWorkKind_Stmt || work_top->kind == CL_ParseWorkKind_Args) &&
                 HasFlags(CL_TokenFlag_Identifier))
        {
            CL_NodeFlags flags = CL_NodeFlagsFromToken((CL_TokenFlags)token->user, lexeme);
            if (HasAnyFlags(flags, CL_NodeFlags_TypeKeyword|CL_NodeFlag_Typedef))
            {
                goto PUSH_TYPE_EXPR;
            }
            
            else if (ALWAYS(flags))
            {
                if ((flags & CL_NodeFlag_Identifier) && CL_IsNil(work_top->first_tag))
                {
                    CL_Node* node = CL_PushNode(arena, flags, lexeme, token->range.min);
                    DLLPushBack_NPZ(&cl_nilNode, work_top->first_tag, work_top->last_tag, node, next, prev);
                }
                else
                {
                    PUSH_TYPE_EXPR:
                    CL_ParseWorkNode* work_prev = work_top;
                    CL_ParseWorkPush(CL_ParseWorkKind_TypeExpr, work_top->parent);
                    work_top->first_tag = work_prev->first_tag;
                    work_top-> last_tag = work_prev-> last_tag;
                    work_prev->last_tag = work_prev->first_tag = &cl_nilNode;
                    inc = 0;
                }
            }
        }
        
        else if ((work_top->kind == CL_ParseWorkKind_Stmt || work_top->kind == CL_ParseWorkKind_Args) &&
                 LexMatch("*") && !CL_IsNil(work_top->first_tag))
        {
            goto PUSH_TYPE_EXPR;
        }
        
        // TODO(long): Function call
        else if (NEVER(work_top->kind == CL_ParseWorkKind_Stmt && LexMatch("(") && (work_top->first_tag->flags & CL_NodeFlag_Identifier)));
        
        // TODO(long): Expression
        else if (NEVER(work_top->kind == CL_ParseWorkKind_Stmt && HasFlags(CL_TokenFlag_Symbol)));
        
        //- rjf: no consumption -> unexpected token! we don't know what to do with this.
        //else CL_ErrorListPushf(arena, &msgs, token->range.min, "Unexpected \"%.*s\" token.", StrExpand(lexeme));
        else Assert(0);
        
        if (inc)
        {
            if (!HasFlags(CL_TokenFlags_Ignorable))
                prevToken = token;
            token += 1;
        }
        
#undef HasFlags
#undef LexMatch
#undef TokMatch
#undef LeftNode
#undef RightNode
    }
    
    //- rjf: fill & return
    CL_ParseResult result = { root, msgs };
    ScratchEnd(scratch);
    return result;
}

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
