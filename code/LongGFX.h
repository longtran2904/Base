/* date = September 25th 2022 4:26 pm */

#ifndef _LONG_G_F_X_H
#define _LONG_G_F_X_H

#ifndef GFX_MAX_WINDOW_SLOTS
#define GFX_MAX_WINDOW_SLOTS 64
#endif

// --------------------------------------------------
// NOTE: GFX Types

typedef u64 GFXWindow;
typedef void GFXDestroyWindow(GFXWindow window);
typedef void GFXResizeWindow(GFXWindow window, u32 width, u32 height);

// --------------------------------------------------
// NOTE: GFX Functions

function b32 InitGFX(void);
function b32 IsGFXWindowValid(GFXWindow window);
function b32 IsGFXWindowEquipped(GFXWindow window);

function GFXWindow CreateGFXWindow(void);
function GFXWindow CreateGFXWindowEx(String title, i32 x, i32 y, i32 width, i32 height);
function void      ShowGFXWindow(GFXWindow window);
function void      CloseGFXWindow(GFXWindow window);

function GFXResizeWindow* GetGFXResizeFunc(void);
function void             SetGFXResizeFunc(GFXResizeWindow* func);

function void* GetGFXWindowUserData(GFXWindow window);
function void  SetGFXWindowUserData(GFXWindow window, void* data);

function void* GetGFXWindowEquippedData(GFXWindow window);
function void  SetGFXWindowEquippedData(GFXWindow window, void* ptr, GFXDestroyWindow* destroy);

function b32 SetGFXWindowVisible(GFXWindow window, b32 visible);
function b32 SetGFXWindowTitle(GFXWindow window, String title);
function b32 SetGFXWindowInnerRect(GFXWindow window, i32 x, i32 y, i32 w, i32 h);
function b32 GetGFXWindowInnerRect(GFXWindow window, i32* x, i32* y, i32* w, i32* h);
function b32 GetGFXWindowOuterRect(GFXWindow window, i32* x, i32* y, i32* w, i32* h);

function b32 SetGFXWindowResizable(GFXWindow window, b32 resizable);
function b32 SetGFXWindowFullScreen(GFXWindow window, b32 fullscreen);

function b32 IsGFXWindowResizable(GFXWindow window);
function b32 IsGFXWindowFullScreen(GFXWindow window);
function b32 IsGFXWindowMinimized(GFXWindow window);
function b32 IsGFXWindowMaximized(GFXWindow window);

function b32  WaitForGFXInput(void);
function void GFXMessageBox(String title, String message);
function void GFXErrorBox(StringList* errors, i32 code);

#endif //_LONG_G_F_X_H
