/* date = October 26th 2025 0:43 am */

#ifndef _LONG_RENDER_H
#define _LONG_RENDER_H

//~ long: Render Types

typedef struct R_Quad R_Quad;
struct R_Quad
{
    r2f32 xy;
    r2f32 uv;
    f32 radius;
    f32 thickness;
    f32 softness;
    v4f32 c0, c1;
};

typedef struct R_QuadNode R_QuadNode;
struct R_QuadNode
{
    R_QuadNode* next;
    R_Quad* quads;
    u64 count;
};

typedef struct R_Texture R_Texture;

//~ long: Helper Types

#ifndef R_QUAD_CHUNK_CAP
#define R_QUAD_CHUNK_CAP KiB(16)
#endif

typedef struct R_QuadList R_QuadList;
struct R_QuadList
{
    R_QuadNode* first;
    R_QuadNode* last;
    u64 nodeCount, totalCount;
};

typedef struct R_Font R_Font;
struct R_Font
{
    FNT_Baked baked;
    R_Texture* texture;
};

typedef struct R_Ctx R_Ctx;
struct R_Ctx
{
    R_Font font;
    R_QuadList list;
    Arena* arena;
    u64 basePos;
};

//~ long: Render Functions

function void R_Init(void);
function void R_Begin(GFXWindow window);
function void R_End(void);
function void R_Submit(R_QuadNode* first, u64 count, R_Texture* texture);

function R_Texture* R_TextureCreate(u32 w, u32 h, void* data);
function void       R_TextureUpdate(R_Texture* texture, r2i32 rect, void* data);
function void       R_TextureDestroy(R_Texture* texture);
function b32        R_TextureValid(R_Texture* texture);

//~ long: Helper Functions

function void R_QuadPush(Arena* arena, R_QuadList* list, R_Quad* quad);
function void  R_StrPush(Arena* arena, R_QuadList* list, FNT_Baked* font, String str, v2f32 p, v4f32 color);

function R_Font R_FontBakeTexture(Arena* arena, FNT_Font* font, FNT_Packer* pack);

//~ long: Context Functions

function void R_CtxBegin(void);
function void R_CtxEnd(void);

#define R_CtxPushRect(...) R_CtxPushQuad(&(R_Quad){ .thickness = 10000.f, .softness = 1.f, __VA_ARGS__})
#define R_CtxPushLine(x, y, color) R_CtxPushRect(.xy = R2F32P((x), (y).min, (x) + 1.f, (y).max), \
                                                 .c0 = (color), .c1 = (color))
function void R_CtxPushQuad(R_Quad* quad);
function void R_CtxPushStr(R_Font* font, String str, v2f32 p, v4f32 color);

#endif //_LONG_RENDER_H
