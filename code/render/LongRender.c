
//~ long: Batch Functions

function void R_BatchPushQuad(Arena* arena, R_Batch* batch, R_Quad* quad)
{
    R_QuadNode* tail = batch->last;
    if (tail == 0 || tail->count >= R_QUAD_CHUNK_CAP)
    {
        R_QuadNode* node = PushStruct(arena, R_QuadNode);
        SLLQueuePush(batch->first, batch->last, node);
        batch->nodeCount += 1;
        tail = batch->last;
    }
    
    if (tail->quads == 0)
        tail->quads = PushArray(arena, R_Quad, R_QUAD_CHUNK_CAP);
    
    tail->quads[tail->count] = *quad;
    tail->count++;
    batch->totalCount++;
}

function void R_BatchPushStr(Arena* arena, R_Batch* batch, FNT_Baked* font, String str, v2f32 p, u32 c, r2f32* clip)
{
    FNT_GlyphLayout* fallback = FNT_GlyphFromCP(font, '?');
    v2f32 pos = p;
    
    for (u8* ptr = str.str,* opl = ptr + str.size; ptr < opl; ++ptr)
    {
        FNT_GlyphLayout* layout = 0;
        {
            u32 codepoint = *ptr;
            // TODO(long): Handle UTF-8
            if (ALWAYS(codepoint < 0x80))
                layout = FNT_GlyphFromCP(font, codepoint);
            
            if (!layout)
                layout = fallback;
        }
        
        r2i32 xyTex = R2I32Size(layout->offset, DimR2I32(layout->xy));
        r2f32 xy = ShiftR2F32(R2F32R2(xyTex), pos);
        pos.x += (f32)layout->advance;
        
        R_Quad quad = (R_Quad){ xy, 0.f, 10000.f, 0.f, 0, { c, c }, layout->uv };
        if (clip)
        {
            quad.flags |= R_QuadFlag_Clipped;
            quad.clip = *clip;
        }
        
        R_BatchPushQuad(arena, batch, &quad);
    }
}

function void R_ListPushBatch(Arena* arena, R_List* list)
{
    R_Batch* batch = PushStruct(arena, R_Batch);
    SLLQueuePush(list->first, list->last, batch);
    list->batchCount++;
}

function void R_ListPrepBatch(Arena* arena, R_List* list, R_Texture texture)
{
    // NOTE(long): The only reason we need R_Batch is that textures are not bindless
    // (see R_Submit for details)
    // If they were, we could just use R_QuadNode directly, with each R_Quad storing its texture
    // (R_List would then be made of R_QuadNode instead of R_Batch)
    // R_Submit would no longer take a texture, nor would R_Ctx
    
    R_Batch* last = list->last;
    if (last == 0 || (texture && last->texture != texture))
    {
        R_ListPushBatch(arena, list);
        list->last->texture = texture;
    }
}

function void R_ListFlush(R_List* list)
{
    for (R_Batch* batch = list->first; batch != 0; batch = batch->next)
        R_Submit(batch->first, batch->totalCount, batch->texture);
}

//~ long: Helper Functions

function R_Font R_FontBakeTexture(Arena* arena, FNT_Font* font, FNT_Packer* pack)
{
    FNT_Baked* baked = PushStruct(arena, FNT_Baked);
    *baked = FNT_FontBake(arena, font, pack);
    R_Texture texture = R_TextureCreate(baked->size.x, baked->size.y, R_TextureFmt_R8, 0);
    
    for (FNT_Glyph* glyph = font->first; glyph; glyph = glyph->next)
    {
        if (glyph->size.x == 0)
            continue;
        
        FNT_GlyphLayout* layout = FNT_GlyphFromCP(baked, glyph->codepoint);
        R_TextureUpdate(texture, layout->xy, glyph->bitmap);
    }
    
    return (R_Font){ baked, texture };
}

function R_Ctx R_CtxMake(Arena* arena, R_List* list)
{
    R_Ctx result = (R_Ctx){ .arena = arena };
    if (!list)
        list = PushStruct(arena, R_List);
    result.list = list;
    return result;
}

function void R_CtxClip(R_Ctx* ctx, r2f32* clip)
{
    ctx->enableClip = !!clip;
    if (ctx->enableClip)
        ctx->clip = *clip;
}

function void R_CtxFont(R_Ctx* ctx, R_Font font)
{
    if (font.baked || font.texture)
        ctx->font = font;
}

function void R_CtxFlush(R_Ctx* ctx)
{
    R_ListFlush(ctx->list);
}

function void R_PushRect(R_Ctx* ctx, r2f32 xy, f32 r, u32 c)
{
    R_PushQuad(ctx, &(R_Quad){ xy, r, 10000.f, 0.f, 0, { c, c }}, 0);
}

function void R_PushLine(R_Ctx* ctx, r2f32 xy, f32 r, u32 c)
{
    // TODO(long): Optimize this
    v2f32 v = SubV2F32(xy.p1, xy.p0);
    f32 theta = AngleV2F32(v);
    f32 length = MagV2F32(v);
    
    v2f32 center = CenterR2F32(xy);
    v2f32 extents = V2F32(length*.5f + r, r);
    r2f32 rect = R2F32(SubV2F32(center, extents), AddV2F32(center, extents));
    R_PushQuad(ctx, &(R_Quad){ rect, r, 10000.f, theta, 0, .c = { c, c }, }, 0);
}

function void R_PushQuad(R_Ctx* ctx, R_Quad* quad, R_Texture texture)
{
    R_ListPrepBatch(ctx->arena, ctx->list, texture);
    if (!(quad->flags & R_QuadFlag_Clipped) && ctx->enableClip)
    {
        quad->flags |= R_QuadFlag_Clipped;
        quad->clip = ctx->clip;
    }
    R_BatchPushQuad(ctx->arena, ctx->list->last, quad);
}

function void R_PushChar(R_Ctx* ctx, u32 cp, v2f32 p, u32 c)
{
    R_Font font = ctx->font;
    FNT_GlyphLayout* layout = FNT_GlyphFromCP(font.baked, cp);
    if (layout)
    {
        r2i32 xyTex = R2I32Size(layout->offset, DimR2I32(layout->xy));
        r2f32 xy = ShiftR2F32(R2F32R2(xyTex), p);
        R_PushQuad(ctx, &(R_Quad){ xy, 0.f, 10000.f, 0.f, 0, { c, c }, layout->uv }, font.texture);
    }
}

function void R_PushStr(R_Ctx* ctx, String str, v2f32 p, u32 c)
{
    R_ListPrepBatch(ctx->arena, ctx->list, ctx->font.texture);
    R_BatchPushStr(ctx->arena, ctx->list->last, ctx->font.baked,
                   str, p, c, ctx->enableClip ? &ctx->clip : 0);
}
