/* date = October 25th 2025 3:01 pm */

#ifndef _LONG_FONT_H
#define _LONG_FONT_H

//~ long: Font Loose Types

typedef struct FNT_Glyph FNT_Glyph;
struct FNT_Glyph
{
    FNT_Glyph* next;
    u8* bitmap;
    
    v2i32 size;
    v2i32 offset;
    u32 advance;
    
    u32 codepoint;
    u32 index;
};

typedef struct FNT_Metrics FNT_Metrics;
struct FNT_Metrics
{
    f32 upem;
    f32 ascender;
    f32 descender;
    f32 maxAdvance;
    
    f32 textHeight;
    f32 lineHeight;
#define FNT_LineGap(m) ((m).lineHeight - (m).textHeight)
};

typedef Flags8 FNT_RasterFlags;
enum
{
    FNT_RasterFlag_Smooth = (1<<0),
    FNT_RasterFlag_Hinted = (1<<1),
    FNT_RasterFlag_Light  = (1<<2)|FNT_RasterFlag_Hinted,
};

typedef struct FNT_LoadParams FNT_LoadParams;
struct FNT_LoadParams
{
    FNT_RasterFlags flags;
    u16 size;
    u16 dpi;
    String path;
};

typedef struct FNT_Font FNT_Font;
struct FNT_Font
{
    FNT_Glyph* first;
    FNT_Glyph* last;
    FNT_LoadParams params;
    FNT_Metrics metrics;
};

//~ long: Font Baked Types

typedef struct FNT_MapNode FNT_MapNode;
struct FNT_MapNode
{
    FNT_MapNode *next;
    u32 codepoint;
    u32 index;
};

typedef struct FNT_MapSlot FNT_MapSlot;
struct FNT_MapSlot
{
    FNT_MapNode *first;
    FNT_MapNode *last;
};

typedef struct FNT_GlyphLayout FNT_GlyphLayout;
struct FNT_GlyphLayout
{
    r2i32 xy;
    r2f32 uv;
    v2i32 offset;
    u32 advance;
};

typedef struct FNT_Baked FNT_Baked;
struct FNT_Baked
{
#define FNT_CP_SLOT_COUNT 128
    FNT_MapSlot* slots;
    FNT_GlyphLayout* glyphs;
    
    u32 count;
    v2i32 size;
    
    FNT_Metrics metrics;
};

//~ long: Font Packer

typedef struct FNT_Packer FNT_Packer;
struct FNT_Packer
{
    // NOTE(long): Public fields that can be set before calling FontBake
    // Values should be powers of two; zero uses defaults
    v2i32 maxSize;
    v2i32 size;
    
    // NOTE(long): These fields are for internal use by FontBake and must not be modified
    v2i32 p;
    i32 lineHeight;
};

//~ long: Font Functions

function FNT_Font FNT_FontOpen(Arena* arena, FNT_LoadParams* params);

function v2i32 FNT_PackRect(FNT_Packer* pack, v2i32 dim);
function FNT_Baked FNT_FontBake(Arena* arena, FNT_Font* font, FNT_Packer* pack);
function FNT_GlyphLayout* FNT_GlyphFromCP(FNT_Baked* font, u32 codepoint);

#endif //_LONG_FONT_H
