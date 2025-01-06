
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

global String CL_Qualifiers[] = {
    StrConst("const"), StrConst("volatile"), StrConst("restrict"), // Type Qualifiers
    StrConst("extern"), StrConst("static"), // Storage Class Specifiers
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

typedef struct CL_ParseCtx CL_ParseCtx;
struct CL_ParseCtx
{
    String data;
    CL_Node* parent;
    MD_Node* iter;
    Arena* arena;
};

//function CL_Node* CL_ParseStmt(CL_ParseCtx* parse)
//{
//MD_Node* node = CL_CurrNode(parse);
//if ((node->flags & MD_NodeFlag_Identifier) && !(node->flags & MD_NodeFlag_IsBeforeSemicolon) && !md_node_is_nil(node->next))
//{
//CL_Node* base = 0;
//if (MD_NodeMatch(node, "typedef"))
//CL_ParseTypedef(parse);
//else if (MD_NodeMatch(node, "struct"))
//base = CL_ParseStruct(parse);
//else if (MD_NodeMatch(node, "union"))
//base = CL_ParseUnion(parse);
//else if (MD_NodeMatch(node, "enum"))
//base = CL_ParseEnum(parse);
//else
//base = CL_ParseVarType(parse);

//if (base)
//{
//MD_Node* name = CL_CurrNode(parse);
//if ((name->flags & MD_NodeFlag_Identifier) && !(name->flags & MD_NodeFlag_IsAfterSemicolon))
//{
//if (body->flags & MD_NodeFlag_HasParenLeft)
//{
//CL_Node* proc = CL_ParseProc(context);
//}

//else if (MD_NodeMatch(name->next, "=") || (name->flags & MD_NodeFlag_IsBeforeSemicolon))
//{
//typeNode = CL_PushNode(arena, isTypedef ? CL_NodeType_Typedef : CL_NodeType_Decl, name, base, root);
//while (!(name->flags & MD_NodeFlag_IsBeforeSemicolon) && !md_node_is_nil(name->next))
//name = name->next;
//node = name;
//}
//CL_PushNode(parse, type, name, base);
//}
//}
//}
//else
//{
//CL_Node* expr = CL_ParseExpr(arena, node, 1);
//if (!CL_NodeIsNil(expr))
//CL_PushChild(parent, expr);
//}
//}

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
