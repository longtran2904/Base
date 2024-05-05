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

//~ NOTE(long): Setup Window

function GFXWindow GFXCreateWindowEx(String title, i32 x, i32 y, i32 width, i32 height);
function void GFXShowWindow(GFXWindow window);
function void GFXCloseWindow(GFXWindow window);

#define GFXCreateWindow(title) GFXCreateWindowEx(StrLit(title), CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT)

function void* GetGFXWindowEquippedData(GFXWindow window);
function void  SetGFXWindowEquippedData(GFXWindow window, void* ptr, GFXDestroyWindow* destroy);

//~ NOTE(long): Setup User Data

function GFXResizeWindow* GetGFXResizeFunc(void);
function void             SetGFXResizeFunc(GFXResizeWindow* func);

function void* GetGFXWindowUserData(GFXWindow window);
function void  SetGFXWindowUserData(GFXWindow window, void* data);

//~ NOTE(long): Get/Set Window's Values

function b32 SetGFXWindowVisible(GFXWindow window, b32 visible);
function b32 SetGFXWindowTitle(GFXWindow window, String title);

function b32 SetGFXWindowInnerRect(GFXWindow window, i32 x, i32 y, i32 w, i32 h);
function b32 SetGFXWindowOuterRect(GFXWindow window, i32 x, i32 y, i32 w, i32 h);
function b32 GetGFXWindowInnerRect(GFXWindow window, i32* x, i32* y, i32* w, i32* h);
function b32 GetGFXWindowOuterRect(GFXWindow window, i32* x, i32* y, i32* w, i32* h);

function b32 SetGFXWindowResizable(GFXWindow window, b32 resizable);
function b32 SetGFXWindowFullScreen(GFXWindow window, b32 fullscreen);

function b32 IsGFXWindowResizable(GFXWindow window);
function b32 IsGFXWindowFullScreen(GFXWindow window);
function b32 IsGFXWindowMinimized(GFXWindow window);
function b32 IsGFXWindowMaximized(GFXWindow window);

function b32 GFXIsWindowValid(GFXWindow window);
function b32 GFXIsWindowEquipped(GFXWindow window);

//~ NOTE(long): Error/Message Box

function void GFXMessageBox(String title, String message);
function void GFXErrorBox(Logger* logger, i32 code);
function void GFXErrorFmt(Arena* arena, Record* record, char* fmt, va_list args);
#define GFXCheckError(...) do { Logger _errors_ = ErrorEnd(); GFXErrorBox(&_errors_, (i32){ __VA_ARGS__+0 }); } while (0)
#define GFXErrorBlock(arena, code, ...) for (struct { i32 i; Logger log; } __dummy__ = { .i = (ErrorBegin(arena, __VA_ARGS__), 0) }; \
                                             __dummy__.i == 0; \
                                             __dummy__.i++, GFXErrorBox((__dummy__.log = ErrorEnd(), &__dummy__.log), (code)))

#endif //_LONG_G_F_X_H
