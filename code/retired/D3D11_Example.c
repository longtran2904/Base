
#include "Base.h"
#include "LongOS.h"
#include "Base.c"
#include "LongOS_Win32.c"

typedef u64 GFXWindow;
#include "LongGFX_D3D11.h"

// d3d11.h is for all the ID3D11... types and functions. It has already included dxgi.h and d3dcommon.h
// dxgi.h is for all the IDXGI... types and functions
// d3dcommon.h is for blob (ReadFileToBlob, ID3D10Blob, etc) and enums like: D3D_DRIVER_TYPE, D3D_FEATURE_LEVEL

// D3Dcompiler.h is for D3DCompile and these flags:
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

// DirectXColors.h: Standard colors
#define AliceBlue             (f32[4]){ 0.941176534f, 0.972549081f, 1.000000000f, 1.000000000f }
#define AntiqueWhite          (f32[4]){ 0.980392218f, 0.921568692f, 0.843137324f, 1.000000000f }
#define Aqua                  (f32[4]){ 0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f }
#define Aquamarine            (f32[4]){ 0.498039246f, 1.000000000f, 0.831372619f, 1.000000000f }
#define Azure                 (f32[4]){ 0.941176534f, 1.000000000f, 1.000000000f, 1.000000000f }
#define Beige                 (f32[4]){ 0.960784376f, 0.960784376f, 0.862745166f, 1.000000000f }
#define Bisque                (f32[4]){ 1.000000000f, 0.894117713f, 0.768627524f, 1.000000000f }
#define Black                 (f32[4]){ 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f }
#define BlanchedAlmond        (f32[4]){ 1.000000000f, 0.921568692f, 0.803921640f, 1.000000000f }
#define Blue                  (f32[4]){ 0.000000000f, 0.000000000f, 1.000000000f, 1.000000000f }
#define BlueViolet            (f32[4]){ 0.541176498f, 0.168627456f, 0.886274576f, 1.000000000f }
#define Brown                 (f32[4]){ 0.647058845f, 0.164705887f, 0.164705887f, 1.000000000f }
#define BurlyWood             (f32[4]){ 0.870588303f, 0.721568644f, 0.529411793f, 1.000000000f }
#define CadetBlue             (f32[4]){ 0.372549027f, 0.619607866f, 0.627451003f, 1.000000000f }
#define Chartreuse            (f32[4]){ 0.498039246f, 1.000000000f, 0.000000000f, 1.000000000f }
#define Chocolate             (f32[4]){ 0.823529482f, 0.411764741f, 0.117647067f, 1.000000000f }
#define Coral                 (f32[4]){ 1.000000000f, 0.498039246f, 0.313725501f, 1.000000000f }
#define CornflowerBlue        (f32[4]){ 0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f }
#define Cornsilk              (f32[4]){ 1.000000000f, 0.972549081f, 0.862745166f, 1.000000000f }
#define Crimson               (f32[4]){ 0.862745166f, 0.078431375f, 0.235294133f, 1.000000000f }
#define Cyan                  (f32[4]){ 0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f }
#define DarkBlue              (f32[4]){ 0.000000000f, 0.000000000f, 0.545098066f, 1.000000000f }
#define DarkCyan              (f32[4]){ 0.000000000f, 0.545098066f, 0.545098066f, 1.000000000f }
#define DarkGoldenrod         (f32[4]){ 0.721568644f, 0.525490224f, 0.043137256f, 1.000000000f }
#define DarkGray              (f32[4]){ 0.662745118f, 0.662745118f, 0.662745118f, 1.000000000f }
#define DarkGreen             (f32[4]){ 0.000000000f, 0.392156899f, 0.000000000f, 1.000000000f }
#define DarkKhaki             (f32[4]){ 0.741176486f, 0.717647076f, 0.419607878f, 1.000000000f }
#define DarkMagenta           (f32[4]){ 0.545098066f, 0.000000000f, 0.545098066f, 1.000000000f }
#define DarkOliveGreen        (f32[4]){ 0.333333343f, 0.419607878f, 0.184313729f, 1.000000000f }
#define DarkOrange            (f32[4]){ 1.000000000f, 0.549019635f, 0.000000000f, 1.000000000f }
#define DarkOrchid            (f32[4]){ 0.600000024f, 0.196078449f, 0.800000072f, 1.000000000f }
#define DarkRed               (f32[4]){ 0.545098066f, 0.000000000f, 0.000000000f, 1.000000000f }
#define DarkSalmon            (f32[4]){ 0.913725555f, 0.588235319f, 0.478431404f, 1.000000000f }
#define DarkSeaGreen          (f32[4]){ 0.560784340f, 0.737254918f, 0.545098066f, 1.000000000f }
#define DarkSlateBlue         (f32[4]){ 0.282352954f, 0.239215702f, 0.545098066f, 1.000000000f }
#define DarkSlateGray         (f32[4]){ 0.184313729f, 0.309803933f, 0.309803933f, 1.000000000f }
#define DarkTurquoise         (f32[4]){ 0.000000000f, 0.807843208f, 0.819607913f, 1.000000000f }
#define DarkViolet            (f32[4]){ 0.580392182f, 0.000000000f, 0.827451050f, 1.000000000f }
#define DeepPink              (f32[4]){ 1.000000000f, 0.078431375f, 0.576470613f, 1.000000000f }
#define DeepSkyBlue           (f32[4]){ 0.000000000f, 0.749019623f, 1.000000000f, 1.000000000f }
#define DimGray               (f32[4]){ 0.411764741f, 0.411764741f, 0.411764741f, 1.000000000f }
#define DodgerBlue            (f32[4]){ 0.117647067f, 0.564705908f, 1.000000000f, 1.000000000f }
#define Firebrick             (f32[4]){ 0.698039234f, 0.133333340f, 0.133333340f, 1.000000000f }
#define FloralWhite           (f32[4]){ 1.000000000f, 0.980392218f, 0.941176534f, 1.000000000f }
#define ForestGreen           (f32[4]){ 0.133333340f, 0.545098066f, 0.133333340f, 1.000000000f }
#define Fuchsia               (f32[4]){ 1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f }
#define Gainsboro             (f32[4]){ 0.862745166f, 0.862745166f, 0.862745166f, 1.000000000f }
#define GhostWhite            (f32[4]){ 0.972549081f, 0.972549081f, 1.000000000f, 1.000000000f }
#define Gold                  (f32[4]){ 1.000000000f, 0.843137324f, 0.000000000f, 1.000000000f }
#define Goldenrod             (f32[4]){ 0.854902029f, 0.647058845f, 0.125490203f, 1.000000000f }
#define Gray                  (f32[4]){ 0.501960814f, 0.501960814f, 0.501960814f, 1.000000000f }
#define Green                 (f32[4]){ 0.000000000f, 0.501960814f, 0.000000000f, 1.000000000f }
#define GreenYellow           (f32[4]){ 0.678431392f, 1.000000000f, 0.184313729f, 1.000000000f }
#define Honeydew              (f32[4]){ 0.941176534f, 1.000000000f, 0.941176534f, 1.000000000f }
#define HotPink               (f32[4]){ 1.000000000f, 0.411764741f, 0.705882370f, 1.000000000f }
#define IndianRed             (f32[4]){ 0.803921640f, 0.360784322f, 0.360784322f, 1.000000000f }
#define Indigo                (f32[4]){ 0.294117659f, 0.000000000f, 0.509803951f, 1.000000000f }
#define Ivory                 (f32[4]){ 1.000000000f, 1.000000000f, 0.941176534f, 1.000000000f }
#define Khaki                 (f32[4]){ 0.941176534f, 0.901960850f, 0.549019635f, 1.000000000f }
#define Lavender              (f32[4]){ 0.901960850f, 0.901960850f, 0.980392218f, 1.000000000f }
#define LavenderBlush         (f32[4]){ 1.000000000f, 0.941176534f, 0.960784376f, 1.000000000f }
#define LawnGreen             (f32[4]){ 0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f }
#define LemonChiffon          (f32[4]){ 1.000000000f, 0.980392218f, 0.803921640f, 1.000000000f }
#define LightBlue             (f32[4]){ 0.678431392f, 0.847058892f, 0.901960850f, 1.000000000f }
#define LightCoral            (f32[4]){ 0.941176534f, 0.501960814f, 0.501960814f, 1.000000000f }
#define LightCyan             (f32[4]){ 0.878431439f, 1.000000000f, 1.000000000f, 1.000000000f }
#define LightGoldenrodYellow  (f32[4]){ 0.980392218f, 0.980392218f, 0.823529482f, 1.000000000f }
#define LightGreen            (f32[4]){ 0.564705908f, 0.933333397f, 0.564705908f, 1.000000000f }
#define LightGray             (f32[4]){ 0.827451050f, 0.827451050f, 0.827451050f, 1.000000000f }
#define LightPink             (f32[4]){ 1.000000000f, 0.713725507f, 0.756862819f, 1.000000000f }
#define LightSalmon           (f32[4]){ 1.000000000f, 0.627451003f, 0.478431404f, 1.000000000f }
#define LightSeaGreen         (f32[4]){ 0.125490203f, 0.698039234f, 0.666666687f, 1.000000000f }
#define LightSkyBlue          (f32[4]){ 0.529411793f, 0.807843208f, 0.980392218f, 1.000000000f }
#define LightSlateGray        (f32[4]){ 0.466666698f, 0.533333361f, 0.600000024f, 1.000000000f }
#define LightSteelBlue        (f32[4]){ 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f }
#define LightYellow           (f32[4]){ 1.000000000f, 1.000000000f, 0.878431439f, 1.000000000f }
#define Lime                  (f32[4]){ 0.000000000f, 1.000000000f, 0.000000000f, 1.000000000f }
#define LimeGreen             (f32[4]){ 0.196078449f, 0.803921640f, 0.196078449f, 1.000000000f }
#define Linen                 (f32[4]){ 0.980392218f, 0.941176534f, 0.901960850f, 1.000000000f }
#define Magenta               (f32[4]){ 1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f }
#define Maroon                (f32[4]){ 0.501960814f, 0.000000000f, 0.000000000f, 1.000000000f }
#define MediumAquamarine      (f32[4]){ 0.400000036f, 0.803921640f, 0.666666687f, 1.000000000f }
#define MediumBlue            (f32[4]){ 0.000000000f, 0.000000000f, 0.803921640f, 1.000000000f }
#define MediumOrchid          (f32[4]){ 0.729411781f, 0.333333343f, 0.827451050f, 1.000000000f }
#define MediumPurple          (f32[4]){ 0.576470613f, 0.439215720f, 0.858823597f, 1.000000000f }
#define MediumSeaGreen        (f32[4]){ 0.235294133f, 0.701960802f, 0.443137288f, 1.000000000f }
#define MediumSlateBlue       (f32[4]){ 0.482352972f, 0.407843173f, 0.933333397f, 1.000000000f }
#define MediumSpringGreen     (f32[4]){ 0.000000000f, 0.980392218f, 0.603921592f, 1.000000000f }
#define MediumTurquoise       (f32[4]){ 0.282352954f, 0.819607913f, 0.800000072f, 1.000000000f }
#define MediumVioletRed       (f32[4]){ 0.780392230f, 0.082352944f, 0.521568656f, 1.000000000f }
#define MidnightBlue          (f32[4]){ 0.098039225f, 0.098039225f, 0.439215720f, 1.000000000f }
#define MintCream             (f32[4]){ 0.960784376f, 1.000000000f, 0.980392218f, 1.000000000f }
#define MistyRose             (f32[4]){ 1.000000000f, 0.894117713f, 0.882353008f, 1.000000000f }
#define Moccasin              (f32[4]){ 1.000000000f, 0.894117713f, 0.709803939f, 1.000000000f }
#define NavajoWhite           (f32[4]){ 1.000000000f, 0.870588303f, 0.678431392f, 1.000000000f }
#define Navy                  (f32[4]){ 0.000000000f, 0.000000000f, 0.501960814f, 1.000000000f }
#define OldLace               (f32[4]){ 0.992156923f, 0.960784376f, 0.901960850f, 1.000000000f }
#define Olive                 (f32[4]){ 0.501960814f, 0.501960814f, 0.000000000f, 1.000000000f }
#define OliveDrab             (f32[4]){ 0.419607878f, 0.556862772f, 0.137254909f, 1.000000000f }
#define Orange                (f32[4]){ 1.000000000f, 0.647058845f, 0.000000000f, 1.000000000f }
#define OrangeRed             (f32[4]){ 1.000000000f, 0.270588249f, 0.000000000f, 1.000000000f }
#define Orchid                (f32[4]){ 0.854902029f, 0.439215720f, 0.839215755f, 1.000000000f }
#define PaleGoldenrod         (f32[4]){ 0.933333397f, 0.909803987f, 0.666666687f, 1.000000000f }
#define PaleGreen             (f32[4]){ 0.596078455f, 0.984313786f, 0.596078455f, 1.000000000f }
#define PaleTurquoise         (f32[4]){ 0.686274529f, 0.933333397f, 0.933333397f, 1.000000000f }
#define PaleVioletRed         (f32[4]){ 0.858823597f, 0.439215720f, 0.576470613f, 1.000000000f }
#define PapayaWhip            (f32[4]){ 1.000000000f, 0.937254965f, 0.835294187f, 1.000000000f }
#define PeachPuff             (f32[4]){ 1.000000000f, 0.854902029f, 0.725490212f, 1.000000000f }
#define Peru                  (f32[4]){ 0.803921640f, 0.521568656f, 0.247058839f, 1.000000000f }
#define Pink                  (f32[4]){ 1.000000000f, 0.752941251f, 0.796078503f, 1.000000000f }
#define Plum                  (f32[4]){ 0.866666734f, 0.627451003f, 0.866666734f, 1.000000000f }
#define PowderBlue            (f32[4]){ 0.690196097f, 0.878431439f, 0.901960850f, 1.000000000f }
#define Purple                (f32[4]){ 0.501960814f, 0.000000000f, 0.501960814f, 1.000000000f }
#define Red                   (f32[4]){ 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f }
#define RosyBrown             (f32[4]){ 0.737254918f, 0.560784340f, 0.560784340f, 1.000000000f }
#define RoyalBlue             (f32[4]){ 0.254901975f, 0.411764741f, 0.882353008f, 1.000000000f }
#define SaddleBrown           (f32[4]){ 0.545098066f, 0.270588249f, 0.074509807f, 1.000000000f }
#define Salmon                (f32[4]){ 0.980392218f, 0.501960814f, 0.447058856f, 1.000000000f }
#define SandyBrown            (f32[4]){ 0.956862807f, 0.643137276f, 0.376470625f, 1.000000000f }
#define SeaGreen              (f32[4]){ 0.180392161f, 0.545098066f, 0.341176480f, 1.000000000f }
#define SeaShell              (f32[4]){ 1.000000000f, 0.960784376f, 0.933333397f, 1.000000000f }
#define Sienna                (f32[4]){ 0.627451003f, 0.321568638f, 0.176470593f, 1.000000000f }
#define Silver                (f32[4]){ 0.752941251f, 0.752941251f, 0.752941251f, 1.000000000f }
#define SkyBlue               (f32[4]){ 0.529411793f, 0.807843208f, 0.921568692f, 1.000000000f }
#define SlateBlue             (f32[4]){ 0.415686309f, 0.352941185f, 0.803921640f, 1.000000000f }
#define SlateGray             (f32[4]){ 0.439215720f, 0.501960814f, 0.564705908f, 1.000000000f }
#define Snow                  (f32[4]){ 1.000000000f, 0.980392218f, 0.980392218f, 1.000000000f }
#define SpringGreen           (f32[4]){ 0.000000000f, 1.000000000f, 0.498039246f, 1.000000000f }
#define SteelBlue             (f32[4]){ 0.274509817f, 0.509803951f, 0.705882370f, 1.000000000f }
#define Tan                   (f32[4]){ 0.823529482f, 0.705882370f, 0.549019635f, 1.000000000f }
#define Teal                  (f32[4]){ 0.000000000f, 0.501960814f, 0.501960814f, 1.000000000f }
#define Thistle               (f32[4]){ 0.847058892f, 0.749019623f, 0.847058892f, 1.000000000f }
#define Tomato                (f32[4]){ 1.000000000f, 0.388235331f, 0.278431386f, 1.000000000f }
#define Transparent           (f32[4]){ 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f }
#define Turquoise             (f32[4]){ 0.250980407f, 0.878431439f, 0.815686345f, 1.000000000f }
#define Violet                (f32[4]){ 0.933333397f, 0.509803951f, 0.933333397f, 1.000000000f }
#define Wheat                 (f32[4]){ 0.960784376f, 0.870588303f, 0.701960802f, 1.000000000f }
#define White                 (f32[4]){ 1.000000000f, 1.000000000f, 1.000000000f, 1.000000000f }
#define WhiteSmoke            (f32[4]){ 0.960784376f, 0.960784376f, 0.960784376f, 1.000000000f }
#define Yellow                (f32[4]){ 1.000000000f, 1.000000000f, 0.000000000f, 1.000000000f }
#define YellowGreen           (f32[4]){ 0.603921592f, 0.803921640f, 0.196078449f, 1.000000000f }

#define KEEP_PREFIX

#define DXGI_FUNCS(X) \
    X(HRESULT, CreateDXGIFactory, (REFIID, void**))
#define FUNCTION_VALUE(X) DXGI_FUNCS(X)
#define FUNCTION_PREFIX W32
#define POINTER_PREFIX w32
#include "XFunction.h"

#define D3D11_FUNCS(X) \
    X(HRESULT, D3D11CreateDevice, (IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, D3D_FEATURE_LEVEL*, \
                                   UINT, UINT, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**)) \
    X(HRESULT, D3D11CreateDeviceAndSwapChain, (IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, D3D_FEATURE_LEVEL*, \
                                               UINT, UINT, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**, \
                                               ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**))
#define FUNCTION_VALUE(X) D3D11_FUNCS(X)
#include "XFunction.h"

#define D3DCOMPILER_FUNCS(X) \
    X(HRESULT, D3DReadFileToBlob, (LPCWSTR, ID3DBlob**)) \
    X(HRESULT, D3DCompile, (LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*, \
                            LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**))
#define FUNCTION_VALUE(X) D3DCOMPILER_FUNCS(X)
#include "XFunction.h"

#undef KEEP_PREFIX
#undef FUNCTION_PREFIX
#undef POINTER_PREFIX

#if 0
typedef HRESULT W32CreateDXGIFactoryFunc(REFIID, void**);
typedef HRESULT W32D3D11CreateDeviceFunc(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, D3D_FEATURE_LEVEL*,
                                         UINT, UINT, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
typedef HRESULT W32D3D11CreateDeviceAndSwapChainFunc(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, D3D_FEATURE_LEVEL*,
                                                     UINT, UINT, DXGI_SWAP_CHAIN_DESC*, IDXGISwapChain**,
                                                     ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
typedef HRESULT W32D3DReadFileToBlobFunc(LPCWSTR, ID3DBlob**);
typedef HRESULT W32D3DCompileFunc(LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*, ID3DInclude*,
                                  LPCSTR, LPCSTR, UINT, UINT, ID3DBlob**, ID3DBlob**);

global W32CreateDXGIFactoryFunc* w32CreateDXGIFactory = 0;
global W32D3D11CreateDeviceFunc* w32D3D11CreateDevice = 0;
global W32D3D11CreateDeviceAndSwapChainFunc* w32D3D11CreateDeviceAndSwapChain = 0;
global W32D3DReadFileToBlobFunc* w32D3DReadFileToBlob = 0;
global W32D3DCompileFunc* w32D3DCompile = 0;
#endif

#if 0
global IID* iid_IDXGIFactory;
global IID* iid_ID3D11Texture2D;
global IID* iid_ID3D11Debug;
#else
#define iid_IDXGIFactory (&IID_IDXGIFactory)
#define iid_ID3D11Texture2D (&IID_ID3D11Texture2D)
#define iid_ID3D11Debug (&IID_ID3D11Debug)
#endif

//~ NOTE(long): Win32 OpenGL Globals

global String win32GraphicsError = {0};
//global HMODULE win32D3D11Module = {0};

typedef struct W32D3D11Window W32D3D11Window;
struct W32D3D11Window
{
    union
    {
        W32D3D11Window* next;
        struct
        {
            HWND wnd;
            IDXGISwapChain* swapchain;
            ID3D11Texture2D* buffer;
            ID3D11RenderTargetView* view;
        };
    };
};

global W32D3D11Window  win32WindowSlots[64];
global W32D3D11Window* win32WindowFree = 0;

#define  GET_PROC_ADDR(f, m, n) (*(PROC*)(&(f))) = GetProcAddress((m), (n))
#define GRAPHICS_WINDOW_CLASS_NAME "LongD3D11Graphics"

#define W32HandleFromSlot(s) (GFXWindow)(((s) - win32WindowSlots) + 1)
#define W32SlotFromHandle(h) (win32WindowSlots + ((h) - 1))
#define W32IsHandleValid(h) ((1 <= (h)) && \
                             ((h) <= ArrayCount(win32WindowSlots)))

ID3D11Device* device = 0;
ID3D11DeviceContext* context = 0;
ID3D11Debug* dbg = 0;

IDXGIFactory* factory = 0;

global char vshaderText[] =
"float4 main(float2 pos : POSITION) : SV_POSITION\n"
"{\n"
"return float4(pos, 0, 1);\n"
"}\n";

global char pshaderText[] =
"float4 main() : SV_TARGET\n"
"{\n"
"return float4(1.0f, 1.0f, 1.0f, 1.0f);\n"
"}\n";

global f32 vertices[] = {
    +0.0f,  0.5f,
    -0.5f, -0.5f,
    +0.5f, -0.5f
};

global u32 indices[] = { 0, 1, 2 };

LRESULT W32GraphicsWindowProc(HWND   hwnd,
                              UINT   msg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    LRESULT result = 0;
    switch (msg)
    {
        case WM_CLOSE:
        {
            ExitProcess(0);
        } break;
        
        default:
        {
            result = DefWindowProc(hwnd, msg, wParam, lParam);
        } break;
    }
    return result;
}

function b32 InitD3D11(void)
{
    String error = {0};
    
    // Setup window slots
    {
        ZeroFixedArr(win32WindowSlots);
        win32WindowFree = win32WindowSlots;
        W32D3D11Window* last = win32WindowSlots + ArrayCount(win32WindowSlots) - 1;
        for (W32D3D11Window* slot = win32WindowSlots; slot < last; ++slot)
            slot->next = (slot + 1);
    }
    
    // Load modules and functions
    {
#define X(r, n, p) if (error.size == 0) \
    { \
        GET_PROC_ADDR(w32##n, scopeModule, Stringify(n)); \
        if (!w32##n) error = StrLit("Error: Failed to load "Stringify(n)"\n"); \
    }
        
        // -- dxgi.dll --
        {
            HMODULE dxgiModule = LoadLibraryA("dxgi.dll");
            if (!dxgiModule)
                error = StrLit("Error: Failed to load dxgi.dll\n");
            
            if (error.size == 0)
            {
                HMODULE scopeModule = dxgiModule;
                DXGI_FUNCS(X)
            }
        }
        
        // -- d3d11.dll --
        if (error.size == 0)
        {
            HMODULE d3d11Module = LoadLibraryA("d3d11.dll");
            if (!d3d11Module)
                error = StrLit("Error: Failed to load d3d11.dll\n");
            
            if (error.size == 0)
            {
                HMODULE scopeModule = d3d11Module;
                D3D11_FUNCS(X)
            }
        }
        
        // -- d3dcompiler_47.dll --
        if (error.size == 0)
        {
            // TODO: deal with the fact that there're multiple versions of this dll
            HMODULE d3dcompilerModule = LoadLibraryA("d3dcompiler_47.dll");
            if (!d3dcompilerModule)
                StrLit("Error: Failed to load d3dcompiler_47.dll\n");
            
            if (error.size == 0)
            {
                HMODULE scopeModule = d3dcompilerModule;
                D3DCOMPILER_FUNCS(X)
            }
        }
#undef X
    }
    
    {
        // -- Create device --
        if (error.size == 0)
        {
            HRESULT result = w32D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, D3D11_CREATE_DEVICE_DEBUG,
                                                  0, 0, D3D11_SDK_VERSION, &device, 0, &context);
            if (result != S_OK)
                error = StrLit("Error: Failed to create device\n");
        }
        
        // -- Create factory --
        if (error.size == 0)
        {
            HRESULT result = w32CreateDXGIFactory(iid_IDXGIFactory, &factory);
            if (result != S_OK)
                error = StrLit("Error: Failed to create DXGI factory\n");
        }
        
        // -- Create dbg --
        if (error.size == 0)
        {
            HRESULT result = ID3D11Device_QueryInterface(device, iid_ID3D11Debug, &dbg);
            if (result != S_OK)
                error = StrLit("Error: Failed to query dbg\n");
        }
    }
    
    // register window class
    {
        WNDCLASS wndClass = {
            .style = CS_HREDRAW|CS_VREDRAW,
            .lpfnWndProc = W32GraphicsWindowProc,
            .hInstance = W32GetInstance(),
            .lpszClassName = GRAPHICS_WINDOW_CLASS_NAME,
        };
        
        ATOM atom = RegisterClass(&wndClass);
        if (!atom)
        {
            MessageBox(0, "Error: Failed registering class\n", "error", MB_OK);
            ExitProcess(1);
        }
    }
    
    return error.size == 0;
}

function GFXWindow CreateD3D11Window(void)
{
    String error = {0};
    
    W32D3D11Window* slotPtr = win32WindowFree;
    if (slotPtr == 0)
        error = StrLit("Reached max window count!");
    
    // create d3d11 window
    HWND d3d11Wnd = 0;
    if (error.size == 0)
        d3d11Wnd = CreateWindow(GRAPHICS_WINDOW_CLASS_NAME, "TITLE ME",
                                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                0, 0, W32GetInstance(), 0);
    if (!d3d11Wnd)
        error = StrLit("Error: Failed to create graphics window\n");
    
    IDXGISwapChain* swapchain = 0;
    ID3D11RenderTargetView* view = 0;
    ID3D11Texture2D* backBuffer = 0;
    
    if (error.size == 0)
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
            .OutputWindow = d3d11Wnd,
            .Windowed = true,
        };
        IDXGIFactory_CreateSwapChain(factory, (IUnknown*)device, &swapChainDesc, &swapchain);
        if (!swapchain)
            error = StrLit("Error: Failed to create swapchain\n");
    }
    
    // Create backBuffer
    if (error.size == 0)
    {
        DWORD bufferResult = IDXGISwapChain_GetBuffer(swapchain, 0, iid_ID3D11Texture2D, &backBuffer);
        if (bufferResult != S_OK)
            error = StrLit("Error: Can't create backBuffer\n");
    }
    
    // Create view
    if (error.size == 0)
    {
        DWORD viewResult = ID3D11Device_CreateRenderTargetView(device, (ID3D11Resource*)backBuffer, 0, &view);
        if (viewResult != S_OK)
            error = StrLit("Error: Can't create view\n");
    }
    
    // Viewport
    if (error.size == 0)
    {
        D3D11_VIEWPORT viewport = {
            .Width = (f32)1024,
            .Height = (f32)768,
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f,
        };
        ID3D11DeviceContext_RSSetViewports(context, 1, &viewport);
    }
    
    GFXWindow result = 0;
    if (error.str == 0)
    {
        SLLStackPop(win32WindowFree);
        slotPtr->wnd = d3d11Wnd;
        slotPtr->swapchain = swapchain;
        slotPtr->buffer = backBuffer;
        slotPtr->view = view;
        result = W32HandleFromSlot(slotPtr);
    }
    else
        win32GraphicsError = error;
    return result;
}

function void ShowD3D11Window(GFXWindow window)
{
    if (W32IsHandleValid(window))
    {
        W32D3D11Window* slot = W32SlotFromHandle(window);
        ShowWindow(slot->wnd, SW_SHOW);
    }
}

function ID3D11DeviceContext* GetD3D11DeviceCtx(void)
{
    return context;
}

function ID3D11RenderTargetView* GetD3D11ViewFromWindow(GFXWindow window)
{
    if (W32IsHandleValid(window))
        return W32SlotFromHandle(window)->view;
    return 0;
}

function IDXGISwapChain* GetDXGISwapChainFromWindow(GFXWindow window)
{
    if (W32IsHandleValid(window))
        return W32SlotFromHandle(window)->swapchain;
    return 0;
}

function String GetD3D11Error(void)
{
    return win32GraphicsError;
}

int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd)
{
    W32WinMainInit(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    ScratchBegin(scratch);
    
    if (!InitD3D11())
    {
        MessageBox(0, GetD3D11Error().str, "error", MB_OK);
        ExitProcess(1);
    }
    
    GFXWindow d3d11Wnd = CreateD3D11Window();
    if (!d3d11Wnd)
    {
        MessageBox(0, GetD3D11Error().str, "error", MB_OK);
        ExitProcess(1);
    }
    
    ShowD3D11Window(d3d11Wnd);
    
    ID3D11DeviceContext* ctx = GetD3D11DeviceCtx();
    ID3D11RenderTargetView* view = GetD3D11ViewFromWindow(d3d11Wnd);
    IDXGISwapChain* swapchain = GetDXGISwapChainFromWindow(d3d11Wnd);
    
#if 0
    // Vertex Shaders
    if (error.size == 0)
    {
        ID3DBlob* vshaderBlob = 0;
        ID3DBlob* vshaderErrors = 0;
        
        UINT vblobFlags = D3DCOMPILE_ENABLE_STRICTNESS|D3DCOMPILE_DEBUG;
        DWORD vblobResult = w32D3DCompile(vshaderText, sizeof(vshaderText) - 1, 0, 0, 0, "main", "vs_5_0",
                                          vblobFlags, 0, &vshaderBlob, &vshaderErrors);
        if (vblobResult != S_OK)
        {
            MessageBox(0, "Error: Can't compile vertex shader\n", "error", MB_OK);
            if (vshaderErrors)
            {
                char* msg = ID3D10Blob_GetBufferPointer(vshaderErrors);
                error = PushStrf(scratch, false, msg);
            }
        }
        
        if (error.size == 0)
        {
            D3D11_INPUT_ELEMENT_DESC layout[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            
            DWORD vshaderResult = ID3D11Device_CreateVertexShader(device, ID3D10Blob_GetBufferPointer(vshaderBlob),
                                                                  ID3D10Blob_GetBufferSize(vshaderBlob), 0, &vshader);
            if (vshaderResult != S_OK)
                error = StrLit("Error: Can't compile vertext shader\n");
            
            if (error.size == 0)
            {
                DWORD layoutResult = ID3D11Device_CreateInputLayout(device, layout, ArrayCount(layout),
                                                                    ID3D10Blob_GetBufferPointer(vshaderBlob),
                                                                    ID3D10Blob_GetBufferSize(vshaderBlob), &inLayout);
                if (layoutResult != S_OK)
                    error = StrLit("Error: Can't create input layout\n");
            }
        }
        
        if (vshaderBlob)
            ID3D10Blob_Release(vshaderBlob);
        if (vshaderErrors)
            ID3D10Blob_Release(vshaderErrors);
    }
    
    // Pixel Shaders
    if (error.size == 0)
    {
        ID3DBlob* pshaderBlob = 0;
        ID3DBlob* pshaderErrors = 0;
        DWORD pblobResult = w32D3DCompile(pshaderText, sizeof(pshaderText) - 1, 0, 0, 0, "main", "ps_5_0",
                                          D3DCOMPILE_ENABLE_STRICTNESS|D3DCOMPILE_DEBUG, 0, &pshaderBlob, &pshaderErrors);
        if (pblobResult != S_OK)
            error = PushStrf(scratch, false, "Error: Can't compile pixel shader\n%s\n",
                             pshaderErrors ? ID3D10Blob_GetBufferPointer(pshaderErrors) : 0);
        
        if (error.size == 0)
        {
            DWORD pshaderResult = ID3D11Device_CreatePixelShader(device, ID3D10Blob_GetBufferPointer(pshaderBlob),
                                                                 ID3D10Blob_GetBufferSize(pshaderBlob), 0, &pshader);
            if (pshaderResult != S_OK)
                error = StrLit("Error: Can't create pixel shader\n");
        }
        
        if (pshaderBlob)
            ID3D10Blob_Release(pshaderBlob);
        if (pshaderErrors)
            ID3D10Blob_Release(pshaderErrors);
    }
    
    // Mesh
    if (error.size == 0)
    {
        D3D11_BUFFER_DESC vbufferDesc = {
            .Usage = D3D11_USAGE_DEFAULT,
            .ByteWidth = sizeof(f32) * ArrayCount(vertices),
            .BindFlags = D3D11_BIND_VERTEX_BUFFER,
        };
        D3D11_SUBRESOURCE_DATA vbufferInitData = { .pSysMem = vertices };
        DWORD vbufferResult = ID3D11Device_CreateBuffer(device, &vbufferDesc, &vbufferInitData, &vbuffer);
        if (vbufferResult != S_OK)
            error = StrLit("Error: Can't create vertex buffer\n");
        
        D3D11_BUFFER_DESC ibufferDesc = {
            .Usage = D3D11_USAGE_DEFAULT,
            .ByteWidth = sizeof(f32) * ArrayCount(vertices),
            .BindFlags = D3D11_BIND_VERTEX_BUFFER,
        };
        D3D11_SUBRESOURCE_DATA ibufferInitData = { .pSysMem = indices };
        DWORD ibufferResult = ID3D11Device_CreateBuffer(device, &ibufferDesc, &ibufferInitData, &ibuffer);
        if (ibufferResult != S_OK)
            error = StrLit("Error: Can't create pixel buffer\n");
    }
#endif
    
    for (;;)
    {
        MSG msg = {0};
        BOOL getMessage = GetMessage(&msg, 0, 0, 0);
        if (getMessage <= 0)
            break;
        DispatchMessage(&msg);
        
        {
            ID3D11DeviceContext_ClearRenderTargetView(ctx, view, MidnightBlue);
            ID3D11DeviceContext_OMSetRenderTargets(ctx, 1, &view, 0);
            
#if 0
            UINT stride = sizeof(float);
            UINT offset = 0;
            ID3D11DeviceContext_IASetVertexBuffers(ctx, 0, 1, &vbuffer, &stride, &offset);
            ID3D11DeviceContext_IASetIndexBuffer(ctx, ibuffer, DXGI_FORMAT_R32_UINT, 0);
            ID3D11DeviceContext_IASetInputLayout(ctx, inLayout);
            ID3D11DeviceContext_IASetPrimitiveTopology(ctx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            
            ID3D11DeviceContext_VSSetShader(ctx, vshader, 0, 0);
            ID3D11DeviceContext_PSSetShader(ctx, pshader, 0, 0);
            ID3D11DeviceContext_DrawIndexed(ctx, ArrayCount(indices), 0, 0);
#endif
            
            IDXGISwapChain_Present(swapchain, 0, 0);
        }
    }
    
    {
        ID3D11Device_Release(device);
        ID3D11DeviceContext_Release(ctx);
        ID3D11Debug_Release(dbg);
        
        IDXGISwapChain_Release(swapchain);
        IDXGIFactory_Release(factory);
        
        ID3D11RenderTargetView_Release(view);
        
#if 0
        ID3D11VertexShader_Release(vshader);
        ID3D11InputLayout_Release(inLayout);
        ID3D11PixelShader_Release(pshader);
        
        ID3D11Buffer_Release(vbuffer);
        ID3D11Buffer_Release(ibuffer);
#endif
    }
    
    ScratchEnd(scratch);
    return 0;
}