
//~ long: Helper Functions

function void R_QuadPush(Arena* arena, R_QuadList* list, R_Quad* quad)
{
    R_QuadNode* tail = list->last;
    if (tail == 0 || tail->count >= R_QUAD_CHUNK_CAP)
    {
        R_QuadNode* node = PushStruct(arena, R_QuadNode);
        SLLQueuePush(list->first, list->last, node);
        list->nodeCount++;
        tail = node;
    }
    
    if (tail->quads == 0)
        tail->quads = PushArray(arena, R_Quad, R_QUAD_CHUNK_CAP);
    
    tail->quads[tail->count] = *quad;
    tail->count++;
    list->totalCount++;
}

function R_Font R_FontBakeTexture(Arena* arena, FNT_Font* font, FNT_Packer* pack)
{
    FNT_Baked baked = FNT_FontBake(arena, font, pack);
    R_Texture* texture = R_TextureCreate(baked.size.x, baked.size.y, 0);
    
    for (FNT_Glyph* glyph = font->first; glyph; glyph = glyph->next)
    {
        if (glyph->size.x == 0)
            continue;
        
        FNT_GlyphLayout* layout = FNT_GlyphFromCP(&baked, glyph->codepoint);
        R_TextureUpdate(texture, layout->xy, glyph->bitmap);
    }
    
    return (R_Font){ baked, texture };
}

function void R_StrPush(Arena* arena, R_QuadList* list, FNT_Baked* font, String str, v2f32 p, v4f32 color)
{
    FNT_GlyphLayout* fallback = FNT_GlyphFromCP(font, '?');
    v2f32 pos = p;
    
    for (u8* ptr = str.str,* opl = ptr + str.size; ptr < opl; ++ptr)
    {
        FNT_GlyphLayout* layout = 0;
        {
            u32 codepoint = *ptr;
            if (ALWAYS(codepoint < 0x80))
                layout = FNT_GlyphFromCP(font, codepoint);
            
            if (!layout)
                layout = fallback;
        }
        
        r2i32 xyTex = R2I32Size(layout->offset, DimR2I32(layout->xy));
        r2f32 xy = ShiftR2F32(R2F32R(xyTex), pos);
        pos.x += (f32)layout->advance;
        
        R_QuadPush(arena, list, &(R_Quad){ xy, layout->uv, 0.f, 10000.f, 0.f, color, color });
    }
}

//~ long: Context Functions

threadvar R_Ctx* drawCtx;

function void R_CtxBegin(void)
{
    if (drawCtx == 0)
    {
        Arena* arena = ArenaMake(1);
        drawCtx = PushStruct(arena, R_Ctx);
        drawCtx->arena = arena;
        drawCtx->basePos = ArenaPos(arena);
    }
    
    ZeroStruct(&drawCtx->list);
    drawCtx->font.texture = 0;
}

function void R_CtxEnd(void)
{
    R_Submit(drawCtx->list.first, drawCtx->list.totalCount, drawCtx->font.texture);
    ArenaPopTo(drawCtx->arena, drawCtx->basePos);
    
    ZeroStruct(&drawCtx->list);
    drawCtx->font.texture = 0;
}

function void R_CtxPushQuad(R_Quad* quad)
{
    R_QuadPush(drawCtx->arena, &drawCtx->list, quad);
}

function void R_CtxPushStr(R_Font* font, String str, v2f32 p, v4f32 color)
{
    R_Ctx* ctx = drawCtx;
    if (ctx->font.texture && ctx->font.texture != font->texture)
        R_CtxEnd();
    
    ctx->font = *font;
    R_StrPush(ctx->arena, &ctx->list, &font->baked, str, p, color);
}

function void R_CtxPushChar(R_Font* font, u32 codepoint, v2f32 p, v4f32 color)
{
    R_Ctx* ctx = drawCtx;
    if (ctx->font.texture && ctx->font.texture != font->texture)
        R_CtxEnd();
    ctx->font = *font;
    
    FNT_GlyphLayout* layout = FNT_GlyphFromCP(&font->baked, codepoint);
    if (layout)
    {
        r2i32 xyTex = R2I32Size(layout->offset, DimR2I32(layout->xy));
        r2f32 xy = ShiftR2F32(R2F32R(xyTex), p);
        R_QuadPush(ctx->arena, &ctx->list, &(R_Quad){ xy, layout->uv, 0.f, 10000.f, 0.f, color, color });
    }
}
