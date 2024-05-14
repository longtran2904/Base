/* date = September 25th 2022 4:26 pm */

#ifndef _LONG_G_F_X_H
#define _LONG_G_F_X_H

#ifndef GFX_MAX_WINDOW_SLOTS
#define GFX_MAX_WINDOW_SLOTS 64
#endif

//~ NOTE(long): GFX Types

typedef u64 GFXWindow;
typedef void GFXDestroyWindow(GFXWindow window);
typedef void GFXResizeWindow(GFXWindow window, u32 width, u32 height);

//~ NOTE(long): Setup GFX

function b32 GFXInit(void);
function b32 GFXWaitForInput(void);
function b32 GFXPeekInput(void);

//~ NOTE(long): Setup Window

function GFXWindow GFXCreateWindowEx(String title, i32 x, i32 y, i32 width, i32 height);
function void GFXShowWindow(GFXWindow window);
function void GFXCloseWindow(GFXWindow window);

#define GFXCreateWindow(title) GFXCreateWindowEx(StrLit(title), CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT)

function void* GFXWindowGetEquipment(GFXWindow window);
function void  GFXWindowEquipData(GFXWindow window, void* ptr, GFXDestroyWindow* destroy);

//~ NOTE(long): Setup User Data

function GFXResizeWindow* GFXGetResizeFunc(void);
function void             GFXSetResizeFunc(GFXResizeWindow* func);

function void* GFXWindowGetUserData(GFXWindow window);
function void  GFXWindowSetUserData(GFXWindow window, void* data);

//~ NOTE(long): Get/Set Window's Values

function b32 GFXWindowIsValid(GFXWindow window);
function b32 GFXWindowIsEquipped(GFXWindow window);

typedef u32 GFXFlags;
enum
{
    // TODO(long)
    FLAG_HINT_VSYNC      = 1 << 2, // Set to try enabling V-Sync on GPU
    FLAG_HINT_MSAA_4X    = 1 << 3, // Set to try enabling MSAA 4X
    FLAG_HINT_INTERLACED = 1 << 4, // Set to try enabling interlaced video format (for V3D)
    
    FLAG_MODE_FULLSCREEN = 1 << 5, // Set to run program in fullscreen
    
    FLAG_WINDOW_MINIMIZED   = 1 <<  8, // Set to minimize window (iconify)
    FLAG_WINDOW_MAXIMIZED   = 1 <<  9, // Set to maximize window (expanded to monitor)
    FLAG_WINDOW_RESIZABLE   = 1 << 10, // Set to allow resizable window
    FLAG_WINDOW_UNFOCUSED   = 1 << 11, // Set to window non focused
    FLAG_WINDOW_HIDDEN      = 1 << 12, // Set to hide window
    FLAG_WINDOW_TOPMOST     = 1 << 13, // Set to window always on top
    
    // TODO(long)
    FLAG_WINDOW_TRANSPARENT,
    FLAG_WINDOW_HIGHDPI,
    
    // @RECONSIDER(long)
    FLAG_MODE_BORDERLESS,    // Set to run program in borderless windowed mode
    FLAG_WINDOW_ALWAYS_RUN,  // Set to allow windows running while minimized
    FLAG_WINDOW_UNDECORATED, // Set to disable window decoration (frame and buttons)
    FLAG_WINDOW_PASSTHROUGH, // Set to support mouse passthrough, only supported when FLAG_WINDOW_UNDECORATED
};

function GFXFlags GFXWindowGetFlags(GFXWindow window);
function b32      GFXWindowSetFlags(GFXWindow window, GFXFlags flags, b32 value);
#define GFXWindowHasFlags(window, flags) HasAnyFlags(GFXWindowGetFlags((window)), (flags))

function String GFXWindowGetTitle(Arena* arena, GFXWindow window);
function b32    GFXWindowSetTitle(GFXWindow window, String title);

function b32 GFXWindowSetInnerRect(GFXWindow window, i32 x, i32 y, i32 w, i32 h);
function b32 GFXWindowSetOuterRect(GFXWindow window, i32 x, i32 y, i32 w, i32 h);
function b32 GFXWindowGetInnerRect(GFXWindow window, i32* x, i32* y, i32* w, i32* h);
function b32 GFXWindowGetOuterRect(GFXWindow window, i32* x, i32* y, i32* w, i32* h);

//~ NOTE(long): Error/Message Box

function void GFXMessageBox(String title, String message);
function void GFXErrorBox(Logger* logger, i32 code);
function void GFXErrorFmt(Arena* arena, Record* record, char* fmt, va_list args);
#define GFXCheckError(...) do { Logger _errors_ = ErrorEnd(); GFXErrorBox(&_errors_, (i32){ __VA_ARGS__+0 }); } while (0)
#define GFXErrorBlock(arena, code, ...) for (struct { i32 i; Logger log; } __dummy__ = { .i = (ErrorBegin(arena, __VA_ARGS__), 0) }; \
                                             __dummy__.i == 0; \
                                             __dummy__.i++, GFXErrorBox((__dummy__.log = ErrorEnd(), &__dummy__.log), (code)))

#endif //_LONG_G_F_X_H
