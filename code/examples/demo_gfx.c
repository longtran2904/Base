#include "DefaultMemory.h"
#include "Base.h"
#include "LongOS.h"
#include "Base.c"
#include "LongOS_Win32.c"

#include "LongGFX.h"
#include "LongGFX_Win32.c"
#include "LongGFX_OpenGL.h"
#include "Win32_OpenGL.c"
#include "LongGFX_D3D11.h"
#include "Win32_D3D11.c"

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
            BeginGLRender(window);
            
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
            
            GLEnum error = glGetError();
            
            glClear(GL_COLOR_BUFFER_BIT);
            
            GLFloat flts[4];
            glGetFloatv(GL_VIEWPORT, flts);
            
            error = glGetError();
            glBegin(GL_TRIANGLES);
            glColor3f (1, 1, 1);
            glVertex3f(-1, -1, 0);
            glVertex3f(1, -1, 0);
            glVertex3f(0, 1, 0);
            glEnd();
            error = glGetError();
            
            error = glGetError();
            
            EndGLRender();
        } break;
        
        case Renderer_D3D:
        {
            ID3D11RenderTargetView* view = BeginD3D11Render(window);
            ID3D11DeviceContext_ClearRenderTargetView(ctx, view, MidnightBlue);
            ID3D11DeviceContext_OMSetRenderTargets(ctx, 1, &view, 0);
            EndD3D11Render(view);
        } break;
    }
}

#define TEST_WINDOW_COUNT 6

#pragma WarnDisable(28251) // Inconsistent annotation for WinMain
int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd)
#pragma WarnEnable(28251)
{
    W32WinMainInit(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    
    ScratchBlock(scratch)
    {
        GFXErrorBlock(scratch, 1, .callback = GFXErrorFmt)
        {
            GFXInit();
            InitGL();
            //InitD3D11();
            
            //ErrorFmt("Test Error 1");
            //ErrorFmt("Test Error 2");
        }
        
        GFXSetResizeFunc(WindowResizeHandler);
        
        GFXWindow windows[TEST_WINDOW_COUNT] = {0};
        for (u32 i = 0; i < TEST_WINDOW_COUNT; ++i)
        {
            i32 w = 400, h = 200;
            i32 isGL = /*i % 2*/0;
            
            w = w * (i + 1);
            h = h * (i + 1);
            
            GFXErrorBlock(scratch, 1)
            {
                windows[i] = GFXCreateWindowEx(StrPushf(scratch, "Window: %u", i), CW_USEDEFAULT, CW_USEDEFAULT, w, h);
                if (isGL == 0)
                    EquipGLWindow(windows[i]);
                else
                    EquipD3D11Window(windows[i]);
            }
            
            GFXWindowSetUserData(windows[i], PtrFromInt(isGL));
            GFXShowWindow(windows[i]);
            
            if (i == 3)
            {
                i32 x, y;
                GFXWindowGetInnerRect(windows[i-1], &x, &y, &w, &h);
                GFXWindowSetOuterRect(windows[i], x, y, w, h);
            }
        }
        
        u32 count = 0;
        for (TempArena temp = TempBegin(scratch); ; TempEnd(temp))
        {
            if (!GFXPeekInput())
                break;
            
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
                    WindowResizeHandler(windows[i], 0, 0);
                }
            }
            
            if (!activeWindowCount)
                break;
        }
        
        GFXErrorBlock(scratch, 1)
        {
            //FreeD3D11();
            FreeGL();
            InitGL();
            FreeGL();
        }
    }
    
    return 0;
}
