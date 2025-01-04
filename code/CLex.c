
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
    u64 ptrCount = 0;
    REPEAT:
    while (node->flags & MD_NodeFlag_Symbol)
    {
        for (u64 i = 0; i < node->string.size; ++i, ++ptrCount)
            if (node->string.str[i] != '*')
                goto DONE;
        node = node->next;
    }
    
    if (MD_NodeCompareArr(node, CL_Qualifiers, ArrayCount(CL_Qualifiers)))
    {
        node = node->next;
        goto REPEAT;
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
    
    b32 isTypedef = StrCompare(node->prev->string, StrLit("typedef"), 0);
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

function CL_Node* CL_MDParseText(Arena* arena, String filename, String text)
{
    CL_Node* root = CL_PushNode(arena, 0, &md_nil_node, &md_nil_node, 0);
    MD_ParseResult parse = MD_ParseText(arena, filename, text);
    String typeKeywords[] = { StrLit("struct"), StrLit("union"), StrLit("enum") };
    
    for (MD_EachNode(node, parse.root->first))
    {
        CL_Node* typeNode = 0;
        
        // NOTE(long): This doesn't parse types (struct/union/enum) or functions. Those are parsed later.
        if (StrCompare(node->string, StrLit("typedef"), 0))
        {
            MD_Node* base = node->next;
            if ((base->flags & MD_NodeFlag_Identifier) && !MD_NodeCompareArr(base, typeKeywords, ArrayCount(typeKeywords)))
            {
                MD_Node* name = CL_NodeSkipBase(base->next);
                
                if (name->flags & MD_NodeFlag_Identifier && !(name->next->flags & MD_NodeFlag_HasParenLeft))
                {
                    typeNode = CL_PushNode(arena, CL_NodeType_Typedef, name, base, root);
                    node = name;
                }
            }
        }
        
        else if (StrCompare(node->string, StrLit("enum"), 0))
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
        
        else if (StrCompare(node->string, StrLit("union"), 0))
        {
            typeNode = CL_PushType(arena, CL_NodeType_Union, MD_NodeFlag_HasBraceLeft, node, root);
            goto PARSE_BODY;
        }
        
        else if (StrCompare(node->string, StrLit("struct"), 0))
        {
            typeNode = CL_PushType(arena, CL_NodeType_Struct, MD_NodeFlag_HasBraceLeft, node, root);
            
            PARSE_BODY:
            MD_Node* typeBody = typeNode->body;
            MD_Node* firstMember = typeBody->first;
            
            if (firstMember->flags & MD_NodeFlag_Identifier)
            {
                for (MD_EachNode(member, firstMember))
                {
                    REPEAT:
                    MD_Node* memberType = member;
                    MD_Node* memberName = member->next;
                    
                    if (MD_NodeCompareArr(memberType, ArrayExpand(String, StrLit("struct"), StrLit("union"))))
                    {
                        if (memberName->flags & MD_NodeFlag_HasBraceLeft)
                        {
                            if (memberName->flags & MD_NodeFlag_IsBeforeSemicolon)
                            {
                                member = memberName->first;
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
                        CL_PushNode(arena, CL_NodeType_Decl, memberName, memberType, typeNode);
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
                        
                        if (memberName->flags & MD_NodeFlag_IsBeforeComma)
                        {
                            memberName = memberName->next;
                            goto NEXT_MEMBER;
                        }
                        
                        member = memberName;
                        while (!(member->next->flags & MD_NodeFlag_IsAfterSemicolon) && !md_node_is_nil(member->next))
                            member = member->next;
                        
                        while (member->parent != typeBody)
                        {
                            if (md_node_is_nil(member->next))
                                member = member->parent;
                            else
                                break;
                        }
                    }
                    else
                    {
                        // TODO(long): Error if false
                        // NOTE(long): MD_NodeFlag_HasParenLeft means this is a complex declaration
                    }
                }
            }
        }
        
        // TODO(long): declaration
        //else if ((node->flags & MD_NodeFlag_Identifier) && (node->next->flags & MD_NodeFlag_Identifier) &&
        //(HasAnyFlags(node->prev->flags, MD_NodeFlag_HasBraceRight|MD_NodeFlag_IsBeforeSemicolon) || 
        //md_node_is_nil(node->prev)))
        //{
        //}
        
        else if ((node->flags & MD_NodeFlag_Identifier) && (node->next->flags & MD_NodeFlag_HasParenLeft) /*&&
                 ((node->next->flags & MD_NodeFlag_IsBeforeSemicolon) || (node->next->next->flags & MD_NodeFlag_HasBraceLeft))*/)
        {
            typeNode = CL_PushType(arena, CL_NodeType_Proc, MD_NodeFlag_HasParenLeft, node->prev, root);
            MD_Node* firstArg = typeNode->body->first;
            
            if (!md_node_is_nil(firstArg->next))
            {
                for (MD_EachNode(arg, firstArg))
                {
                    if (arg->flags & MD_NodeFlag_Identifier)
                    {
                        MD_Node* argType = arg;
                        arg = CL_NodeSkipBase(arg->next);
                        
                        b32 has_name = (arg->flags & MD_NodeFlag_Identifier) && !(arg->flags & MD_NodeFlag_IsAfterComma);
                        CL_PushNode(arena, CL_NodeType_Decl, has_name ? arg : &md_nil_node, argType, typeNode);
                    }
                    else break;
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
