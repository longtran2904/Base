/* date = October 26th 2025 0:43 am */

#ifndef _LONG_RENDER_H
#define _LONG_RENDER_H

//~ TODO(long):
// [ ] Add D3D11 backend
// [ ] Introduce a vtable for all API-specific functions
// [X] Error when the user try to init or begin the renderer multiple times
// [X] Shorten all the QuadFlag names
// [X] Support additional texture formats (currently only R8 is implemented)
// [X] Query the texture format

//~ long: Render Types

typedef enum R_TextureFmt
{
    R_TextureFmt_R8,
    R_TextureFmt_RG8,
    R_TextureFmt_RGBA8,
    R_TextureFmt_SRGB8,
    R_TextureFmt_R16,
    R_TextureFmt_RG16,
    R_TextureFmt_RGBA16,
    R_TextureFmt_R32,
    R_TextureFmt_RG32,
    R_TextureFmt_RGBA32,
} R_TextureFmt;

typedef u64 R_Texture;

typedef Flags32 R_QuadFlags;
enum
{
    R_QFlag_SharpTR   = (1 << 0),
    R_QFlag_SharpTL   = (1 << 1),
    R_QFlag_SharpBR   = (1 << 2),
    R_QFlag_SharpBL   = (1 << 3),
    R_QFlag_GradientH = (1 << 4),
    R_QFlag_GradientV = (1 << 5),
    R_QFlag_Clipped   = (1 << 6),
};

typedef struct R_Quad R_Quad;
struct R_Quad
{
    r2f32 xy;
    f32 roundness, thickness, theta;
    u32 flags;
    
    union
    {
        struct { u32 c0, c1; };
        u32 c[2];
    };
    
    r2f32 uv;
    r2f32 clip;
};

typedef struct R_QuadNode R_QuadNode;
struct R_QuadNode
{
    R_QuadNode* next;
    R_Quad* quads;
    u64 count;
};

//~ long: Batch Types

#ifndef R_QUAD_CHUNK_CAP
#define R_QUAD_CHUNK_CAP KiB(16)
#endif

typedef struct R_Batch R_Batch;
struct R_Batch
{
    R_Batch *next;
    R_QuadNode* first;
    R_QuadNode* last;
    u64 nodeCount, totalCount;
    R_Texture texture;
};

typedef struct R_List R_List;
struct R_List
{
    R_Batch* first;
    R_Batch* last;
    u64 batchCount;
};

//~ long: Helper Types

typedef struct R_Font R_Font;
struct R_Font
{
    FNT_Baked* baked;
    R_Texture texture;
};

typedef struct R_Ctx R_Ctx;
struct R_Ctx
{
    b32 enableClip;
    r2f32 clip;
    R_Font font;
    R_List* list;
    Arena* arena;
};

//~ long: Render Functions

function void R_Init(void);
function void R_Begin(GFXWindow window);
function void R_End(void);
function void R_Submit(R_QuadNode* first, u64 count, R_Texture texture);

function R_TextureFmt R_FmtFromTexture(R_Texture texture);
function R_Texture R_TextureCreate (u32 w, u32 h, R_TextureFmt fmt, void* data);
function void      R_TextureUpdate (R_Texture texture, r2i32 rect, void* data);
function void      R_TextureDestroy(R_Texture texture);

//~ long: Batch Functions

function void R_BatchPushQuad(Arena* arena, R_Batch* batch, R_Quad* quad);
function void R_BatchPushStr (Arena* arena, R_Batch* batch, FNT_Baked* font, String str, v2f32 p, u32 c, r2f32* clip);

// NOTE(long): These are used by the R_Ctx system to automatically handle textures
// The user should generally not need to call these
function void R_ListPushBatch(Arena* arena, R_List* list);
function void R_ListPrepBatch(Arena* arena, R_List* list, R_Texture texture);
function void R_ListFlush(R_List* list);

//~ long: Helper Functions

function R_Font R_FontBakeTexture(Arena* arena, FNT_Font* font, FNT_Packer* pack);

function R_Ctx R_CtxMake(Arena* arena, R_List* list);
function void  R_CtxClip(R_Ctx* ctx, r2f32* clip);
function void  R_CtxFont(R_Ctx* ctx, R_Font font);
function void  R_CtxFlush(R_Ctx* ctx);

function void R_PushRect(R_Ctx* ctx, r2f32 xy, f32 r, u32 c);
function void R_PushLine(R_Ctx* ctx, r2f32 xy, f32 r, u32 c);
function void R_PushQuad(R_Ctx* ctx, R_Quad* quad, R_Texture texture);
function void R_PushChar(R_Ctx* ctx, u32 cp, v2f32 p, u32 c);
function void R_PushStr (R_Ctx* ctx, String str, v2f32 p, u32 c);

#endif //_LONG_RENDER_H
