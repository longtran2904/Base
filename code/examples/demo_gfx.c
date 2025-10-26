#include "Base.h"
#include "Base.c"

#include "LongGFX.h"
#include "LongGFX_Win32.c"

#include "LongOGL.h"
#include "LongGFX_OpenGL.h"
#include "Win32_OpenGL.c"

#include "LongGFX_D3D11.h"
#include "Win32_D3D11.c"

#include "LongFont.h"
#include "LongIcons.h"
#include "LongFont.c"
#include "LongFont_FreeType.c"

#include "LongRender.h"
#include "LongRender.c"
#include "LongRender_OpenGL.c"

function void WindowResizeHandler(GFXWindow window, u32 width, u32 height)
{
    UNUSED(width);
    UNUSED(height);
    OGL_Begin(window);
    glClearColor(0.0f, 1.0f, 1.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    OGL_End();
}

int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd)
{
    W32WinMainInit(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    
    ScratchBlock(scratch)
    {
        GFXErrorBlock(scratch, 1, .callback = GFXErrorFmt)
        {
            GFXInit();
            OGL_Init();
            //InitD3D11();
            R_Init();
        }
        
        GFXSetResizeFunc(WindowResizeHandler);
        
        GFXWindow window = 0;
        GFXErrorBlock(scratch, 1)
        {
            window = GFXCreateWindowEx(StrLit("My Window"), CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800);
            OGL_WindowEquip(window);
            GFXShowWindow(window);
        }
        
        String paths[] = {
            StrLit("data/liberation-mono.ttf"),
            StrLit("data/Inconsolata-Regular.ttf"),
            StrLit("data/JetBrainsMono-Regular.ttf"),
            StrLit("data/MonaspaceNeon-Regular.otf"),
            StrLit("data/unifont-16.0.04.otf"),
            StrLit("data/fontello.ttf"),
        };
        
        FNT_LoadParams params = {
            .flags = FNT_RasterFlag_Smooth|FNT_RasterFlag_Hinted,
            .size = 15, .dpi = 96,
        };
        
        R_Font fonts[ArrayCount(paths)] = {0};
        for (u32 i = 0; i < ArrayCount(paths); ++i)
        {
            params.path = paths[i];
            FNT_Font looseFont = FNT_FontOpen(scratch, &params);
            fonts[i] = R_FontBakeTexture(scratch, &looseFont, &(FNT_Packer){.size = V2I32(512, 512)});
        }
        
        v4f32 colors[] = {
            V4F32(1.00f, 0.00f, 0.00f, 1.00f),
            V4F32(0.86f, 1.00f, 1.00f, 1.00f),
            V4F32(0.88f, 0.98f, 1.00f, 1.00f),
            V4F32(0.90f, 0.96f, 1.00f, 1.00f),
            
            V4F32(0.00f, 1.00f, 0.00f, 1.00f),
            V4F32(1.00f, 0.86f, 1.00f, 1.00f),
            V4F32(1.00f, 0.88f, 0.98f, 1.00f),
            V4F32(1.00f, 0.90f, 0.96f, 1.00f),
            
            V4F32(0.00f, 0.00f, 1.00f, 1.00f),
            V4F32(1.00f, 1.00f, 0.86f, 1.00f),
            V4F32(0.98f, 1.00f, 0.88f, 1.00f),
            V4F32(0.96f, 1.00f, 0.90f, 1.00f),
        };
        
        f32 lineX = 40.5f;
        
        u64 prevFrame = 0;
        u64 frameBegin = OSNowUS();
        
        u64 fps = GFXWindowRefreshRate(window)/4;
        //u64 fps = 10;
        u64 delta = MB(1)/fps;
        u64 markLoop = 120*delta;
        u64 frameIdx = 0;
        
        u64 markHistory[60] = {0};
        u64  endHistory[60] = {0};
        b8   badHistory[60] = {0};
        
        for (TempArena temp = TempBegin(scratch); ; TempEnd(temp))
        {
            if (!GFXPeekInput())
                break;
            
            lineX += 0.05f;
            markHistory[frameIdx % ArrayCount(markHistory)] = frameBegin;
            
            DeferBlock(R_Begin(window), R_End())
            {
                R_QuadList list = {0};
                
                R_QuadPush(scratch, &list, &(R_Quad){
                               R2F32P(200.f, 200.f, 300.f, 300.f), (r2f32){0},
                               10.f, 5.f, 1.f,
                               V4F32(1.f, .2f, 0.f, 1.f), V4F32(1.f, 0.f, .2f, 1.f)
                           });
                
                R_QuadPush(scratch, &list, &(R_Quad){
                               R2F32P(100.f, 400.f, 400.f, 500.f), (r2f32){0},
                               20.f, 10000.f, 1.f,
                               V4F32(1.f, 1.f, 1.f, 1.f), V4F32(0.f, 0.f, 0.f, 1.f)
                           });
                
                R_QuadPush(scratch, &list, &(R_Quad){
                               R2F32Size(V2F32(600.f, 400.f), V2F32V(fonts[0].baked.size)), R2F32P(0.f, 0.f, 1.f, 1.f),
                               0.f, 10000.f, 1.f,
                               V4F32(1.f, 1.f, 1.f, 1.f), V4F32(1.f, 1.f, 1.f, 1.f)
                           });
                
                R_Submit(list.first, list.totalCount, fonts[0].texture);
                
                DeferBlock(R_CtxBegin(), R_CtxEnd())
                {
                    v4f32 c0 = V4F32(0.9f, 0.1f, 0.0f, 1.0f);
                    v4f32 c1 = V4F32(0.8f, 0.0f, 0.0f, 1.0f);
                    v4f32 c2 = V4F32(0.1f, 0.9f, 0.0f, 1.0f);
                    v4f32 c3 = V4F32(0.0f, 0.9f, 0.1f, 1.0f);
                    
                    R_CtxPushLine(lineX, R1F32(300, 400), V4F32(1, 1, 1, 1));
                    
                    R_CtxPushRect(R2F32P( 5,  5, 45, 45), (r2f32){0}, 5.f, 10000.f, 1.f, c0, c1);
                    R_CtxPushRect(R2F32P(55,  5, 95, 45), (r2f32){0}, 5.f, 10000.f, 1.f, c0, c1);
                    R_CtxPushRect(R2F32P( 5, 50, 45, 70), (r2f32){0}, 0.f, 10000.f, 1.f, c2, c3);
                    R_CtxPushRect(R2F32P(55, 50, 95, 70), (r2f32){0}, 0.f, 10000.f, 1.f, c3, c2);
                    R_CtxPushStr(&fonts[1], StrLit("The quick brown fox jumps over the lazy dog."),
                                 V2F32(25.f, 50.f), V4F32(1.0f, 1.0f, 0.5f, 1.0f));
                    
                    R_CtxPushRect(R2F32P( 5, 105, 45, 145), (r2f32){0}, 5.f, 10000.f, 1.f, c0, c1);
                    R_CtxPushRect(R2F32P(55, 105, 95, 145), (r2f32){0}, 5.f, 10000.f, 1.f, c0, c1);
                    R_CtxPushRect(R2F32P( 5, 150, 45, 170), (r2f32){0}, 0.f, 10000.f, 1.f, c2, c1);
                    R_CtxPushRect(R2F32P(55, 150, 95, 170), (r2f32){0}, 0.f, 10000.f, 1.f, c1, c2);
                    R_CtxPushStr(&fonts[2], StrLit("Hello, world!"),
                                 V2F32(25.f, 150.f), V4F32(1.0f, 1.0f, 0.5f, 1.0f));
                    
                    R_CtxPushChar(&fonts[ArrayCount(fonts)-1], ICON_FOLDER, V2F32(25, 250), V4F32(.8f, 0.f, .4f, 1.f));
                    R_CtxPushChar(&fonts[ArrayCount(fonts)-1], ICON_CANCEL, V2F32(50, 250), V4F32(.8f, 0.f, .4f, 1.f));
                    
                    String frameStr = StrPushf(scratch, "%llu", prevFrame);
                    R_CtxPushStr(&fonts[0], frameStr, V2F32(600, 50), V4F32(1.f, 1.f, 0.f, 1.f));
                    
                    r1f32 xRange = R1F32(200, 700);
                    r1f32  yMark = R1F32(100, 110);
                    r1f32   yEnd = R1F32(110, 120);
                    
                    u64* markPtr = markHistory;
                    u64*  endPtr =  endHistory;
                    b8*   badPtr =  badHistory;
                    
                    for (u64 i = 0; i < ArrayCount(markHistory); ++i, ++markPtr, ++endPtr, ++badPtr)
                    {
                        f32 markT = DivF32(((*markPtr)%markLoop), markLoop);
                        f32  endT = DivF32(((* endPtr)%markLoop), markLoop);
                        
                        f32 xMark = Lerp(xRange.min, xRange.max, markT);
                        f32  xEnd = Lerp(xRange.min, xRange.max,  endT);
                        
                        v4f32 color = colors[i % ArrayCount(colors)];
                        R_CtxPushLine(xMark, yMark, color);
                        R_CtxPushLine( xEnd,  yEnd, color);
                        
                        if (*badPtr)
                            R_CtxPushRect(R2F32P(xMark-2, yMark.min-2, xMark+3, yMark.min+3), (r2f32){0},
                                          0.5f, 10000.f, 1.f, V4F32(1, 0, 0, 1), V4F32(1, 0, 0, 1));
                    }
                }
            }
            
            if (!GFXWindowIsValid(window))
                break;
            
            {
                u64 targetEnd = frameBegin + delta;
                u64 now = OSNowUS();
                endHistory[frameIdx % ArrayCount(endHistory)] = now;
                
                if (now < targetEnd)
                {
                    u64 leftover = targetEnd - now;
                    Sleep((DWORD)(leftover / 1000));
                    now = OSNowUS();
                }
                
                prevFrame = now - frameBegin;
                
                b8 good = targetEnd - now < delta/16;
                if (good)
                    frameBegin = targetEnd;
                else
                    frameBegin = now;
                
                badHistory[frameIdx % ArrayCount(badHistory)] = !good;
                ++frameIdx;
            }
        }
        
        GFXErrorBlock(scratch, 1)
        {
            //FreeD3D11();
            OGL_Free();
            OGL_Init();
            OGL_Free();
        }
    }
    
    return 0;
}
