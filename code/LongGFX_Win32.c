
// --------------------------------------------------
// NOTE: Internal Win32 APIs

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
    Assert(1 <= window && window <= GFX_MAX_WINDOW_SLOTS);
    W32Window* result = w32WindowSlots + window - 1;
    Assert(result->wnd);
    return result;
}

function GFXWindow W32GFXHandleFromHWND(HWND wnd)
{
	if (wnd)
	{
		for (u32 i = 0; i < GFX_MAX_WINDOW_SLOTS; ++i)
			if (w32WindowSlots[i].wnd == wnd)
                return i + 1;
	}
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
				u16 height = (lParam >> 16);
				w32ResizingFunc(window, width, height);
			}
		} break;
		
        case WM_CLOSE:
        {
			GFXWindow window = W32GFXHandleFromHWND(hwnd);
			if (window)
				CloseGFXWindow(window);
        } break;
		
        default:
        {
            result = DefWindowProc(hwnd, msg, wParam, lParam);
        } break;
    }
    return result;
}

// --------------------------------------------------
// NOTE: GFX APIs

function b32 IsGFXWindowValid(GFXWindow window)
{
    return (1 <= window) && (window <= ArrayCount(w32WindowSlots)) && (w32WindowSlots + window - 1)->wnd != 0;
}

function GFXResizeWindow* GetGFXResizeFunc(void)
{
	return w32ResizingFunc;
}

function void SetGFXResizeFunc(GFXResizeWindow* func)
{
	w32ResizingFunc = func;
}

function void* GetGFXWindowUserData(GFXWindow window)
{
	return W32WindowFromGFXHandle(window)->userData;
}

function void SetGFXWindowUserData(GFXWindow window, void* data)
{
	W32WindowFromGFXHandle(window)->userData = data;
}

function void* GetGFXWindowEquippedData(GFXWindow window)
{
    return W32WindowFromGFXHandle(window)->equippedData;
}

function void SetGFXWindowEquippedData(GFXWindow window, void* ptr, GFXDestroyWindow* destroy)
{
	W32Window* slot = W32WindowFromGFXHandle(window);
	slot->equippedData = ptr;
	slot->destroyFunc = destroy;
}

function b32 IsGFXWindowEquipped(GFXWindow window)
{
    return W32WindowFromGFXHandle(window)->equippedData != 0;
}

function b32 InitGFX(void)
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

function GFXWindow CreateGFXWindow(void)
{
	return CreateGFXWindowEx(StrLit("TITLE ME"), CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT);
}

function GFXWindow CreateGFXWindowEx(String title, i32 x, i32 y, i32 width, i32 height)
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

function void ShowGFXWindow(GFXWindow window)
{
	W32Window* slot = W32WindowFromGFXHandle(window);
	ShowWindow(slot->wnd, SW_SHOW);
}

function b32 SetGFXWindowVisible(GFXWindow window, b32 visible)
{
	return ShowWindow(W32WindowFromGFXHandle(window)->wnd, visible ? SW_SHOW : SW_HIDE);
}

function b32 SetGFXWindowTitle(GFXWindow window, String title)
{
	BeginScratch(scratch);
	b32 result = SetWindowText(W32WindowFromGFXHandle(window)->wnd, StrToCStr(scratch, title));
	EndScratch(scratch);
	return result;
}

function b32 SetGFXWindowInnerRect(GFXWindow window, i32 x, i32 y, i32 w, i32 h)
{
	b32 result = false;
	HWND wnd = W32WindowFromGFXHandle(window)->wnd;
	LONG style = GetWindowLong(wnd, GWL_STYLE);
	RECT r = {
		.left = x, .top = y, .right = x + w, .bottom = y + h
	};
	if (AdjustWindowRect(&r, style, false))
		if (SetWindowPos(wnd, HWND_TOP,
						 r.left, r.top, r.right - r.left, r.bottom - r.top,
						 0))
            result = true;
	
	return result;
}

function b32 GetGFXWindowInnerRect(GFXWindow window, i32* x, i32* y, i32* w, i32* h)
{
	b32 result = false;
	HWND wnd = W32WindowFromGFXHandle(window)->wnd;
	RECT rect = {0};
	
	if (GetClientRect(wnd, &rect) && ClientToScreen(wnd, (POINT*)(&rect.left)) && ClientToScreen(wnd, (POINT*)(&rect.right)))
	{
		if (x) *x = rect.left;
		if (y) *y = rect.top;
		if (w) *w = rect.right - rect.left;
		if (h) *h = rect.bottom - rect.top;
		result = true;
	}
	
	return result;
}

function b32 GetGFXWindowOuterRect(GFXWindow window, i32* x, i32* y, i32* w, i32* h)
{
	b32 result = false;
	HWND wnd = W32WindowFromGFXHandle(window)->wnd;
	RECT rect = {0};
	
	if (GetWindowRect(wnd, &rect))
	{
		if (x) *x = rect.left;
		if (y) *y = rect.top;
		if (w) *w = rect.right - rect.left;
		if (h) *h = rect.bottom - rect.top;
		result = true;
	}
	
	return result;
}

function void CloseGFXWindow(GFXWindow window)
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

#define W32_WS_BORDERED (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU)
#define W32_WS_RESIZABLE (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)

function b32 SetGFXWindowResizable(GFXWindow window, b32 resizable)
{
	b32 result = false;
	W32Window* slot = W32WindowFromGFXHandle(window);
	DWORD style = GetWindowLong(slot->wnd, GWL_STYLE);
	DWORD newStyle = 0;
	
	if (resizable)
		newStyle = style | W32_WS_RESIZABLE;
	else
		newStyle = style & ~W32_WS_RESIZABLE;
	
	if (SetWindowLong(slot->wnd, GWL_STYLE, newStyle))
	{
		slot->canResize = resizable;
		result = true;
	}
	
	return result;
}

function b32 SetGFXWindowFullScreen(GFXWindow window, b32 fullscreen)
{
	b32 result = false;
	
	W32Window* slot = W32WindowFromGFXHandle(window);
	DWORD style = GetWindowLong(slot->wnd, GWL_STYLE);
	b32 isFullScreenAlready = !(style & WS_OVERLAPPEDWINDOW);
	if (fullscreen != isFullScreenAlready)
	{
		// change to fullscreen
		if (fullscreen)
		{
			MONITORINFO monitorInfo = { sizeof(monitorInfo) };
			if (GetWindowPlacement(slot->wnd, &slot->place) &&
				GetMonitorInfo(MonitorFromWindow(slot->wnd, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
				if (SetWindowLong(slot->wnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW))
                    result = SetWindowPos(slot->wnd, HWND_TOP,
                                          monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
                                          monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                                          monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                                          SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
		
		// change to windowed
		else
		{
			DWORD newStyle = style | W32_WS_BORDERED;
			if (slot->canResize)
				newStyle |= W32_WS_RESIZABLE;
			SetWindowLong(slot->wnd, GWL_STYLE, newStyle);
			SetWindowPlacement(slot->wnd, &slot->place);
			result = SetWindowPos(slot->wnd, NULL, 0, 0, 0, 0,
								  SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
								  SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	
	return result;
}

function b32 IsGFXWindowResizable(GFXWindow window)
{
	DWORD style = GetWindowLong(W32WindowFromGFXHandle(window)->wnd, GWL_STYLE);
	return !!(style & W32_WS_RESIZABLE);
}

function b32 IsGFXWindowFullScreen(GFXWindow window)
{
    DWORD style = GetWindowLong(W32WindowFromGFXHandle(window)->wnd, GWL_STYLE);
    return !(style & WS_OVERLAPPEDWINDOW);
}

function b32 IsGFXWindowMinimized(GFXWindow window)
{
	b32 result = IsIconic(W32WindowFromGFXHandle(window)->wnd);
    return result;
}

function b32 IsGFXWindowMaximized(GFXWindow window)
{
    b32 result = IsZoomed(W32WindowFromGFXHandle(window)->wnd);
    return result;
}

function b32 WaitForGFXInput(void)
{
    b32 result = true;
    
    MSG msg = {0};
    BOOL getMessage = GetMessage(&msg, 0, 0, 0);
    if (getMessage <= 0)
        result = false;
    else
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
        BeginScratch(scratch);
        StringList errors = {0};
        for (u64 i = 0; i < logger->count; ++i)
            StrListPush(scratch, &errors, logger->records[i].log);
        
        String error = StrJoin(scratch, &errors,
                               .pre = StrPushf(scratch, "The process has encountered %d error(s):\n", errors.nodeCount),
                               .mid = StrLit(".\n"), .post = StrLit(".\n"));
        MessageBoxA(0, error.str, "Error", MB_OK|MB_ICONERROR);
        EndScratch(scratch);
        
        if (code)
            ExitOSProcess(code);
    }
}

function void GFXErrorFmt(Arena* arena, Record* record, char* fmt, va_list args)
{
    record->log = StrPushfv(arena, fmt, args);
    record->log = StrPushf(arena, "%s:%d: %s", record->file, record->line, record->log.str);
}
