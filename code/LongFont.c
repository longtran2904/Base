
function v2i32 FNT_PackRect(FNT_Packer* pack, v2i32 rectSize)
{
    b32 success = 1;
    v2i32 p = {0};
    
    if (pack->size.x == 0) pack->size.x = 512;
    if (pack->size.y == 0) pack->size.y = 512;
    if (pack->maxSize.x == 0) pack->maxSize.x = KiB(8);
    if (pack->maxSize.y == 0) pack->maxSize.y = KiB(8);
    
    if (rectSize.x <= pack->maxSize.x && rectSize.y <= pack->maxSize.y)
    {
        b32 fat = pack->size.x > pack->size.y;
        
        if (pack->p.x + rectSize.x > pack->size.x)
        {
            pack->p.x = fat ? pack->size.x/2 : 0;
            pack->p.y += pack->lineHeight;
            pack->lineHeight = 0;
        }
        
        if (pack->p.y + rectSize.y > pack->size.y)
        {
            if (fat)
            {
                pack->p = V2I32(0, pack->size.y);
                pack->size.y *= 2;
            }
            
            else
            {
                pack->p = V2I32(pack->size.x, 0);
                pack->size.x *= 2;
            }
            
            pack->lineHeight = 0;
        }
        
        p = pack->p;
        pack->p.x += rectSize.x;
        pack->lineHeight = Max(pack->lineHeight, rectSize.y);
        
        pack->size.x = Max(pack->size.x, pack->p.x);
        pack->size.y = Max(pack->size.y, pack->p.y + pack->lineHeight);
        
        if (pack->size.x <= pack->maxSize.x && pack->size.y <= pack->maxSize.y)
            success = 1;
    }
    
    if (!success)
    {
        p = (v2i32){0};
        ErrorFmt("The rect size (%d, %d) is too big to fit inside (%d, %d)",
                 rectSize.x, rectSize.y, pack->size.x, pack->size.y);
    }
    return p;
}

function FNT_Baked FNT_FontBake(Arena* arena, FNT_Font* font, FNT_Packer* pack)
{
    FNT_Baked result = { .metrics = font->metrics, };
    if (!font->first)
        return result;
    
    pack->p = (v2i32){0};
    pack->lineHeight = 0;
    
    for (FNT_Glyph* glyph = font->first; glyph; glyph = glyph->next)
        result.glyphCount++;
    
    u32 slotCount = RoundUpPow2(result.glyphCount) * 2;
    result.slotCount = slotCount;
    result.slots = PushArray(arena, FNT_MapSlot, result.slotCount);
    result.glyphs = PushArray(arena, FNT_GlyphLayout, result.glyphCount);
    
    u32 glyphIdx = 0;
    for (FNT_Glyph* glyph = font->first; glyph; glyph = glyph->next, ++glyphIdx)
    {
        u32 cp = glyph->codepoint;
        
        FNT_MapNode* node = PushStruct(arena, FNT_MapNode);
        node->codepoint = cp;
        node->index = glyphIdx;
        
        FNT_MapSlot* slot = result.slots + (cp % slotCount);
        SLLQueuePush(slot->first, slot->last, node);
        
        v2i32 p = FNT_PackRect(pack, glyph->size);
        FNT_GlyphLayout* layout = result.glyphs + glyphIdx;
        layout->xy = R2I32Size(p, glyph->size);
        layout->offset = glyph->offset;
        layout->advance = glyph->advance;
    }
    
    result.size = pack->size;
    for (u32 i = 0; i < result.glyphCount; ++i)
    {
        FNT_GlyphLayout* layout = result.glyphs+i;
        v2f32 min = V2F32(DivF32(layout->xy.x0, pack->size.x), DivF32(layout->xy.y0, pack->size.y));
        v2f32 max = V2F32(DivF32(layout->xy.x1, pack->size.x), DivF32(layout->xy.y1, pack->size.y));
        layout->uv = R2F32(min, max);
    }
    
    return result;
}

function FNT_GlyphLayout* FNT_GlyphFromCP(FNT_Baked* font, u32 codepoint)
{
    u32 slotIdx = codepoint % font->slotCount;
    for (FNT_MapNode* node = font->slots[slotIdx].first; node; node = node->next)
        if (node->codepoint == codepoint)
            return font->glyphs + node->index;
    return 0;
}
