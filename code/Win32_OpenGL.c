//~ NOTE(long): Win32 OpenGL Types

typedef struct W32OpenGLWindow W32OpenGLWindow;
struct W32OpenGLWindow
{
    int dummy;
};

//~ NOTE(long): WGL Definitions

#define WGL_FUNCS(X) \
    X(HGLRC, CreateContext, (HDC dc)) \
    X(BOOL,  DeleteContext, (HGLRC rc)) \
    X(BOOL,  MakeCurrent, (HDC dc, HGLRC rc)) \
    X(PROC,  GetProcAddress, (LPCSTR name))
#define FUNCTION_VALUE(X) WGL_FUNCS(X)
#define FUNCTION_PREFIX W32Wgl
#define POINTER_PREFIX w32Wgl
#include "XFunction.h"

#define WGL_EXT_FUNCS(X) \
    X(BOOL,  ChoosePixelFormatARB, (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, \
                                    UINT nMaxFormats, int *piFormats, UINT *nNumFormats)) \
    X(HGLRC, CreateContextAttribsARB, (HDC dc, HGLRC share, const int* attribList))
#define FUNCTION_VALUE(X) WGL_EXT_FUNCS(X)
#define FUNCTION_PREFIX W32Wgl
#define POINTER_PREFIX w32Wgl
#include "XFunction.h"

// WGL_ARB_PIXEL_FORMAT constants
#define WGL_NUMBER_PIXEL_FORMATS_ARB            0x2000
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_NEED_PALETTE_ARB                    0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB             0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB              0x2006
#define WGL_SWAP_METHOD_ARB                     0x2007
#define WGL_NUMBER_OVERLAYS_ARB                 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB                0x2009
#define WGL_TRANSPARENT_ARB                     0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB           0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB         0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB          0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB         0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB         0x203B
#define WGL_SHARE_DEPTH_ARB                     0x200C
#define WGL_SHARE_STENCIL_ARB                   0x200D
#define WGL_SHARE_ACCUM_ARB                     0x200E
#define WGL_SUPPORT_GDI_ARB                     0x200F
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_STEREO_ARB                          0x2012
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_RED_SHIFT_ARB                       0x2016
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_GREEN_SHIFT_ARB                     0x2018
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_BLUE_SHIFT_ARB                      0x201A
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_ALPHA_SHIFT_ARB                     0x201C
#define WGL_ACCUM_BITS_ARB                      0x201D
#define WGL_ACCUM_RED_BITS_ARB                  0x201E
#define WGL_ACCUM_GREEN_BITS_ARB                0x201F
#define WGL_ACCUM_BLUE_BITS_ARB                 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB                0x2021
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_AUX_BUFFERS_ARB                     0x2024
#define WGL_NO_ACCELERATION_ARB                 0x2025
#define WGL_GENERIC_ACCELERATION_ARB            0x2026
#define WGL_FULL_ACCELERATION_ARB               0x2027
#define WGL_SWAP_EXCHANGE_ARB                   0x2028
#define WGL_SWAP_COPY_ARB                       0x2029
#define WGL_SWAP_UNDEFINED_ARB                  0x202A
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C

// WGL_ARB_Create_Context constants
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define ERROR_INVALID_VERSION_ARB               0x2095
#define ERROR_INVALID_PROFILE_ARB               0x2096

//~ NOTE(long): GL Definitions

typedef i32 GLInt;
typedef u32 GLBitfield;
typedef u32 GLEnum;
typedef u32 GLSizei;
typedef f32 GLFloat;

#define GL_FUNCS(X) \
    X(void, glClearColor, (GLFloat red, GLFloat green, GLFloat blue, GLFloat alpha)) \
    X(void, glClear     , (GLBitfield mask)) \
    X(void, glBegin, (GLEnum mode)) \
    X(void, glEnd, (void)) \
    X(void, glColor3f,  (GLFloat red, GLFloat green, GLFloat blue)) \
    X(void, glVertex3f, (GLFloat x, GLFloat y, GLFloat z)) \
    X(void, glFlush, (void))\
    X(GLEnum, glGetError, (void)) \
    X(void, glGetFloatv, (GLEnum pname, GLFloat* params)) \
    X(void, glViewport, (GLInt x, GLInt y, GLSizei width, GLSizei height))

#define FUNCTION_VALUE(X) GL_FUNCS(X)

#define FUNCTION_PREFIX GL
#include "XFunction.h"

#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_TRIANGLES        0x0004
#define GL_VIEWPORT         0x0BA2

//~ NOTE(long): Win32 OpenGL Globals

global W32OpenGLWindow w32OpenGLSlots[GFX_MAX_WINDOW_SLOTS] = {0};
global HMODULE w32OpenGLModule = 0;
global HGLRC w32OpenGLContext = 0;
global int w32OpenGLPixelFormat = 0;

global HDC  w32RenderDC = 0;
global HWND w32RenderWnd = 0;
#define BOOTSTRAP_WINDOW_CLASS_NAME "LongOpenGLBootstrap"

//~ NOTE(long): System OpenGL Functions

function b32 InitGL(void)
{
    b32 error = 0;
    
    // Get instance
    HINSTANCE instance = W32GetInstance();
    
    // Setup opengl
    {
        if (w32OpenGLModule != 0)
            ErrorSet("OpenGL has already intialized", error);
        
        if (!error)
        {
            w32OpenGLModule = LoadLibrary("opengl32.dll");
            if (w32OpenGLModule == 0)
                ErrorSet("Failed to initialize opengl.dll", error);
        }
    }
    
    if (!error)
    {
#define X(r, n, p)  GET_PROC_ADDR(w32Wgl##n, w32OpenGLModule, Stringify(Concat(wgl, n)));
        WGL_FUNCS(X)
#undef X
        
        b32 missingWglFunc = false;
#define X(r, n, p) if (w32##Wgl##n == 0) { missingWglFunc = true; }
        WGL_FUNCS(X)
#undef X
        
        if (missingWglFunc)
            ErrorSet("Failed to load wgl function(s)", error);
    }
    
    // Create bootstrap window
    HWND bootstrapWindow = 0;
    ATOM atom = 0;
    if (!error)
    {
        WNDCLASS wndClass = {
            .lpfnWndProc = W32GraphicsWindowProc,
            .hInstance = instance,
            .lpszClassName = BOOTSTRAP_WINDOW_CLASS_NAME,
        };
        
        atom = RegisterClass(&wndClass);
        if (atom == 0)
            ErrorSet("Failed to resgister class", error);
        
        if (!error)
        {
            HWND hwnd = CreateWindow(BOOTSTRAP_WINDOW_CLASS_NAME,
                                     "opengl-bootstrap-window",
                                     0, 0, 0, 0, 0,     // style, x,y,w,h
                                     0, 0, instance, 0 // parent, menu, inst, param
                                     );
            
            if (hwnd == 0)
                ErrorSet("Failed to create window", error);
            else
                bootstrapWindow = hwnd;
        }
    }
    
    // Create bootstrap context
    HGLRC bootstrapContext = 0;
    if (!error)
    {
        HDC dc = GetDC(bootstrapWindow);
        
        PIXELFORMATDESCRIPTOR formatDesc = {
            .nSize = sizeof(formatDesc),
            .nVersion = 1,
            .dwFlags = PFD_SUPPORT_OPENGL,
            .cColorBits = 24,
            .cRedBits = 8,
            .cRedShift = 0,
            .cGreenBits = 8,
            .cGreenShift = 8,
            .cBlueBits = 8,
            .cBlueShift = 16,
        };
        
        int formatIDX = ChoosePixelFormat(dc, &formatDesc);
        if (formatIDX == 0)
            ErrorSet("Failed to choose bootstrap pixel format", error);
        
        if (!error)
            if (!SetPixelFormat(dc, formatIDX, &formatDesc))
                ErrorSet("Failed to create bootstrap pixel format", error);
        
        if (!error)
        {
            HGLRC hglrc = w32WglCreateContext(dc);
            if (hglrc == 0)
                ErrorSet("Failed to create bootstrap context", error);
            
            // Load wgl ext functions
            {
                if (!error)
                    w32WglMakeCurrent(dc, hglrc);
#define X(r, n, p) \
    if (!error) \
    { \
        (*(PROC*)&w32Wgl##n) = w32WglGetProcAddress("wgl"Stringify(n)); \
        if (w32Wgl##n == 0) ErrorSet("Failed to initialize wgl"Stringify(n), error); \
    }
                WGL_EXT_FUNCS(X);
#undef X
            }
            
            if (!error)
                bootstrapContext = hglrc;
        }
        
        ReleaseDC(bootstrapWindow, dc);
    }
    
	HWND dummyWindow = 0;
    if (!error)
        dummyWindow = CreateWindow(GRAPHICS_WINDOW_CLASS_NAME, "LongDummy",
                                   0, 0, 0, 0, 0,
                                   0, 0, W32GetInstance(), 0);
    
	if (dummyWindow)
	{
		// Create real context
		HDC dc = GetDC(dummyWindow);
		
        int formatAttribsI[] = {
            WGL_DRAW_TO_WINDOW_ARB, TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_SWAP_METHOD_ARB, WGL_SWAP_EXCHANGE_ARB,
            WGL_SUPPORT_OPENGL_ARB, TRUE,
            WGL_DOUBLE_BUFFER_ARB, TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 8,
            WGL_RED_BITS_ARB, 8,
            WGL_GREEN_BITS_ARB, 8,
            WGL_BLUE_BITS_ARB, 8,
            0
        };
        
        UINT numFormats = 0;
        BOOL cpf = w32WglChoosePixelFormatARB(dc, formatAttribsI, 0, 1, &w32OpenGLPixelFormat, &numFormats);
        if (!cpf || numFormats == 0)
            ErrorSet("Failed to choose graphics pixel format", error);
        
		if (!error)
		{
			PIXELFORMATDESCRIPTOR formatDesc = {0};
			BOOL spf = SetPixelFormat(dc, w32OpenGLPixelFormat, &formatDesc);
			if (!spf)
				ErrorSet("Failed to set graphics pixel format", error);
		}
		
		if (!error)
		{
			int attribs[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				WGL_CONTEXT_MINOR_VERSION_ARB, 3,
				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
				WGL_CONTEXT_PROFILE_MASK_ARB, /*WGL_CONTEXT_CORE_PROFILE_BIT_ARB*/WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
				0
			};
			
			w32OpenGLContext = w32WglCreateContextAttribsARB(dc, 0, attribs);
			if (!w32OpenGLContext)
				ErrorSet("Failed to create graphics context", error);
		}
		
		// Load opengl functions
		{
#define X(r, n, p) if (!error) \
    { \
        GET_PROC_ADDR(n, w32OpenGLModule, Stringify(n)); \
        if (!n) ErrorSet("Failed to load "Stringify(n), error); \
    }
            GL_FUNCS(X);
#undef X
		}
		
		ReleaseDC(dummyWindow, dc);
	}
	
    // Clean up "temps"
    {
        if (bootstrapContext && !w32WglDeleteContext(bootstrapContext))
            ErrorSet("Failed to destroy the bootstrap context", error);
		
        if (bootstrapWindow && !DestroyWindow(bootstrapWindow))
            ErrorSet("Failed to destroy the bootstrap window", error);
		
		if (dummyWindow && !DestroyWindow(dummyWindow))
			ErrorSet("Failed to destroy the dummy context", error);
        
        if (atom && !UnregisterClass(BOOTSTRAP_WINDOW_CLASS_NAME, instance))
            ErrorSet("Failed to unregister the bootstrap class", error);
    }
    
	// Clean up "non-temps"
    if (error)
    {
        // Clear modules
        if (w32OpenGLModule)
			FreeLibrary(w32OpenGLModule);
		
		if (w32OpenGLContext)
			Assert(w32WglDeleteContext(w32OpenGLContext));
        
        // Clear function pointers
#define X(r, n, p)  w32Wgl##n = 0;
        WGL_FUNCS(X);
        WGL_EXT_FUNCS(X);
#undef X
        
#define X(r, n, p) n = 0;
        GL_FUNCS(X);
#undef X
		
		w32OpenGLModule = 0;
		w32OpenGLContext = 0;
		w32OpenGLPixelFormat = 0;
    }
    
    return !error;
}

function b32 FreeGL()
{
    b32 error = 0;
    
    // Clear OpenGL
    {
        if (!w32OpenGLModule)
            ErrorSet("opengl.dll has already been freed", error);
        else if (!FreeLibrary(w32OpenGLModule))
            ErrorSet("Failed to free opengl.dll", error);
        
        if (!w32OpenGLContext)
            ErrorSet("The graphics context has already been deleted", error);
        else if (!w32WglDeleteContext(w32OpenGLContext))
            ErrorSet("Failed to delete the graphics context", error);
        
        ReleaseDC(w32RenderWnd, w32RenderDC);
    }
    
    // Clear function pointers
    {
#define X(r, n, p)  w32Wgl##n = 0;
        WGL_FUNCS(X);
        WGL_EXT_FUNCS(X);
#undef X
        
#define X(r, n, p) n = 0;
        GL_FUNCS(X);
#undef X
    }
    
    // Clear globals
    {
        w32OpenGLModule = 0;
        w32OpenGLContext = 0;
        w32OpenGLPixelFormat = 0;
        
        w32RenderDC = 0;
        w32RenderWnd = 0;
    }
    
    return !error;
}

function void W32CloseOpenGLWindow(GFXWindow window)
{
    W32OpenGLWindow* slot = w32OpenGLSlots + window - 1;
    ZeroStruct(slot);
}

function b32 EquipGLWindow(GFXWindow window)
{
    b32 error = 1;
	if (!GFXWindowIsValid(window))
		ErrorFmt("Invalid window handle: %llu", window);
	else if (GFXWindowIsEquipped(window))
		ErrorFmt("Window is already equipped: %llu", window);
    else
        error = 0;
	
	if (!error)
	{
		W32Window* slot = W32WindowFromGFXHandle(window);
		HDC dc = GetDC(slot->wnd);
		
		PIXELFORMATDESCRIPTOR formatDesc = {0};
		BOOL spf = SetPixelFormat(dc, w32OpenGLPixelFormat, &formatDesc);
		if (!spf)
			ErrorSet("Failed to set graphics pixel format", error);
		
		ReleaseDC(slot->wnd, dc);
		
		if (!error)
		{
			W32OpenGLWindow* equipped = w32OpenGLSlots + window - 1;
			equipped->dummy = 1;
			GFXWindowEquipData(window, equipped, W32CloseOpenGLWindow);
		}
		
		if (error)
		{
			ZeroStruct(slot);
			ZeroStruct(w32OpenGLSlots + window - 1);
		}
	}
    
    return !error;
}

function void BeginGLRender(GFXWindow window)
{
    if (GFXWindowIsValid(window) && w32RenderDC == 0)
    {
        W32Window* slot = W32WindowFromGFXHandle(window);
        w32RenderWnd = slot->wnd;
        w32RenderDC = GetDC(w32RenderWnd);
		w32WglMakeCurrent(w32RenderDC, w32OpenGLContext);
        u32 w, h;
        GFXWindowGetInnerRect(window, 0, 0, &w, &h);
        glViewport(0, 0, w, h);
        DEBUG(error, GLEnum error = glGetError());
    }
}

function void EndGLRender(void)
{
    if (w32RenderDC != 0)
    {
        SwapBuffers(w32RenderDC);
        ReleaseDC(w32RenderWnd, w32RenderDC);
        w32RenderDC = 0;
        w32RenderWnd = 0;
    }
}
