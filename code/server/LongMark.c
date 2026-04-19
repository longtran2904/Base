
function LM_ParseResult LM_ParseText(Arena *arena, String filename, String text)
{
    ScratchBegin(scratch, arena);
    TokenChunkList tokens = {0};
    u8* firstByte = text.str;
    u8* oplByte = firstByte + text.size;
    u8* byte = firstByte;
    
    //- long: Scanning
    while (byte < oplByte)
    {
        TokenFlags flags = 0;
        u64 start = byte - firstByte;
        
#define TokenInit(flg, size) Stmnt(flags = (flg); byte += (size))
#define Check2Bytes(str) (byte+1 < oplByte && byte[0] == (str)[0] && byte[1] == (str)[1])
        if (0) { }
        
        // #
        // paragraph
        // * _
        // ** __
        // ~~
        // >
        // - + *
        // 1. 1)
        // - [ ]
        // `
        // ```
        // --- *** ___
        
        // escaping
        // <br>
        
        //- long: Heading
        else if (NoParent() && *byte == '#')
        {
            type = LM_NodeType_Heading;
            while (*byte++ == '#')
            {
                IncreaseParentHeading();
            }
        }
        
        //- long: Quote
        else if (NoParent() && *byte == '>')
        {
            type = LM_NodeType_Quote;
            byte += 1;
        }
        
        //- long: Strong
        else if (HasParent() && (Check2Bytes("**") || Check2Bytes("__")))
        {
            u8 style = byte[0];
            b32 prevIsSpace = ;
            b32 nextIsSpace = ;
            
            if (IsParentEnd(style))
            {
                if (!prevIsSpace)
                    if (style != '_' || nextIsSpace)
                        PopParent();
            }
            
            else
            {
                if (!nextIsSpace)
                    if (style != '_' || prevIsSpace)
                        type = LM_NodeType_Strong;
            }
            
            byte += 2;
        }
        
        //- long: Emph
        else if (HasParent() && (*byte == '*' || *byte == '_'))
        {
            type = LM_NodeType_Emph;
            byte += 1;
            goto STRONG;
        }
        
        //- long: Strikethrough
        else if (HasParent() && Check2Bytes("~~"))
        {
            type = LM_NodeType_Strike;
            byte += 2;
        }
        
        //- long: Paragraph
        else
        {
            type = LM_NodeType_TextBlock;
        }
        
        //- long: Invalid characters
        else byte++;
        
#undef TokenInit
#undef Check2Bytes
        
        //- long: Push Token
        u64 opl = byte - firstByte;
        Assert(opl > start);
        
        if (!(flags & CL_TokenFlags_Ignorable))
            blankLine = 0;
        if (isPreproc)
            flags |= TokenFlag_Preproc;
        
        Token token = { .flags = flags, .range = {start, opl} };
        TokenChunkListPush(scratch, &tokens, 4096, token);
    }
    
    TokenArray result = TokenArrayFromChunkList(arena, &tokens);
    ScratchEnd(scratch);
    
    for (u64 i = 1; i < result.count; ++i)
    {
        Token token1 = result.tokens[i-1];
        Token token2 = result.tokens[i];
        Assert(token2.range.min == token1.range.max);
        
        if (i == 1)
            Assert(token1.range.min == 0);
        if (i == result.count - 1)
            Assert(token2.range.max == text.size);
    }
    
    return result;
}

function String LM_RenderHTML(Arena* arena, LM_Node* root)
{
    for (LM_Node* node = root->first; node; node = node->next)
    {
        switch (node->type)
        {
            
        }
    }
}
