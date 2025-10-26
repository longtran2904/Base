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

enum
{
    Renderer_GL,
    Renderer_D3D,
};

function void WindowResizeHandler(GFXWindow window, u32 width, u32 height)
{
    UNUSED(width);
    UNUSED(height);
    
    u32 renderer = (u32)IntFromPtr(GFXWindowGetUserData(window));
    switch (renderer)
    {
        case Renderer_GL:
        {
            OGL_Begin(window);
            
            GFXFlags flags = GFXWindowGetFlags(window);
            if (flags & FLAG_WINDOW_MAXIMIZED)
                glClearColor(0.f, 1.f, 1.f, 1.f);
            else if (flags & FLAG_WINDOW_MINIMIZED)
                glClearColor(1.f, 0.f, 1.f, 1.f);
            else if (flags & FLAG_MODE_FULLSCREEN)
                glClearColor(1.f, 1.f, 0.f, 1.f);
            else if (window % 2 == 0)
                glClearColor(0, 0, 0, 1);
            else
                glClearColor(1.0f, 1.0f, 1.0f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            GLfloat flts[4];
            glGetFloatv(GL_VIEWPORT, flts);
            
#if 0
            glBegin(GL_TRIANGLES);
            glColor3f (1, 1, 1);
            glVertex3f(-1, -1, 0);
            glVertex3f(1, -1, 0);
            glVertex3f(0, 1, 0);
            glEnd();
#endif
            
            OGL_End();
        } break;
        
        case Renderer_D3D:
        {
            ID3D11RenderTargetView* view = BeginD3D11Render(window);
            ID3D11DeviceContext* ctx = GetD3D11DeviceCtx();
            ID3D11DeviceContext_ClearRenderTargetView(ctx, view, MidnightBlue);
            ID3D11DeviceContext_OMSetRenderTargets(ctx, 1, &view, 0);
            EndD3D11Render(view);
        } break;
    }
}

#define TEST_WINDOW_COUNT 1

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
            
#if 0
            ErrorFmt("Test Error 1");
            ErrorFmt("Test Error 2");
#endif
        }
        
        //GFXSetResizeFunc(WindowResizeHandler);
        
        GFXWindow windows[TEST_WINDOW_COUNT] = {0};
        for (u32 i = 0; i < TEST_WINDOW_COUNT; ++i)
        {
            i32 w = 1200, h = 800;
            i32 isGL = /*i % 2*/1;
            
            w = w * (i + 1);
            h = h * (i + 1);
            
            GFXErrorBlock(scratch, 1)
            {
                windows[i] = GFXCreateWindowEx(StrPushf(scratch, "Window: %u", i), CW_USEDEFAULT, CW_USEDEFAULT, w, h);
                if (isGL)
                    OGL_WindowEquip(windows[i]);
                else
                    EquipD3D11Window(windows[i]);
            }
            
            GFXWindowSetUserData(windows[i], PtrFromInt(isGL ? Renderer_GL : Renderer_D3D));
            GFXShowWindow(windows[i]);
            
            if (i == 3)
            {
                i32 x, y;
                GFXWindowGetInnerRect(windows[i-1], &x, &y, &w, &h);
                GFXWindowSetOuterRect(windows[i], x, y, w, h);
            }
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
        
        u32 count = 0;
        for (TempArena temp = TempBegin(scratch); ; TempEnd(temp))
        {
            if (!GFXPeekInput())
                break;
            
            DeferBlock(R_Begin(windows[0]), R_End())
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
                               R2F32P(600.f, 100.f, 700.f, 300.f), (r2f32){0},
                               0.f, 10000.f, 1.f,
                               V4F32(1.f, 1.f, 1.f, 1.f), V4F32(1.f, 1.f, 1.f, 0.f)
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
                }
            }
            
            count++;
#define TEST_TOGGLE_TIMER 150
            if (count % TEST_TOGGLE_TIMER == 0)
            {
#define TEST_VISIBILITY 0
#define TEST_OUTERINNER 0
#define TEST_FULLSCREEN 0
                
#define TEST_RESIZABLE 0
#define TEST_UNFOCUSED 0
#define TEST_MINMAX    0
                
                if (TEST_VISIBILITY)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-1];
                    if (GFXWindowIsValid(window))
                    {
                        b32 visible = count % (TEST_TOGGLE_TIMER * 2);
                        GFXWindowSetFlags(window, FLAG_WINDOW_HIDDEN, !visible);
                        if (visible)
                            GFXWindowSetTitle(window, StrPushf(scratch, "Toggle Windows: %u", count / 100));
                        visible = !visible;
                    }
                }
                
                if (TEST_OUTERINNER)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-2];
                    if (GFXWindowIsValid(window))
                    {
                        i32 x, y, w, h;
                        if (count % (TEST_TOGGLE_TIMER * 2))
                        {
                            i32 innerX, innerY, innerW, innerH;
                            
                            GFXWindowGetOuterRect(window, &x, &y, &w, &h);
                            GFXWindowGetInnerRect(window, &innerX, &innerY, &innerW, &innerH);
                            
                            GFXWindowSetInnerRect(window, x, y, w, h);
                            GFXWindowGetInnerRect(window, &innerX, &innerY, &innerW, &innerH);
                        }
                        else
                        {
                            GFXWindowGetInnerRect(window, &x, &y, &w, &h);
                            GFXWindowSetOuterRect(window, x, y, w, h);
                        }
                    }
                }
                
                if (TEST_FULLSCREEN)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-3];
                    if (GFXWindowIsValid(window))
                    {
                        if (count % (TEST_TOGGLE_TIMER * 2))
                        {
                            GFXWindowSetFlags(window, FLAG_WINDOW_MAXIMIZED, 0);
                            GFXWindowSetFlags(window, FLAG_MODE_FULLSCREEN, 1);
                        }
                        else
                        {
                            GFXWindowSetFlags(window, FLAG_MODE_FULLSCREEN, 0);
                            GFXWindowSetFlags(window, FLAG_WINDOW_MAXIMIZED, 1);
                        }
                    }
                }
                
                if (TEST_RESIZABLE)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-4];
                    if (GFXWindowIsValid(window))
                        GFXWindowSetFlags(window, FLAG_WINDOW_RESIZABLE, count % (TEST_TOGGLE_TIMER * 2));
                }
                
                if (TEST_MINMAX)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-5];
                    if (GFXWindowIsValid(window))
                    {
                        u32 loop = (count % (TEST_TOGGLE_TIMER * 4)) / TEST_TOGGLE_TIMER;
                        switch (loop)
                        {
                            case 2:
                            case 0: GFXWindowSetFlags(window, FLAG_WINDOW_MINIMIZED|FLAG_WINDOW_MAXIMIZED, 0); break;
                            case 1: GFXWindowSetFlags(window, FLAG_WINDOW_MAXIMIZED, 1); break;
                            case 3: GFXWindowSetFlags(window, FLAG_WINDOW_MINIMIZED, 1); break;
                        }
                    }
                }
                
                if (TEST_UNFOCUSED)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-6];
                    if (GFXWindowIsValid(window))
                        GFXWindowSetFlags(window, FLAG_WINDOW_UNFOCUSED, count % (TEST_TOGGLE_TIMER * 2) == 0);
                }
            }
            
            u32 activeWindowCount = 0;
            for (u32 i = 0; i < TEST_WINDOW_COUNT; ++i)
            {
                if (GFXWindowIsValid(windows[i]))
                {
                    activeWindowCount++;
                    //WindowResizeHandler(windows[i], 0, 0);
                }
            }
            
            if (!activeWindowCount)
                break;
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
