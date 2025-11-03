#include "Base.h"
#include "Base.c"

#include "gfx/LongGFX.h"
#include "gfx/LongGFX_Win32.c"

#include "ogl/LongOGL.h"
#include "ogl/LongOGL_Win32.c"

////////////////////////////////
//- Experiment Configuration

// When this is 0 colors in vertex attributes and uniforms
// that are converted (sRGB -> Linear) are converted using
// the approximate (sRGB -> Linear) (f(x)=x^2.2); When this
// is 1 those colors are converted using an accurate
// (sRGB -> Linear) conversion.
//  This has no effect on colors sampled from sRGB textures
// which are not converted by shader code, but by the OpenGL
// implementation itself. By default this is zero to show the
// difference between the approximate and accurate
// conversions in test #3.
#define USE_ACCURATE_CONVERSIONS_IN_SHADER 0

////////////////////////////////
//- Types, Functions & Globals

#define PtrOffsetOf(T,m) PtrFromInt(OffsetOf(T,m))

//- wgl stuff
static f32 graphics_w = 0;
static f32 graphics_h = 0;

static void wgl_helper_begin_render(GFXWindow window)
{
    OGL_Begin(window);
    i32 w, h;
    if (GFXWindowGetInnerRect(window, 0, 0, &w, &h))
    {
        graphics_w = (f32)w;
        graphics_h = (f32)h;
    }
}

static void wgl_helper_end_render(void)
{
    OGL_End();
}

//- opengl render stuff
typedef struct Vertex
{
    f32 px;
    f32 py;
    f32 uvx;
    f32 uvy;
    f32 cr;
    f32 cg;
    f32 cb;
    f32 ca;
    f32 rx0;
    f32 ry0;
    f32 rx1;
    f32 ry1;
} Vertex;


static void opengl_render_init(void);

static void opengl_draw_basic_geometry(Vertex *v, u64 count);
static void opengl_draw_srgb_in_geometry(Vertex *v, u64 count);
static void opengl_draw_rect_geometry(Vertex *v, u64 count);
static void opengl_draw_texture_geometry(Vertex *v, u64 count, u32 texture);
static void opengl_draw_texrgb_geometry(Vertex *v, u64 count, u32 texture,
                                        f32 r, f32 g, f32 b, f32 a);



////////////////////////////////
//- Circle Generator

static f32
integral_indef(f32 r, f32 x){
    Assert(fabsf(x) <= fabsf(r));
    f32 t = asinf(x/r);
    f32 result = r*r*0.5f*(t + 0.5f*sinf(2*t));
    return(result);
}

static f32
integral_def(f32 r, f32 min_x, f32 max_x){
    f32 max_v = integral_indef(r, max_x);
    f32 min_v = integral_indef(r, min_x);
    f32 result = max_v - min_v;
    return(result);
}

static b32
approx_equal(f32 a, f32 b){
    b32 result = (a - 0.0078125f <= b && b <= a + 0.0078125f);
    return(result);
}

static void
sanity_test_integrals(void){
    {
        f32 a = integral_def(1.f, -1.f, 1.f);
        Assert(approx_equal(a, 3.141593f*0.5f));
    }
    {
        f32 a = integral_def(2.f, -2.f, 2.f);
        Assert(approx_equal(a, 3.141593f*2.f));
    }
    {
        f32 a = integral_def(2.f, 0.f, 2.f);
        Assert(approx_equal(a, 3.141593f));
    }
    {
        f32 a0 = integral_def(3.f, 0.f, 1.f);
        f32 a1 = integral_def(3.f, 1.f, 2.f);
        f32 a2 = integral_def(3.f, 2.f, 3.f);
        Assert(approx_equal(a0 + a1 + a2, 3.141593f*9.f/4.f));
    }
}

static f32
intersection_area(f32 cir_r, f32 unit_box_x0, f32 unit_box_y0){
    f32 rr = cir_r*cir_r;
    
    f32 rel_y0 = unit_box_y0;
    f32 rel_y1 = unit_box_y0 + 1.f;
    f32 rel_x0 = unit_box_x0;
    f32 rel_x1 = unit_box_x0 + 1.f;
    
    f32 y0y0 = rel_y0*rel_y0;
    f32 y1y1 = rel_y1*rel_y1;
    
    f32 x0x0 = rel_x0*rel_x0;
    f32 x1x1 = rel_x1*rel_x1;
    
    // check each corner for hitting the circle
    b32 corner_bl = ((y0y0 + x0x0) <= rr);
    b32 corner_tl = ((y1y1 + x0x0) <= rr);
    b32 corner_br = ((y0y0 + x1x1) <= rr);
    b32 corner_tr = ((y1y1 + x1x1) <= rr);
    
    // calculate area
    f32 a = 0.f;
#define COMB(a,b,c,d) ((a)|((b)<<1)|((c)<<2)|((d)<<3))
    switch (COMB(corner_bl, corner_tl, corner_br, corner_tr)){
        // full miss
        case 0x0: a = 0.f; break;
        // full hit
        case 0xF: a = 1.f; break;
        
        // 1 corner hit
        case COMB(1,0,0,0): case COMB(0,1,0,0):
        case COMB(0,0,1,0): case COMB(0,0,0,1):
        {
            f32 intersection_y = 0.f;
            if (corner_bl || corner_br){
                intersection_y = rel_y0;
            }
            else{
                intersection_y = rel_y1;
            }
            
            f32 intersection_x = sqrtf(rr - intersection_y*intersection_y);
            if (intersection_x < rel_x0 || rel_x1 < intersection_x){
                intersection_x = -intersection_x;
            }
            
            f32 x_min = 0.f;
            f32 x_max = 0.f;
            if (corner_bl || corner_tl){
                x_min = rel_x0;
                x_max = intersection_x;
            }
            else{
                x_min = intersection_x;
                x_max = rel_x1;
            }
            
            f32 h = fabsf(intersection_y);
            a = integral_def(cir_r, x_min, x_max) - h*(x_max - x_min);
        }break;
        
        // 2 corner hits (horizontal edge)
        case COMB(1,0,1,0): case COMB(0,1,0,1):
        {
            f32 h = 0.f;
            if (corner_bl){
                h = rel_y0;
            }
            else{
                h = -rel_y1;
            }
            a = integral_def(cir_r, rel_x0, rel_x1) - h;
        }break;
        
        // 2 corner hits (vertical edge)
        case COMB(1,1,0,0): case COMB(0,0,1,1):
        {
            f32 w = 0.f;
            if (corner_bl){
                w = rel_x0;
            }
            else{
                w = -rel_x1;
            }
            a = integral_def(cir_r, rel_y0, rel_y1) - w;
        }break;
        
        // 3 corner hits
        case COMB(0,1,1,1): case COMB(1,0,1,1):
        case COMB(1,1,0,1): case COMB(1,1,1,0):
        {
            f32 intersection_y = 0.f;
            if (!corner_bl || !corner_br){
                intersection_y = rel_y0;
            }
            else{
                intersection_y = rel_y1;
            }
            
            f32 intersection_x = sqrtf(rr - intersection_y*intersection_y);
            if (intersection_x < rel_x0 || rel_x1 < intersection_x){
                intersection_x = -intersection_x;
            }
            
            f32 w_extra = 0.f;
            f32 x_min = 0.f;
            f32 x_max = 0.f;
            if (!corner_bl || !corner_tl){
                x_min = rel_x0;
                x_max = intersection_x;
                w_extra = rel_x1 - intersection_x;
            }
            else{
                x_min = intersection_x;
                x_max = rel_x1;
                w_extra = intersection_x - rel_x0;
            }
            
            f32 h = fabsf(intersection_y) - 1.f;
            a = integral_def(cir_r, x_min, x_max) - h*(x_max - x_min) + w_extra;
        }break;
        
        // these cases should be impossible
        default:
        {
            a = -1.f;
        }break;
    }
#undef COMB
    
    return(a);
}

static void
sanity_test_circle_area(void){
    {
        f32 cr = 5.f;
        f32 cx = 5.f;
        f32 cy = 5.f;
        f32 sum_area = 0.f;
        for (u32 y = 0; y < 10; y += 1){
            f32 box_y0 = (f32)y;
            for (u32 x = 0; x < 10; x += 1){
                f32 box_x0 = (f32)x;
                f32 a = intersection_area(cr, box_x0 - cx, box_y0 - cy);
                Assert(0.f <= a && a <= 1.f);
                sum_area += a;
            }
        }
        f32 formula_area = 3.141593f*cr*cr;
        Assert(approx_equal(sum_area, formula_area));
    }
    
    {
        f32 cr = 8.f;
        f32 cx = 8.1f;
        f32 cy = 8.7f;
        f32 sum_area = 0.f;
        for (u32 y = 0; y < 20; y += 1){
            f32 box_y0 = (f32)y;
            for (u32 x = 0; x < 20; x += 1){
                f32 box_x0 = (f32)x;
                f32 a = intersection_area(cr, box_x0 - cx, box_y0 - cy);
                Assert(0.f <= a && a <= 1.f);
                sum_area += a;
            }
        }
        f32 formula_area = 3.141593f*cr*cr;
        Assert(approx_equal(sum_area, formula_area));
    }
}

static void
render_circle_in_buffer(f32 cir_r, f32 cir_x, f32 cir_y,
                        u8 *buf, u32 buf_side_length){
    for (u32 y = 0; y < buf_side_length; y += 1){
        f32 box_y0 = (f32)y;
        for (u32 x = 0; x < buf_side_length; x += 1){
            f32 box_x0 = (f32)x;
            
            // calculate area of the intersection (circle & box)
            f32 a = intersection_area(cir_r, box_x0 - cir_x, box_y0 - cir_y);
            Assert(0.f <= a && a <= 1.f);
            
            u32 vraw = (u32)(256.f*a);
            u8 v = (vraw <= 255)?((u8)vraw):(u8)255;
            buf[x + buf_side_length*y] = v;
        }
    }
}



#define GL_FUNC_X_LIST() \
    /* common stuff */ \
    X(glGetError, GLenum, (void)) \
    X(glViewport, void, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X(glScissor, void, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X(glClearColor, void, (GLfloat r,GLfloat g,GLfloat b,GLfloat a)) \
    X(glClear, void, (GLbitfield mask)) \
    X(glBlendFunc, void, (GLenum sfactor, GLenum dfactor)) \
    X(glBlendFuncSeparate, void, (GLenum srcRGB,GLenum dstRGB,GLenum srcAlpha,GLenum dstAlpha)) \
    X(glBlendEquation, void, (GLenum mode)) \
    X(glBlendEquationSeparate, void, (GLenum modeRGB, GLenum modeAlpha)) \
    X(glDisable, void, (GLenum cap)) \
    X(glEnable, void, (GLenum cap)) \
    X(glPixelStorei, void, (GLenum pname, GLint param)) \
    X(glReadPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)) \
    /* buffers */ \
    X(glGenBuffers, void, (GLsizei n, GLuint *buffers)) \
    X(glDeleteBuffers, void, (GLsizei n, const GLuint *buffers)) \
    X(glBindBuffer, void, (GLenum target, GLuint buffer)) \
    X(glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage)) \
    X(glBufferSubData, void, (GLenum target, GLintptr offset, GLsizeiptr size, const void *data)) \
    X(glGenVertexArrays, void, (GLsizei n, GLuint *arrays)) \
    X(glDeleteVertexArrays, void, (GLsizei n, const GLuint *arrays)) \
    X(glBindVertexArray, void, (GLuint array)) \
    /* textures */ \
    X(glGenTextures, void, (GLsizei n, GLuint *textures)) \
    X(glActiveTexture, void, (GLenum texture)) \
    X(glDeleteTextures, void, (GLsizei n, const GLuint *textures)) \
    X(glBindTexture, void, (GLenum target, GLuint texture)) \
    X(glIsTexture, GLboolean, (GLuint texture)) \
    X(glTexParameteri, void, (GLenum target, GLenum pname, GLint param)) \
    X(glTexImage1D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X(glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X(glTexSubImage1D, void, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)) \
    X(glTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)) \
    X(glTexImage2DMultisample, void, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    /* shaders */ \
    X(glAttachShader, void, (GLuint program, GLuint shader)) \
    X(glCompileShader, void, (GLuint shader)) \
    X(glCreateProgram, GLuint, (void)) \
    X(glCreateShader, GLuint, (GLenum type)) \
    X(glDeleteProgram, void, (GLuint program)) \
    X(glDeleteShader, void, (GLuint shader)) \
    X(glGetProgramiv, void, (GLuint program, GLenum pname, GLint *params)) \
    X(glGetProgramInfoLog, void, (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X(glGetShaderiv, void, (GLuint shader, GLenum pname, GLint *params)) \
    X(glGetShaderInfoLog, void, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X(glLinkProgram, void, (GLuint program)) \
    X(glShaderSource, void, (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length)) \
    X(glUseProgram, void, (GLuint program)) \
    X(glGetUniformLocation, GLint, (GLuint program, const GLchar *name)) \
    /* draw calls */ \
    X(glDrawArrays, void, (GLenum mode, GLint first, GLsizei count)) \
    X(glDrawElements, void,  (GLenum mode, GLsizei count, GLenum type, const void *indices)) \
    X(glDrawArraysInstanced, void, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount)) \
    X(glDrawElementsInstanced, void, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount)) \
    X(glEnableVertexAttribArray, void, (GLuint index)) \
    X(glDisableVertexAttribArray, void, (GLuint index)) \
    X(glVertexAttribPointer, void, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)) \
    X(glVertexAttribIPointer, void, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X(glVertexAttribDivisor, void, (GLuint index, GLuint divisor)) \
    X(glUniform1f, void, (GLint location, GLfloat v0)) \
    X(glUniform2f, void, (GLint location, GLfloat v0, GLfloat v1)) \
    X(glUniform3f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X(glUniform4f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X(glUniform1i, void, (GLint location, GLint v0)) \
    X(glUniform1fv, void, (GLint location, GLsizei count, const GLfloat *value)) \
    X(glUniform2fv, void, (GLint location, GLsizei count, const GLfloat *value)) \
    X(glUniform3fv, void, (GLint location, GLsizei count, const GLfloat *value)) \
    X(glDrawBuffers, void, (GLsizei n, const GLenum *bufs)) \
    /* framebuffers */ \
    X(glGenFramebuffers, void, (GLsizei n, GLuint *framebuffers)) \
    X(glDeleteFramebuffers, void, (GLsizei n, const GLuint *framebuffers)) \
    X(glBindFramebuffer, void, (GLenum target, GLuint framebuffer)) \
    X(glIsFramebuffer, GLboolean, (GLuint framebuffer)) \
    X(glCheckFramebufferStatus, GLenum, (GLenum target)) \
    X(glFramebufferTexture1D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X(glFramebufferTexture2D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X(glFramebufferTexture3D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)) \
    X(glFramebufferRenderbuffer, void, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X(glFramebufferTexture, void, (GLenum target, GLenum attachment, GLuint texture, GLint level)) \
    X(glBlitFramebuffer, void, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X(glGetFramebufferAttachmentParameteriv, void, (GLenum target, GLenum attachment, GLenum pname, GLint *params))


////////////////////////////////
//- OpenGL Render

static char basic_vshader[] =
"#version 330\n"
"uniform vec2 u_view_xform;\n"
"layout (location = 0) in vec2 v_p;\n"
"layout (location = 1) in vec4 v_c;\n"
"out vec4 f_c;\n"
"void main(){\n"
"vec2 norm_pos = v_p*u_view_xform + vec2(-1.0, -1.0);\n"
"gl_Position = vec4(norm_pos, 0.0, 1.0);\n"
"f_c = v_c;\n"
"}\n"
;

static char basic_fshader[] =
"#version 330\n"
"in vec4 f_c;\n"
"out vec4 out_color;\n"
"void main(){\n"
"out_color = f_c;\n"
"}\n"
;

static GLuint basic_program = 0;
static GLint  basic_u_view_xform = -1;




static char srgb_in_vshader[] =
"#version 330\n"
"\n"

#if USE_ACCURATE_CONVERSIONS_IN_SHADER
"float lin_from_srgb(float x){\n"
"float r = 0;\n"
"if (x <= 0.04045) r = x/12.92;\n"
"else              r = pow(((x + 0.055)/1.055), 2.4);\n"
"return(r);\n"
"}\n"
"\n"
"float srgb_from_lin(float x){\n"
"float r = 0;\n"
"if (x <= 0.0031308) r = x*12.92;\n"
"else                r = pow(x, 1/2.4)*1.055 - 0.055;\n"
"return(r);\n"
"}\n"
#endif

"\n"
"uniform vec2 u_view_xform;\n"
"layout (location = 0) in vec2 v_p;\n"
"layout (location = 1) in vec4 v_c;\n"
"out vec4 f_c;\n"
"void main(){\n"
"vec2 norm_pos = v_p*u_view_xform + vec2(-1.0, -1.0);\n"

#if USE_ACCURATE_CONVERSIONS_IN_SHADER
"float lin_r = lin_from_srgb(v_c.r);\n"
"float lin_g = lin_from_srgb(v_c.g);\n"
"float lin_b = lin_from_srgb(v_c.b);\n"
#else
"float lin_r = pow(v_c.r, 2.2);\n"
"float lin_g = pow(v_c.g, 2.2);\n"
"float lin_b = pow(v_c.b, 2.2);\n"
#endif


"gl_Position = vec4(norm_pos, 0.0, 1.0);\n"
"f_c = vec4(lin_r, lin_g, lin_b, v_c.a);\n"
"}\n"
;

static char srgb_in_fshader[] =
"#version 330\n"
"in vec4 f_c;\n"
"out vec4 out_color;\n"
"void main(){\n"
"out_color = f_c;\n"
"}\n"
;

static GLuint srgb_in_program = 0;
static GLint  srgb_in_u_view_xform = -1;




static char rect_vshader[] =
"#version 330\n"
"uniform vec2 u_view_xform;\n"
"layout (location = 0) in vec2 v_p;\n"
"layout (location = 1) in vec4 v_r;\n"
"out vec4 f_r;\n"
"void main(){\n"
"vec2 norm_pos = v_p*u_view_xform + vec2(-1.0, -1.0);\n"
"gl_Position = vec4(norm_pos, 0.0, 1.0);\n"
"f_r = v_r;\n"
"}\n"
;

static char rect_fshader[] =
"#version 330\n"
"in vec4 gl_FragCoord;\n"
"in vec4 f_r;\n"
"out vec4 out_color;\n"
"void main(){\n"
"float pix_x0 = gl_FragCoord.x - 0.5;\n"
"float pix_y0 = gl_FragCoord.y - 0.5;\n"
"float pix_x1 = gl_FragCoord.x + 0.5;\n"
"float pix_y1 = gl_FragCoord.y + 0.5;\n"
"float int_x0 = max(pix_x0, f_r.x);\n"
"float int_y0 = max(pix_y0, f_r.y);\n"
"float int_x1 = min(pix_x1, f_r.z);\n"
"float int_y1 = min(pix_y1, f_r.w);\n"
"float cover = (int_x1 - int_x0)*(int_y1 - int_y0);\n"
"out_color = vec4(1, 1, 1, cover);\n"
"}\n"
;

static GLuint rect_program = 0;
static GLint  rect_u_view_xform = -1;




static char texture_vshader[] =
"#version 330\n"
"uniform vec2 u_view_xform;\n"
"layout (location = 0) in vec2 v_p;\n"
"layout (location = 1) in vec2 v_uv;\n"
"out vec2 f_uv;\n"
"void main(){\n"
"vec2 norm_pos = v_p*u_view_xform + vec2(-1.0, -1.0);\n"
"gl_Position = vec4(norm_pos, 0.0, 1.0);\n"
"f_uv = v_uv;\n"
"}\n"
;

static char texture_fshader[] =
"#version 330\n"
"uniform sampler2D u_texture;\n"
"in  vec2 f_uv;\n"
"out vec4 out_color;\n"
"void main(){\n"
"out_color = vec4(texture(u_texture, f_uv).rgb, 1);\n"
"}\n"
;

static GLuint texture_program = 0;
static GLint  texture_u_view_xform = -1;
static GLint  texture_u_texture = -1;



static char texrgb_vshader[] =
"#version 330\n"
"uniform vec2 u_view_xform;\n"
"layout (location = 0) in vec2 v_p;\n"
"layout (location = 1) in vec2 v_uv;\n"
"out vec2 f_uv;\n"
"void main(){\n"
"vec2 norm_pos = v_p*u_view_xform + vec2(-1.0, -1.0);\n"
"gl_Position = vec4(norm_pos, 0.0, 1.0);\n"
"f_uv = v_uv;\n"
"}\n"
;

static char texrgb_fshader[] =
"#version 330\n"
"uniform sampler2D u_texture;\n"
"uniform vec4 u_color;\n"
"in  vec2 f_uv;\n"
"out vec4 out_color;\n"
"void main(){\n"
"float texture_a = texture(u_texture, f_uv).r;\n"
"out_color = vec4(u_color.rgb, u_color.a*texture_a);\n"
"}\n"
;

static GLuint texrgb_program = 0;
static GLint  texrgb_u_view_xform = -1;
static GLint  texrgb_u_texture = -1;
static GLint  texrgb_u_color = -1;



static GLuint canvas_texture = 0;
static GLuint canvas_framebuffer = 0;

static GLuint test3_texture = 0;

#define       CIRCLE5_RADIUS 5.f
#define       CIRCLE5_X 6.f
#define       CIRCLE5_Y 6.f
#define       CIRCLE5_SIDE 12
static GLuint circle5_texture = 0;

#define       CIRCLE25_RADIUS 25.f
#define       CIRCLE25_X 30.f
#define       CIRCLE25_Y 30.f
#define       CIRCLE25_SIDE 60
static GLuint circle25_texture = 0;

#define       CIRCLE125_RADIUS 125.f
#define       CIRCLE125_X 150.f
#define       CIRCLE125_Y 150.f
#define       CIRCLE125_SIDE 300
static GLuint circle125_texture = 0;


#define       CALIBRATION_SIDE 120
static GLuint calibration_texture = 0;


static void
opengl_render_init(void){
    ScratchBegin(scratch);
    
    //- setup basic shader program
    {
        OGL_Shader shaders[2] = {0};
        shaders[0] = OGL_MakeShader(scratch, basic_vshader, GL_VERTEX_SHADER);
        shaders[1] = OGL_MakeShader(scratch, basic_fshader, GL_FRAGMENT_SHADER);
        basic_program = OGL_MakeProgram(scratch, shaders, 2).handle;
        basic_u_view_xform = glGetUniformLocation(basic_program, "u_view_xform");
    }
    
    //- setup "srgb in" shader program
    {
        OGL_Shader shaders[2] = {0};
        shaders[0] = OGL_MakeShader(scratch, srgb_in_vshader, GL_VERTEX_SHADER);
        shaders[1] = OGL_MakeShader(scratch, srgb_in_fshader, GL_FRAGMENT_SHADER);
        srgb_in_program = OGL_MakeProgram(scratch, shaders, 2).handle;
        srgb_in_u_view_xform = glGetUniformLocation(basic_program, "u_view_xform");
    }
    
    //- setup rect shader program
    {
        OGL_Shader shaders[2] = {0};
        shaders[0] = OGL_MakeShader(scratch, rect_vshader, GL_VERTEX_SHADER);
        shaders[1] = OGL_MakeShader(scratch, rect_fshader, GL_FRAGMENT_SHADER);
        rect_program = OGL_MakeProgram(scratch, shaders, 2).handle;
        rect_u_view_xform = glGetUniformLocation(rect_program, "u_view_xform");
    }
    
    //- setup texture shader program
    {
        OGL_Shader shaders[2] = {0};
        shaders[0] = OGL_MakeShader(scratch, texture_vshader, GL_VERTEX_SHADER);
        shaders[1] = OGL_MakeShader(scratch, texture_fshader, GL_FRAGMENT_SHADER);
        texture_program = OGL_MakeProgram(scratch, shaders, 2).handle;
        texture_u_view_xform = glGetUniformLocation(texture_program, "u_view_xform");
        texture_u_texture = glGetUniformLocation(texture_program, "u_texture");
    }
    
    //- setup texture rgb shader program
    {
        OGL_Shader shaders[2] = {0};
        shaders[0] = OGL_MakeShader(scratch, texrgb_vshader, GL_VERTEX_SHADER);
        shaders[1] = OGL_MakeShader(scratch, texrgb_fshader, GL_FRAGMENT_SHADER);
        texrgb_program = OGL_MakeProgram(scratch, shaders, 2).handle;
        texrgb_u_view_xform = glGetUniformLocation(texrgb_program, "u_view_xform");
        texrgb_u_texture = glGetUniformLocation(texrgb_program, "u_texture");
        texrgb_u_color = glGetUniformLocation(texrgb_program, "u_color");
    }
    
    //- vertex array object
    {
        GLuint vao = 0;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
    }
    
    //- vertex array buffer
    {
        GLuint vertex_buffer = 0;
        glGenBuffers(1, &vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    }
    
    //- canvas texture
    {
        glGenTextures(1, &canvas_texture);
        glBindTexture(GL_TEXTURE_2D, canvas_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0,
                     GL_RGB, GL_FLOAT, 0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    //- canvas framebuffer
    {
        glGenFramebuffers(1, &canvas_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, canvas_framebuffer);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             canvas_texture, 0);
    }
    
    //- enable alpha blend
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
    }
    
    //- test3 texture
    {
        u8 test3_img[16*3] = {0};
        for (u8 i = 0; i < 16; i += 1){
            u8 i3 = 3*i;
            for (u8 j = 0; j < 3; j += 1){
                test3_img[i3 + j] = 0x08 + 0x10*i;
            }
        }
        
        glGenTextures(1, &test3_texture);
        glBindTexture(GL_TEXTURE_2D, test3_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 16, 1, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, test3_img);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    //- circle5 texture
    {
        u8 buf[CIRCLE5_SIDE*CIRCLE5_SIDE] = {0};
        render_circle_in_buffer(CIRCLE5_RADIUS, CIRCLE5_X, CIRCLE5_Y,
                                buf, CIRCLE5_SIDE);
        
        u8 img[CIRCLE5_SIDE*CIRCLE5_SIDE*3] = {0};
        for (u32 i = 0; i < CIRCLE5_SIDE*CIRCLE5_SIDE; i += 1){
            u8 c = buf[i];
            img[i*3 + 0] = c;
            img[i*3 + 1] = c;
            img[i*3 + 2] = c;
        }
        
        glGenTextures(1, &circle5_texture);
        glBindTexture(GL_TEXTURE_2D, circle5_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CIRCLE5_SIDE, CIRCLE5_SIDE, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, img);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    //- circle25 texture
    {
        u8 buf[CIRCLE25_SIDE*CIRCLE25_SIDE] = {0};
        render_circle_in_buffer(CIRCLE25_RADIUS, CIRCLE25_X, CIRCLE25_Y,
                                buf, CIRCLE25_SIDE);
        
        u8 img[CIRCLE25_SIDE*CIRCLE25_SIDE*3] = {0};
        for (u32 i = 0; i < CIRCLE25_SIDE*CIRCLE25_SIDE; i += 1){
            u8 c = buf[i];
            img[i*3 + 0] = c;
            img[i*3 + 1] = c;
            img[i*3 + 2] = c;
        }
        
        glGenTextures(1, &circle25_texture);
        glBindTexture(GL_TEXTURE_2D, circle25_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CIRCLE25_SIDE, CIRCLE25_SIDE, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, img);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    //- circle125 texture
    {
        u8 buf[CIRCLE125_SIDE*CIRCLE125_SIDE] = {0};
        render_circle_in_buffer(CIRCLE125_RADIUS, CIRCLE125_X, CIRCLE125_Y,
                                buf, CIRCLE125_SIDE);
        
        u8 img[CIRCLE125_SIDE*CIRCLE125_SIDE*3] = {0};
        for (u32 i = 0; i < CIRCLE125_SIDE*CIRCLE125_SIDE; i += 1){
            u8 c = buf[i];
            img[i*3 + 0] = c;
            img[i*3 + 1] = c;
            img[i*3 + 2] = c;
        }
        
        glGenTextures(1, &circle125_texture);
        glBindTexture(GL_TEXTURE_2D, circle125_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CIRCLE125_SIDE, CIRCLE125_SIDE, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, img);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    //- calibration texture
    {
        u8 buf[CALIBRATION_SIDE*CALIBRATION_SIDE] = {0};
        // checkerboards
        for (u32 x = 0; x < (CALIBRATION_SIDE/2); x += 1){
            for (u32 y = 0; y < (CALIBRATION_SIDE/2); y += 1){
                b32 b = (((x + y)&1) == 1);
                u8 c = b?255:0;
                buf[x + y*CALIBRATION_SIDE] = c;
                buf[x + (CALIBRATION_SIDE/2) + (y + (CALIBRATION_SIDE/2))*CALIBRATION_SIDE] = c;
            }
        }
        // (1/2) gray srgb
        for (u32 x = 0; x < (CALIBRATION_SIDE/2); x += 1){
            for (u32 y = 0; y < (CALIBRATION_SIDE/2); y += 1){
                buf[x + (CALIBRATION_SIDE/2) + y*CALIBRATION_SIDE] = 128;
            }
        }
        // (1/2) gray linear
        for (u32 x = 0; x < (CALIBRATION_SIDE/2); x += 1){
            for (u32 y = 0; y < (CALIBRATION_SIDE/2); y += 1){
                buf[x + (y + (CALIBRATION_SIDE/2))*CALIBRATION_SIDE] = 186;
            }
        }
        
        u8 img[CALIBRATION_SIDE*CALIBRATION_SIDE*3] = {0};
        for (u32 i = 0; i < CALIBRATION_SIDE*CALIBRATION_SIDE; i += 1){
            u8 c = buf[i];
            img[i*3 + 0] = c;
            img[i*3 + 1] = c;
            img[i*3 + 2] = c;
        }
        
        glGenTextures(1, &calibration_texture);
        glBindTexture(GL_TEXTURE_2D, calibration_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, CALIBRATION_SIDE, CALIBRATION_SIDE, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, img);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    //- check for setup errors
    if (glGetError() != 0)
        GFXMessageBox(StrLit("Error"), StrLit("error in opengl renderer initialization"));
    ScratchEnd(scratch);
}

static void
opengl_draw_basic_geometry(Vertex *v, u64 count){
    glBufferData(GL_ARRAY_BUFFER, sizeof(*v)*count, v, GL_STREAM_DRAW);
    
    glUseProgram(basic_program);
    glUniform2f(basic_u_view_xform, 2.f/graphics_w, 2.f/graphics_h);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0,
                          sizeof(Vertex), PtrOffsetOf(Vertex, px));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, 0,
                          sizeof(Vertex), PtrOffsetOf(Vertex, cr));
    
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)count);
}

static void
opengl_draw_srgb_in_geometry(Vertex *v, u64 count){
    glBufferData(GL_ARRAY_BUFFER, sizeof(*v)*count, v, GL_STREAM_DRAW);
    
    glUseProgram(srgb_in_program);
    glUniform2f(srgb_in_u_view_xform, 2.f/graphics_w, 2.f/graphics_h);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0,
                          sizeof(Vertex), PtrOffsetOf(Vertex, px));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, 0,
                          sizeof(Vertex), PtrOffsetOf(Vertex, cr));
    
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)count);
}

static void
opengl_draw_rect_geometry(Vertex *v, u64 count){
    glBufferData(GL_ARRAY_BUFFER, sizeof(*v)*count, v, GL_STREAM_DRAW);
    
    glUseProgram(rect_program);
    glUniform2f(rect_u_view_xform, 2.f/graphics_w, 2.f/graphics_h);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0,
                          sizeof(Vertex), PtrOffsetOf(Vertex, px));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, 0,
                          sizeof(Vertex), PtrOffsetOf(Vertex, rx0));
    
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)count);
}

static void
opengl_draw_texture_geometry(Vertex *v, u64 count, u32 texture){
    glBufferData(GL_ARRAY_BUFFER, sizeof(*v)*count, v, GL_STREAM_DRAW);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glUseProgram(texture_program);
    glUniform2f(texture_u_view_xform, 2.f/graphics_w, 2.f/graphics_h);
    glUniform1i(texture_u_texture, 0);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0,
                          sizeof(Vertex), PtrOffsetOf(Vertex, px));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0,
                          sizeof(Vertex), PtrOffsetOf(Vertex, uvx));
    
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)count);
}

static void
opengl_draw_texrgb_geometry(Vertex *v, u64 count, u32 texture,
                            f32 r, f32 g, f32 b, f32 a){
    glBufferData(GL_ARRAY_BUFFER, sizeof(*v)*count, v, GL_STREAM_DRAW);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glUseProgram(texrgb_program);
    glUniform2f(texrgb_u_view_xform, 2.f/graphics_w, 2.f/graphics_h);
    glUniform1i(texrgb_u_texture, 0);
    glUniform4f(texrgb_u_color, r, g, b, a);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0,
                          sizeof(Vertex), PtrOffsetOf(Vertex, px));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0,
                          sizeof(Vertex), PtrOffsetOf(Vertex, uvx));
    
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)count);
}




////////////////////////////////
//- Main

typedef struct Rect{
    f32 x0;
    f32 y0;
    f32 x1;
    f32 y1;
    f32 r;
    f32 g;
    f32 b;
    f32 a;
} Rect;

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nShowCmd;
    
    sanity_test_integrals();
    sanity_test_circle_area();
    
    W32WinMainInit(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    GFXInit();
    OGL_Init();
    
    GFXWindow window = GFXCreateWindowEx(StrLit("SRGB Test"), 0, 0, 0, 0);
    OGL_WindowEquip(window);
    GFXShowWindow(window);
    GFXWindowSetFlags(window, FLAG_MODE_FULLSCREEN, 1);
    
    u64 frame_counter = 0;
    for (;;)
    {
        if (!GFXWaitForInput()) break;
        if (!GFXWindowIsValid(window)) break;
        
        // begin render
        wgl_helper_begin_render(window);
        
        // initialization
        if (frame_counter == 0)
            opengl_render_init();
        
        // screen layout
        f32 center_x = graphics_w*0.5f;
        f32 divider_half_w = 3.f;
        f32 divider_min_x = (f32)(i32)(center_x - divider_half_w);
        f32 divider_max_x = divider_min_x + 2.f*divider_half_w;
        
        f32 lside_grad_x0 = 352.f;
        f32 lside_grad_x1 = lside_grad_x0 + 256.f;
        f32 rside_grad_x0 = 1312.f;
        f32 rside_grad_x1 = rside_grad_x0 + 256.f;
        
        f32 test1_y = graphics_h - 10.f;
        f32 test2_y = graphics_h - 60.f;
        f32 test3_y = graphics_h - 110.f;
        f32 test4_y = graphics_h - 160.f;
        f32 test5_y = graphics_h - 340.f;
        f32 test6_y = graphics_h - 450.f;
        f32 test7_y = graphics_h - 480.f;
        f32 test8_y = graphics_h - 800.f;
        
        f32 calib_y = graphics_h - 10.f;
        f32 calib_x = 10.f;
        
        // render to canvas frame buffer
        {
            glBindFramebuffer(GL_FRAMEBUFFER, canvas_framebuffer);
            glViewport(0, 0, (i32)graphics_w, (i32)graphics_h);
            
            // black background everywhere
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            // draw divider
            {
                Vertex v[6];
                v[0].px = divider_min_x; v[0].py = 0.f;
                v[1].px = divider_min_x; v[1].py = graphics_h;
                v[2].px = divider_max_x; v[2].py = 0.f;
                v[3].px = v[1].px;       v[3].py = v[1].py;
                v[4].px = v[2].px;       v[4].py = v[2].py;
                v[5].px = divider_max_x; v[5].py = graphics_h;
                for (u32 i = 0; i < 6; i += 1){
                    v[i].cr = 1.f;
                    v[i].cg = 1.f;
                    v[i].cb = 1.f;
                    v[i].ca = 1.f;
                }
                
                opengl_draw_basic_geometry(v, 6);
            }
            
            // two sided test
            for (u32 test_idx = 0; test_idx < 2; test_idx += 1){
                f32 grad_x0 = lside_grad_x0;
                f32 grad_x1 = lside_grad_x1;
                if (test_idx == 1){
                    grad_x0 = rside_grad_x0;
                    grad_x1 = rside_grad_x1;
                }
                
                // 1: draw gradient
                {
                    f32 grad_y1 = test1_y;
                    f32 grad_y0 = test1_y - 40.f;
                    
                    Vertex v[6];
                    v[0].px = grad_x0; v[0].py = grad_y0;
                    v[1].px = grad_x0; v[1].py = grad_y1;
                    v[2].px = grad_x1; v[2].py = grad_y0;
                    v[3].px = v[1].px; v[3].py = v[1].py;
                    v[4].px = v[2].px; v[4].py = v[2].py;
                    v[5].px = grad_x1; v[5].py = grad_y1;
                    for (u32 i = 0; i < 6; i += 1){
                        if (v[i].px == grad_x0){
                            v[i].cr = 1.f; v[i].cg = 1.f; v[i].cb = 1.f; v[i].ca = 1.f;
                        }
                        else{
                            v[i].cr = 0.f; v[i].cg = 0.f; v[i].cb = 0.f; v[i].ca = 1.f;
                        }
                    }
                    
                    opengl_draw_basic_geometry(v, 6);
                }
                
                // 2: draw extremes and midpoint
                {
                    f32 box_x[4] = {0};
                    box_x[0] = grad_x0;
                    box_x[1] = (f32)(i32)((grad_x0*2.f + grad_x1)/3.f);
                    box_x[2] = (f32)(i32)((grad_x0 + grad_x1*2.f)/3.f);
                    box_x[3] = grad_x1;
                    
                    f32 val[3] = { 1.f, 0.5f, 0.f };
                    
                    f32 grad_y1 = test2_y;
                    f32 grad_y0 = test2_y - 40.f;
                    
                    for (u32 j = 0; j < 3; j += 1){
                        f32 box_x0 = box_x[j];
                        f32 box_x1 = box_x[j + 1];
                        
                        f32 valj = val[j];
                        
                        Vertex v[6];
                        v[0].px = box_x0;  v[0].py = grad_y0;
                        v[1].px = box_x0;  v[1].py = grad_y1;
                        v[2].px = box_x1;  v[2].py = grad_y0;
                        v[3].px = v[1].px; v[3].py = v[1].py;
                        v[4].px = v[2].px; v[4].py = v[2].py;
                        v[5].px = box_x1;  v[5].py = grad_y1;
                        for (u32 i = 0; i < 6; i += 1){
                            v[i].cr = valj; v[i].cg = valj; v[i].cb = valj; v[i].ca = 1.f;
                        }
                        
                        opengl_draw_basic_geometry(v, 6);
                    }
                    
                }
                
                // 3: compare x^2.2 with texture sampling for (sRGB -> Linear)
                if (test_idx == 1){
                    f32 grad_y2 = test3_y;
                    f32 grad_y1 = test3_y - 20.f;
                    f32 grad_y0 = test3_y - 40.f;
                    
                    // convert (sRGB -> Linear) by texture sampling
                    {
                        Vertex v[6];
                        v[0].px  = grad_x0; v[0].py  = grad_y1;
                        v[0].uvx = 0.f;     v[0].uvy = 1.f;
                        v[1].px  = grad_x0; v[1].py  = grad_y2;
                        v[1].uvx = 0.f;     v[1].uvy = 0.f;
                        v[2].px  = grad_x1; v[2].py  = grad_y1;
                        v[2].uvx = 1.f;     v[2].uvy = 1.f;
                        v[5].px  = grad_x1; v[5].py  = grad_y2;
                        v[5].uvx = 1.f;     v[5].uvy = 0.f;
                        
                        v[3] = v[1];
                        v[4] = v[2];
                        
                        opengl_draw_texture_geometry(v, 6, test3_texture);
                    }
                    
                    // convert (sRGB -> Linear) by pow(x, 2.2) in shader
                    {
                        f32 delta_x = (grad_x1 - grad_x0)/16.f;
                        
                        for (u32 i = 0; i < 16; i += 1){
                            f32 l = (f32)(i32)(grad_x0 + (f32)i*delta_x);
                            f32 r = (f32)(i32)(l + delta_x);
                            
                            Vertex v[6];
                            v[0].px = l; v[0].py = grad_y0;
                            v[1].px = l; v[1].py = grad_y1;
                            v[2].px = r; v[2].py = grad_y0;
                            v[5].px = r; v[5].py = grad_y1;
                            v[3] = v[1];
                            v[4] = v[2];
                            
                            f32 val = (f32)(0x08 + 0x10*i)/255.f;
                            for (u32 j = 0; j < 6; j += 1){
                                v[j].cr = val;
                                v[j].cg = val;
                                v[j].cb = val;
                                v[j].ca = 1.f;
                            }
                            
                            opengl_draw_srgb_in_geometry(v, 6);
                        }
                    }
                    
                }
                
                // 4: color gradient
                {
                    f32 color_pairs[12][3] = {
                        {1.f, 0.f, 0.f}, {1.f, 1.f, 0.f},
                        {1.f, 1.f, 0.f}, {0.f, 1.f, 0.f},
                        {0.f, 1.f, 0.f}, {0.f, 1.f, 1.f},
                        {0.f, 1.f, 1.f}, {0.f, 0.f, 1.f},
                        {0.f, 0.f, 1.f}, {1.f, 0.f, 1.f},
                        {1.f, 0.f, 1.f}, {1.f, 0.f, 0.f},
                    };
                    
                    f32 cursor_y = test4_y;
                    
                    for (u32 j = 0; j < 12; j += 2){
                        f32 grad_y1 = cursor_y;
                        f32 grad_y0 = cursor_y - 20.f;
                        cursor_y -= 30.f;
                        
                        f32 *col[2] = {0};
                        col[0] = color_pairs[j];
                        col[1] = color_pairs[j + 1];
                        
                        Vertex v[6];
                        v[0].px = grad_x0; v[0].py = grad_y0;
                        v[1].px = grad_x0; v[1].py = grad_y1;
                        v[2].px = grad_x1; v[2].py = grad_y0;
                        v[3].px = v[1].px; v[3].py = v[1].py;
                        v[4].px = v[2].px; v[4].py = v[2].py;
                        v[5].px = grad_x1; v[5].py = grad_y1;
                        for (u32 i = 0; i < 6; i += 1){
                            u32 k = 0;
                            if (v[i].px == grad_x1){
                                k = 1;
                            }
                            v[i].cr = col[k][0];
                            v[i].cg = col[k][1];
                            v[i].cb = col[k][2];
                            v[i].ca = 1.f;
                        }
                        
                        if (test_idx == 0){
                            opengl_draw_basic_geometry(v, 6);
                        }
                        else{
                            opengl_draw_srgb_in_geometry(v, 6);
                        }
                    }
                }
                
                // 5: transparency blending
                {
                    Rect rect[20] = {0};
                    u32 recti = 0;
                    
                    f32 top_y = test5_y;
                    f32 bot_y = top_y - 100.f;
                    
                    rect[recti].x0 = grad_x0;
                    rect[recti].y0 = bot_y;
                    rect[recti].x1 = grad_x1;
                    rect[recti].y1 = top_y;
                    rect[recti].r = 1.f;
                    rect[recti].g = 1.f;
                    rect[recti].b = 1.f;
                    rect[recti].a = 1.f;
                    recti += 1;
                    
                    for (u32 i = 0; i < 3; i += 1){
                        rect[recti].x0 = grad_x0;
                        rect[recti].y0 = top_y - 10.f - 30.f*(f32)i;
                        rect[recti].x1 = grad_x1;
                        rect[recti].y1 = top_y - 30.f - 30.f*(f32)i;
                        rect[recti].r = (i == 0)?1.f:0.f;
                        rect[recti].g = (i == 1)?1.f:0.f;
                        rect[recti].b = (i == 2)?1.f:0.f;
                        rect[recti].a = 1.f;
                        recti += 1;
                    }
                    
                    for (u32 j = 0; j < 2; j += 1){
                        for (u32 i = 0; i < 3; i += 1){
                            rect[recti].x0 = grad_x0 + 10.f + 30.f*(f32)i + 100.f*(f32)j;
                            rect[recti].y0 = bot_y;
                            rect[recti].x1 = grad_x0 + 30.f + 30.f*(f32)i + 100.f*(f32)j;
                            rect[recti].y1 = top_y;
                            rect[recti].r = (i == 0)?1.f:0.f;
                            rect[recti].g = (i == 1)?1.f:0.f;
                            rect[recti].b = (i == 2)?1.f:0.f;
                            rect[recti].a = (j == 0)?0.5f:0.75f;
                            recti += 1;
                        }
                    }
                    
                    for (u32 j = 0; j < recti; j += 1){
                        Vertex v[6] = {0};
                        v[0].px = rect[j].x0; v[0].py = rect[j].y0;
                        v[1].px = rect[j].x0; v[1].py = rect[j].y1;
                        v[2].px = rect[j].x1; v[2].py = rect[j].y0;
                        v[3].px = rect[j].x0; v[3].py = rect[j].y1;
                        v[4].px = rect[j].x1; v[4].py = rect[j].y0;
                        v[5].px = rect[j].x1; v[5].py = rect[j].y1;
                        for (u32 k = 0; k < 6; k += 1){
                            v[k].cr = rect[j].r;
                            v[k].cg = rect[j].g;
                            v[k].cb = rect[j].b;
                            v[k].ca = rect[j].a;
                        }
                        
                        if (test_idx == 0){
                            opengl_draw_basic_geometry(v, 6);
                        }
                        else{
                            opengl_draw_srgb_in_geometry(v, 6);
                        }
                    }
                }
                
                // 6: sub-pixel rectangle anti-aliasing
                {
                    f32 top_y = test6_y;
                    f32 bot_y = test6_y - 20.f;
                    
                    f32 x_cursor = grad_x0;
                    for (u32 i = 0; i < 16; i += 1){
                        f32 l = x_cursor + 4.0625f*(f32)i;
                        f32 r = l + 2.f;
                        
                        f32 floor_l = (f32)(i32)l;
                        f32 ceil_r  = (f32)(i32)r;
                        if (ceil_r < (f32)r){
                            ceil_r += 1.f;
                        }
                        
                        Vertex v[6];
                        v[0].px = floor_l; v[0].py = bot_y;
                        v[1].px = floor_l; v[1].py = top_y;
                        v[2].px = ceil_r;  v[2].py = bot_y;
                        v[3].px = floor_l; v[3].py = top_y;
                        v[4].px = ceil_r;  v[4].py = bot_y;
                        v[5].px = ceil_r;  v[5].py = top_y;
                        for (u32 j = 0; j < 6; j += 1){
                            v[j].rx0 = l;
                            v[j].ry0 = bot_y;
                            v[j].rx1 = r;
                            v[j].ry1 = top_y;
                        }
                        
                        opengl_draw_rect_geometry(v, 6);
                    }
                }
                
                // 7: circles rendered with analytical antialiasing via integration
                {
                    f32 y = test7_y;
                    f32 x = grad_x0;
                    
                    // circle5
                    {
                        f32 y1 = y;
                        f32 y0 = y - CIRCLE5_SIDE;
                        f32 x0 = x;
                        f32 x1 = x + CIRCLE5_SIDE;
                        
                        x += CIRCLE5_SIDE + 10;
                        
                        Vertex v[6];
                        v[0].px  = x0;   v[0].py  = y0;
                        v[0].uvx = 0.f;  v[0].uvy = 0.f;
                        v[1].px  = x0;   v[1].py  = y1;
                        v[1].uvx = 0.f;  v[1].uvy = 1.f;
                        v[2].px  = x1;   v[2].py  = y0;
                        v[2].uvx = 1.f;  v[2].uvy = 0.f;
                        v[3].px  = x0;   v[3].py  = y1;
                        v[3].uvx = 0.f;  v[3].uvy = 1.f;
                        v[4].px  = x1;   v[4].py  = y0;
                        v[4].uvx = 1.f;  v[4].uvy = 0.f;
                        v[5].px  = x1;   v[5].py  = y1;
                        v[5].uvx = 1.f;  v[5].uvy = 1.f;
                        
                        opengl_draw_texture_geometry(v, 6, circle5_texture);
                    }
                    
                    // circle25
                    {
                        f32 y1 = y;
                        f32 y0 = y - CIRCLE25_SIDE;
                        f32 x0 = x;
                        f32 x1 = x + CIRCLE25_SIDE;
                        
                        x += CIRCLE25_SIDE + 10;
                        
                        Vertex v[6];
                        v[0].px  = x0;   v[0].py  = y0;
                        v[0].uvx = 0.f;  v[0].uvy = 0.f;
                        v[1].px  = x0;   v[1].py  = y1;
                        v[1].uvx = 0.f;  v[1].uvy = 1.f;
                        v[2].px  = x1;   v[2].py  = y0;
                        v[2].uvx = 1.f;  v[2].uvy = 0.f;
                        v[3].px  = x0;   v[3].py  = y1;
                        v[3].uvx = 0.f;  v[3].uvy = 1.f;
                        v[4].px  = x1;   v[4].py  = y0;
                        v[4].uvx = 1.f;  v[4].uvy = 0.f;
                        v[5].px  = x1;   v[5].py  = y1;
                        v[5].uvx = 1.f;  v[5].uvy = 1.f;
                        
                        opengl_draw_texture_geometry(v, 6, circle25_texture);
                    }
                    
                    // circle125
                    {
                        f32 y1 = y;
                        f32 y0 = y - CIRCLE125_SIDE;
                        f32 x0 = x;
                        f32 x1 = x + CIRCLE125_SIDE;
                        
                        x += CIRCLE125_SIDE + 10;
                        
                        Vertex v[6];
                        v[0].px  = x0;   v[0].py  = y0;
                        v[0].uvx = 0.f;  v[0].uvy = 0.f;
                        v[1].px  = x0;   v[1].py  = y1;
                        v[1].uvx = 0.f;  v[1].uvy = 1.f;
                        v[2].px  = x1;   v[2].py  = y0;
                        v[2].uvx = 1.f;  v[2].uvy = 0.f;
                        v[3].px  = x0;   v[3].py  = y1;
                        v[3].uvx = 0.f;  v[3].uvy = 1.f;
                        v[4].px  = x1;   v[4].py  = y0;
                        v[4].uvx = 1.f;  v[4].uvy = 0.f;
                        v[5].px  = x1;   v[5].py  = y1;
                        v[5].uvx = 1.f;  v[5].uvy = 1.f;
                        
                        opengl_draw_texture_geometry(v, 6, circle125_texture);
                    }
                }
                
                // 8: color on color circles
                {
                    f32 back[][4] = {
                        {1.f, 0.f, 0.f, 1.f},
                        {1.f, 0.f, 0.f, 1.f},
                        {0.f, 1.f, 0.f, 1.f},
                        {0.f, 1.f, 0.f, 1.f},
                        {0.f, 0.f, 1.f, 1.f},
                        {0.f, 0.f, 1.f, 1.f},
                    };
                    f32 fore[][4] = {
                        {0.f, 1.f, 0.f, 1.f},
                        {0.f, 0.f, 1.f, 1.f},
                        {1.f, 0.f, 0.f, 1.f},
                        {0.f, 0.f, 1.f, 1.f},
                        {1.f, 0.f, 0.f, 1.f},
                        {0.f, 1.f, 0.f, 1.f},
                    };
                    
                    
                    
                    f32 y = test8_y;
                    f32 x = grad_x0;
                    
                    for (u32 k = 0; k < 6; k += 1){
                        // background
                        {
                            f32 x0 = x;
                            f32 x1 = x + CIRCLE5_SIDE + CIRCLE25_SIDE + 20.f;
                            f32 y1 = y;
                            f32 y0 = y - CIRCLE25_SIDE;
                            
                            Vertex v[6];
                            v[0].px = x0; v[0].py = y0;
                            v[1].px = x0; v[1].py = y1;
                            v[2].px = x1; v[2].py = y0;
                            v[3].px = x0; v[3].py = y1;
                            v[4].px = x1; v[4].py = y0;
                            v[5].px = x1; v[5].py = y1;
                            for (u32 i = 0; i < 6; i += 1){
                                v[i].cr = back[k][0];
                                v[i].cg = back[k][1];
                                v[i].cb = back[k][2];
                                v[i].ca = back[k][3];
                            }
                            
                            if (test_idx == 0){
                                opengl_draw_basic_geometry(v, 6);
                            }
                            else{
                                opengl_draw_srgb_in_geometry(v, 6);
                            }
                        }
                        
                        // circle5
                        {
                            f32 y1 = y;
                            f32 y0 = y - CIRCLE5_SIDE;
                            f32 x0 = x;
                            f32 x1 = x + CIRCLE5_SIDE;
                            
                            x += CIRCLE5_SIDE + 10;
                            
                            Vertex v[6];
                            v[0].px  = x0;   v[0].py  = y0;
                            v[0].uvx = 0.f;  v[0].uvy = 0.f;
                            v[1].px  = x0;   v[1].py  = y1;
                            v[1].uvx = 0.f;  v[1].uvy = 1.f;
                            v[2].px  = x1;   v[2].py  = y0;
                            v[2].uvx = 1.f;  v[2].uvy = 0.f;
                            v[3].px  = x0;   v[3].py  = y1;
                            v[3].uvx = 0.f;  v[3].uvy = 1.f;
                            v[4].px  = x1;   v[4].py  = y0;
                            v[4].uvx = 1.f;  v[4].uvy = 0.f;
                            v[5].px  = x1;   v[5].py  = y1;
                            v[5].uvx = 1.f;  v[5].uvy = 1.f;
                            
                            opengl_draw_texrgb_geometry(v, 6, circle5_texture,
                                                        fore[k][0], fore[k][1],
                                                        fore[k][2], fore[k][3]);
                        }
                        
                        // circle25
                        {
                            f32 y1 = y;
                            f32 y0 = y - CIRCLE25_SIDE;
                            f32 x0 = x;
                            f32 x1 = x + CIRCLE25_SIDE;
                            
                            x += CIRCLE25_SIDE + 10;
                            
                            Vertex v[6];
                            v[0].px  = x0;   v[0].py  = y0;
                            v[0].uvx = 0.f;  v[0].uvy = 0.f;
                            v[1].px  = x0;   v[1].py  = y1;
                            v[1].uvx = 0.f;  v[1].uvy = 1.f;
                            v[2].px  = x1;   v[2].py  = y0;
                            v[2].uvx = 1.f;  v[2].uvy = 0.f;
                            v[3].px  = x0;   v[3].py  = y1;
                            v[3].uvx = 0.f;  v[3].uvy = 1.f;
                            v[4].px  = x1;   v[4].py  = y0;
                            v[4].uvx = 1.f;  v[4].uvy = 0.f;
                            v[5].px  = x1;   v[5].py  = y1;
                            v[5].uvx = 1.f;  v[5].uvy = 1.f;
                            
                            opengl_draw_texrgb_geometry(v, 6, circle25_texture,
                                                        fore[k][0], fore[k][1],
                                                        fore[k][2], fore[k][3]);
                        }
                        
                        // move y down after 2
                        if ((k + 1) % 2 == 0){
                            x = grad_x0;
                            y -= CIRCLE25_SIDE;
                        }
                    }
                }
                
                // 9: color calibration blocks
                if (test_idx == 0){
                    f32 y1 = calib_y;
                    f32 y0 = calib_y - CALIBRATION_SIDE;
                    f32 x0 = calib_x;
                    f32 x1 = calib_x + CALIBRATION_SIDE;
                    
                    Vertex v[6];
                    v[0].px  = x0;   v[0].py  = y0;
                    v[0].uvx = 0.f;  v[0].uvy = 0.f;
                    v[1].px  = x0;   v[1].py  = y1;
                    v[1].uvx = 0.f;  v[1].uvy = 1.f;
                    v[2].px  = x1;   v[2].py  = y0;
                    v[2].uvx = 1.f;  v[2].uvy = 0.f;
                    v[3].px  = x0;   v[3].py  = y1;
                    v[3].uvx = 0.f;  v[3].uvy = 1.f;
                    v[4].px  = x1;   v[4].py  = y0;
                    v[4].uvx = 1.f;  v[4].uvy = 0.f;
                    v[5].px  = x1;   v[5].py  = y1;
                    v[5].uvx = 1.f;  v[5].uvy = 1.f;
                    
                    opengl_draw_texture_geometry(v, 6, calibration_texture);
                }
                
            }
            
        }
        
        // resolve canvas to screen
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, canvas_framebuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            
            i32 w = (i32)graphics_w;
            i32 h = (i32)graphics_h;
            
            // left side resolve *without* GL_FRAMEBUFFER_SRGB
            glBlitFramebuffer(0, 0, w/2, h,
                              0, 0, w/2, h,
                              GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);
            
            // right side resolve *with* GL_FRAMEBUFFER_SRGB
            glEnable(GL_FRAMEBUFFER_SRGB);
            glBlitFramebuffer(w/2, 0, w, h,
                              w/2, 0, w, h,
                              GL_COLOR_BUFFER_BIT,
                              GL_NEAREST);
            glDisable(GL_FRAMEBUFFER_SRGB);
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        
        // after render numerical value checks
        {
            // 1: each value differs from it's neighbors by exactly 1
            //    (w/ 4-byte stride; alpha channel not included)
            u8 buf[256*4] = {0};
            glReadPixels((i32)lside_grad_x0, (i32)test1_y - 5, 256, 1,
                         GL_RGBA, GL_UNSIGNED_BYTE, buf);
            for (u32 i = 1; i < 256; i += 1){
                Assert(buf[(i - 1)*4] - 1 == buf[i*4]);
            }
            
            
            
        }
        
        
        // check for gl errors from this frame
        if (glGetError() != 0)
            GFXMessageBox(StrLit("Error"), StrLit("error in opengl frame render"));
        
        // end render
        wgl_helper_end_render();
        
        // frame counter
        frame_counter += 1;
    }
    
    return(0);
}



////////////////////////////////
//- WGL Definitions

#define WGL_DRAW_TO_WINDOW_ARB    0x2001
#define WGL_ACCELERATION_ARB      0x2003
#define WGL_SWAP_METHOD_ARB       0x2007
#define WGL_SUPPORT_OPENGL_ARB    0x2010
#define WGL_DOUBLE_BUFFER_ARB     0x2011
#define WGL_PIXEL_TYPE_ARB        0x2013
#define WGL_COLOR_BITS_ARB        0x2014
#define WGL_RED_BITS_ARB          0x2015
#define WGL_GREEN_BITS_ARB        0x2017
#define WGL_BLUE_BITS_ARB         0x2019
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_SWAP_EXCHANGE_ARB     0x2028
#define WGL_TYPE_RGBA_ARB         0x202B

//- WGL_ARB_create_context constants

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB         0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB  0x9126
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001

//- wgl funcs x-list

#define WGL_FUNCS_X_LIST() \
    X(wglCreateContext,  HGLRC, (HDC dc))                     \
    X(wglDeleteContext,  BOOL,  (HGLRC glrc))                 \
    X(wglMakeCurrent,    BOOL,  (HDC dc,HGLRC glrc))          \
    X(wglGetProcAddress, PROC,  (LPCSTR name))

//- wgl extension funcs x-list

#define WGL_EXT_FUNCS_X_LIST() \
    X(wglChoosePixelFormatARB, BOOL, (HDC,int*,FLOAT*,UINT,int*,UINT*)) \
    X(wglCreateContextAttribsARB, HGLRC, (HDC dc,HGLRC share,int*atri))


//- wgl function types

#define X(N,R,P) typedef R W32_##N P;
WGL_FUNCS_X_LIST()
WGL_EXT_FUNCS_X_LIST()
#undef X

//- wgl function pointers

#define X(N,R,P) static W32_##N * w32_##N = 0;
WGL_FUNCS_X_LIST()
WGL_EXT_FUNCS_X_LIST()
#undef X


//$ graphical //
