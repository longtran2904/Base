
//~ long: Lexing Functions

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

function TokenArray CL_TokenArrayFromStr(Arena* arena, String text)
{
    ScratchBegin(scratch, arena);
    Scanner scanner = {
        .source = text,
        .flags = (CL_Scan_Whitespace|CL_Line_Cont_Comments|CL_Line_Cont_Preprocs|CL_Scan_Preproc|CL_Scan_Comments|
                  CL_Scan_SingleQuotes|CL_Scan_DoubleQuotes|CL_Scan_Idents|CL_Scan_Nums|CL_Scan_Symbols),
        
        .lineContinuation = '\\',
        .preproc = StrLit("#"),
        
        .commentDelim = StrLit("//"),
        .commentMulti = StrLit("/*"),
        
        .escapeChar = '\\',
        .strPrefixes = StrList(scratch, ArrayExpand(String, StrLit("u8"), StrLit("u"), StrLit("U"), StrLit("L"))),
        
        .preIdentSymbols = StrLit("_"),
        .midIdentSymbols = StrLit("_"),
        
        .preNumSymbols = StrLit("."),
        .midNumSymbols = StrLit("."),
        .exponents = StrLit("eEpP"),
        
        // NOTE(long): Order matters!!!
        .symbols = StrList(scratch, ArrayExpand(String, StrLit("..."), StrLit("->"), StrLit("##"), StrLit("#"),
                                                StrLit("<<="), StrLit(">>="), StrLit("<="), StrLit(">="),
                                                StrLit("<<"), StrLit(">>"), StrLit(">"), StrLit("<"),
                                                
                                                StrLit("&&"), StrLit("||"), StrLit("++"), StrLit("--"),
                                                StrLit("&="), StrLit("|="), StrLit("+="), StrLit("-="),
                                                StrLit("&"), StrLit("|"), StrLit("+"), StrLit("-"), 
                                                
                                                StrLit("*="), StrLit("/="), StrLit("!="),
                                                StrLit("^="), StrLit("%="), StrLit("=="),
                                                StrLit("*"), StrLit("/"), StrLit("!"),
                                                StrLit("^"), StrLit("%"), StrLit("="),
                                                
                                                StrLit("["), StrLit("]"), StrLit("("), StrLit(")"),
                                                StrLit("{"), StrLit("}"), StrLit(","), StrLit(";"),
                                                StrLit(":"), StrLit("?"), StrLit("~"))),
    };
    
    TokenChunkList tokens = {0};
    while (scanner.pos < scanner.source.size)
    {
        Token token = ScannerNext(&scanner);
        TokenChunkListPush(scratch, &tokens, 4096, token);
    }
    
    TokenArray result = TokenArrayFromChunkList(arena, &tokens);
    ScratchEnd(scratch);
    return result;
}

//~ NOTE(long): MD Functions

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

//~ NOTE(long): CL Functions

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

function CL_Node* CL_PushNode(Arena* arena, CL_NodeFlags flags, String string, u64 offset)
{
    CL_Node* result = PushStruct(arena, CL_Node);
    *result = cl_nilNode;
    result->flags = flags;
    result->string = string;
    result->offset = offset;
    return result;
}

#define CL_IsNil(node) CheckNil(&cl_nilNode, node)
#define CL_NodePushChild(node, child) \
    Stmnt((child)->parent = (node); DLLPushBack_NPZ(&cl_nilNode, (node)->first, (node)->last, (child), next, prev))
#define CL_NodePushArg(node, arg) \
    Stmnt((arg)->parent = (node); DLLPushBack_NPZ(&cl_nilNode, (node)->firstArg, (node)->lastArg, (arg), next, prev))
#define CL_NodePushTag(node, tag) \
    Stmnt((tag)->parent = (node); DLLPushBack_NPZ(&cl_nilNode, (node)->firstTag, (node)->lastTag, (tag), next, prev))

function void CL_DebugTree(CL_Node* root)
{
    for (CL_Node* node = root->first; !CL_IsNil(node); node = node->next)
    {
        Assert(node->next       != node);
        Assert(node->prev       != node);
        Assert(node->parent     != node);
        Assert(node->first != node);
        Assert(node->last  != node);
        Assert(node->firstTag != node);
        Assert(node->lastTag  != node);
        Assert(node->firstArg != node);
        Assert(node->lastArg  != node);
        
        Assert(CL_IsNil(node->first->prev));
        Assert(CL_IsNil(node->last->next));
        if (node->first == node->last)
            Assert(CL_IsNil(node->first->next));
        
        Assert(CL_IsNil(node->firstTag->prev));
        Assert(CL_IsNil(node->lastTag->next));
        if (node->firstTag == node->lastTag)
            Assert(CL_IsNil(node->firstTag->next));
        
        Assert(CL_IsNil(node->firstArg->prev));
        Assert(CL_IsNil(node->lastArg->next));
        if (node->firstArg == node->lastArg)
            Assert(CL_IsNil(node->firstArg->next));
        
        // TODO(long): Check for references
        
        if (!CL_IsNil(node->next))
        {
            Assert(node->next->prev == node);
            Assert(node->next->parent == node->parent);
            
            if (!CL_IsNil(node->first))
            {
                Assert(node->first != node->next->first && node->first != node->next->last);
                Assert(node-> next != node->next->first && node-> next != node->next->last);
            }
        }
        
        CL_DebugTree(node->first);
        CL_DebugTree(node->firstTag);
        CL_DebugTree(node->firstArg);
    }
}

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

function CL_NodeFlags CL_NodeFlagsFromToken(CL_NodeFlags flags, String lexeme)
{
    CL_NodeFlags result = 0;
    
    if (flags & TokenFlag_Symbol ) result |= CL_NodeFlag_Symbol;
    if (flags & TokenFlag_Numeric) result |= CL_NodeFlag_Numeric;
    if (flags & TokenFlag_String ) result |= CL_NodeFlag_String;
    
    if (flags & TokenFlag_Identifier)
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

// NOTE(long): RTL is Right-To-Left

// TODO(long): C99 compound literal
#define CL_OPERATORS(X) \
    X(SuffixInc,        "++", Suffix,   18,  "") \
    X(SuffixDec,        "--", Suffix,   18,  "") \
    X(Call,              "(", Suffix,   18, ")") \
    X(Index,             "[", Suffix,   18, "]") \
    X(Dot,               ".", Binary,   18,  "") \
    X(Arrow,            "->", Binary,   18,  "") \
    X(PrefixInc,        "++", Prefix,   16,  "") \
    X(PrefixDec,        "--", Prefix,   16,  "") \
    X(UnaryPlus,         "+", Prefix,   16,  "") \
    X(UnaryMinus,        "-", Prefix,   16,  "") \
    X(LogicalNot,        "!", Prefix,   16,  "") \
    X(BitwiseNot,        "~", Prefix,   16,  "") \
    X(Cast,              "(", Prefix,   16, ")") \
    X(Deref,             "*", Prefix,   16,  "") \
    X(Addrof,            "&", Prefix,   16,  "") \
    X(Sizeof,       "sizeof", Prefix,   16,  "") \
    X(Alignof,    "_Alignof", Prefix,   16,  "") \
    X(Mul,               "*", Binary,   14,  "") \
    X(Div,               "/", Binary,   14,  "") \
    X(Mod,               "%", Binary,   14,  "") \
    X(Add,               "+", Binary,   13,  "") \
    X(Sub,               "-", Binary,   13,  "") \
    X(LShift,           "<<", Binary,   12,  "") \
    X(RShift,           ">>", Binary,   12,  "") \
    X(Less,              "<", Binary,   11,  "") \
    X(LessEqual,        "<=", Binary,   11,  "") \
    X(Greater,           ">", Binary,   11,  "") \
    X(GreaterEqual,     ">=", Binary,   11,  "") \
    X(Equal,            "==", Binary,   10,  "") \
    X(NotEqual,         "!=", Binary,   10,  "") \
    X(BitwiseAnd,        "&", Binary,    9,  "") \
    X(BitwiseXor,        "^", Binary,    8,  "") \
    X(BitwiseOr,         "|", Binary,    7,  "") \
    X(LogicalAnd,       "&&", Binary,    6,  "") \
    X(LogicalOr,        "||", Binary,    5,  "") \
    X(TernaryCond,       "?", BinaryRTL, 4, ":") \
    X(Assign,            "=", BinaryRTL, 3,  "") \
    X(AssignAdd,        "+=", BinaryRTL, 3,  "") \
    X(AssignSub,        "-=", BinaryRTL, 3,  "") \
    X(AssignMul,        "*=", BinaryRTL, 3,  "") \
    X(AssignDiv,        "/=", BinaryRTL, 3,  "") \
    X(AssignMod,        "%=", BinaryRTL, 3,  "") \
    X(AssignLShift,    "<<=", BinaryRTL, 3,  "") \
    X(AssignRShift,    ">>=", BinaryRTL, 3,  "") \
    X(AssignBitwiseAnd, "&=", BinaryRTL, 3,  "") \
    X(AssignBitwiseXor, "^=", BinaryRTL, 3,  "") \
    X(AssignBitwiseOr,  "|=", BinaryRTL, 3,  "") \
    X(Comma,             ",", Binary,    1,  "") \
    X(Grouping,          "(", Nil,       1, ")") \

typedef enum
{
    CL_ExprOpKind_Nil,
#define X(name, str, oprKind, prec, closeOp) CL_ExprOpKind_##name,
    CL_OPERATORS(X)
#undef X
    CL_ExprOpKind_Count,
} CL_ExprOpKind;

typedef enum
{
    CL_ExprKind_Nil,
    CL_ExprKind_Prefix,
    CL_ExprKind_Suffix,
    CL_ExprKind_Binary,
    CL_ExprKind_BinaryRTL,
    CL_ExprKind_Count
} CL_ExprKind;

readonly global String CL_ExprOpKind_names[] = {
    StrConst("Nil"),
#define X(name, str, oprKind, prec, closeOp) StrConst(#name),
    CL_OPERATORS(X)
#undef X
};

readonly global String cl_exprSymbolTable[CL_ExprOpKind_Count] = {
    StrConst(""),
#define X(name, str, oprKind, prec, closeOp) StrConst(str),
    CL_OPERATORS(X)
#undef X
};

readonly global i8 cl_exprPrecedenceTable[CL_ExprOpKind_Count] = {
    0,
#define X(name, str, oprKind, prec, closeOp) prec,
    CL_OPERATORS(X)
#undef X
};

readonly global CL_ExprKind cl_exprKindTable[CL_ExprOpKind_Count] = {
    CL_ExprKind_Nil,
#define X(name, str, oprKind, prec, closeOp) CL_ExprKind_##oprKind,
    CL_OPERATORS(X)
#undef X
};

readonly global String cl_exprCloseSymbol[CL_ExprOpKind_Count] = {
    StrConst(""),
#define X(name, str, oprKind, prec, closeOp) StrConst(closeOp),
    CL_OPERATORS(X)
#undef X
};

function CL_ExprOpKind CL_ExprOpFromStr(String lexeme, CL_ParseWorkNode* work_top)
{
    CL_Node* parent = work_top->parent;
    b32 lookupPrefix = 0;
    
    // Max parent
    if (NoFlags(parent->flags, CL_NodeFlag_Symbol))
        lookupPrefix = !(parent->last->flags & CL_NodeFlags_Atom);
    
    // Only an unfinished prefix op or nil op can have zero child
    else if (CL_IsNil(parent->first))
    {
        // Only true for ops _inside_ an unclosed op (parentheses, cast, ternary, etc)
        if (work_top->next->parent == work_top->parent)
            lookupPrefix = CL_IsNil(parent->ref);
        
        // TODO(long): language-specific hack to differentiate between cast and parentheses grouping
        // Change this later when you change the type expression/operator parsing
        else
            lookupPrefix = CL_IsNil(parent->ref) || !!(parent->ref->flags & CL_NodeFlags_Atom);
    }
    
    // This is only true for valid (1-child) suffix op or unfinished (1-child) binary op
    else if (parent->offset > parent->last->offset)
    {
        for (u64 i = 0; i < CL_ExprOpKind_Count; ++i)
        {
            if (StrCompare(parent->string, cl_exprSymbolTable[i], 0))
            {
                CL_ExprKind kind = cl_exprKindTable[i];
                if (kind != CL_ExprKind_Prefix)
                {
                    lookupPrefix = kind == CL_ExprKind_Binary;
                    break;
                }
            }
        }
    }
    
    CL_ExprOpKind op = CL_ExprOpKind_Nil;
    for (u64 i = 1; i < CL_ExprOpKind_Count && !op; ++i)
        if (StrCompare(lexeme, cl_exprSymbolTable[i], 0))
            if (lookupPrefix == (cl_exprKindTable[i] == CL_ExprKind_Prefix || cl_exprKindTable[i] == CL_ExprKind_Nil))
                op = i;
    
    return op;
}

function CL_ExprOpKind CL_ExprOpFromNode(CL_Node* node)
{
    CL_ExprOpKind result = 0;
    b32 lookupPre = 0, lookupBin = 0, lookupNil = 0, lookupBinOrSuf = 0;
    
    if (node->offset < node->first->offset)
        lookupPre = 1;
    
    else if (CL_IsNil(node->first))
    {
        // TODO(long): language-specific hack to differentiate between cast and parentheses grouping
        // Change this later when you change the type expression/operator parsing
        if (CL_IsNil(node->ref) || (node->ref->flags & CL_NodeFlags_Atom))
            lookupPre = 1; // unfinished prefix ops
        else
            lookupNil = 1;
    }
    
    else if (node->first != node->last)
        lookupBin = 1;
    
    else if (node->offset > node->first->offset)
        lookupBinOrSuf = 1;
    
    for (u64 i = 0; i < CL_ExprOpKind_Count && !result; ++i)
    {
        if (StrCompare(node->string, cl_exprSymbolTable[i], 0))
        {
            CL_ExprKind kind = cl_exprKindTable[i];
            
            if (lookupBinOrSuf && (kind != CL_ExprKind_Prefix && kind != CL_ExprKind_Nil))
                result = (CL_ExprOpKind)i;
            else if (lookupBin && (kind == CL_ExprKind_Binary || kind == CL_ExprKind_BinaryRTL))
                result = (CL_ExprOpKind)i;
            
            else if (lookupPre && kind == CL_ExprKind_Prefix)
                result = (CL_ExprOpKind)i;
            else if (lookupNil && kind == CL_ExprKind_Nil)
                result = (CL_ExprOpKind)i;
        }
    }
    
    return result;
}

function i32 CL_ExprPrec(CL_ExprOpKind kind)
{
    return cl_exprPrecedenceTable[kind];
}

function b32 CL_ExprIsRTL(CL_ExprOpKind kind)
{
    // NOTE(long): All prefix ops are RTL while all postfix ops are LTR
    CL_ExprKind exprKind = cl_exprKindTable[kind];
    return exprKind == CL_ExprKind_Prefix || exprKind == CL_ExprKind_BinaryRTL;
}

function b32 CL_ExprIsCloseSymbol(CL_ExprOpKind op, String str)
{
    return StrCompare(cl_exprCloseSymbol[op], str, 0);
}

function CL_Node* CL_BaseTypeFromTag(CL_ParseWorkNode* work_top)
{
    CL_Node* base = work_top->first_tag;
    if (!CL_IsNil(base))
    {
        if (!StrCompare(base->string, StrLit("*"), 0))
            for (CL_Node* tag = base; !CL_IsNil(tag); tag = tag->next)
                if (HasAnyFlags(tag->flags, CL_NodeFlag_Identifier|CL_NodeFlags_Leaf|
                                CL_NodeFlags_TypeKeyword|CL_NodeFlag_Decl))
                    base = tag;
        
        DLLRemove_NPZ(&cl_nilNode, work_top->first_tag, work_top->last_tag, base, next, prev);
        for (CL_Node* tag = work_top->first_tag; !CL_IsNil(tag); tag = tag->next)
            CL_NodePushTag(base, tag);
        work_top->first_tag = work_top->last_tag = &cl_nilNode;
    }
    return base;
}

/*
decl
- base type
- args
- tags

var
- base type
- tags

field
- base type
- tags

args
- base type
- tags

type
- inheritance base type?
- struct/enum/union?
- args?
- tags

proc
- return type (multiple?)
- args
- header vs body?
- tags

expr
- tags
- inside paren/bracket
*/

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
#define HasFlags(flgs) (token->flags & (flgs))
        
#define CL_IsTypeDecl(node) (((node)->flags & CL_NodeFlag_Decl) && ((node)->ref->flags & CL_NodeFlags_TypeKeyword))
        
        //- rjf: unpack token
        String lexeme = Substr(text, token->range.min, token->range.max);
        b32 inc = 1;
        
        CL_Node* decl = work_top->last_tag;
        b32 hasName = 0;
        
        //- long: Skip Whitespace/Preproc/Comments
        if (HasFlags(CL_TokenFlags_Ignorable)) { }
        
        //- long: Enum Body (reuse this for array literal)
        else if (work_top->kind == CL_ParseWorkKind_Stmt && HasFlags(TokenFlag_Identifier) &&
                 (work_top->parent->ref->flags & CL_NodeFlag_Enum))
        {
            CL_NodeFlags flags = CL_NodeFlagsFromToken(token->flags, lexeme);
            if (ALWAYS(flags & CL_NodeFlag_Identifier))
            {
                CL_Node* node = CL_PushNode(arena, CL_NodeFlag_Decl, lexeme, token->range.min);
                CL_NodePushChild(work_top->parent, node);
                work_top->parent = node;
            }
        }
        else if ((work_top->parent->flags & CL_NodeFlag_Decl) && LexMatch(",") &&
                 (work_top->next->parent->ref->flags & CL_NodeFlag_Enum))
        {
            work_top->first_tag = work_top->last_tag = &cl_nilNode;
            work_top->parent = work_top->parent->parent;
            Assert(work_top->kind == CL_ParseWorkKind_Stmt);
            Assert(work_top->parent == work_top->next->parent);
        }
        
#define CL_IsDecl(work_top) (work_top->kind == CL_ParseWorkKind_TypeExpr && \
                             work_top->parent == work_top->next->parent  && \
                             (LexMatch("(") || LexMatch(")") || LexMatch("[") || /*can   be nested in other parens*/ \
                              LexMatch(",") || LexMatch("=") || LexMatch(";") || /*can't be nested in other parens*/ \
                              LexMatch("{") || LexMatch("}")))
        
        //- long: Declaration
        else if (CL_IsDecl(work_top) && work_top->last_tag != work_top->first_tag &&
                 (work_top->last_tag->flags & CL_NodeFlag_Identifier) && !(work_top->last_tag->flags & CL_NodeFlag_Decl))
        {
            hasName = 1;
            if (work_top->next->kind == CL_ParseWorkKind_Args || work_top->next->kind == CL_ParseWorkKind_Expr)
            {
                if (!StrCompare(work_top->first_tag->string, StrLit("*"), 0))
                {
                    hasName = 0;
                    b32 hasType = 0, hasBrace = LexMatch("{");
                    
                    for (CL_Node* tag = work_top->first_tag; tag != decl && !hasName; tag = tag->next)
                    {
                        if (HasAnyFlags(tag->flags, CL_NodeFlag_Identifier|CL_NodeFlags_Leaf|CL_NodeFlag_Decl))
                        {
                            if (hasType)
                                hasType = 0;
                            else
                                hasName = 1;
                        }
                        
                        else if (HasAnyFlags(tag->flags, CL_NodeFlags_TypeKeyword))
                        {
                            if (hasBrace)
                                hasName = 1;
                            else
                                hasType = 1;
                        }
                    }
                }
            }
            
            PUSH_DECL:
            if (hasName)
                DLLRemove_NPZ(&cl_nilNode, work_top->first_tag, work_top->last_tag, work_top->last_tag, next, prev);
            else
                decl = CL_PushNode(arena, CL_NodeFlag_Decl, StrPrefix(lexeme, 0), token->range.min);
            
            b32 isArg = 0;
            {
                CL_ParseWorkNode* parentWork = work_top->next;
                while (parentWork->kind == CL_ParseWorkKind_TypeExpr)
                    parentWork = parentWork->next;
                isArg = parentWork->kind == CL_ParseWorkKind_Args;
            }
            
            if (isArg)
                CL_NodePushArg(work_top->parent, decl);
            else
                CL_NodePushChild(work_top->parent, decl);
            
            work_top->parent = decl;
            decl->flags |= CL_NodeFlag_Decl;
            if (work_top->last_tag->flags & CL_NodeFlags_TypeKeyword)
            {
                decl->ref = work_top->last_tag;
                DLLRemove_NPZ(&cl_nilNode, work_top->first_tag, work_top->last_tag, work_top->last_tag, next, prev);
            }
            else
                decl->ref = CL_BaseTypeFromTag(work_top);
            
            if (CL_IsNil(decl->ref)) // @useless_paren
                Assert(work_top->next->kind == CL_ParseWorkKind_TypeExpr);
            else
                decl->ref->parent = decl;
            
            inc = 0;
        }
        
        else if (CL_IsDecl(work_top) && (work_top->last_tag->flags & CL_NodeFlags_TypeKeyword))
        {
            goto PUSH_DECL;
        }
        
        else if (CL_IsDecl(work_top) && (work_top->last_tag->flags & CL_NodeFlag_Identifier) &&
                 work_top->next->kind == CL_ParseWorkKind_TypeExpr)
        {
            hasName = 1;
            goto PUSH_DECL;
        }
        
        else if (CL_IsDecl(work_top) && (work_top->next->kind == CL_ParseWorkKind_Args ||
                                         work_top->next->kind == CL_ParseWorkKind_Expr) &&
                 (StrCompare(work_top->last_tag->string, StrLit("*"), 0) ||
                  HasAnyFlags(work_top->last_tag->flags, CL_NodeFlags_Leaf|CL_NodeFlag_Decl)))
        {
            goto PUSH_DECL;
        }
        
        //- long: Type's Body
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && LexMatch("{") &&
                 (CL_WorkHasFlags(work_top, CL_NodeFlags_TypeKeyword|CL_NodeFlag_Typedef) ||
                  !CL_IsNil(work_top->parent->ref)))
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
                    CL_Node* type = work_top->parent;
                    CL_Node* base = CL_PushNode(arena, type->flags, type->string, type->offset);
                    
                    DLLPushBack_NPZ(&cl_nilNode, work_top->first_tag, work_top->last_tag, base, next, prev);
                    work_top->parent = work_top->parent->parent;
                }
            }
        }
        
        //- long: Clean up
        else if (LexMatch(";"))
        {
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
        
        else if (LexMatch("}"))
        {
            while (work_top->kind != CL_ParseWorkKind_Stmt && work_top->kind != CL_ParseWorkKind_Args)
                CL_ParseWorkPop();
            if (ALWAYS(work_top != &first_work))
                CL_ParseWorkPop();
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
                CL_Node* base = work_top->parent->ref;
                while (!CL_IsNil(base->ref))
                    base = base->ref;
                
                if (!CL_IsNil(base))
                {
                    CL_Node* firstTag = base->firstTag;
                    base = CL_PushNode(arena, base->flags, base->string, base->offset);
                    
                    for (CL_Node* tag = firstTag; !CL_IsNil(tag); tag = tag->next)
                    {
                        CL_Node* newTag = CL_PushNode(arena, tag->flags, tag->string, tag->offset);
                        CL_NodePushTag(base, newTag);
                    }
                }
                
                work_top->first_tag = work_top->last_tag = base;
                if (ALWAYS((work_top->parent->flags & CL_NodeFlag_Decl) ||
                           work_top->parent->parent->ref == work_top->parent))
                    work_top->parent = work_top->next->parent;
            }
        }
        
        //- long: Type Expression
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && HasFlags(TokenFlag_Identifier))
        {
            CL_NodeFlags flags = CL_NodeFlagsFromToken(token->flags, lexeme);
            if (ALWAYS(flags))
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
                ptr->ref = base;
                base->parent = ptr;
            }
            work_top->first_tag = work_top->last_tag = ptr;
        }
        
        //- long: Push/Pop Paren
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && LexMatch("("))
        {
            if (work_top->parent != work_top->next->parent)
                CL_ParseWorkPush(CL_ParseWorkKind_Args, work_top->parent);
            
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
            CL_Node* base = work_top->parent->ref;
            if (CL_IsNil(base)) // @useless_paren
                base = work_top->parent;
            CL_ParseWorkPop();
            
            if (work_top->kind == CL_ParseWorkKind_TypeExpr)
            {
                while (!CL_IsNil(base->ref))
                    base = base->ref;
                
                work_top->parent = base;
                base->ref = work_top->first_tag;
                base->ref->parent = base;
                
                Assert(base->parent != base && base != base->ref && base->parent != base->ref);
            }
            else if (ALWAYS(work_top->kind == CL_ParseWorkKind_Args))
                CL_ParseWorkPop(); // NOTE(long): This can also be `inc = 0;`
        }
        
        //- long: Push/Pop Bracket
        else if (work_top->kind == CL_ParseWorkKind_TypeExpr && LexMatch("["))
        {
            CL_Node* node = CL_PushNode(arena, CL_NodeFlag_Brack, lexeme, token->range.min);
            CL_Node* parent = work_top->parent;
            node->ref = parent->ref;
            
            if (parent != root)
            {
                while (parent->ref->flags & CL_NodeFlag_Brack)
                    parent = parent->ref;
                
                if (!CL_IsNil(parent->ref))
                {
                    parent->ref->parent = node;
                    node->ref = parent->ref;
                }
                
                parent->ref = node;
                node->parent = parent;
            }
            else CL_NodePushChild(parent, node);
            
            CL_ParseWorkPush(CL_ParseWorkKind_Expr, node);
        }
        else if ((work_top->parent->flags & CL_NodeFlag_Brack) && LexMatch("]"))
            CL_ParseWorkPop();
        
        //- long: Expr
        else if (((work_top->parent->parent->ref->flags & CL_NodeFlag_Enum) ||
                  work_top->kind == CL_ParseWorkKind_TypeExpr) && LexMatch("="))
        {
            CL_ParseWorkPush(CL_ParseWorkKind_Expr, work_top->parent);
        }
        
        else if (work_top->kind == CL_ParseWorkKind_Expr && LexMatch(",") &&
                 (work_top->next->kind == CL_ParseWorkKind_TypeExpr ||
                  (work_top->next->parent->parent->ref->flags & CL_NodeFlag_Enum)))
        {
            CL_ParseWorkPop();
            inc = 0;
        }
        
        else if (work_top->kind == CL_ParseWorkKind_Expr && HasFlags(CL_TokenFlags_Literal|TokenFlag_Identifier))
        {
            CL_NodeFlags flags = CL_NodeFlagsFromToken(token->flags, lexeme);
            CL_Node* atom = CL_PushNode(arena, flags, lexeme, token->range.min);
            CL_Node* expr = work_top->parent;
            Assert(expr->first == expr->last);
            
            if (expr == work_top->next->parent && (expr->flags & CL_NodeFlag_Symbol))
                expr->ref = atom;
            else
                CL_NodePushChild(expr, atom);
        }
        
        else if (work_top->kind == CL_ParseWorkKind_Expr && HasFlags(TokenFlag_Symbol))
        {
            CL_Node* parent = work_top->parent;
            CL_ExprOpKind op = CL_ExprOpFromStr(lexeme, work_top);
            
            if (op)
            {
                i32 prec = CL_ExprPrec(op);
                b32 leftToRight = !CL_ExprIsRTL(op);
                
                b32 isUnary = 0;
                for (; parent != work_top->next->parent; parent = parent->parent)
                {
                    Assert(parent->flags & CL_NodeFlag_Symbol);
                    CL_ExprOpKind parentOp = CL_ExprOpFromNode(parent);
                    if (NEVER(parentOp == CL_ExprOpKind_Nil))
                        break;
                    
                    CL_ExprKind parentKind = cl_exprKindTable[parentOp];
                    if (parentKind == CL_ExprKind_Nil)
                    {
                        Assert(CL_IsNil(parent->first) && !CL_IsNil(parent->ref));
                        continue;
                    }
                    isUnary = parentKind != CL_ExprKind_Binary && parentKind != CL_ExprKind_BinaryRTL;
                    
                    i32 parentPrec = CL_ExprPrec(parentOp);
                    Assert(parentPrec);
                    if (parentPrec < prec || (parentPrec == prec && !leftToRight))
                        break;
                }
                
                CL_Node* expr = CL_PushNode(arena, CL_NodeFlag_Symbol, lexeme, token->range.min);
                b32 pushExpr = 1, popRight = 0;
                
                if (parent == work_top->next->parent)
                {
                    if (parent->flags & CL_NodeFlag_Symbol) // unclosed op
                    {
                        if (!CL_IsNil(parent->ref))
                            CL_NodePushChild(expr, parent->ref);
                        parent->ref = expr;
                        expr->parent = parent;
                        pushExpr = 0;
                    }
                    
                    else // Max parent
                    {
                        if (parent->last->flags & CL_NodeFlags_Atom)
                            popRight = 1;
                        if (work_top->parent != work_top->next->parent && (parent->last->flags & CL_NodeFlag_Symbol))
                            popRight = 1;
                    }
                }
                
                else if (parent->first != parent->last) // Binary operators
                {
                    Assert(parent->last == parent->first->next);
                    popRight = 1;
                }
                
                else if (!CL_IsNil(parent->first) && isUnary) // Unary operators
                    popRight = 1;
                
                if (pushExpr)
                {
                    if (popRight)
                    {
                        CL_Node* parentRight = parent->last;
                        DLLRemove_NPZ(&cl_nilNode, parent->first, parent->last, parentRight, next, prev);
                        CL_NodePushChild(expr, parentRight);
                    }
                    
                    CL_NodePushChild(parent, expr);
                }
                
                work_top->parent = expr;
                if (cl_exprCloseSymbol[op].size)
                    CL_ParseWorkPush(CL_ParseWorkKind_Expr, expr);
            }
            
            else if (work_top->next->parent->flags & CL_NodeFlag_Symbol)
            {
                CL_ExprOpKind topOp = CL_ExprOpFromNode(work_top->next->parent);
                if (CL_ExprIsCloseSymbol(topOp, lexeme))
                    CL_ParseWorkPop();
            }
            
            else
            {
                CL_ParseWorkPop();
                inc = 0;
            }
        }
        
        //- long: Identifier
        else if ((work_top->kind == CL_ParseWorkKind_Stmt || work_top->kind == CL_ParseWorkKind_Args) &&
                 LexMatch("*") && !CL_IsNil(work_top->first_tag))
            goto PUSH_TYPE_EXPR;
        
        else if (work_top->kind == CL_ParseWorkKind_Args && HasFlags(TokenFlag_Identifier))
            goto PUSH_TYPE_EXPR;
        
        else if (work_top->kind == CL_ParseWorkKind_Stmt && HasFlags(TokenFlag_Identifier))
        {
            CL_NodeFlags flags = CL_NodeFlagsFromToken(token->flags, lexeme);
            if (ALWAYS(flags))
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
        
        else if (work_top->kind == CL_ParseWorkKind_Stmt && HasFlags(TokenFlag_Symbol))
        {
            Assert(work_top->first_tag == work_top->last_tag);
            CL_Node* atom = work_top->first_tag;
            work_top->first_tag = work_top->last_tag = &cl_nilNode;
            
            CL_ParseWorkPush(CL_ParseWorkKind_Expr, work_top->parent);
            if (!CL_IsNil(atom))
            {
                Assert(atom->flags & CL_NodeFlags_Atom);
                CL_NodePushChild(work_top->parent, atom);
            }
            inc = 0;
        }
        
        else if (work_top->kind == CL_ParseWorkKind_Stmt && HasFlags(TokenFlag_Numeric))
        {
            Assert(work_top->first_tag == work_top->last_tag);
            CL_Node* atom = CL_PushNode(arena, CL_NodeFlag_Numeric, lexeme, token->range.min);
            CL_ParseWorkPush(CL_ParseWorkKind_Expr, work_top->parent);
            CL_NodePushChild(work_top->parent, atom);
        }
        
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
    }
    
    //- rjf: fill & return
    CL_DebugTree(root);
    CL_ParseResult result = { root, msgs };
    ScratchEnd(scratch);
    return result;
}

function MetaTable CL_TableFromTokens(Arena* arena, String text, TokenArray array)
{
    MetaTable result = {0};
    u64 cap = array.count / 3;
    
#define SkipNextNoWhitespace(token) Stmnt(token++; \
                                          while ((token->flags & CL_TokenFlags_Ignorable) && token < opl) token++;)
    for (Token* token = array.tokens,* opl = array.tokens+array.count; token < opl; ++token)
    {
        MetaFlags flags = 0;
        String name = {0};
        String path = {0};
        
        if (token->flags & TokenFlag_Identifier)
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
                    if (token && (token->flags & TokenFlag_Identifier))
                    {
                        name = StrFromToken(text, *token);
                        
                        if (flags & MetaFlag_EmbedFile)
                        {
                            SkipNextNoWhitespace(token);
                            if (token && TokenMatch(text, *token, StrLit(",")))
                            {
                                SkipNextNoWhitespace(token);
                                if (token && (token->flags & TokenFlag_String))
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
