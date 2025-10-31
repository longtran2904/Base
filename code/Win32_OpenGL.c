
//~ NOTE(long): OpenGL Helpers

function OGL_Shader OGL_MakeShader(Arena* arena, char* src, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, 0);
    glCompileShader(shader);
    
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    char* buffer = PushArray(arena, char, logLength + 1);
    GLint length = 0;
    glGetShaderInfoLog(shader, logLength + 1, &length, buffer);
    
    // Handle Error
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    
    if (status == 0)
    {
        glDeleteShader(shader);
        shader = 0;
    }
    
    return (OGL_Shader){ shader, Str(buffer, length) };
}

function OGL_Shader OGL_MakeProgram(Arena* arena, OGL_Shader* shaders, u64 count)
{
    GLuint program = glCreateProgram();
    for (int i = 0; i < count; ++i)
        glAttachShader(program, shaders[i].handle);
    glLinkProgram(program);
    
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    char* buffer = PushArray(arena, char, logLength + 1);
    GLint length = 0;
    glGetProgramInfoLog(program, logLength + 1, &length, buffer);
    
    // Handle Error
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    
    if (status == 0)
    {
        glDeleteProgram(program);
        program = 0;
    }
    
    return (OGL_Shader){ program, Str(buffer, length) };
}

//~ NOTE(long): Win32 OpenGL

typedef struct W32OpenGLWindow W32OpenGLWindow;
struct W32OpenGLWindow
{
    int dummy;
};

global W32OpenGLWindow w32OpenGLSlots[GFX_MAX_WINDOW_SLOTS] = {0};
global HMODULE w32OpenGLModule = 0;
global HGLRC w32OpenGLContext = 0;
global int w32OpenGLPixelFormat = 0;

global HDC  w32RenderDC = 0;
global HWND w32RenderWnd = 0;
global HWND w32CoreWnd = 0;
#define BOOTSTRAP_WINDOW_CLASS_NAME "LongOpenGLBootstrap"

//~ NOTE(long): OpenGL API

// NOTE(long): Here's how things work:
// Khronos defines the OpenGL API, while GPU vendors implement it in their drivers
// But how do applications actually *load* those functions? -> That’s where Microsoft comes in
// Microsoft provides the Windows-side interface (WGL) and the OpenGL loader opengl32.dll
// Up through OpenGL 1.1, you could GetProcAddress on opengl32.dll to load the old GL functions
//
// After OpenGL 1.1, Microsoft effectively stopped updating their OpenGL support
// They had Direct3D by then, and chose not to expose newer OpenGL versions
// The OpenGL ARB solved this by standardizing a WGL extension that lets drivers expose modern functionality
// You can load these extensions using wglGetProcAddress, obtained from the old opengl32.dll
// When you call wglGetProcAddress, Windows dispatches the request to your GPU driver’s ICD
//
// So here’s the general bootstrapping process:
// 1. Load the base GL and WGL functions from opengl32.dll (including wglGetProcAddress)
// 2. Create an invisible "bootstrap" window to initialize an old OpenGL 1.1 context
// 3. Use that context and wglGetProcAddress to load modern WGL extension functions
// 4. Use those new functions to create a modern OpenGL context
// 5. Clean up
//
// Ok, we understand the bootstrap window, but what about the dummy window? What is it for?
// It exists to support multiple windows in OpenGL by sharing a single context across them
// Whenever we need to draw to a window, we can simply pass it to wglMakeCurrent
// This shared OpenGL context is created on the hidden dummy window
// (From mmozeiko: https://gist.github.com/mmozeiko/ed2ad27f75edf9c26053ce332a1f6647)

function b32 OGL_Init(void)
{
    b32 error = 0;
    
    // Get instance
    HINSTANCE instance = W32GetInstance();
    
    // Setup opengl
    {
        if (w32OpenGLModule != 0)
            ErrorSet(error, "OpenGL has already intialized");
        
        if (!error)
        {
            w32OpenGLModule = LoadLibraryW(L"opengl32.dll");
            if (w32OpenGLModule == 0)
                ErrorSet(error, "Failed to initialize opengl.dll");
        }
    }
    
    if (!error)
    {
#define X(r, n, p)  W32_GET_PROC_ADDR(w32Wgl##n, w32OpenGLModule, Stringify(Concat(wgl, n)));
        WGL_FUNCS(X)
#undef X
        
        b32 missingWglFunc = false;
#define X(r, n, p) if (w32##Wgl##n == 0) { missingWglFunc = true; }
        WGL_FUNCS(X)
#undef X
        
        if (missingWglFunc)
            ErrorSet(error, "Failed to load wgl function(s)");
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
            ErrorSet(error, "Failed to resgister class");
        
        if (!error)
        {
            HWND hwnd = CreateWindow(BOOTSTRAP_WINDOW_CLASS_NAME,
                                     "opengl-bootstrap-window",
                                     0, 0, 0, 0, 0,     // style, x,y,w,h
                                     0, 0, instance, 0 // parent, menu, inst, param
                                     );
            
            if (hwnd == 0)
                ErrorSet(error, "Failed to create window");
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
            ErrorSet(error, "Failed to choose bootstrap pixel format");
        
        if (!error)
            if (!SetPixelFormat(dc, formatIDX, &formatDesc))
                ErrorSet(error, "Failed to create bootstrap pixel format");
        
        if (!error)
        {
            HGLRC hglrc = w32WglCreateContext(dc);
            if (hglrc == 0)
                ErrorSet(error, "Failed to create bootstrap context");
            
            // Load wgl ext functions
            {
                if (!error)
                    w32WglMakeCurrent(dc, hglrc);
#define X(r, n, p) \
    if (!error) \
    { \
        WGL_GET_PROC_ADDR(w32Wgl##n, "wgl"Stringify(n)); \
        if (w32Wgl##n == 0) ErrorSet(error, "Failed to initialize wgl"Stringify(n)); \
    }
                WGL_EXT_FUNCS(X);
#undef X
            }
            
            if (!error)
                bootstrapContext = hglrc;
        }
        
        ReleaseDC(bootstrapWindow, dc);
    }
    
    if (!error)
        w32CoreWnd = CreateWindow(GRAPHICS_WINDOW_CLASS_NAME, "LongDummy",
                                  0, 0, 0, 0, 0,
                                  0, 0, W32GetInstance(), 0);
    
	if (w32CoreWnd)
	{
		// Create real context
		HDC dc = GetDC(w32CoreWnd);
		
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
            ErrorSet(error, "Failed to choose graphics pixel format");
        
		if (!error)
		{
			PIXELFORMATDESCRIPTOR formatDesc = {0};
			BOOL spf = SetPixelFormat(dc, w32OpenGLPixelFormat, &formatDesc);
			if (!spf)
				ErrorSet(error, "Failed to set graphics pixel format");
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
				ErrorSet(error, "Failed to create graphics context");
		}
		
		// Load opengl functions
		{
#define X(r, n, p) if (!error) \
    { \
        W32_GET_PROC_ADDR(n, w32OpenGLModule, Stringify(n)); \
        if (!n) ErrorSet(error, "Failed to load "Stringify(n)); \
    }
            GL_FUNCS(X);
#undef X
            
#define X(r, n, p) if (!error) \
    { \
        WGL_GET_PROC_ADDR(n, Stringify(n));\
        if (!n) ErrorSet(error, "Faield to load "Stringify(n));\
    }
            GL_EXT_FUNCS(X);
#undef X
		}
		
		ReleaseDC(w32CoreWnd, dc);
	}
	
    // Clean up "temps"
    {
        if (bootstrapContext && !w32WglDeleteContext(bootstrapContext))
            ErrorSet(error, "Failed to destroy the bootstrap context");
		
        if (bootstrapWindow && !DestroyWindow(bootstrapWindow))
            ErrorSet(error, "Failed to destroy the bootstrap window");
		
#if 0
		if (w32CoreWnd && !DestroyWindow(w32CoreWnd))
			ErrorSet(error, "Failed to destroy the dummy context");
#endif
        
        if (atom && !UnregisterClass(BOOTSTRAP_WINDOW_CLASS_NAME, instance))
            ErrorSet(error, "Failed to unregister the bootstrap class");
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

function b32 OGL_Free()
{
    b32 error = 0;
    
    // Clear OpenGL
    {
        if (!w32OpenGLModule)
            ErrorSet(error, "opengl.dll has already been freed");
        else if (!FreeLibrary(w32OpenGLModule))
            ErrorSet(error, "Failed to free opengl.dll");
        
        if (!w32OpenGLContext)
            ErrorSet(error, "The graphics context has already been deleted");
        else if (!w32WglDeleteContext(w32OpenGLContext))
            ErrorSet(error, "Failed to delete the graphics context");
        
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

function b32 OGL_WindowEquip(GFXWindow window)
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
			ErrorSet(error, "Failed to set graphics pixel format");
		
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

function void OGL_Begin(GFXWindow window)
{
    if (GFXWindowIsValid(window) && w32RenderDC == 0)
    {
        W32Window* slot = W32WindowFromGFXHandle(window);
        w32RenderWnd = slot->wnd;
        w32RenderDC = GetDC(w32RenderWnd);
		w32WglMakeCurrent(w32RenderDC, w32OpenGLContext);
        DEBUG(error, GLenum error = glGetError());
    }
}

function void OGL_End(void)
{
    if (w32RenderDC != 0)
    {
        SwapBuffers(w32RenderDC);
        ReleaseDC(w32RenderWnd, w32RenderDC);
        w32RenderDC = 0;
        w32RenderWnd = 0;
    }
}
