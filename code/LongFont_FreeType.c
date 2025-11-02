
#pragma push_macro("internal")
#undef internal
#include "freetype/freetype.h"
#pragma pop_macro("internal")

function FNT_Font FNT_FontOpen(Arena* arena, FNT_LoadParams* params)
{
    FNT_Font font = {0};
    FNT_LoadParams p = *params;
    if (p.path.size == 0)
        return font;
    
    if (p.size == 0) p.size = 32;
    if (p. dpi == 0) p. dpi = 96; // TODO(long): Query the platform DPI instead of using a fixed value
    font.params = p;
    
    ScratchBegin(scratch, arena);
    TempArena temp = TempBegin(arena);
    b32 error = 0;
    
    //- long: FreeType Init
    FT_Library ft = {0};
    FT_Error initError = FT_Init_FreeType(&ft);
    if (initError)
        ErrorSet(error, "Failed to initialize FreeType");
    
    FT_Face face = {0};
    if (!error)
    {
        String data = OSReadFile(scratch, p.path);
        FT_Open_Args args = { FT_OPEN_MEMORY, data.str, (FT_Long)data.size, };
        if (FT_Open_Face(ft, &args, 0, &face))
            ErrorSet(error, "Failed to load the font: %.*s", StrExpand(p.path));
    }
    
    //- long: Sizing
    if (!error)
    {
        FT_F26Dot6 charSize = p.size << 6;
        FT_UInt dpi = (FT_UInt)p.dpi;
        if (FT_Set_Char_Size(face, charSize, charSize, dpi, dpi))
            ErrorSet(error, "Failed to set the font size: %u", p.size);
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
        if (p.flags & FNT_RasterFlag_Hinted)
            loadFlags |= FT_LOAD_FORCE_AUTOHINT|(FT_LOAD_TARGET_LIGHT*
                                                 ((p.flags & FNT_RasterFlag_Light) == FNT_RasterFlag_Light));
        else
            loadFlags |= FT_LOAD_NO_AUTOHINT|FT_LOAD_NO_HINTING;
        
        for (FNT_Glyph* glyph = font.first; glyph; glyph = glyph->next)
        {
            if (FT_Load_Glyph(face, glyph->index, loadFlags))
            {
                ErrorFmt("Failed to load glyph %u at index %u", glyph->codepoint, glyph->index);
                continue;
            }
            
            FT_GlyphSlot ftGlyph = face->glyph;
            glyph->advance = ftGlyph->advance.x >> 6;
            glyph->offset = V2I32(ftGlyph->bitmap_left, ftGlyph->bitmap_top);
            
            FT_Bitmap bitmap = ftGlyph->bitmap;
            Assert(bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);
            Assert(!!bitmap.width == !!bitmap.rows);
            glyph->size = V2I32(bitmap.width, bitmap.rows);
            
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
                b32 mono = !(p.flags & FNT_RasterFlag_Smooth);
                
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
