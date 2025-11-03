/* date = October 26th 2025 2:43 am */

#ifndef _LONG_O_G_L_H
#define _LONG_O_G_L_H

//~ long: GL Types

typedef u8 GLubyte;
typedef char GLchar;
typedef unsigned char GLboolean;
typedef i32 GLint;
typedef i32 GLsizei;
typedef u32 GLbitfield;
typedef u32 GLenum;
typedef u32 GLuint;
typedef f32 GLfloat;
typedef iptr GLintptr;
typedef iptr GLsizeiptr;

//~ long: GL Constants

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

#define GL_VERSION    0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_NEAREST    0x2600
#define GL_LINEAR     0x2601

#define GL_TEXTURE_INTERNAL_FORMAT 0x1003
#define GL_TEXTURE_MAG_FILTER      0x2800
#define GL_TEXTURE_MIN_FILTER      0x2801
#define GL_TEXTURE_WRAP_S          0x2802
#define GL_TEXTURE_WRAP_T          0x2803

#define GL_REPEAT 0x2901

#define GL_FUNC_ADD              0x8006
#define GL_FUNC_REVERSE_SUBTRACT 0x800B
#define GL_FUNC_SUBTRACT         0x800A
#define GL_MIN                   0x8007
#define GL_MAX                   0x8008

#define GL_RGB8   0x8051
#define GL_RGBA8  0x8058
#define GL_RGBA16 0x805B

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

#define GL_MAX_TEXTURE_IMAGE_UNITS  0x8872
#define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF

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

//~ long: GL Functions

#define GL_FUNCS(X) \
    X(GLenum, GetError, (void)) \
    X(const GLubyte*, GetString, (GLenum name)) \
    X(void, Viewport, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X(void, Scissor, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X(void, ClearColor, (GLfloat r, GLfloat g, GLfloat b, GLfloat a)) \
    X(void, Clear, (GLbitfield mask)) \
    X(void, BlendFunc, (GLenum sfactor, GLenum dfactor)) \
    X(void, Disable, (GLenum cap)) \
    X(void, Enable, (GLenum cap)) \
    X(void, PixelStorei, (GLenum pname, GLint param)) \
    X(void, ReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)) \
    X(void, Color3f,  (GLfloat red, GLfloat green, GLfloat blue)) \
    X(void, Vertex3f, (GLfloat x, GLfloat y, GLfloat z)) \
    X(void, Flush, (void)) \
    X(void, GetFloatv, (GLenum pname, GLfloat* data)) \
    X(void, GetIntegerv, (GLenum pname, GLint* data)) \
    X(void, GetTexLevelParameteriv, (GLenum target, GLint level, GLenum pname, GLint * params)) \
    X(void, TexParameteri, (GLenum target, GLenum pname, GLint param)) \
    X(void, TexImage1D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, \
                         GLenum format, GLenum type, const void* pixels)) \
    X(void, TexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, \
                         GLint border, GLenum format, GLenum type, const void* pixels))

#define FUNCTION_VALUE(X) GL_FUNCS(X)
#define FUNCTION_PREFIX OGL_
#define POINTER_PREFIX gl
#include "XFunction.h"

#define GL_EXT_FUNCS(X) \
    /* blending */ \
    X(void, BlendFuncSeparate, (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
    X(void, BlendEquation, (GLenum mode)) \
    X(void, BlendEquationSeparate, (GLenum modeRGB, GLenum modeAlpha)) \
    /* buffers */ \
    X(void, GenBuffers, (GLsizei n, GLuint* buffers)) \
    X(void, DeleteBuffers, (GLsizei n, const GLuint* buffers)) \
    X(void, BindBuffer, (GLenum target, GLuint buffer)) \
    X(void, BufferData, (GLenum target, GLsizeiptr size, const void* data, GLenum usage)) \
    X(void, BufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const void* data)) \
    X(void, GenVertexArrays, (GLsizei n, GLuint* arrays)) \
    X(void, DeleteVertexArrays, (GLsizei n, const GLuint* arrays)) \
    X(void, BindVertexArray, (GLuint array)) \
    X(void, ActiveTexture, (GLenum texture)) \
    /* textures */ \
    X(void, GenTextures, (GLsizei n, GLuint* textures)) \
    X(void, DeleteTextures, (GLsizei n, const GLuint* textures)) \
    X(void, BindTexture, (GLenum target, GLuint texture)) \
    X(GLboolean, IsTexture, (GLuint texture)) \
    X(void, TexSubImage1D, (GLenum target, GLint level, GLint xoffset, GLsizei width, \
                            GLenum format, GLenum type, const void* pixels)) \
    X(void, TexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, \
                            GLenum format, GLenum type, const void* pixels)) \
    X(void, TexImage2DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, \
                                    GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    /* shaders */ \
    X(void, AttachShader, (GLuint program, GLuint shader)) \
    X(void, CompileShader, (GLuint shader)) \
    X(GLuint, CreateProgram, (void)) \
    X(GLuint, CreateShader, (GLenum type)) \
    X(void, DeleteProgram, (GLuint program)) \
    X(void, DeleteShader, (GLuint shader)) \
    X(void, GetProgramiv, (GLuint program, GLenum pname, GLint* params)) \
    X(void, GetProgramInfoLog, (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog)) \
    X(void, GetShaderiv, (GLuint shader, GLenum pname, GLint* params)) \
    X(void, GetShaderInfoLog, (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog)) \
    X(void, LinkProgram, (GLuint program)) \
    X(void, ShaderSource, (GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)) \
    X(void, UseProgram, (GLuint program)) \
    /* draw calls */ \
    X(void, DrawArrays, (GLenum mode, GLint first, GLsizei count)) \
    X(void, DrawElements, (GLenum mode, GLsizei count, GLenum type, const void* indices)) \
    X(void, DrawArraysInstanced, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount)) \
    X(void, DrawElementsInstanced, (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount)) \
    X(void, EnableVertexAttribArray, (GLuint index)) \
    X(void, DisableVertexAttribArray, (GLuint index)) \
    X(void, VertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, \
                                  GLsizei stride, const void* pointer)) \
    X(void, VertexAttribIPointer, (GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)) \
    X(void, VertexAttribDivisor, (GLuint index, GLuint divisor)) \
    X(GLint, GetUniformLocation, (GLuint program, const GLchar* name)) \
    X(void, Uniform1f, (GLint location, GLfloat v0)) \
    X(void, Uniform2f, (GLint location, GLfloat v0, GLfloat v1)) \
    X(void, Uniform3f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X(void, Uniform4f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X(void, Uniform1i, (GLint location, GLint v0)) \
    X(void, Uniform1fv, (GLint location, GLsizei count, const GLfloat* value)) \
    X(void, Uniform2fv, (GLint location, GLsizei count, const GLfloat* value)) \
    X(void, Uniform3fv, (GLint location, GLsizei count, const GLfloat* value)) \
    X(void, DrawBuffers, (GLsizei n, const GLenum* bufs)) \
    /* framebuffers */ \
    X(void, GenFramebuffers, (GLsizei n, GLuint* framebuffers)) \
    X(void, DeleteFramebuffers, (GLsizei n, const GLuint* framebuffers)) \
    X(void, BindFramebuffer, (GLenum target, GLuint framebuffer)) \
    X(GLboolean, IsFramebuffer, (GLuint framebuffer)) \
    X(GLenum, CheckFramebufferStatus, (GLenum target)) \
    X(void, FramebufferTexture1D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X(void, FramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X(void, FramebufferTexture3D, (GLenum target, GLenum attachment, GLenum textarget, \
                                   GLuint texture, GLint level, GLint zoffset)) \
    X(void, FramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X(void, FramebufferTexture, (GLenum target, GLenum attachment, GLuint texture, GLint level)) \
    X(void, BlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, \
                              GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X(void, GetFramebufferAttachmentParameteriv, (GLenum target, GLenum attachment, GLenum pname, GLint* params)) \

#define FUNCTION_VALUE(X) GL_EXT_FUNCS(X)
#define FUNCTION_PREFIX OGL_
#define POINTER_PREFIX gl
#include "XFunction.h"

//~ long: WGL Functions

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

//~ long: WGL Constants

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

//~ long: OGL System Functions

function b32 OGL_Init(void);
function b32 OGL_Free(void);
function b32 OGL_WindowEquip(GFXWindow window);

function void OGL_Begin(GFXWindow window);
function void OGL_End(void);

//~ long: OGL Helper Types

typedef GLuint OGL_Handle;

typedef struct OGL_Shader OGL_Shader;
struct OGL_Shader
{
    OGL_Handle handle;
    String log;
};

//~ long: OGL Helper Functions

function OGL_Shader OGL_MakeShader(Arena* arena, char* src, GLenum type);
function OGL_Shader OGL_MakeProgram(Arena* arena, OGL_Shader* shaders, u64 count);

function OGL_Handle OGL_TextureCreate (GLint textureFmt, u32 w, u32 h, void* data);
function void       OGL_TextureUpdate (OGL_Handle texture, r2i32 rect, void* data);
function void       OGL_TextureDestroy(OGL_Handle texture);
function b32        OGL_TextureValid  (OGL_Handle texture);

#endif //_LONG_O_G_L_H
