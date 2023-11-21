
// --------------------------------------------------
// NOTE: D3DCOMPILE Flags (D3DComipler.h)

#define D3DCOMPILE_DEBUG                                (1 << 0)
#define D3DCOMPILE_SKIP_VALIDATION                      (1 << 1)
#define D3DCOMPILE_SKIP_OPTIMIZATION                    (1 << 2)
#define D3DCOMPILE_PACK_MATRIX_ROW_MAJOR                (1 << 3)
#define D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR             (1 << 4)
#define D3DCOMPILE_PARTIAL_PRECISION                    (1 << 5)
#define D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT             (1 << 6)
#define D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT             (1 << 7)
#define D3DCOMPILE_NO_PRESHADER                         (1 << 8)
#define D3DCOMPILE_AVOID_FLOW_CONTROL                   (1 << 9)
#define D3DCOMPILE_PREFER_FLOW_CONTROL                  (1 << 10)
#define D3DCOMPILE_ENABLE_STRICTNESS                    (1 << 11)
#define D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY       (1 << 12)
#define D3DCOMPILE_IEEE_STRICTNESS                      (1 << 13)
#define D3DCOMPILE_OPTIMIZATION_LEVEL0                  (1 << 14)
#define D3DCOMPILE_OPTIMIZATION_LEVEL1                  0
#define D3DCOMPILE_OPTIMIZATION_LEVEL2                  ((1 << 14) | (1 << 15))
#define D3DCOMPILE_OPTIMIZATION_LEVEL3                  (1 << 15)
#define D3DCOMPILE_RESERVED16                           (1 << 16)
#define D3DCOMPILE_RESERVED17                           (1 << 17)
#define D3DCOMPILE_WARNINGS_ARE_ERRORS                  (1 << 18)
#define D3DCOMPILE_RESOURCES_MAY_ALIAS                  (1 << 19)
#define D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES   (1 << 20)
#define D3DCOMPILE_ALL_RESOURCES_BOUND                  (1 << 21)
#define D3DCOMPILE_DEBUG_NAME_FOR_SOURCE                (1 << 22)
#define D3DCOMPILE_DEBUG_NAME_FOR_BINARY                (1 << 23)

// --------------------------------------------------
// NOTE: DXGI Definitions

#define DXGI_FUNCS(X) \
X(HRESULT, CreateDXGIFactory, (REFIID, void**))
#define FUNCTION_VALUE(X) DXGI_FUNCS(X)
#define FUNCTION_PREFIX W32
#define POINTER_PREFIX w32
#define KEEP_PREFIX
#include "XFunction.h"

// --------------------------------------------------
// NOTE: D3D11 Definitions

#define D3D11_FUNCS(X) \
X(HRESULT, D3D11CreateDevice, (IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, D3D_FEATURE_LEVEL*, \
UINT, UINT, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**)) \
X(HRESULT, D3D11CreateDeviceAndSwapChain, (IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, D3D_FEATURE_LEVEL*, \
UINT, UINT, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**, \
ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**))
#define FUNCTION_VALUE(X) D3D11_FUNCS(X)
#include "XFunction.h"

// --------------------------------------------------
// NOTE: D3DCompiler Definitions

#define D3DCOMPILER_FUNCS(X) \
X(HRESULT, D3DReadFileToBlob, (LPCWSTR, ID3DBlob**)) \
X(HRESULT, D3DCompile, (LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, \
LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**))
#define FUNCTION_VALUE(X) D3DCOMPILER_FUNCS(X)
#include "XFunction.h"

#undef KEEP_PREFIX
#undef POINTER_PREFIX
#undef FUNCTION_PREFIX

// --------------------------------------------------
// NOTE: Win32 D3D11 Globals

#define iid_IDXGIFactory (&IID_IDXGIFactory)
#define iid_ID3D11Texture2D (&IID_ID3D11Texture2D)
#define iid_ID3D11Debug (&IID_ID3D11Debug)

typedef struct W32D3D11Window W32D3D11Window;
struct W32D3D11Window
{
    IDXGISwapChain* swapchain;
};

global W32D3D11Window w32D3D11Slots[GFX_MAX_WINDOW_SLOTS] = {0};

ID3D11Device* device = 0;
ID3D11DeviceContext* ctx = 0;
ID3D11Debug* dbg = 0;

HMODULE dxgiModule = 0;
HMODULE d3d11Module = 0;
HMODULE d3dcompilerModule = 0;

function b32 InitD3D11(void)
{
    // Load modules and functions
    {
#define X(r, n, p) if (!HasError()) \
{ \
GET_PROC_ADDR(w32##n, scopeModule, Stringify(n)); \
if (!w32##n) SetConstError("Failed to load "Stringify(n)); \
}
        
        // -- dxgi.dll --
        {
            if (dxgiModule != 0)
                SetConstError("dxgi.dll has already initialized");
            
            if (!HasError())
            {
                dxgiModule = LoadLibraryA("dxgi.dll");
                if (!dxgiModule)
                    SetConstError("Failed to load dxgi.dll");
            }
            
            if (!HasError())
            {
                HMODULE scopeModule = dxgiModule;
                DXGI_FUNCS(X)
            }
        }
        
        // -- d3d11.dll --
        if (!HasError())
        {
            if (d3d11Module != 0)
                SetConstError("d3d11Module.dll has already initialized");
            
            if (!HasError())
            {
                d3d11Module = LoadLibraryA("d3d11.dll");
                if (!d3d11Module)
                    SetConstError("Failed to load d3d11.dll");
            }
            
            if (!HasError())
            {
                HMODULE scopeModule = d3d11Module;
                D3D11_FUNCS(X)
            }
        }
        
        // -- d3dcompiler_47.dll --
        if (!HasError())
        {
            if (d3dcompilerModule != 0)
                SetConstError("d3dcompiler_47.dll has already initialized");
            
            if (!HasError())
            {
                // TODO: deal with the fact that there're multiple versions of this dll
                d3dcompilerModule = LoadLibraryA("d3dcompiler_47.dll");
                if (!d3dcompilerModule)
                    SetConstError("Failed to load d3dcompiler_47.dll");
            }
            
            if (!HasError())
            {
                HMODULE scopeModule = d3dcompilerModule;
                D3DCOMPILER_FUNCS(X)
            }
        }
#undef X
    }
    
    {
        // -- Create device --
        if (!HasError())
        {
            HRESULT result = w32D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_DEBUG,
                                                  0, 0, D3D11_SDK_VERSION, &device, 0, &ctx);
            if (result != S_OK)
                SetConstError("Failed to create device");
        }
        
        // -- Create dbg --
        if (!HasError())
        {
            HRESULT result = ID3D11Device_QueryInterface(device, iid_ID3D11Debug, &dbg);
            if (result != S_OK)
                SetConstError("Failed to query dbg");
        }
    }
    
	// Clean up "non-temps"
    if (HasError())
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
        device = 0;
        ctx = 0;
        dbg = 0;
		
        dxgiModule = 0;
        d3d11Module = 0;
        d3dcompilerModule = 0;
    }
    
    return !HasError();
}

function void W32CloseD3D11Window(GFXWindow window)
{
    W32D3D11Window* slot = w32D3D11Slots + window - 1;
    if (slot->swapchain)
        IDXGISwapChain_Release(slot->swapchain);
    ID3D11DeviceContext_ClearState(ctx);
    ID3D11DeviceContext_Flush(ctx);
    ZeroStruct(slot);
}

function b32 EquipD3D11Window(GFXWindow window)
{
    if (!IsGFXWindowValid(window))
        SetConstError("Handle isn't valid");
    
    if (!HasError())
    {
        W32Window* slot = W32WindowFromGFXHandle(window);
        
        IDXGISwapChain* swapchain = 0;
        ID3D11RenderTargetView* view = 0;
        ID3D11Texture2D* backBuffer = 0;
        
        if (!HasError())
        {
			// Create factory
			IDXGIFactory* factory = 0;
			HRESULT factoryResult = w32CreateDXGIFactory(iid_IDXGIFactory, &factory);
			if (factoryResult != S_OK)
				SetConstError("Failed to create factory");
			
			if (!HasError())
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
				
				IDXGIFactory_CreateSwapChain(factory, (IUnknown*)device, &swapChainDesc, &swapchain);
				if (!swapchain)
					SetConstError("Failed to create swapchain");
				
				IDXGIFactory_Release(factory);
			}
        }
        
		if (!HasError())
		{
			W32D3D11Window* equipped = w32D3D11Slots + window - 1;
			equipped->swapchain = swapchain;
			SetGFXWindowEquippedData(window, equipped, W32CloseD3D11Window);
		}
    }
    
    return !HasError();
}

function ID3D11DeviceContext* GetD3D11DeviceCtx(void)
{
    return ctx;
}

global IDXGISwapChain* w32D3D11SwapChain = 0;

function ID3D11RenderTargetView* BeginD3D11Render(GFXWindow window)
{
	ID3D11RenderTargetView* result = 0;
	
	if (IsGFXWindowValid(window) && w32D3D11SwapChain == 0)
	{
		W32D3D11Window* slot = w32D3D11Slots + window - 1;
		ID3D11Texture2D* buffer = 0;
		DWORD bufferResult = IDXGISwapChain_GetBuffer(slot->swapchain, 0, iid_ID3D11Texture2D, &buffer);
		if (bufferResult == S_OK)
		{
			ID3D11RenderTargetView* view = 0;
			DWORD viewResult = ID3D11Device_CreateRenderTargetView(device, (ID3D11Resource*)buffer, 0, &view);
            if (viewResult == S_OK)
			{
				result = view;
				w32D3D11SwapChain = slot->swapchain;
			}
		}
		
		if (buffer != 0)
			ID3D11Texture2D_Release(buffer);
	}
	
	return result;
}

function void EndD3D11Render(ID3D11RenderTargetView* view)
{
	if (w32D3D11SwapChain)
	{
		IDXGISwapChain_Present(w32D3D11SwapChain, 0, 0);
		ID3D11RenderTargetView_Release(view);
		w32D3D11SwapChain = 0;
	}
}
