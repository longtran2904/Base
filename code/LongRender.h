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

//~ long: Render Functions

function void R_Init(void);

function void R_Begin(GFXWindow window);
function void R_End(void);

function void R_Submit(R_QuadNode* first, u64 count, R_Texture* texture);
function R_Texture* R_TextureCreate(void* data, u32 w, u32 h);
function void       R_TextureDestroy(R_Texture* texture);
function b32        R_TextureValid(R_Texture* texture);

#endif //_LONG_RENDER_H
