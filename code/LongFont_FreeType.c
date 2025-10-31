#pragma push_macro("internal")
#undef internal
#include "freetype/freetype.h"
#pragma pop_macro("internal")

function FNT_Font FNT_FontOpen(Arena* arena, FNT_LoadParams* params)
{
    ScratchBegin(scratch, arena);
    TempArena temp = TempBegin(arena);
    b32 error = 0;
    FNT_Font font = {0};
    
    // TODO(long): If size and dpi are zero, use some default values
    font.params = *params;
    
    //- long: FreeType Init
    FT_Library ft = {0};
    FT_Error initError = FT_Init_FreeType(&ft);
    error = initError;
    
    FT_Face face = {0};
    if (!error)
    {
        String data = OSReadFile(scratch, params->path);
        error = FT_Open_Face(ft, &(FT_Open_Args){
                                 .flags = FT_OPEN_MEMORY,
                                 .memory_base = data.str,
                                 .memory_size = (FT_Long)data.size,
                             }, 0, &face);
    }
    
    //- long: Sizing
    if (!error)
    {
        FT_F26Dot6 charSize = params->size << 6;
        FT_UInt dpi = (FT_UInt)params->dpi;
        error = FT_Set_Char_Size(face, charSize, charSize, dpi, dpi);
    }
    
    //- long: Metrics Calculation
    FNT_Metrics* met = &font.metrics;
    if (!error)
    {
        met->upem       = (f32)face->units_per_EM;
        met->ascender   = (f32)(face->size->metrics.ascender    >> 6);
        met->descender  = (f32)(face->size->metrics.descender   >> 6);
        met->maxAdvance = (f32)(face->size->metrics.max_advance >> 6);
        met->textHeight = (f32)(face->size->metrics.height      >> 6);
        met->lineHeight = met->textHeight + ClampBot(1.f, met->textHeight - (met->ascender - met->descender));
    }
    
    //- long: Codepoint -> Glyph
    if (!error)
    {
        FT_UInt glyphIdx = 0;
        FT_ULong codepoint = FT_Get_First_Char(face, &glyphIdx);
        
        while (glyphIdx != 0)
        {
            FNT_Glyph* glyph = PushStruct(arena, FNT_Glyph);
            glyph->codepoint = codepoint;
            glyph->index = glyphIdx;
            
            SLLQueuePush(font.first, font.last, glyph);
            codepoint = FT_Get_Next_Char(face, codepoint, &glyphIdx);
        }
    }
    
    //- long: Glyph Rasterization
    if (!error)
    {
        u32 loadFlags = FT_LOAD_RENDER;
        if (params->flags & FNT_RasterFlag_Hinted)
            loadFlags |= FT_LOAD_FORCE_AUTOHINT|(FT_LOAD_TARGET_LIGHT*
                                                 ((params->flags & FNT_RasterFlag_Light) == FNT_RasterFlag_Light));
        else
            loadFlags |= FT_LOAD_NO_AUTOHINT|FT_LOAD_NO_HINTING;
        
        for (FNT_Glyph* glyph = font.first; glyph; glyph = glyph->next)
        {
            if (FT_Load_Glyph(face, glyph->index, loadFlags))
                continue;
            
            FT_GlyphSlot ftGlyph = face->glyph;
            glyph->advance = ftGlyph->advance.x >> 6;
            glyph->offset = V2I32(ftGlyph->bitmap_left, ftGlyph->bitmap_top);
            
            FT_Bitmap bitmap = ftGlyph->bitmap;
            Assert(bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
            glyph->size = V2I32(bitmap.width, bitmap.rows);
            Assert((bitmap.width == 0 && bitmap.rows == 0) || (bitmap.width != 0 && bitmap.rows != 0));
            
            u8* srcLine = bitmap.buffer;
            if (srcLine)
            {
                // NOTE(long): FreeType bitmaps are always stored top-to-bottom in memory
                // If bitmap.pitch is negative, bitmap.buffer points to the last (bottom) row
                // We flip the pitch here because our renderer expects the origin at the bottom-left
                i32 pitch = -bitmap.pitch;
                if (pitch < 0)
                    srcLine += -pitch * (bitmap.rows - 1);
                
                u8* dst = glyph->bitmap = PushArray(arena, u8, bitmap.width * bitmap.rows);
                b32 mono = !(params->flags & FNT_RasterFlag_Smooth);
                
                for (u32 y = 0; y < bitmap.rows; ++y)
                {
                    u8* src = srcLine;
                    for (u32 x = 0; x < bitmap.width; ++x)
                    {
                        u8 pixel = *src++;
                        if (mono)
                            pixel = (pixel >= 128) * 255;
                        *dst++ = pixel;
                    }
                    
                    srcLine += pitch;
                }
                
                glyph->offset.y -= bitmap.rows;
            }
        }
    }
    
    //- long: Clean up
    if (!initError)
        FT_Done_FreeType(ft);
    
    if (error)
    {
        font = (FNT_Font){0};
        TempEnd(temp);
    }
    
    ScratchEnd(scratch);
    return font;
}
