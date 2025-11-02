#include "Base.h"
#include "Base.c"

#include "LongGFX.h"
#include "LongGFX_Win32.c"

#include "LongOGL.h"
#include "LongOGL_Win32.c"

#include "LongD3D11.h"
#include "LongD3D11_Win32.c"

#include "LongFont.h"
#include "LongIcons.h"
#include "LongFont.c"
#include "LongFont_FreeType.c"

#include "LongRender.h"
#include "LongRender.c"
#include "LongRender_OpenGL.c"

#define U32LinFromSRGB4(r, g, b, a) U32LinFromSRGB(Pack4F32(r, g, b, a))
#define U32LinFromSRGB3(r, g, b)    U32LinFromSRGB(Pack4F32(r, g, b, 1.f))

function void WindowResizeHandler(GFXWindow window, u32 width, u32 height)
{
    UNUSED(width);
    UNUSED(height);
    OGL_Begin(window);
    glClearColor(0.0f, 1.0f, 1.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    OGL_End();
}

typedef struct FrameInfo FrameInfo;
struct FrameInfo
{
    u32 frameIdx;
    u32 fps;
    
    u64 start;
    u64 end;
    u64 current;
};

function void FrameStart(FrameInfo* info)
{
    info->current = OSNowUS();
}

function void FrameEnd(FrameInfo* info)
{
    u64 delta = MB(1)/info->fps;
    u64 targetEnd = info->current + delta;
    u64 now = OSNowUS();
    
    if (now < targetEnd)
    {
        u64 leftover = targetEnd - now;
        Sleep((DWORD)(leftover / 1000));
        now = OSNowUS();
    }
    
    u64 frameEnd = now;
    if (targetEnd - now < delta/16)
        frameEnd = targetEnd;
    
    info->start = info->current;
    info->end = info->current = frameEnd;
    info->frameIdx++;
}

function void DrawQuads(R_Font* fonts, u64 fontello, FrameInfo* info, GFXWindow window)
{
    u32 white = 0xFFFFFFFF;
    u32 black = 0xFF000000;
    
    u32 c0 = U32LinFromSRGB3(0.9f, 0.1f, 0.0f);
    u32 c1 = U32LinFromSRGB3(0.5f, 0.9f, 0.0f);
    u32 c2 = U32LinFromSRGB3(0.0f, 0.9f, 0.5f);
    u32 c3 = U32LinFromSRGB3(1.0f, 1.0f, 0.5f);
    u32 c4 = U32LinFromSRGB3(1.0f, 1.0f, 0.0f);
    u32 c5 = U32LinFromSRGB3(0.1f, 0.5f, 1.0f);
    u32 c6 = U32LinFromSRGB3(1.0f, 1.0f, 0.0f);
    u32 c7 = U32LinFromSRGB3(0.0f, 1.0f, 1.0f);
    u32 c8 = U32LinFromSRGB3(0.4f, 0.9f, 0.1f);
    
    u64 frameTime = info->end - info->start;
    u32 loopSpeed = info->fps * 2;
    u32 frameIdx = info->frameIdx;
    
    r2f32 clip = {0};
    b32 hasClip = frameIdx < loopSpeed;
    if (hasClip)
    {
        i32 width = 0;
        GFXWindowGetInnerRect(window, 0, 0, &width, 0);
        f32 t = (f32)frameIdx * DivF32(width, loopSpeed);
        clip = R2F32P(0, 0, t, t);
    }
    
    //- Manual System
    ScratchBlock(scratch)
    {
        R_Batch batch = {0};
        v2f32  rectBase = V2F32(600.f, 250.f);
        v2f32  rectSize = V2F32(100.f, 100.f);
        v2f32 rectShift = AddV2F32(rectSize, V2F32(25.f, 25.f));
        v2f32 atlasSize = V2F32V(fonts[0].baked->size);
        
        R_BatchPushQuad(scratch, &batch, &(R_Quad){
                            R2F32Size((v2f32){0}, rectSize), 10.f, 5.f,
                            .c = { U32LinFromSRGB3(1.f, .2f, 0.f), U32LinFromSRGB3(1.f, 0.f, .2f) },
                        });
        
        R_BatchPushQuad(scratch, &batch, &(R_Quad){
                            R2F32Size(V2F32(rectShift.x, 0), rectSize), 50.f, 30.f, .c = { c7, c8 },
                            R2F32Size((v2f32){0}, DivV2F32(ScaleV2F32(rectSize, 1.5f), atlasSize)),
                        });
        
        R_BatchPushQuad(scratch, &batch, &(R_Quad){
                            R2F32Size(V2F32(0.f, rectShift.y), atlasSize),
                            .c = { white, white }, R2F32P(0.f, 0.f, 1.f, 1.f),
                        });
        
        for (R_QuadNode* node = batch.first; node; node = node->next)
        {
            for (u32 i = 0; i < node->count; ++i)
            {
                R_Quad* quad = node->quads + i;
                quad->xy = ShiftR2F32(quad->xy, rectBase);
                
                if (hasClip)
                {
                    quad->flags |= R_QuadFlag_Clipped;
                    quad->clip = clip;
                }
            }
        }
        
        R_Submit(batch.first, batch.totalCount, fonts[0].texture);
    }
    
    //- Automatic System
    ScratchBlock(scratch)
    {
        R_Ctx ctx = R_CtxMake(scratch, 0);
        if (hasClip)
            R_CtxClip(&ctx, &clip);
        
        //- Basic Shapes
        R_PushQuad(&ctx, &(R_Quad){ R2F32P( 5.f, 5.f, 45.f, 45.f), 10.f, 5.f, .c = { c0, c0 }, }, 0);
        R_PushRect(&ctx, R2F32P(55.f, 5.f, 95.f, 45.f), 10.f, c0);
        R_PushRect(&ctx, R2F32P(5.f, 50.f, 45.f, 70.f), 0.f, c1);
        R_PushQuad(&ctx, &(R_Quad){
                       R2F32P(55.f, 50.f, 95.f, 70.f), 15.f, .c = { c1, c2 },
                       .flags = R_QuadFlag_SharpCornerTR|R_QuadFlag_SharpCornerBL,
                   }, 0);
        
        //- Fonts
        R_CtxFont(&ctx, fonts[0]);
        R_PushStr(&ctx, StrLit("Hello, World!"), V2F32(25.f, 100.f), c3);
        
        R_CtxFont(&ctx, fonts[1]);
        R_PushStr(&ctx, StrLit("The quick brown fox jumps over the lazy dog."),
                  V2F32(25.f, 150.f), c3);
        
        R_CtxFont(&ctx, fonts[2]);
        R_PushStr(&ctx, StrPushf(scratch, "Delta: %llu us", frameTime), V2F32(125.f, 50.f), c4);
        
        //- Icons
        R_CtxFont(&ctx, fonts[fontello]);
        R_PushChar(&ctx, ICON_LOCK, V2F32(250.f, 100.f), c5);
        R_PushChar(&ctx, ICON_LOCK_OPEN_ALT, V2F32(275.f, 100.f), c5);
        
        //- Rotation/Gradient
        R_PushQuad(&ctx, &(R_Quad){
                       R2F32Size(V2F32(350.f, 90.f), V2F32(30.f, 30.f)),
                       500.f, 50.f, DivF32(frameIdx, loopSpeed) * TAU_F32,
                       R_QuadFlag_SharpCornerBL, { white, white },
                   }, 0);
        
        R_PushQuad(&ctx, &(R_Quad){
                       R2F32Size(V2F32(425.f, 75.f), V2F32(100.f, 2.f)),
                       1.f, 10.f, DivF32(frameIdx, loopSpeed * 2) * TAU_F32,
                       R_QuadFlag_HzGradient, { c6, c7 }
                   }, 0);
        
        R_PushQuad(&ctx, &(R_Quad){
                       R2F32P(125.f, 5.f, 250.f, 20.f),
                       .flags = R_QuadFlag_HzGradient, { black, white },
                   }, 0);
        
        //- Lines
        {
            v2f32 origin = V2F32(100.f, 200.f);
            v2f32 p[6] = {
                V2F32(  0.f,   0.f), V2F32(50.f,  50.f), V2F32(150.f,   0.f),
                V2F32(100.f, 200.f), V2F32( 0.f, 100.f), V2F32(100.f, 100.f),
            };
            
            for (u32 i = 1; i < ArrayCount(p); ++i)
            {
                r2f32 line = R2F32(p[i - 1], p[i]);
                line = ShiftR2F32(line, origin);
                R_PushLine(&ctx, line, 2.f, c8);
            }
        }
        
        R_CtxFlush(&ctx);
    }
}

#define DebugTimer(title, duration) \
    OutputDebugStringA(StrPushf(scratch, title ": %f ms\n", (f64)DivF32(duration, 1000.f)).str)

int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd)
{
    W32WinMainInit(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    
    ScratchBlock(scratch)
    {
        TIME_BLOCK_US(duration, DebugTimer("Init GFX", duration))
        {
            GFXErrorBlock(scratch, 1, .callback = GFXErrorFmt)
            {
                GFXInit();
                OGL_Init();
                InitD3D11();
                R_Init();
            }
            
            GFXSetResizeFunc(WindowResizeHandler);
        }
        
        const String paths[] = {
            StrLit("data/liberation-mono.ttf"),
            StrLit("data/Inconsolata-Regular.ttf"),
            StrLit("data/JetBrainsMono-Regular.ttf"),
            StrLit("data/MonaspaceNeon-Regular.otf"),
            StrLit("data/unifont-16.0.04.otf"),
            StrLit("data/fontello.ttf"),
        };
        
        R_Font fonts[ArrayCount(paths)];
        u64 fontello = ArrayCount(fonts) - 1;
        
        TIME_BLOCK_US(duration, DebugTimer("Load Font", duration))
        {
            GFXErrorBlock(scratch, 1, .callback = GFXErrorFmt)
            {
                for (u32 i = 0; i < ArrayCount(fonts); ++i)
                {
                    FNT_Font looseFont = FNT_FontOpen(scratch, &(FNT_LoadParams){
                                                          .flags = FNT_RasterFlag_Smooth|FNT_RasterFlag_Hinted,
                                                          .size = 15, .dpi = 96, .path = paths[i],
                                                      });
                    fonts[i] = R_FontBakeTexture(scratch, &looseFont, &(FNT_Packer){.size = V2I32(512, 512)});
                }
            }
        }
        
        u64 timer = OSNowUS();
        GFXWindow window = 0;
        GFXErrorBlock(scratch, 1, .callback = GFXErrorFmt)
        {
            window = GFXCreateWindowEx(StrLit("My Window"), CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800);
            OGL_WindowEquip(window);
            GFXShowWindow(window);
        }
        
        FrameInfo* info = &(FrameInfo){ .fps = (u32)GFXWindowRefreshRate(window), };
        for (TempArena temp = TempBegin(scratch); 1; TempEnd(temp))
        {
            if (info->frameIdx == 0)
            {
                timer = OSNowUS() - timer;
                DebugTimer("Show Window", timer);
            }
            
            DeferBlock(FrameStart(info), FrameEnd(info))
                DeferBlock(R_Begin(window), R_End())
                    DrawQuads(fonts, fontello, info, window);
            
            if (!GFXPeekInput()) break;
            if (!GFXWindowIsValid(window)) break;
        }
        
        GFXErrorBlock(scratch, 1)
        {
            FreeD3D11();
            OGL_Free();
            OGL_Init();
            OGL_Free();
        }
    }
    
    return 0;
}
