#include "Base.h"
#include "Base.c"

#include "LongGFX.h"
#include "LongGFX_Win32.c"

#include "LongOGL.h"
#include "LongGFX_OpenGL.h"
#include "Win32_OpenGL.c"

#include "LongGFX_D3D11.h"
#include "Win32_D3D11.c"

#include "LongFont.h"
#include "LongIcons.h"
#include "LongFont.c"
#include "LongFont_FreeType.c"

#include "LongRender.h"
#include "LongRender.c"
#include "LongRender_OpenGL.c"

#define MULTI_SAMPLE 0
#define U32LinFromSRGB4(r, g, b, a) U32LinFromSRGB(Pack4F32(r, g, b, a))

function void WindowResizeHandler(GFXWindow window, u32 width, u32 height)
{
    UNUSED(width);
    UNUSED(height);
    OGL_Begin(window);
    glClearColor(0.0f, 1.0f, 1.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    OGL_End();
}

typedef struct Rect Rect;
struct Rect
{
    v2f32 p0, p1;
    f32 radius, thick;
    v4f32 c[2];
    u32 flags;
    f32 theta;
    v2f32 uv0, uv1;
    r2f32 clip;
};

typedef struct Vertex Vertex;
struct Vertex
{
    v2f32 p, center, extent;
    f32 radius, thick;
    u32 c[2];
    u32 flags;
    f32 theta;
    v2f32 uv0, uv1;
    r2f32 clip;
};

#define R_Flag_SharpCornerTL (1 << 0)
#define R_Flag_SharpCornerTR (1 << 1)
#define R_Flag_SharpCornerBL (1 << 2)
#define R_Flag_SharpCornerBR (1 << 3)
#define R_Flag_HzGradient    (1 << 4)

global char glsl_sdf_vshader[] =
"#version 330\n"
"uniform vec2 u_view_xform;\n"
"layout (location =  0) in vec2  v_p;\n"
"layout (location =  1) in vec2  v_center;\n"
"layout (location =  2) in vec2  v_extent;\n"
"layout (location =  3) in float v_radius;\n"
"layout (location =  4) in float v_thick;\n"
"layout (location =  5) in float v_theta;\n"
"layout (location =  6) in uint  v_c0;\n"
"layout (location =  7) in uint  v_c1;\n"
"layout (location =  8) in uint  v_flags;\n"
"layout (location =  9) in vec4  v_uvrect;\n"
"layout (location = 10) in vec4  v_clip_rect;\n"
"flat out vec2  f_center;\n"
"flat out vec2  f_extent;\n"
"flat out float f_radius;\n"
"flat out float f_thick;\n"
"flat out float f_theta;\n"
"flat out vec4  f_c0;\n"
"flat out vec4  f_c1;\n"
"flat out uint  f_flags;\n"
"flat out vec4  f_uvrect;\n"
"flat out vec4  f_clip_rect;\n"
"void main(){\n"
// normalize pos
"vec2 norm_pos = v_p*u_view_xform + vec2(-1.0, -1.0);\n"
// unpack colors
GLSL_UNPACK_COLOR("v_c0", "c00", "c01", "c02", "c03")
GLSL_UNPACK_COLOR("v_c1", "c10", "c11", "c12", "c13")
// fill outputs
"gl_Position = vec4(norm_pos, 0.0, 1.0);\n"
"f_center   = v_center;\n"
"f_extent   = v_extent;\n"
"f_radius = v_radius;\n"
"f_thick  = v_thick;\n"
"f_theta  = v_theta;\n"
"f_c0     = vec4(c00, c01, c02, c03);\n"
"f_c1     = vec4(c10, c11, c12, c13);\n"
"f_flags  = v_flags;\n"
"f_uvrect = v_uvrect;\n"
"f_clip_rect = v_clip_rect;\n"
"}\n";

global char glsl_sdf_fshader[] =
"#version 330\n"
"uniform sampler2D u_tex;\n"
"flat in vec2 f_center;\n"
"flat in vec2 f_extent;\n"
"flat in float f_radius;\n"
"flat in float f_thick;\n"
"flat in float f_theta;\n"
"flat in vec4 f_c0;\n"
"flat in vec4 f_c1;\n"
"flat in uint f_flags;\n"
"flat in vec4 f_uvrect;\n"
"flat in vec4 f_clip_rect;\n"
"in vec4 gl_FragCoord;\n"
"out vec4 out_color;\n"
"void main(){\n"

// discard fragment outside clip rect
"if ((gl_FragCoord.x <  f_clip_rect.x) ||\n"
"    (gl_FragCoord.y <  f_clip_rect.y) ||\n"
"    (gl_FragCoord.x >= f_clip_rect.z) ||\n"
"    (gl_FragCoord.y >= f_clip_rect.w)) { discard; }\n"

// label properties
"float half_thick = f_thick*0.5;\n"
"bool hz_gradient = ((f_flags&0x10u) != 0u);\n"

// apply rotation
"vec2 q = gl_FragCoord.xy - f_center;\n"
"float sin_theta = sin(f_theta);\n"
"float cos_theta = cos(f_theta);\n"
"vec2 p = vec2(+cos_theta*q.x + sin_theta*q.y,\n"
"              -sin_theta*q.x + cos_theta*q.y);\n"

// modify radius for quadrant
"float rad = f_radius;\n"
"uint quadrant = uint(p.x < 0) + 2u * uint(p.y < 0);\n"
"if ((f_flags & (1u << quadrant)) != 0u) rad = 0;\n"

"float r_b = -f_extent.y + rad;\n"
"float r_t = -r_b;\n"
"float r_l = -f_extent.x + rad;\n"
"float r_r = -r_l;\n"

// (p * rect) -> distance
"float r_in_x = max(r_l - p.x, p.x - r_r);\n"
"float r_in_y = max(r_b - p.y, p.y - r_t);\n"
"float r_in_max = max(r_in_x, r_in_y);\n"
"float r_in_dist = min(0, r_in_max);\n"

"vec2 r_ex_np = vec2(clamp(p.x, r_l, r_r), clamp(p.y, r_b, r_t));\n"
"vec2 r_ex_d  = p - r_ex_np;\n"
"float r_ex_dist = sqrt(r_ex_d.x*r_ex_d.x + r_ex_d.y*r_ex_d.y);\n"

"float dist = r_ex_dist + r_in_dist - rad;\n"
"if (half_thick > 0) dist = abs(dist + half_thick) - half_thick;\n"

// distance -> alpha
"float sdf_a_unclamped = (0.5 - dist);\n"
"float sdf_a = clamp(sdf_a_unclamped, 0, 1);\n"

// texture sampling
"float uv_xtu = (p.x + f_extent.x)/(2*f_extent.x);\n"
"float uv_ytu = (p.y + f_extent.y)/(2*f_extent.y);\n"
"float s = 1;\n"
"if (f_uvrect.z > 0){\n"
"  ivec2 texdim = textureSize(u_tex, 0);\n"
"  vec2 htex = vec2(0.5/texdim.x, 0.5/texdim.y);\n"
"  float uv_xu = f_uvrect.x + (f_uvrect.z - f_uvrect.x)*uv_xtu;\n"
"  float uv_yu = f_uvrect.y + (f_uvrect.w - f_uvrect.y)*uv_ytu;\n"
"  float uv_x = clamp(uv_xu, f_uvrect.x + htex.x, f_uvrect.z - htex.x);\n"
"  float uv_y = clamp(uv_yu, f_uvrect.y + htex.y, f_uvrect.w - htex.y);\n"
"  s = texture(u_tex, vec2(uv_x, uv_y)).r;\n"
"}\n"

// color interpolation
"float color_tu = uv_ytu;\n"
"if (hz_gradient) color_tu = uv_xtu;\n"
"float color_t = clamp(color_tu, 0, 1);\n"
"vec4 color = f_c0 + (f_c1 - f_c0)*color_t;\n"

// final color
"out_color = vec4(color.rgb, sdf_a*color.a*s);\n"
"}\n";

global OGL_Shader vshader = {0};
global OGL_Shader fshader = {0};
global OGL_Shader program = {0};

global GLuint vao = 0;
global GLuint vbo = 0;

global GLint viewTransform = -1;
global GLint sdfTexture = -1;
global GLuint fallbackTexture = 0;

global GLuint msaaFbuffer = 0;
global GLuint msaaTexture = 0;

function void DrawRects(Rect* r, u32 count, GLuint texture, v2f32 windim)
{
    if (!glIsTexture(texture))
        texture = fallbackTexture;
    
    ScratchBlock(scratch)
    {
        Vertex* v = PushArray(scratch, Vertex, count*6);
        for (u32 i = 0; i < count; ++i)
        {
            Vertex* vv = v + i*6;
            
            v2f32  p0 = r[i].p0;
            v2f32  p1 = r[i].p1;
            u32 flags = r[i].flags;
            f32 theta = r[i].theta;
            v2f32 uv0 = r[i].uv0;
            v2f32 uv1 = r[i].uv1;
            r2f32 clip = r[i].clip;
            
            // transform corners to center & half dim
            v2f32 center = ScaleV2F32(AddV2F32(p0, p1), 0.5f);
            v2f32 extent = ScaleV2F32(SubV2F32(p1, p0), 0.5f);
            
            // clamp radius
            f32 radius = r[i].radius;
            {
                f32 minSpan = Min(p1.x - p0.x, p1.y - p0.y);
                radius = ClampTop(radius, minSpan * 0.5f);
            }
            
            // threshold/clamp thickness
            f32 thick = r[i].thick;
            if (thick < 1.f)
                thick = 0.f;
            thick = ClampTop(thick, 10000.f);
            
            // setup enclosing primitives
            Assert(p0.x <= p1.x && p0.y <= p1.y);
            {
                v2f32 p00g = V2F32(Floor_f32(p0.x), Floor_f32(p0.y));
                v2f32 p11g = V2F32( Ceil_f32(p1.x),  Ceil_f32(p1.y));
                v2f32 p01g = V2F32(p00g.x, p11g.y);
                v2f32 p10g = V2F32(p11g.x, p00g.y);
                
                if (theta != 0.f)
                {
                    f32 sin = Sin_f32(theta);
                    f32 cos = Cos_f32(theta);
                    
                    v2f32 vx = V2F32(+extent.x*cos, extent.x*sin);
                    v2f32 vy = V2F32(-extent.y*sin, extent.y*cos);
                    
                    p00g = SubV2F32(SubV2F32(center, vx), vy);
                    p01g = AddV2F32(SubV2F32(center, vx), vy);
                    p10g = SubV2F32(AddV2F32(center, vx), vy);
                    p11g = AddV2F32(AddV2F32(center, vx), vy);
                }
                
                vv[0].p = p00g;
                vv[1].p = p01g;
                vv[2].p = p10g;
                vv[3].p = p01g;
                vv[4].p = p10g;
                vv[5].p = p11g;
            }
            
            // pack color
            u32 c0 = PackV4F32(r[i].c[0]);
            u32 c1 = PackV4F32(r[i].c[1]);
            
            // extend clip if zero
            if (clip.x1 == 0 && clip.y1 == 0)
                clip.p1 = V2F32(10000, 10000);
            
            for (u32 j = 0; j < 6; ++j)
            {
                vv[j].center = center;
                vv[j].extent = extent;
                vv[j].radius = radius;
                vv[j].thick = thick;
                vv[j].theta = theta;
                vv[j].c[0] = c0;
                vv[j].c[1] = c1;
                vv[j].uv0 = uv0;
                vv[j].uv1 = uv1;
                vv[j].flags = flags;
                vv[j].clip = clip;
            }
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, count*6*sizeof(*v), v, GL_STREAM_DRAW);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        glUseProgram(program.handle);
        glUniform2f(viewTransform, 2.f/windim.x, 2.f/windim.y);
        glUniform1i(sdfTexture, 0);
        
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, count*6);
    }
}

typedef struct FrameInfo FrameInfo;
struct FrameInfo
{
    r1u64 frames[60];
    u32 frameIdx;
    u32 fps;
    u32 loopSpeed;
    f32 lineX;
};

function void FrameStart(FrameInfo* info)
{
    info->frames[info->frameIdx % ArrayCount(info->frames)].start = OSNowUS();
    info->lineX += 0.05f;
}

function void FrameEnd(FrameInfo* info)
{
    u64 frameCount = ArrayCount(info->frames);
    u32 frameIdx = info->frameIdx++ % frameCount;
    
    u64 delta = MB(1)/info->fps;
    u64 targetEnd = info->frames[frameIdx].start + delta;
    u64 now = OSNowUS();
    info->frames[frameIdx].end = now;
    
    if (now < targetEnd)
    {
        u64 leftover = targetEnd - now;
        Sleep((DWORD)(leftover / 1000));
        now = OSNowUS();
    }
    
    u64 frameEnd = now;
    if (targetEnd - now < delta/16)
        frameEnd = targetEnd;
    info->frames[frameIdx].end = frameEnd;
}

function void DrawQuads(R_Font* fonts, u64 count, u64 fontello, FrameInfo* info)
{
    DEBUG(fonts);
    DEBUG(count);
    DEBUG(fontello);
    DEBUG(info);
    
    ScratchBlock(scratch)
    {
        R_Batch batch = {0};
        
        R_BatchPushQuad(scratch, &batch, &(R_Quad){
                            R2F32P(200.f, 200.f, 300.f, 300.f),
                            10.f, 5.f, 0.f, 0,
                            { U32LinFromSRGB4(1.f, .2f, 0.f, 1.f), U32LinFromSRGB4(1.f, 0.f, .2f, 1.f) },
                        });
        
        R_BatchPushQuad(scratch, &batch, &(R_Quad){
                            R2F32P(100.f, 400.f, 400.f, 500.f),
                            20.f, 10000.f, 0.f, 0,
                            { U32LinFromSRGB4(1.f, 1.f, 1.f, 1.f), U32LinFromSRGB4(0.f, 0.f, 0.f, 1.f) },
                        });
        
        R_BatchPushQuad(scratch, &batch, &(R_Quad){
                            R2F32Size(V2F32(600.f, 400.f), V2F32V(fonts[0].baked.size)),
                            0.f, 10000.f, 0.f, 0,
                            { U32LinFromSRGB4(1.f, 1.f, 1.f, 1.f), U32LinFromSRGB4(1.f, 1.f, 1.f, 1.f) }, R2F32P(0.f, 0.f, 1.f, 1.f)
                        });
        
        R_Submit(batch.first, batch.totalCount, batch.texture);
    }
    
    ScratchBlock(scratch)
    {
        R_Ctx ctx = R_CtxMake(scratch, 0);
        
        u32 c0 = U32LinFromSRGB4(0.9f, 0.1f, 0.0f, 1.0f);
        u32 c1 = U32LinFromSRGB4(0.8f, 0.0f, 0.0f, 1.0f);
        u32 c2 = U32LinFromSRGB4(0.1f, 0.9f, 0.0f, 1.0f);
        u32 c3 = U32LinFromSRGB4(0.0f, 0.9f, 0.1f, 1.0f);
        
        R_PushRect(&ctx, R2F32P( 5,  5, 45, 45), 5.f, c0);
        R_PushRect(&ctx, R2F32P(55,  5, 95, 45), 5.f, c1);
        R_PushRect(&ctx, R2F32P( 5, 50, 45, 70), 0.f, c2);
        R_PushRect(&ctx, R2F32P(55, 50, 95, 70), 0.f, c3);
        R_CtxFont(&ctx, fonts + 1);
        R_PushStr(&ctx, StrLit("The quick brown fox jumps over the lazy dog."),
                  V2F32(25.f, 50.f), U32LinFromSRGB4(1.0f, 1.0f, 0.5f, 1.0f));
        
        R_PushRect(&ctx, R2F32P( 5, 105, 45, 145), 5.f, c0);
        R_PushRect(&ctx, R2F32P(55, 105, 95, 145), 5.f, c1);
        R_PushRect(&ctx, R2F32P( 5, 150, 45, 170), 0.f, c2);
        R_PushRect(&ctx, R2F32P(55, 150, 95, 170), 0.f, c3);
        R_PushStr(&ctx, StrLit("Hello, world!"),
                  V2F32(25.f, 150.f), U32LinFromSRGB4(1.0f, 1.0f, 0.5f, 1.0f));
        
        R_CtxFont (&ctx, &fonts[fontello]);
        R_PushChar(&ctx, ICON_FOLDER, V2F32(25, 250), U32LinFromSRGB4(0.f, .4f, .8f, 1.f));
        R_PushChar(&ctx, ICON_CANCEL, V2F32(50, 250), U32LinFromSRGB4(0.f, .4f, .8f, 1.f));
        
        R_PushLine(&ctx, R2F32P(info->lineX, 300, info->lineX, 400), U32LinFromSRGB4(1, 1, 1, 1));
        
        u64 frameCount = ArrayCount(info->frames);
        {
            u64 idx = info->frameIdx ? (info->frameIdx-1)%frameCount : 0;
            String frameStr = StrPushf(scratch, "%llu", DimR1U64(info->frames[idx]));
            R_CtxFont(&ctx, fonts);
            R_PushStr(&ctx, frameStr, V2F32(600, 50), U32LinFromSRGB4(1.f, 1.f, 0.f, 1.f));
        }
        
        r1f32 xRange = R1F32(200, 700);
        r1f32  yMark = R1F32(100, 110);
        r1f32   yEnd = R1F32(110, 120);
        
        const v4f32 colors[] = {
            V4F32(1.00f, 0.00f, 0.00f, 1.00f),
            V4F32(0.86f, 1.00f, 1.00f, 1.00f),
            V4F32(0.88f, 0.98f, 1.00f, 1.00f),
            V4F32(0.90f, 0.96f, 1.00f, 1.00f),
            
            V4F32(0.00f, 1.00f, 0.00f, 1.00f),
            V4F32(1.00f, 0.86f, 1.00f, 1.00f),
            V4F32(1.00f, 0.88f, 0.98f, 1.00f),
            V4F32(1.00f, 0.90f, 0.96f, 1.00f),
            
            V4F32(0.00f, 0.00f, 1.00f, 1.00f),
            V4F32(1.00f, 1.00f, 0.86f, 1.00f),
            V4F32(0.98f, 1.00f, 0.88f, 1.00f),
            V4F32(0.96f, 1.00f, 0.90f, 1.00f),
        };
        
        u64 delta = MB(1)/info->fps;
        u64 markLoop = info->loopSpeed * delta;
        
        for (u64 i = 0; i < frameCount; ++i)
        {
            r1u64 frame = info->frames[i];
            f32 markT = DivF32(frame.min % markLoop, markLoop);
            f32  endT = DivF32(frame.max % markLoop, markLoop);
            
            f32 xMark = Lerp(xRange.min, xRange.max, markT);
            f32  xEnd = Lerp(xRange.min, xRange.max,  endT);
            
            u32 color = PackV4F32(colors[i % ArrayCount(colors)]);
            R_PushLine(&ctx, R2F32R1(R1F32(xMark, xMark), yMark), color);
            R_PushLine(&ctx, R2F32R1(R1F32( xEnd,  xEnd),  yEnd), color);
            
            if (frame.max - frame.min != delta)
                R_PushRect(&ctx, R2F32P(xMark-2, yMark.min-2, xMark+3, yMark.min+3),
                           0.5f, U32LinFromSRGB4(1, 0, 0, 1));
        }
        
        R_CtxFlush(&ctx);
    }
}

typedef struct Grid Grid;
struct Grid
{
    f32 size;
    v2f32 pos;
    v2f32 dim;
};

function void FillTestRects(Rect* rects, f32 t, f32 tPrev)
{
    UNUSED(rects);
    UNUSED(t);
    UNUSED(tPrev);
    u32 shape_i = 0;
    
    // srgb test gradient
#if 1
    {
        rects[shape_i].p0 = V2F32(550.f, 350.f);
        rects[shape_i].p1 = V2F32(700.f, 380.f);
        rects[shape_i].c[0] = V4F32(1.f, 1.f, 1.f, 1.f);
        rects[shape_i].c[1] = V4F32(1.f, 1.f, 1.f, 0.f);
        rects[shape_i].flags = R_Flag_HzGradient;
        shape_i += 1;
    }
#endif
    
    // eliptical moving square
#if 1
    {
        v2f32 p = V2F32(200 + Cos_f32(t*TAU_F32*5)*200,
                        300 + Sin_f32(t*TAU_F32*5)*300);
        
        rects[shape_i].p0  = p;
        rects[shape_i].p1  = AddV2F32(p, V2F32(5.f, 5.f));
        rects[shape_i].c[0] = V4F32(1.f, 1.0f, 0.f, 1.f);
        rects[shape_i].c[1] = V4F32(0.f, 0.2f, 1.0f, 1.f);
        shape_i += 1;
    }
#endif
    
    // still thin lines
#if 1
    {
        f32 x[4] = { 1.f, 3.25f, 5.5f, 7.75f };
        f32 y_min = 8.f;
        f32 y_max = 13.f;
        
        for (u32 i = 0; i < 4; i += 1){
            rects[shape_i].p0  = V2F32(x[i], y_min);
            rects[shape_i].p1  = V2F32(x[i] + 1.f, y_max);
            rects[shape_i].c[0] = V4F32(1.f, 1.f, 1.f, 1.f);
            rects[shape_i].c[1] = V4F32(1.f, 1.f, 1.f, 1.f);
            shape_i += 1;
        }
    }
#endif
    
    // slow thin line
#if 1
    {
        f32 x = 5 + Sin_f32(t*TAU_F32)*5;
        f32 x_prev = 5 + Sin_f32(tPrev*TAU_F32)*5;
        f32 y_min = -5.f;
        f32 y_max =  5.f;
        
        f32 lx = x;
        f32 rx = x + 1;
        
        f32 vel_x = x - x_prev;
        f32 speed_x = Abs_f32(vel_x);
        if (0.005f <= speed_x)
        {
            if (vel_x < 0.f)
                lx -= 0.2f;
            else
                rx += 0.2f;
        }
        
        rects[shape_i].p0  = V2F32(lx, y_min);
        rects[shape_i].p1  = V2F32(rx, y_max);
        rects[shape_i].c[0] = V4F32(1.f, 1.f, 1.f, 1.f);
        rects[shape_i].c[1] = V4F32(1.f, 1.f, 1.f, 1.f);
        shape_i += 1;
    }
#endif
    
    // still square outline
#if 1
    {
        f32 x = 50, y = 50;
        
        v2f32 hdim = V2F32(50.f, 1.f);
        rects[shape_i].radius = 1.f;
        rects[shape_i].thick = 10.f;
        rects[shape_i].theta = t*7;
        rects[shape_i].flags = R_Flag_HzGradient;
        
        rects[shape_i].p0   = V2F32(x - hdim.x, y - hdim.y);
        rects[shape_i].p1   = V2F32(x + hdim.x, y + hdim.y);
        rects[shape_i].c[0] = V4F32(1.f, 1.f, 0.f, 1.f);
        rects[shape_i].c[1] = V4F32(0.f, 1.f, 1.f, 1.f);
        shape_i += 1;
    }
#endif
    
    // slow square outline
#if 1
    {
        f32 x      = 5 + Sin_f32(t*TAU_F32)*5;
        f32 y      = 5 + Sin_f32(t*TAU_F32*2)*2;
        
        f32 hdim = 6;
        
        //v2f32 p = V2F32(x, y);
        
        rects[shape_i].p0  = V2F32(x - hdim, y - hdim);
        rects[shape_i].p1  = V2F32(x + hdim, y + hdim);
        rects[shape_i].c[0] = V4F32(1.f, 0.f, 0.f, 0.5f);
        rects[shape_i].c[1] = V4F32(0.f, 1.f, 0.f, 0.2f);
        rects[shape_i].radius = 0.f;
        rects[shape_i].thick = 2.f;
        shape_i += 1;
    }
#endif
    
    // textured squares
#if 1
    {
        f32 x      = 300.f + Sin_f32(t*TAU_F32)*15;
        f32 x_prev = 300.f + Sin_f32(tPrev*TAU_F32)*15;
        f32 y      = 300.f + Sin_f32(t*TAU_F32*2)*6;
        f32 y_prev = 300.f + Sin_f32(tPrev*TAU_F32*2)*6;
        
        v2f32 p = V2F32(x, y);
        f32 spacing = 70.f;
        
        f32 hdim = 16;
        
        rects[shape_i].p0 = V2F32(p.x - hdim, p.y - hdim);
        rects[shape_i].p1 = V2F32(p.x + hdim, p.y + hdim);
        rects[shape_i].uv0 = V2F32(0.0f, 0.0f);
        rects[shape_i].uv1 = V2F32(0.5f, 0.5f);
        shape_i += 1;
        
        p.x += spacing;
        
        rects[shape_i].p0 = V2F32(p.x - hdim, p.y - hdim);
        rects[shape_i].p1 = V2F32(p.x + hdim, p.y + hdim);
        rects[shape_i].uv0 = V2F32(0.5f, 0.0f);
        rects[shape_i].uv1 = V2F32(1.0f, 0.5f);
        shape_i += 1;
        
        p.x -= spacing;
        p.y += spacing;
        
        rects[shape_i].p0 = V2F32(p.x - hdim, p.y - hdim);
        rects[shape_i].p1 = V2F32(p.x + hdim, p.y + hdim);
        rects[shape_i].uv0 = V2F32(0.0f, 0.5f);
        rects[shape_i].uv1 = V2F32(0.5f, 1.0f);
        shape_i += 1;
        
        p.x += spacing;
        
        rects[shape_i].p0 = V2F32(p.x - hdim, p.y - hdim);
        rects[shape_i].p1 = V2F32(p.x + hdim, p.y + hdim);
        rects[shape_i].uv0 = V2F32(0.5f, 0.5f);
        rects[shape_i].uv1 = V2F32(1.0f, 1.0f);
        shape_i += 1;
        
        
        for (u32 i = shape_i - 4; i < shape_i; i += 1){
            rects[i].c[0] = V4F32(0.7f, 0.0f, 0.0f, 1.f);
            rects[i].c[1] = V4F32(0.5f, 0.5f, 0.5f, 1.f);
            if ((i%2) == 1){
                rects[i].flags |= R_Flag_HzGradient;
            }
        }
    }
#endif
}

function void DrawVertLine(FrameInfo* info, Grid* grid)
{
    u64 frameLoop = info->fps*10;
    u32 frameIdx = info->frameIdx;
    f32 t = DivF32(frameIdx % frameLoop, frameLoop);
    f32 tPrev = 0;
    if (frameIdx > 0)
        tPrev = DivF32((frameIdx-1) % frameLoop, frameLoop);
    
    // test geometry from parameters
    Rect testRects[20] = {0};
    FillTestRects(testRects, t, tPrev);
    
    // "fine" geometry
    Rect fineRects[ArrayCount(testRects)] = {0};
    for (u64 i = 0; i < ArrayCount(fineRects); ++i)
    {
        if (testRects[i].theta > 0.f)
            continue;
        fineRects[i].p0   = AddV2F32(grid->pos, ScaleV2F32(testRects[i].p0, grid->size));
        fineRects[i].p1   = AddV2F32(grid->pos, ScaleV2F32(testRects[i].p1, grid->size));
        fineRects[i].c[0] = V4F32(1.f, 0.f, 1.f, .25f);
        fineRects[i].c[1] = V4F32(1.f, 0.f, 1.f, .25f);
        fineRects[i].thick = 10000;
    }
    
    v2f32 base = V2F32(100, 100);
    for (u64 i = 0; i < ArrayCount(testRects); ++i)
    {
        testRects[i].p0  = AddV2F32(testRects[i].p0, base);
        testRects[i].p1  = AddV2F32(testRects[i].p1, base);
        
        if (testRects[i].clip.x1 != 0 || testRects[i].clip.y1 != 0)
            testRects[i].clip = ShiftR2F32(testRects[i].clip, base);
        else
            testRects[i].clip = testRects[i].clip;
    }
    
    // draw geometry
    {
#if MULTI_SAMPLE
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFbuffer);
#endif
        
        DrawRects(testRects, ArrayCount(testRects), 0, grid->dim);
        
#if MULTI_SAMPLE
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFbuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        
        glBlitFramebuffer(0, 0, screenDim.x, screenDim.y, 0, 0, screenDim.x, screenDim.y,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
    }
    DrawRects(fineRects, ArrayCount(fineRects), 0, grid->dim);
}


function void Draw10x10(Grid* grid)
{
    // read 10x10 block
    u8 buf[10*10*4];
    glReadPixels(100, 100, 10, 10, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    
    // outline
    {
        f32 outlineSize = 10.f*grid->size + 2.f;
        v2f32 p00 = SubV2F32(grid->pos, V2F32(2.f, 2.f));
        v2f32 p11 = AddV2F32(grid->pos, V2F32(outlineSize, outlineSize));
        DrawRects(&(Rect){ p00, p11, .thick = 10000, .c[0] = V4F32(1, 1, 1, 1), .c[1] = V4F32(1, 1, 1, 1) }, 1, 0, grid->dim);
    }
    
    // main grid
    for (u32 y = 0; y < 10; ++y)
    {
        for (u32 x = 0; x < 10; ++x)
        {
            v2f32 p00 = AddV2F32(grid->pos, V2F32((f32)x*grid->size, (f32)y*grid->size));
            v2f32 p01 = AddV2F32(p00, V2F32(1*grid->size, 0.f));
            v2f32 p10 = AddV2F32(p00, V2F32(0.f, grid->size));
            v2f32 p11 = V2F32(p01.x, p10.y);
            
            u8* colorPtr = buf + 4*(x + 10*y);
            v4f32 color = V4F32((f32)colorPtr[0]/255.f, (f32)colorPtr[1]/255.f,
                                (f32)colorPtr[2]/255.f, (f32)colorPtr[3]/255.f);
            DrawRects(&(Rect){ p00, p11, .thick = 10000, .c[0] = color, .c[1] = color }, 1, 0, grid->dim);
        }
    }
}

int WinMain(HINSTANCE hInstance,
            HINSTANCE hPrevInstance,
            LPSTR lpCmdLine,
            int nShowCmd)
{
    W32WinMainInit(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    
    ScratchBlock(scratch)
    {
        GFXErrorBlock(scratch, 1, .callback = GFXErrorFmt)
        {
            GFXInit();
            OGL_Init();
            InitD3D11();
            R_Init();
        }
        
        GFXSetResizeFunc(WindowResizeHandler);
        
        GFXWindow window = 0;
        GFXErrorBlock(scratch, 1, .callback = GFXErrorFmt)
        {
            window = GFXCreateWindowEx(StrLit("My Window"), CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800);
            OGL_WindowEquip(window);
            GFXShowWindow(window);
        }
        
        FrameInfo* info = &(FrameInfo){
            .lineX = 40.5f, .loopSpeed = 120,
            .fps = (u32)GFXWindowRefreshRate(window),
            //.fps = 10,
        };
        
#if 1
        GFXErrorBlock(scratch, 1, .callback = GFXErrorFmt)
        {
            vshader = OGL_MakeShader(scratch, glsl_sdf_vshader, GL_VERTEX_SHADER);
            if (vshader.log.size > 0)
                ErrorFmt("VERTEX:\n%.*s", StrExpand(vshader.log));
            
            fshader = OGL_MakeShader(scratch, glsl_sdf_fshader, GL_FRAGMENT_SHADER);
            if (fshader.log.size > 0)
                ErrorFmt("FRAGMENT:\n%.*s", StrExpand(fshader.log));
            
            program = OGL_MakeProgram(scratch, ArrayExpand(OGL_Shader, vshader, fshader));
            if (program.log.size > 0)
                ErrorFmt("Program:\n%.*s", StrExpand(program.log));
            
            viewTransform = glGetUniformLocation(program.handle, "u_view_xform");
            sdfTexture    = glGetUniformLocation(program.handle, "u_tex");
            
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
            
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, p)));
            
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, center)));
            
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, extent)));
            
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, radius)));
            
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 1, GL_FLOAT, false, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, thick)));
            
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 1, GL_FLOAT, false, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, theta)));
            
            glEnableVertexAttribArray(6);
            glVertexAttribIPointer(6, 1, GL_UNSIGNED_INT, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, c[0])));
            
            glEnableVertexAttribArray(7);
            glVertexAttribIPointer(7, 1, GL_UNSIGNED_INT, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, c[1])));
            
            glEnableVertexAttribArray(8);
            glVertexAttribIPointer(8, 1, GL_UNSIGNED_INT, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, flags)));
            
            glEnableVertexAttribArray(9);
            glVertexAttribPointer(9, 4, GL_FLOAT, false, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, uv0)));
            
            glEnableVertexAttribArray(10);
            glVertexAttribPointer(10, 4, GL_FLOAT, false, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, clip)));
            
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            
            {
                u8* bitmap = PushArray(scratch, u8, 16);
                for (u32 i = 0; i < 16; ++i)
                    bitmap[i] = 0xFF;
                
                glGenTextures(1, &fallbackTexture);
                glBindTexture(GL_TEXTURE_2D, fallbackTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            
#if MULTI_SAMPLE
            // setup a multi-sample texture
            glGenTextures(1, &msaaTexture);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaTexture);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 16, GL_RGB, 2048, 2048, true);
            if (glGetError() || msaaTexture == 0)
                ErrorFmt("Failed to setup multi-sample texture");
            
            // setup a frame buffer
            glGenFramebuffers(1, &msaaFbuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, msaaFbuffer);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, msaaTexture, 0);
            if (glGetError() || msaaFbuffer == 0)
                ErrorFmt("Failed to setup multi-sample framebuffer");
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
        }
#endif
        
        const String paths[] = {
            StrLit("data/liberation-mono.ttf"),
            StrLit("data/Inconsolata-Regular.ttf"),
            StrLit("data/JetBrainsMono-Regular.ttf"),
            StrLit("data/MonaspaceNeon-Regular.otf"),
            StrLit("data/unifont-16.0.04.otf"),
            StrLit("data/fontello.ttf"),
        };
        
        R_Font fonts[ArrayCount(paths)];
        u64 fontCount = ArrayCount(fonts);
        u64  fontello = fontCount - 1;
        
        for (u32 i = 0; i < fontCount; ++i)
        {
            FNT_Font looseFont = FNT_FontOpen(scratch, &(FNT_LoadParams){
                                                  .flags = FNT_RasterFlag_Smooth|FNT_RasterFlag_Hinted,
                                                  .size = 15, .dpi = 96,
                                                  .path = paths[i],
                                              });
            fonts[i] = R_FontBakeTexture(scratch, &looseFont, &(FNT_Packer){.size = V2I32(512, 512)});
        }
        
        for (TempArena temp = TempBegin(scratch); ; TempEnd(temp))
        {
            if (!GFXPeekInput())
                break;
            if (!GFXWindowIsValid(window))
                break;
            
            v2i32 screenDim = {0};
            Grid* grid = &(Grid){ 20.f, V2F32(600.f, 100.f) };
            if (GFXWindowGetInnerRect(window, 0, 0, &screenDim.x, &screenDim.y))
                grid->dim = V2F32V(screenDim);
            
            FrameStart(info);
            DeferBlock(R_Begin(window), R_End())
            {
                DrawQuads(fonts, fontCount, fontello, info);
                //DrawVertLine(info, grid);
                //Draw10x10(grid);
            }
            FrameEnd(info);
        }
        
        GFXErrorBlock(scratch, 1)
        {
            //FreeD3D11();
            OGL_Free();
            OGL_Init();
            OGL_Free();
        }
    }
    
    return 0;
}
