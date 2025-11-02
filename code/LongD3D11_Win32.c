
//~ NOTE(long): Win32 D3D11 Globals

#define iid_IDXGIFactory (&IID_IDXGIFactory)
#define iid_ID3D11Texture2D (&IID_ID3D11Texture2D)
#define iid_ID3D11Debug (&IID_ID3D11Debug)

typedef struct W32D3D11Window W32D3D11Window;
struct W32D3D11Window
{
    IDXGISwapChain* swapchain;
};

global W32D3D11Window w32D3D11Slots[GFX_MAX_WINDOW_SLOTS] = {0};

global ID3D11Device* d3d11Device = 0;
global ID3D11DeviceContext* d3d11Ctx = 0;
global ID3D11Debug* d3d11Dbg = 0;
global IDXGISwapChain* dxgiSwapChain = 0;

HMODULE dxgiModule = 0;
HMODULE d3d11Module = 0;
HMODULE d3dcompilerModule = 0;

#define D3D_COMPILER_DLL "d3dcompiler_47.dll"

function b32 InitD3D11(void)
{
    b32 error = 0;
    
    // Load modules and functions
    {
#define X(r, n, p) if (!error) \
    { \
        W32_GET_PROC_ADDR(w32##n, scopeModule, Stringify(n)); \
        if (!w32##n) ErrorSet(error, "Failed to load "Stringify(n)); \
    }
        
        // -- dxgi.dll --
        {
            if (dxgiModule != 0)
                ErrorSet(error, "dxgi.dll has already initialized");
            
            if (!error)
            {
                dxgiModule = LoadLibraryW(L"dxgi.dll");
                if (!dxgiModule)
                    ErrorSet(error, "Failed to load dxgi.dll");
            }
            
            if (!error)
            {
                HMODULE scopeModule = dxgiModule;
                DXGI_FUNCS(X)
            }
        }
        
        // -- d3d11.dll --
        if (!error)
        {
            if (d3d11Module != 0)
                ErrorSet(error, "d3d11.dll has already initialized");
            
            if (!error)
            {
                d3d11Module = LoadLibraryW(L"d3d11.dll");
                if (!d3d11Module)
                    ErrorSet(error, "Failed to load d3d11.dll");
            }
            
            if (!error)
            {
                HMODULE scopeModule = d3d11Module;
                D3D11_FUNCS(X)
            }
        }
        
        // -- d3dcompiler_47.dll --
        if (!error)
        {
            if (d3dcompilerModule != 0)
                ErrorSet(error, D3D_COMPILER_DLL " has already initialized");
            
            if (!error)
            {
                // TODO: deal with the fact that there're multiple versions of this dll
                d3dcompilerModule = LoadLibraryW(Concat(L, D3D_COMPILER_DLL));
                if (!d3dcompilerModule)
                    ErrorSet(error, "Failed to load " D3D_COMPILER_DLL);
            }
            
            if (!error)
            {
                HMODULE scopeModule = d3dcompilerModule;
                D3DCOMPILER_FUNCS(X)
            }
        }
#undef X
    }
    
    {
        // -- Create d3d11Device --
        if (!error)
        {
            HRESULT result = w32D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_DEBUG,
                                                  0, 0, D3D11_SDK_VERSION, &d3d11Device, 0, &d3d11Ctx);
            if (result != S_OK)
                ErrorSet(error, "Failed to create device");
        }
        
        // -- Create d3d11Dbg --
        if (!error)
        {
            HRESULT result = ID3D11Device_QueryInterface(d3d11Device, iid_ID3D11Debug, &d3d11Dbg);
            if (result != S_OK)
                ErrorSet(error, "Failed to query debug");
        }
    }
    
	// Clean up "non-temps"
    if (error)
    {
        // Clear modules
		if (dxgiModule)
			FreeLibrary(dxgiModule);
		if (d3d11Module)
			FreeLibrary(d3d11Module);
		if (d3dcompilerModule)
			FreeLibrary(d3dcompilerModule);
        
		// clear function pointers
#define X(r, n, p) w32##n = 0;
        DXGI_FUNCS(X);
        D3D11_FUNCS(X);
        D3DCOMPILER_FUNCS(X);
#undef X
        
        // Clear globals
        d3d11Device = 0;
        d3d11Ctx = 0;
        d3d11Dbg = 0;
		
        dxgiModule = 0;
        d3d11Module = 0;
        d3dcompilerModule = 0;
    }
    
    return !error;
}

function void W32CloseD3D11Window(GFXWindow window)
{
    W32D3D11Window* slot = w32D3D11Slots + window - 1;
    if (slot->swapchain)
        IDXGISwapChain_Release(slot->swapchain);
    ID3D11DeviceContext_ClearState(d3d11Ctx);
    ID3D11DeviceContext_Flush(d3d11Ctx);
    ZeroStruct(slot);
}

function b32 EquipD3D11Window(GFXWindow window)
{
    b32 error = 0;
    
    if (!GFXWindowIsValid(window))
        ErrorSet(error, "Handle isn't valid");
    
    if (!error)
    {
        W32Window* slot = W32WindowFromGFXHandle(window);
        
        IDXGISwapChain* swapchain = 0;
        
        if (!error)
        {
			// Create factory
			IDXGIFactory* factory = 0;
			HRESULT factoryResult = w32CreateDXGIFactory(iid_IDXGIFactory, &factory);
			if (factoryResult != S_OK)
				ErrorSet(error, "Failed to create factory");
			
			if (!error)
			{
				// Create swapChain
				DXGI_SWAP_CHAIN_DESC swapChainDesc = {
					.BufferDesc.Width = 1024,
					.BufferDesc.Height = 768,
					.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
					.SampleDesc.Count = 1,
					.SampleDesc.Quality = 0,
					.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
					.BufferCount = 1,
					.OutputWindow = slot->wnd,
					.Windowed = true,
				};
				
				IDXGIFactory_CreateSwapChain(factory, (IUnknown*)d3d11Device, &swapChainDesc, &swapchain);
				if (!swapchain)
					ErrorSet(error, "Failed to create swapchain");
				
				IDXGIFactory_Release(factory);
			}
        }
        
		if (!error)
		{
			W32D3D11Window* equipped = w32D3D11Slots + window - 1;
			equipped->swapchain = swapchain;
			GFXWindowEquipData(window, equipped, W32CloseD3D11Window);
		}
    }
    
    return !error;
}

function b32 FreeD3D11()
{
    b32 error = 0;
    
    // Cleanup windows
    for (i32 i = 0; i < ArrayCount(w32D3D11Slots); ++i)
        if (w32D3D11Slots[i].swapchain)
            IDXGISwapChain_Release(w32D3D11Slots[i].swapchain);
    
    // Cleanup D3D11
    {
        if (d3d11Device) ID3D11Device_Release(d3d11Device);
        else ErrorSet(error, "The graphics device has already been released");
        
        if (d3d11Ctx) ID3D11DeviceContext_Release(d3d11Ctx);
        else ErrorSet(error, "The graphics context has already been released");
        
        if (d3d11Dbg) ID3D11Debug_Release(d3d11Dbg);
        else ErrorSet(error, "The grahpics debugger has already been released");
    }
    
    // Cleanup modules
    {
        if (!dxgiModule)
            ErrorSet(error, "dxgi.dll has already been freed");
        else if (!FreeLibrary(dxgiModule))
            ErrorSet(error, "Failed to free dxgi.dll");
        
        if (!d3d11Module)
            ErrorSet(error, "d3d11.dll has already been freed");
        else if (!FreeLibrary(d3d11Module))
            ErrorSet(error, "Failed to free d3d11.dll");
        
        if (!d3dcompilerModule)
            ErrorSet(error, D3D_COMPILER_DLL " has already been freed");
        else if (!FreeLibrary(d3dcompilerModule))
            ErrorSet(error, "Failed to free " D3D_COMPILER_DLL);
    }
    
    // Cleanup function pointers
    {
#define X(r, n, p) w32##n = 0;
        DXGI_FUNCS(X);
        D3D11_FUNCS(X);
        D3DCOMPILER_FUNCS(X);
#undef X
    }
    
    // Cleanup globals
    {
        dxgiSwapChain = 0;
        d3d11Device = 0;
        d3d11Ctx = 0;
        d3d11Dbg = 0;
        
        dxgiModule = 0;
        d3d11Module = 0;
        d3dcompilerModule = 0;
    }
    
    return !error;
}

function ID3D11DeviceContext* GetD3D11DeviceCtx(void)
{
    return d3d11Ctx;
}

function ID3D11RenderTargetView* BeginD3D11Render(GFXWindow window)
{
	ID3D11RenderTargetView* result = 0;
	
	if (GFXWindowIsValid(window) && dxgiSwapChain == 0)
	{
		W32D3D11Window* slot = w32D3D11Slots + window - 1;
		ID3D11Texture2D* buffer = 0;
		DWORD bufferResult = IDXGISwapChain_GetBuffer(slot->swapchain, 0, iid_ID3D11Texture2D, &buffer);
		if (bufferResult == S_OK)
		{
			ID3D11RenderTargetView* view = 0;
			DWORD viewResult = ID3D11Device_CreateRenderTargetView(d3d11Device, (ID3D11Resource*)buffer, 0, &view);
            if (viewResult == S_OK)
			{
				result = view;
				dxgiSwapChain = slot->swapchain;
			}
		}
		
		if (buffer != 0)
			ID3D11Texture2D_Release(buffer);
	}
	
	return result;
}

function void EndD3D11Render(ID3D11RenderTargetView* view)
{
	if (dxgiSwapChain)
	{
		IDXGISwapChain_Present(dxgiSwapChain, 0, 0);
		ID3D11RenderTargetView_Release(view);
		dxgiSwapChain = 0;
	}
}
