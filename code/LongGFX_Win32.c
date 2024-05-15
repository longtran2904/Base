
//~ NOTE(long): Internal Win32 APIs

typedef struct W32Window W32Window;
struct W32Window
{
    W32Window* next;
	HWND wnd;
	WINDOWPLACEMENT place;
	b32 canResize;
	
	// Application
	void* userData;
	
	// Graphics System Implementation
	GFXDestroyWindow* destroyFunc;
	void* equippedData;
};

global W32Window  w32WindowSlots[GFX_MAX_WINDOW_SLOTS];
global W32Window* w32WindowFree = 0;
global GFXResizeWindow* w32ResizingFunc;

function W32Window* W32WindowFromGFXHandle(GFXWindow window)
{
    local W32Window sentinel = {0};
    W32Window* result = &sentinel;
    if (1 <= window && window <= GFX_MAX_WINDOW_SLOTS)
        result = w32WindowSlots + window - 1;
    return result;
}

function GFXWindow W32GFXHandleFromHWND(HWND wnd)
{
	if (wnd)
		for (u32 i = 0; i < GFX_MAX_WINDOW_SLOTS; ++i)
			if (w32WindowSlots[i].wnd == wnd)
                return i + 1;
	return 0;
}

LRESULT W32GraphicsWindowProc(HWND   hwnd,
                              UINT   msg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    LRESULT result = 0;
    switch (msg)
    {
		case WM_SIZE:
		{
			if (w32ResizingFunc != 0)
			{
				GFXWindow window = W32GFXHandleFromHWND(hwnd);
				u16 width = (lParam & 0xFFFF);
				u16 height = (u16)(lParam >> 16);
				w32ResizingFunc(window, width, height);
			}
		} break;
		
        case WM_CLOSE:
        {
			GFXWindow window = W32GFXHandleFromHWND(hwnd);
			if (window)
				GFXCloseWindow(window);
        } break;
		
        default:
        {
            result = DefWindowProc(hwnd, msg, wParam, lParam);
        } break;
    }
    return result;
}

//~ NOTE(long): GFX APIs

function b32 GFXWindowIsValid(GFXWindow window)
{
    return (1 <= window) && (window <= ArrayCount(w32WindowSlots)) && (w32WindowSlots + window - 1)->wnd != 0;
}

function GFXResizeWindow* GFXGetResizeFunc(void)
{
	return w32ResizingFunc;
}

function void GFXSetResizeFunc(GFXResizeWindow* func)
{
	w32ResizingFunc = func;
}

function void* GFXWindowGetUserData(GFXWindow window)
{
	return W32WindowFromGFXHandle(window)->userData;
}

function void GFXWindowSetUserData(GFXWindow window, void* data)
{
	W32WindowFromGFXHandle(window)->userData = data;
}

function void* GFXWindowGetEquipment(GFXWindow window)
{
    return W32WindowFromGFXHandle(window)->equippedData;
}

function void GFXWindowEquipData(GFXWindow window, void* ptr, GFXDestroyWindow* destroy)
{
    if (GFXWindowIsValid(window))
    {
        W32Window* slot = W32WindowFromGFXHandle(window);
        slot->equippedData = ptr;
        slot->destroyFunc = destroy;
    }
}

function b32 GFXWindowIsEquipped(GFXWindow window)
{
    return W32WindowFromGFXHandle(window)->equippedData != 0;
}

function b32 GFXInit(void)
{
    // Setup window slots
    {
        ZeroFixedArr(w32WindowSlots);
        w32WindowFree = w32WindowSlots;
        W32Window* last = w32WindowSlots + ArrayCount(w32WindowSlots) - 1;
        for (W32Window* slot = w32WindowSlots; slot < last; ++slot)
            slot->next = (slot + 1);
    }
	
#define GRAPHICS_WINDOW_CLASS_NAME "Long_GFX_Graphics"
    b32 error = 0;
    if (!RegisterClass(&(WNDCLASS){
                           .style = CS_HREDRAW|CS_VREDRAW,
                           .lpfnWndProc = W32GraphicsWindowProc,
                           .hInstance = W32GetInstance(),
                           .lpszClassName = GRAPHICS_WINDOW_CLASS_NAME,
                       }))
        ErrorSet("Failed to resgister class", error);
    
    return !error;
}

function GFXWindow GFXCreateWindowEx(String title, i32 x, i32 y, i32 width, i32 height)
{
    b32 error = 0;
    W32Window* slotPtr = w32WindowFree;
    if (slotPtr == 0)
        ErrorSet("Reached max window count", error);
    
    HWND wnd = 0;
    if (!error)
	{
		wnd = CreateWindow(GRAPHICS_WINDOW_CLASS_NAME, title.str,
						   WS_OVERLAPPEDWINDOW,
						   x, y, width, height,
						   0, 0, W32GetInstance(), 0);
        if (!wnd)
			ErrorSet("Failed to create graphics window", error);
	}
    
    GFXWindow result = 0;
    if (!error)
    {
        SLLStackPop(w32WindowFree);
        ZeroStruct(slotPtr);
		slotPtr->wnd = wnd;
		slotPtr->canResize = true;
        result = (GFXWindow)(slotPtr - w32WindowSlots + 1);
    }
    
    return result;
}

function void GFXShowWindow(GFXWindow window)
{
    W32Window* slot = W32WindowFromGFXHandle(window);
    if (slot->wnd)
        ShowWindow(slot->wnd, SW_SHOW);
}

#define W32_WS_BORDERED (WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX)
#define W32_WS_RESIZABLE (WS_THICKFRAME|WS_MAXIMIZEBOX)

function GFXFlags GFXWindowGetFlags(GFXWindow window)
{
	GFXFlags result = 0;
	
    W32Window* slot = W32WindowFromGFXHandle(window);
    if (slot->wnd)
    {
        LONG style = GetWindowLong(slot->wnd, GWL_STYLE);
        result |= (!(style & WS_OVERLAPPEDWINDOW) ? FLAG_MODE_FULLSCREEN : 0);
        
        result |= (style & WS_MINIMIZE) ? FLAG_WINDOW_MINIMIZED : 0;
        result |= (style & WS_MAXIMIZE) ? FLAG_WINDOW_MAXIMIZED : 0;
        result |= HasAllFlags(style, W32_WS_RESIZABLE) ? FLAG_WINDOW_RESIZABLE : 0;
        result |= GetFocus() != slot->wnd ? FLAG_WINDOW_UNFOCUSED : 0;
        result |= !(style & WS_VISIBLE) ? FLAG_WINDOW_HIDDEN : 0;
        result |= (GetWindowLong(slot->wnd, GWL_EXSTYLE) & WS_EX_TOPMOST) ? FLAG_WINDOW_TOPMOST : 0;
    }
    
	return result;
}

function b32 GFXWindowSetFlags(GFXWindow window, GFXFlags flags, b32 value)
{
    b32 result = 0;
    value = !!value;
    
    W32Window* slot = W32WindowFromGFXHandle(window);
    if (slot->wnd)
    {
        LONG style   = GetWindowLong(slot->wnd, GWL_STYLE);
        LONG exStyle = GetWindowLong(slot->wnd, GWL_EXSTYLE);
        
        if ((flags & FLAG_MODE_FULLSCREEN) && !(style & WS_OVERLAPPEDWINDOW) != value)
        {
            b32 success = 0;
            
            if (value)
            {
                MONITORINFO monitorInfo = { sizeof(monitorInfo) };
                if (GetWindowPlacement(slot->wnd, &slot->place) &&
                    GetMonitorInfo(MonitorFromWindow(slot->wnd, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
                    if (SetWindowLong(slot->wnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW))
                        success = SetWindowPos(slot->wnd, HWND_TOP,
                                               monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                                               monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                                               monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                                               SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            }
            
            else
            {
                LONG newStyle = style|W32_WS_BORDERED;
                if (slot->canResize)
                    newStyle |= W32_WS_RESIZABLE;
                SetWindowLong(slot->wnd, GWL_STYLE, newStyle);
                SetWindowPlacement(slot->wnd, &slot->place);
                success = SetWindowPos(slot->wnd, NULL, 0, 0, 0, 0,
                                       SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|
                                       SWP_NOOWNERZORDER|SWP_FRAMECHANGED);
            }
            
            result |= success ? FLAG_MODE_FULLSCREEN : 0;
        }
        
        if ((flags & FLAG_WINDOW_MINIMIZED) && !!(style & WS_MINIMIZE) != value)
            result |= ShowWindow(slot->wnd, value ? SW_MINIMIZE : SW_RESTORE) ? FLAG_WINDOW_MINIMIZED : 0;
        if ((flags & FLAG_WINDOW_MAXIMIZED) && !!(style & WS_MAXIMIZE) != value)
            result |= ShowWindow(slot->wnd, value ? SW_MAXIMIZE : SW_RESTORE) ? FLAG_WINDOW_MAXIMIZED : 0;
        if ((flags & FLAG_WINDOW_HIDDEN) && !(style & WS_VISIBLE) != value)
            result |= ShowWindow(slot->wnd, value ? SW_HIDE : SW_SHOW) ? FLAG_WINDOW_HIDDEN : 0;
        
        if ((flags & FLAG_WINDOW_UNFOCUSED) && (GetFocus() != slot->wnd) != value && value)
            // NOTE(long): Is this enough?
            result |= SetForegroundWindow(slot->wnd) ? FLAG_WINDOW_UNFOCUSED : 0;
        
        if ((flags & FLAG_WINDOW_RESIZABLE) && HasAllFlags(style, W32_WS_RESIZABLE) != value)
        {
            slot->canResize = value;
            LONG newStyle = value ? (style|W32_WS_RESIZABLE) : (style & ~W32_WS_RESIZABLE);
            result |= SetWindowLong(slot->wnd, GWL_STYLE, newStyle) ? FLAG_WINDOW_RESIZABLE : 0;
        }
        
        if ((flags & FLAG_WINDOW_TOPMOST) && !!(exStyle & WS_EX_TOPMOST) != value)
            result |= SetWindowPos(slot->wnd, value ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
                                   SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE) ? FLAG_WINDOW_TOPMOST : 0;
    }
    
    return result;
}

function String GFXWindowGetTitle(Arena* arena, GFXWindow window)
{
    String result = {0};
    ScratchBlock(scratch)
    {
        HWND wnd = W32WindowFromGFXHandle(window)->wnd;
        if (wnd)
        {
            result.size = GetWindowTextLength(wnd);
            if (result.size)
            {
                result.str = ArenaPush(arena, result.size + 1);
                GetWindowText(wnd, result.str, (int)result.size + 1);
            }
        }
    }
    return result;
}

function b32 GFXWindowSetTitle(GFXWindow window, String title)
{
    b32 result = 0;
    ScratchBlock(scratch)
    {
        HWND wnd = W32WindowFromGFXHandle(window)->wnd;
        if (wnd)
            result = SetWindowText(wnd, StrToCStr(scratch, title));
    }
	return result;
}

function b32 GFXWindowSetInnerRect(GFXWindow window, i32 x, i32 y, i32 w, i32 h)
{
	b32 result = false;
    
    HWND wnd = W32WindowFromGFXHandle(window)->wnd;
    if (wnd)
    {
        RECT rect = { .left = x, .top = y, .right = x + w, .bottom = y + h };
        if (AdjustWindowRect(&rect, GetWindowLong(wnd, GWL_STYLE), 0))
            if (SetWindowPos(wnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER))
                result = true;
    }
    
	return result;
}

function b32 GFXWindowSetOuterRect(GFXWindow window, i32 x, i32 y, i32 w, i32 h)
{
    b32 result = false;
    
    HWND wnd = W32WindowFromGFXHandle(window)->wnd;
    if (wnd)
    {
        RECT rect = { .left = x, .top = y, .right = x + w, .bottom = y + h };
        if (SetWindowPos(wnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER))
        {
            RECT smallRect;
            if (DwmGetWindowAttribute(wnd, DWMWA_EXTENDED_FRAME_BOUNDS, &smallRect, sizeof(RECT)) == S_OK) // No drop shadow
            {
                rect.left -= smallRect.left - rect.left;
                rect.top  -= smallRect.top - rect.top;
                rect.right += rect.right - smallRect.right;
                rect.bottom += rect.bottom - smallRect.bottom;
                if (SetWindowPos(wnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER))
                    result = true;
            }
        }
    }
    
    return result;
}

function b32 GFXWindowGetInnerRect(GFXWindow window, i32* x, i32* y, i32* w, i32* h)
{
	b32 result = false;
	HWND wnd = W32WindowFromGFXHandle(window)->wnd;
	RECT rect = {0};
	
    if (wnd && GetClientRect(wnd, &rect))
	{
        if (ClientToScreen(wnd, (POINT*)(&rect.left)) && ClientToScreen(wnd, (POINT*)(&rect.right)))
        {
            if (x) *x = rect.left;
            if (y) *y = rect.top;
            if (w) *w = rect.right - rect.left;
            if (h) *h = rect.bottom - rect.top;
            result = true;
        }
	}
	
	return result;
}

function b32 GFXWindowGetOuterRect(GFXWindow window, i32* x, i32* y, i32* w, i32* h)
{
	b32 result = false;
	HWND wnd = W32WindowFromGFXHandle(window)->wnd;
	RECT rect = {0};
	
	if (wnd && DwmGetWindowAttribute(wnd, DWMWA_EXTENDED_FRAME_BOUNDS, &rect, sizeof(RECT)) == S_OK)
	{
		if (x) *x = rect.left;
		if (y) *y = rect.top;
		if (w) *w = rect.right - rect.left;
		if (h) *h = rect.bottom - rect.top;
		result = true;
	}
	
	return result;
}

function void GFXCloseWindow(GFXWindow window)
{
    W32Window* slot = W32WindowFromGFXHandle(window);
    if (slot->wnd)
    {
        if (slot->destroyFunc)
            slot->destroyFunc(window);
        DestroyWindow(slot->wnd);
        ZeroStruct(slot);
        SLLStackPush(w32WindowFree, slot);
    }
}

function b32 GFXWaitForInput(void)
{
    b32 result = 1;
    
    MSG msg = {0};
    BOOL getMessage = GetMessage(&msg, 0, 0, 0);
    if (getMessage <= 0)
        result = 0;
    else
        DispatchMessage(&msg);
    
    return result;
}

function b32 GFXPeekInput(void)
{
    b32 result = 1;
    
    MSG msg = {0};
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        DispatchMessage(&msg);
    
    return result;
}

function void GFXMessageBox(String title, String message)
{
    MessageBoxA(0, message.str, title.str, MB_OK);
}

function void GFXErrorBox(Logger* logger, i32 code)
{
    if (logger->count)
    {
        ScratchBlock(scratch)
        {
            StringList errors = {0};
            for (u64 i = 0; i < logger->count; ++i)
                StrListPush(scratch, &errors, logger->records[i].log);
            
            String error = StrJoin(scratch, &errors,
                                   .pre = StrPushf(scratch, "The process has encountered %d error(s):\n", errors.nodeCount),
                                   .mid = StrLit(".\n"), .post = StrLit(".\n"));
            MessageBoxA(0, error.str, "Error", MB_OK|MB_ICONERROR);
        }
        
        if (code)
            OSExitProcess(code);
    }
}

function void GFXErrorFmt(Arena* arena, Record* record, char* fmt, va_list args)
{
    record->log = StrPushfv(arena, fmt, args);
    record->log = StrPushf(arena, "%s:%d: %s", record->file, record->line, record->log.str);
}
