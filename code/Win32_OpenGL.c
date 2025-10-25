
//~ NOTE(long): GL Types

typedef char GL_Char;
typedef unsigned char GL_Bool;
typedef i32 GL_Int;
typedef i32 GL_Size;
typedef u32 GL_Bitfield;
typedef u32 GL_Enum;
typedef u32 GL_Uint;
typedef f32 GL_Float;
typedef iptr GL_Iptr;
typedef iptr GL_SizePtr;

//~ NOTE(long): GL Constants

#define GL_POINTS         0x0000
#define GL_LINES          0x0001
#define GL_LINE_LOOP      0x0002
#define GL_LINE_STRIP     0x0003
#define GL_TRIANGLES      0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN   0x0006

#define GL_ZERO                0
#define GL_ONE                 1
#define GL_SRC_COLOR           0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA           0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA           0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR           0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE  0x0308

#define GL_NO_ERROR          0
#define GL_INVALID_ENUM      0x0500
#define GL_INVALID_VALUE     0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_OUT_OF_MEMORY     0x0505

#define GL_VIEWPORT     0x0BA2
#define GL_BLEND        0x0BE2
#define GL_SCISSOR_BOX  0x0C10
#define GL_SCISSOR_TEST 0x0C11

#define GL_UNPACK_SWAP_BYTES  0x0CF0
#define GL_UNPACK_LSB_FIRST   0x0CF1
#define GL_UNPACK_ROW_LENGTH  0x0CF2
#define GL_UNPACK_SKIP_ROWS   0x0CF3
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_UNPACK_ALIGNMENT   0x0CF5
#define GL_PACK_SWAP_BYTES    0x0D00
#define GL_PACK_LSB_FIRST     0x0D01
#define GL_PACK_ROW_LENGTH    0x0D02
#define GL_PACK_SKIP_ROWS     0x0D03
#define GL_PACK_SKIP_PIXELS   0x0D04
#define GL_PACK_ALIGNMENT     0x0D05

#define GL_TEXTURE_1D 0x0DE0
#define GL_TEXTURE_2D 0x0DE1

#define GL_BYTE           0x1400
#define GL_UNSIGNED_BYTE  0x1401
#define GL_SHORT          0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT            0x1404
#define GL_UNSIGNED_INT   0x1405
#define GL_FLOAT          0x1406

#define GL_DEPTH_COMPONENT 0x1902
#define GL_RED   0x1903
#define GL_RGB   0x1907
#define GL_RGBA  0x1908

#define GL_NEAREST 0x2600
#define GL_LINEAR  0x2601

#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S     0x2802
#define GL_TEXTURE_WRAP_T     0x2803

#define GL_REPEAT 0x2901

#define GL_FUNC_ADD              0x8006
#define GL_FUNC_REVERSE_SUBTRACT 0x800B
#define GL_FUNC_SUBTRACT         0x800A
#define GL_MIN                   0x8007
#define GL_MAX                   0x8008

#define GL_RGB8  0x8051
#define GL_RGBA8 0x8058

#define GL_MULTISAMPLE 0x809D

#define GL_BGR  0x80E0
#define GL_BGRA 0x80E1

#define GL_CLAMP_TO_EDGE   0x812F

#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE       0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE     0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE      0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE     0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE     0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE   0x8217
#define GL_FRAMEBUFFER_DEFAULT   0x8218
#define GL_FRAMEBUFFER_UNDEFINED 0x8219

#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A

#define GL_RG         0x8227
#define GL_RG_INTEGER 0x8228
#define GL_R8         0x8229
#define GL_R16        0x822A
#define GL_RG8        0x822B
#define GL_RG16       0x822C
#define GL_R16F       0x822D
#define GL_R32F       0x822E
#define GL_RG16F      0x822F
#define GL_RG32F      0x8230
#define GL_R8I        0x8231
#define GL_R8UI       0x8232
#define GL_R16I       0x8233
#define GL_R16UI      0x8234
#define GL_R32I       0x8235
#define GL_R32UI      0x8236
#define GL_RG8I       0x8237
#define GL_RG8UI      0x8238
#define GL_RG16I      0x8239
#define GL_RG16UI     0x823A
#define GL_RG32I      0x823B
#define GL_RG32UI     0x823C

#define GL_MIRRORED_REPEAT 0x8370

#define GL_TEXTURE0  0x84C0
#define GL_TEXTURE1  0x84C1
#define GL_TEXTURE2  0x84C2
#define GL_TEXTURE3  0x84C3
#define GL_TEXTURE4  0x84C4
#define GL_TEXTURE5  0x84C5
#define GL_TEXTURE6  0x84C6
#define GL_TEXTURE7  0x84C7
#define GL_TEXTURE8  0x84C8
#define GL_TEXTURE9  0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF

#define GL_DEPTH_STENCIL 0x84F9

#define GL_RGBA32F 0x8814
#define GL_RGB32F  0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F  0x881B

#define GL_ARRAY_BUFFER         0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893

#define GL_STREAM_DRAW  0x88E0
#define GL_STREAM_READ  0x88E1
#define GL_STREAM_COPY  0x88E2
#define GL_STATIC_DRAW  0x88E4
#define GL_STATIC_READ  0x88E5
#define GL_STATIC_COPY  0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA

#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER   0x8B31

#define GL_DELETE_STATUS   0x8B80
#define GL_COMPILE_STATUS  0x8B81
#define GL_LINK_STATUS     0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_INFO_LOG_LENGTH 0x8B84

#define GL_SRGB         0x8C40
#define GL_SRGB8        0x8C41
#define GL_SRGB_ALPHA   0x8C42
#define GL_SRGB8_ALPHA8 0x8C43

#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9

#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1

#define GL_FRAMEBUFFER_COMPLETE               0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT  0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED            0x8CDD

#define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#define GL_COLOR_ATTACHMENT0     0x8CE0
#define GL_COLOR_ATTACHMENT1     0x8CE1
#define GL_COLOR_ATTACHMENT2     0x8CE2
#define GL_COLOR_ATTACHMENT3     0x8CE3
#define GL_COLOR_ATTACHMENT4     0x8CE4
#define GL_COLOR_ATTACHMENT5     0x8CE5
#define GL_COLOR_ATTACHMENT6     0x8CE6
#define GL_COLOR_ATTACHMENT7     0x8CE7
#define GL_COLOR_ATTACHMENT8     0x8CE8
#define GL_COLOR_ATTACHMENT9     0x8CE9
#define GL_COLOR_ATTACHMENT10    0x8CEA
#define GL_COLOR_ATTACHMENT11    0x8CEB
#define GL_COLOR_ATTACHMENT12    0x8CEC
#define GL_COLOR_ATTACHMENT13    0x8CED
#define GL_COLOR_ATTACHMENT14    0x8CEE
#define GL_COLOR_ATTACHMENT15    0x8CEF
#define GL_COLOR_ATTACHMENT16    0x8CF0
#define GL_COLOR_ATTACHMENT17    0x8CF1
#define GL_COLOR_ATTACHMENT18    0x8CF2
#define GL_COLOR_ATTACHMENT19    0x8CF3
#define GL_COLOR_ATTACHMENT20    0x8CF4
#define GL_COLOR_ATTACHMENT21    0x8CF5
#define GL_COLOR_ATTACHMENT22    0x8CF6
#define GL_COLOR_ATTACHMENT23    0x8CF7
#define GL_COLOR_ATTACHMENT24    0x8CF8
#define GL_COLOR_ATTACHMENT25    0x8CF9
#define GL_COLOR_ATTACHMENT26    0x8CFA
#define GL_COLOR_ATTACHMENT27    0x8CFB
#define GL_COLOR_ATTACHMENT28    0x8CFC
#define GL_COLOR_ATTACHMENT29    0x8CFD
#define GL_COLOR_ATTACHMENT30    0x8CFE
#define GL_COLOR_ATTACHMENT31    0x8CFF
#define GL_DEPTH_ATTACHMENT      0x8D00
#define GL_STENCIL_ATTACHMENT    0x8D20

#define GL_FRAMEBUFFER 0x8D40

#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56

#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8

#define GL_FRAMEBUFFER_SRGB 0x8DB9

#define GL_TEXTURE_2D_MULTISAMPLE 0x9100

#define GL_DEPTH_BUFFER_BIT   0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT   0x00004000

//~ NOTE(long): GL Functions

#define GL_FUNCS(X) \
    X(GL_Enum, glGetError, (void)) \
    X(void, glViewport, (GL_Int x, GL_Int y, GL_Size width, GL_Size height)) \
    X(void, glScissor, (GL_Int x, GL_Int y, GL_Size width, GL_Size height)) \
    X(void, glClearColor, (GL_Float r, GL_Float g, GL_Float b, GL_Float a)) \
    X(void, glClear, (GL_Bitfield mask)) \
    X(void, glBlendFunc, (GL_Enum sfactor, GL_Enum dfactor)) \
    X(void, glDisable, (GL_Enum cap)) \
    X(void, glEnable, (GL_Enum cap)) \
    X(void, glPixelStorei, (GL_Enum pname, GL_Int param)) \
    X(void, glReadPixels, (GL_Int x, GL_Int y, GL_Size width, GL_Size height, GL_Enum format, GL_Enum type, void* pixels)) \
    X(void, glColor3f,  (GL_Float red, GL_Float green, GL_Float blue)) \
    X(void, glVertex3f, (GL_Float x, GL_Float y, GL_Float z)) \
    X(void, glFlush, (void)) \
    X(void, glGetFloatv, (GL_Enum pname, GL_Float* params)) \
    X(void, glTexParameteri, (GL_Enum target, GL_Enum pname, GL_Int param)) \
    X(void, glTexImage2D, (GL_Enum target, GL_Int level, GL_Int internalformat, GL_Size width, GL_Size height, \
                           GL_Int border, GL_Enum format, GL_Enum type, const void* pixels)) \
    X(void, glTexImage1D, (GL_Enum target, GL_Int level, GL_Int internalformat, GL_Size width, GL_Int border, \
                           GL_Enum format, GL_Enum type, const void* pixels)) \

#define FUNCTION_VALUE(X) GL_FUNCS(X)
#define FUNCTION_PREFIX GL
#include "XFunction.h"

#define GL_EXT_FUNCS(X) \
    /* blending */ \
    X(void, glBlendFuncSeparate, (GL_Enum srcRGB, GL_Enum dstRGB, GL_Enum srcAlpha, GL_Enum dstAlpha)) \
    X(void, glBlendEquation, (GL_Enum mode)) \
    X(void, glBlendEquationSeparate, (GL_Enum modeRGB, GL_Enum modeAlpha)) \
    /* buffers */ \
    X(void, glGenBuffers, (GL_Size n, GL_Uint* buffers)) \
    X(void, glDeleteBuffers, (GL_Size n, const GL_Uint* buffers)) \
    X(void, glBindBuffer, (GL_Enum target, GL_Uint buffer)) \
    X(void, glBufferData, (GL_Enum target, GL_SizePtr size, const void* data, GL_Enum usage)) \
    X(void, glBufferSubData, (GL_Enum target, GL_Iptr offset, GL_SizePtr size, const void* data)) \
    X(void, glGenVertexArrays, (GL_Size n, GL_Uint* arrays)) \
    X(void, glDeleteVertexArrays, (GL_Size n, const GL_Uint* arrays)) \
    X(void, glBindVertexArray, (GL_Uint array)) \
    X(void, glActiveTexture, (GL_Enum texture)) \
    /* textures */ \
    X(void, glGenTextures, (GL_Size n, GL_Uint* textures)) \
    X(void, glDeleteTextures, (GL_Size n, const GL_Uint* textures)) \
    X(void, glBindTexture, (GL_Enum target, GL_Uint texture)) \
    X(GL_Bool, glIsTexture, (GL_Uint texture)) \
    X(void, glTexSubImage1D, (GL_Enum target, GL_Int level, GL_Int xoffset, GL_Size width, GL_Enum format, GL_Enum type, const void* pixels)) \
    X(void, glTexSubImage2D, (GL_Enum target, GL_Int level, GL_Int xoffset, GL_Int yoffset, GL_Size width, GL_Size height, GL_Enum format, GL_Enum type, const void* pixels)) \
    X(void, glTexImage2DMultisample, (GL_Enum target, GL_Size samples, GL_Enum internalformat, GL_Size width, GL_Size height, GL_Bool fixedsamplelocations)) \
    /* shaders */ \
    X(void, glAttachShader, (GL_Uint program, GL_Uint shader)) \
    X(void, glCompileShader, (GL_Uint shader)) \
    X(GL_Uint, glCreateProgram, (void)) \
    X(GL_Uint, glCreateShader, (GL_Enum type)) \
    X(void, glDeleteProgram, (GL_Uint program)) \
    X(void, glDeleteShader, (GL_Uint shader)) \
    X(void, glGetProgramiv, (GL_Uint program, GL_Enum pname, GL_Int* params)) \
    X(void, glGetProgramInfoLog, (GL_Uint program, GL_Size bufSize, GL_Size* length, GL_Char* infoLog)) \
    X(void, glGetShaderiv, (GL_Uint shader, GL_Enum pname, GL_Int* params)) \
    X(void, glGetShaderInfoLog, (GL_Uint shader, GL_Size bufSize, GL_Size* length, GL_Char* infoLog)) \
    X(void, glLinkProgram, (GL_Uint program)) \
    X(void, glShaderSource, (GL_Uint shader, GL_Size count, const GL_Char* const* string, const GL_Int* length)) \
    X(void, glUseProgram, (GL_Uint program)) \
    /* draw calls */ \
    X(void, glDrawArrays, (GL_Enum mode, GL_Int first, GL_Size count)) \
    X(void, glDrawElements, (GL_Enum mode, GL_Size count, GL_Enum type, const void* indices)) \
    X(void, glDrawArraysInstanced, (GL_Enum mode, GL_Int first, GL_Size count, GL_Size instancecount)) \
    X(void, glDrawElementsInstanced, (GL_Enum mode, GL_Size count, GL_Enum type, const void* indices, GL_Size instancecount)) \
    X(void, glEnableVertexAttribArray, (GL_Uint index)) \
    X(void, glDisableVertexAttribArray, (GL_Uint index)) \
    X(void, glVertexAttribPointer, (GL_Uint index, GL_Int size, GL_Enum type, GL_Bool normalized, GL_Size stride, const void* pointer)) \
    X(void, glVertexAttribIPointer, (GL_Uint index, GL_Int size, GL_Enum type, GL_Size stride, const void* pointer)) \
    X(void, glVertexAttribDivisor, (GL_Uint index, GL_Uint divisor)) \
    X(GL_Int, glGetUniformLocation, (GL_Uint program, const GL_Char* name)) \
    X(void, glUniform1f, (GL_Int location, GL_Float v0)) \
    X(void, glUniform2f, (GL_Int location, GL_Float v0, GL_Float v1)) \
    X(void, glUniform3f, (GL_Int location, GL_Float v0, GL_Float v1, GL_Float v2)) \
    X(void, glUniform4f, (GL_Int location, GL_Float v0, GL_Float v1, GL_Float v2, GL_Float v3)) \
    X(void, glUniform1i, (GL_Int location, GL_Int v0)) \
    X(void, glUniform1fv, (GL_Int location, GL_Size count, const GL_Float* value)) \
    X(void, glUniform2fv, (GL_Int location, GL_Size count, const GL_Float* value)) \
    X(void, glUniform3fv, (GL_Int location, GL_Size count, const GL_Float* value)) \
    X(void, glDrawBuffers, (GL_Size n, const GL_Enum* bufs)) \
    /* framebuffers */ \
    X(void, glGenFramebuffers, (GL_Size n, GL_Uint* framebuffers)) \
    X(void, glDeleteFramebuffers, (GL_Size n, const GL_Uint* framebuffers)) \
    X(void, glBindFramebuffer, (GL_Enum target, GL_Uint framebuffer)) \
    X(GL_Bool, glIsFramebuffer, (GL_Uint framebuffer)) \
    X(GL_Enum, glCheckFramebufferStatus, (GL_Enum target)) \
    X(void, glFramebufferTexture1D, (GL_Enum target, GL_Enum attachment, GL_Enum textarget, GL_Uint texture, GL_Int level)) \
    X(void, glFramebufferTexture2D, (GL_Enum target, GL_Enum attachment, GL_Enum textarget, GL_Uint texture, GL_Int level)) \
    X(void, glFramebufferTexture3D, (GL_Enum target, GL_Enum attachment, GL_Enum textarget, GL_Uint texture, GL_Int level, GL_Int zoffset)) \
    X(void, glFramebufferRenderbuffer, (GL_Enum target, GL_Enum attachment, GL_Enum renderbuffertarget, GL_Uint renderbuffer)) \
    X(void, glFramebufferTexture, (GL_Enum target, GL_Enum attachment, GL_Uint texture, GL_Int level)) \
    X(void, glBlitFramebuffer, (GL_Int srcX0, GL_Int srcY0, GL_Int srcX1, GL_Int srcY1, GL_Int dstX0, GL_Int dstY0, GL_Int dstX1, GL_Int dstY1, GL_Bitfield mask, GL_Enum filter)) \
    X(void, glGetFramebufferAttachmentParameteriv, (GL_Enum target, GL_Enum attachment, GL_Enum pname, GL_Int* params)) \

#define FUNCTION_VALUE(X) GL_EXT_FUNCS(X)
#define FUNCTION_PREFIX GL
#include "XFunction.h"

//~ NOTE(long): WGL Functions

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

//~ NOTE(long): WGL Constants
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
#define BOOTSTRAP_WINDOW_CLASS_NAME "LongOpenGLBootstrap"

//~ NOTE(long): OpenGL Helpers

typedef struct GL_Shader GL_Shader;
struct GL_Shader
{
    GL_Uint handle;
    String log;
};

function GL_Shader GL_MakeShader(Arena* arena, char* src, GL_Enum type)
{
    GL_Uint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, 0);
    glCompileShader(shader);
    
    GL_Int logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    char* buffer = PushArray(arena, char, logLength + 1);
    GL_Int length = 0;
    glGetShaderInfoLog(shader, logLength + 1, &length, buffer);
    
    // Handle Error
    GL_Int status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    
    if (status == 0)
    {
        glDeleteShader(shader);
        shader = 0;
    }
    
    return (GL_Shader){ shader, Str(buffer, length) };
}

function GL_Shader GL_MakeProgram(Arena* arena, GL_Shader* shaders, u64 count)
{
    GL_Uint program = glCreateProgram();
    for (int i = 0; i < count; ++i)
        glAttachShader(program, shaders[i].handle);
    glLinkProgram(program);
    
    GL_Int logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    char* buffer = PushArray(arena, char, logLength + 1);
    GL_Int length = 0;
    glGetProgramInfoLog(program, logLength + 1, &length, buffer);
    
    // Handle Error
    GL_Int status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    
    if (status == 0)
    {
        glDeleteProgram(program);
        program = 0;
    }
    
    return (GL_Shader){ program, Str(buffer, length) };
}

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
            w32OpenGLModule = LoadLibraryW(L"opengl32.dll");
            if (w32OpenGLModule == 0)
                ErrorSet("Failed to initialize opengl.dll", error);
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
        WGL_GET_PROC_ADDR(w32Wgl##n, "wgl"Stringify(n)); \
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
        W32_GET_PROC_ADDR(n, w32OpenGLModule, Stringify(n)); \
        if (!n) ErrorSet("Failed to load "Stringify(n), error); \
    }
            GL_FUNCS(X);
#undef X
            
#define X(r, n, p) if (!error) \
    { \
        WGL_GET_PROC_ADDR(n, Stringify(n));\
        if (!n) ErrorSet("Faield to load "Stringify(n), error);\
    }
            GL_EXT_FUNCS(X);
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
        //u32 w, h;
        //GFXWindowGetInnerRect(window, 0, 0, &w, &h);
        //glViewport(0, 0, w, h);
        DEBUG(error, GL_Enum error = glGetError());
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
