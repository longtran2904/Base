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
    v2f32 p0;
    v2f32 p1;
    v2f32 vel;
    v4f32 c;
};

typedef struct Vertex Vertex;
struct Vertex
{
    v2f32 p;
    v4f32 c;
    
    v2f32 p0;
    v2f32 p1;
    v2f32 vel;
};

global char glsl_vert_vshader[] =
"#version 330\n"
"uniform vec2 u_view_xform;\n"
"layout (location = 0) in vec2 v_p;\n"
"layout (location = 1) in vec4 v_c;\n"
"layout (location = 2) in vec4 v_rect;\n"
"layout (location = 3) in vec2 v_vel;\n"
"out vec4 f_c;\n"
"out vec4 f_rect;\n"
"out vec2 f_vel;\n"
"void main(){\n"
"vec2 norm_pos = v_p*u_view_xform + vec2(-1.0, -1.0);\n"
"gl_Position = vec4(norm_pos, 0.0, 1.0);\n"
"f_c = v_c;\n"
"f_rect = v_rect;\n"
"f_vel = v_vel;\n"
"}\n";

global char glsl_vert_fshader[] =
"#version 330\n"
"in vec4 f_c;\n"
"in vec4 f_rect;\n"
"in vec2 f_vel;\n"
"in vec4 gl_FragCoord;\n"
"out vec4 out_color;\n"
"void main(){\n"
"float frag_min_x = gl_FragCoord.x - 0.5;\n"
"float frag_max_x = gl_FragCoord.x + 0.5;\n"
// spatial coverage
"float sp_cover_min_x = max(f_rect.x, frag_min_x);\n"
"float sp_cover_max_x = min(f_rect.z, frag_max_x);\n"
"float sp_a = sp_cover_max_x - sp_cover_min_x;\n"
// temporal coverage
"float tm_rect_min_x = min(f_rect.x, f_rect.x - f_vel.x);\n"
"float tm_rect_max_x = max(f_rect.z, f_rect.z - f_vel.x);\n"
"float tm_cover_min_x = max(tm_rect_min_x, frag_min_x);\n"
"float tm_cover_max_x = min(tm_rect_max_x, frag_max_x);\n"
"float tm_a = tm_cover_max_x - tm_cover_min_x;\n"
// tail strength
"float speed_sqr = f_vel.x*f_vel.x + f_vel.y*f_vel.y;\n"
"float speed_clamped = max(1, sqrt(speed_sqr));\n"
"float tail_strength = 1/speed_clamped;\n"
// combine alpha
"float a = tm_a * tail_strength;\n"
"out_color = vec4(f_c.xyz, f_c.a * a);\n"
"}\n";

OGL_Shader vert_vshader = {0};
OGL_Shader vert_fshader = {0};
OGL_Shader vert_program = {0};
GLint vert_viewTransform = -1;

GLuint msaaFbuffer = 0;
GLuint msaaTexture = 0;

function void DrawGeometry(Vertex* v, u32 count, v2f32 windim)
{
    glBufferData(GL_ARRAY_BUFFER, count*sizeof(*v), v, GL_STREAM_DRAW);
    
    glUseProgram(vert_program.handle);
    glUniform2f(vert_viewTransform, 2.f/windim.x, 2.f/windim.y);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, p)));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, 0, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, c)));
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, 0, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, p0)));
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, 0, sizeof(Vertex), PtrFromInt(OffsetOf(Vertex, vel)));
    
    glDrawArrays(GL_TRIANGLES, 0, count);
}

function void DrawRects(Rect* r, u32 count, v2f32 windim)
{
    ScratchBlock(scratch)
    {
        Vertex* v = PushArray(scratch, Vertex, count*6);
        for (u32 i = 0; i < count; ++i)
        {
            v2f32  p0 = r[i].p0;
            v2f32  p1 = r[i].p1;
            v2f32 vel = r[i].vel;
            v4f32 col = r[i].c;
            
            f32 boost = .15f;
            f32 boostScale = 1.f - boost;
            
            if (-1.f <= vel.x && vel.x <= -0.01f)
                vel.x = (vel.x + 1.f)*boostScale - 1.f;
            else if (0.01f <= vel.x && vel.x <= 1.f)
                vel.x = (vel.x - 1.f)*boostScale + 1.f;
            
            if (-1.f <= vel.y && vel.y <= -0.01f)
                vel.y = (vel.y + 1.f)*boostScale - 1.f;
            else if (0.01f <= vel.y && vel.y <= 1.f)
                vel.y = (vel.y - 1.f)*boostScale + 1.f;
            
            v2f32 p0pre = SubV2F32(p0, vel);
            v2f32 p1pre = SubV2F32(p1, vel);
            v2f32 p0c = V2F32(Min(p0.x, p0pre.x), Min(p0.y, p0pre.y));
            v2f32 p1c = V2F32(Max(p1.x, p1pre.x), Max(p1.y, p1pre.y));
            
            v2f32 p0g = V2F32(Floor_f32(p0c.x), Floor_f32(p0c.y));
            v2f32 p1g = V2F32( Ceil_f32(p1c.x),  Ceil_f32(p1c.y));
            
            Vertex* vv = v + i*6;
            vv[0].p = p0g;
            vv[1].p = V2F32(p0g.x, p1g.y);
            vv[2].p = V2F32(p1g.x, p0g.y);
            vv[3].p = V2F32(p0g.x, p1g.y);
            vv[4].p = V2F32(p1g.x, p0g.y);
            vv[5].p = p1g;
            
            for (u32 j = 0; j < 6; ++j)
            {
                vv[j].p0  = p0;
                vv[j].p1  = p1;
                vv[j].vel = vel;
                vv[j].c   = col;
            }
        }
        
        DrawGeometry(v, count*6, windim);
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
            //InitD3D11();
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
        
        String paths[] = {
            StrLit("data/liberation-mono.ttf"),
            StrLit("data/Inconsolata-Regular.ttf"),
            StrLit("data/JetBrainsMono-Regular.ttf"),
            StrLit("data/MonaspaceNeon-Regular.otf"),
            StrLit("data/unifont-16.0.04.otf"),
            StrLit("data/fontello.ttf"),
        };
        
        FNT_LoadParams params = {
            .flags = FNT_RasterFlag_Smooth|FNT_RasterFlag_Hinted,
            .size = 15, .dpi = 96,
        };
        
        R_Font fonts[ArrayCount(paths)] = {0};
        for (u32 i = 0; i < ArrayCount(paths); ++i)
        {
            params.path = paths[i];
            FNT_Font looseFont = FNT_FontOpen(scratch, &params);
            fonts[i] = R_FontBakeTexture(scratch, &looseFont, &(FNT_Packer){.size = V2I32(512, 512)});
        }
        
        v4f32 colors[] = {
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
        
        f32 lineX = 40.5f;
        
        u64 prevFrame = 0;
        u64 frameBegin = OSNowUS();
        
        u64 fps = GFXWindowRefreshRate(window);
        //u64 fps = 10;
        u64 delta = MB(1)/fps;
        u64 markLoop = 120*delta;
        u64 frameIdx = 0;
        
        u64 markHistory[60] = {0};
        u64  endHistory[60] = {0};
        b8   badHistory[60] = {0};
        
#define MULTI_SAMPLE 0
        
        GFXErrorBlock(scratch, 1, .callback = GFXErrorFmt)
        {
            vert_vshader = OGL_MakeShader(scratch, glsl_vert_vshader, GL_VERTEX_SHADER);
            if (vert_vshader.log.size > 0)
                ErrorFmt("VERTEX:\n%.*s", StrExpand(vert_vshader.log));
            
            vert_fshader = OGL_MakeShader(scratch, glsl_vert_fshader, GL_FRAGMENT_SHADER);
            if (vert_fshader.log.size > 0)
                ErrorFmt("FRAGMENT:\n%.*s", StrExpand(vert_fshader.log));
            
            vert_program = OGL_MakeProgram(scratch, ArrayExpand(OGL_Shader, vert_vshader, vert_fshader));
            if (vert_program.log.size > 0)
                ErrorFmt("Program:\n%.*s", StrExpand(vert_program.log));
            
            vert_viewTransform = glGetUniformLocation(vert_program.handle, "u_view_xform");
            
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
        
        for (TempArena temp = TempBegin(scratch); ; TempEnd(temp))
        {
            if (!GFXPeekInput())
                break;
            if (!GFXWindowIsValid(window))
                break;
            
            lineX += 0.05f;
            markHistory[frameIdx % ArrayCount(markHistory)] = frameBegin;
            
            v2i32 screenDim = {0};
            v2f32 windowDim = {0};
            if (GFXWindowGetInnerRect(window, 0, 0, &screenDim.x, &screenDim.y))
                windowDim = V2F32V(screenDim);
            
            DeferBlock(R_Begin(window), R_End())
            {
#if 0
                R_QuadList list = {0};
                
                R_QuadPush(scratch, &list, &(R_Quad){
                               R2F32P(200.f, 200.f, 300.f, 300.f), (r2f32){0},
                               10.f, 5.f, 1.f,
                               V4F32(1.f, .2f, 0.f, 1.f), V4F32(1.f, 0.f, .2f, 1.f)
                           });
                
                R_QuadPush(scratch, &list, &(R_Quad){
                               R2F32P(100.f, 400.f, 400.f, 500.f), (r2f32){0},
                               20.f, 10000.f, 1.f,
                               V4F32(1.f, 1.f, 1.f, 1.f), V4F32(0.f, 0.f, 0.f, 1.f)
                           });
                
                R_QuadPush(scratch, &list, &(R_Quad){
                               R2F32Size(V2F32(600.f, 400.f), V2F32V(fonts[0].baked.size)), R2F32P(0.f, 0.f, 1.f, 1.f),
                               0.f, 10000.f, 1.f,
                               V4F32(1.f, 1.f, 1.f, 1.f), V4F32(1.f, 1.f, 1.f, 1.f)
                           });
                
                R_Submit(list.first, list.totalCount, fonts[0].texture);
                
                DeferBlock(R_CtxBegin(), R_CtxEnd())
                {
                    v4f32 c0 = V4F32(0.9f, 0.1f, 0.0f, 1.0f);
                    v4f32 c1 = V4F32(0.8f, 0.0f, 0.0f, 1.0f);
                    v4f32 c2 = V4F32(0.1f, 0.9f, 0.0f, 1.0f);
                    v4f32 c3 = V4F32(0.0f, 0.9f, 0.1f, 1.0f);
                    
                    R_CtxPushLine(lineX, R1F32(300, 400), V4F32(1, 1, 1, 1));
                    
                    R_CtxPushRect(.xy = R2F32P( 5,  5, 45, 45), .radius = 5.f, .c0 = c0, .c1 = c1);
                    R_CtxPushRect(.xy = R2F32P(55,  5, 95, 45), .radius = 5.f, .c0 = c0, .c1 = c1);
                    R_CtxPushRect(.xy = R2F32P( 5, 50, 45, 70), .radius = 0.f, .c0 = c2, .c1 = c3);
                    R_CtxPushRect(.xy = R2F32P(55, 50, 95, 70), .radius = 0.f, .c0 = c3, .c1 = c2);
                    R_CtxPushStr(&fonts[1], StrLit("The quick brown fox jumps over the lazy dog."),
                                 V2F32(25.f, 50.f), V4F32(1.0f, 1.0f, 0.5f, 1.0f));
                    
                    R_CtxPushRect(.xy = R2F32P( 5, 105, 45, 145), .radius = 5.f, .c0 = c0, .c1 = c1);
                    R_CtxPushRect(.xy = R2F32P(55, 105, 95, 145), .radius = 5.f, .c0 = c0, .c1 = c1);
                    R_CtxPushRect(.xy = R2F32P( 5, 150, 45, 170), .radius = 0.f, .c0 = c2, .c1 = c1);
                    R_CtxPushRect(.xy = R2F32P(55, 150, 95, 170), .radius = 0.f, .c0 = c1, .c1 = c2);
                    R_CtxPushStr(&fonts[2], StrLit("Hello, world!"),
                                 V2F32(25.f, 150.f), V4F32(1.0f, 1.0f, 0.5f, 1.0f));
                    
                    R_CtxPushChar(&fonts[ArrayCount(fonts)-1], ICON_FOLDER, V2F32(25, 250), V4F32(.8f, 0.f, .4f, 1.f));
                    R_CtxPushChar(&fonts[ArrayCount(fonts)-1], ICON_CANCEL, V2F32(50, 250), V4F32(.8f, 0.f, .4f, 1.f));
                    
                    String frameStr = StrPushf(scratch, "%llu", prevFrame);
                    R_CtxPushStr(&fonts[0], frameStr, V2F32(600, 50), V4F32(1.f, 1.f, 0.f, 1.f));
                    
                    r1f32 xRange = R1F32(200, 700);
                    r1f32  yMark = R1F32(100, 110);
                    r1f32   yEnd = R1F32(110, 120);
                    
                    u64* markPtr = markHistory;
                    u64*  endPtr =  endHistory;
                    b8*   badPtr =  badHistory;
                    
                    for (u64 i = 0; i < ArrayCount(markHistory); ++i, ++markPtr, ++endPtr, ++badPtr)
                    {
                        f32 markT = DivF32(((*markPtr)%markLoop), markLoop);
                        f32  endT = DivF32(((* endPtr)%markLoop), markLoop);
                        
                        f32 xMark = Lerp(xRange.min, xRange.max, markT);
                        f32  xEnd = Lerp(xRange.min, xRange.max,  endT);
                        
                        v4f32 color = colors[i % ArrayCount(colors)];
                        R_CtxPushLine(xMark, yMark, color);
                        R_CtxPushLine( xEnd,  yEnd, color);
                        
                        if (*badPtr)
                            R_CtxPushRect(.xy = R2F32P(xMark-2, yMark.min-2, xMark+3, yMark.min+3),
                                          .radius = 0.5f, .c0 = V4F32(1, 0, 0, 1), .c1 = V4F32(1, 0, 0, 1));
                    }
                }
#endif
                
#if 1
                u64 frameLoop = fps*10;
                f32 t = DivF32(frameIdx % frameLoop, frameLoop);
                f32 tPrev = 0;
                if (frameIdx > 0)
                    tPrev = DivF32((frameIdx-1) % frameLoop, frameLoop);
                
                v4f32 c0 = V4F32(1.f, 1.f, 1.f, 1.f);
                f32 thick = 1.f;
                f32 xMoving = 5.f + Sin_f32(t*TAU_F32)*4.f;
                f32 xMovingPrev = 5.f + Sin_f32(tPrev*TAU_F32)*4.f;
                f32 xVel = xMoving - xMovingPrev;
                r1f32 y1 = R1F32(-6.f, 4.f);
                r1f32 y2 = R1F32(5.f, 15.f);
                
                f32 patterns[4*5] = {
                    xMoving, xVel, y1.min, y1.max,
                    1.0000f, 0.0f, y2.min, y2.max,
                    3.2500f, 0.0f, y2.min, y2.max,
                    5.5000f, 0.0f, y2.min, y2.max,
                    7.7500f, 0.0f, y2.min, y2.max,
                };
                
                Rect testRects[5] = {0};
                for (u64 i = 0; i < ArrayCount(testRects); ++i)
                {
                    f32 x = patterns[4*i + 0];
                    testRects[i].vel = V2F32(patterns[4*i + 1], 0);
                    testRects[i].p0 = V2F32(x        , patterns[4*i + 2]);
                    testRects[i].p1 = V2F32(x + thick, patterns[4*i + 3]);
                    testRects[i].c  = c0;
                }
                
                // draw "test" geometry
                {
#if MULTI_SAMPLE
                    glBindFramebuffer(GL_FRAMEBUFFER, msaaFbuffer);
                    glClearColor(0.f, 0.f, 0.f, 1.f);
                    glClear(GL_COLOR_BUFFER_BIT);
#endif
                    
                    Rect shiftedRects[ArrayCount(testRects)] = {0};
                    for (u64 i = 0; i < ArrayCount(shiftedRects); ++i)
                    {
                        shiftedRects[i].p0  = AddV2F32(testRects[i].p0, V2F32(100, 100));
                        shiftedRects[i].p1  = AddV2F32(testRects[i].p1, V2F32(100, 100));
                        shiftedRects[i].vel = testRects[i].vel;
                        shiftedRects[i].c   = testRects[i].c;
                    }
                    DrawRects(shiftedRects, ArrayCount(shiftedRects), windowDim);
                    
#if MULTI_SAMPLE
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFbuffer);
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
                    
                    glBlitFramebuffer(0, 0, screenDim.x, screenDim.y, 0, 0, screenDim.x, screenDim.y,
                                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
                }
#endif
                
#if 0
                DrawGeometry(ArrayExpand(Vertex,
                                         { AddV2F32(V2F32( 0.f,  0.f), V2F32(100, 100)), V4F32(1.f, 1.f, 1.f, 1.f), },
                                         { AddV2F32(V2F32( 0.f, 10.f), V2F32(100, 100)), V4F32(1.f, 1.f, 0.f, 1.f), },
                                         { AddV2F32(V2F32(10.f,  0.f), V2F32(100, 100)), V4F32(1.f, 0.f, 0.f, 1.f), },
                                         { AddV2F32(V2F32( 0.f, 10.f), V2F32(100, 100)), V4F32(1.f, 1.f, 0.f, 1.f), },
                                         { AddV2F32(V2F32(10.f,  0.f), V2F32(100, 100)), V4F32(1.f, 0.f, 0.f, 1.f), },
                                         { AddV2F32(V2F32(10.f, 10.f), V2F32(100, 100)), V4F32(0.f, 0.f, 0.f, 1.f), }),
                             windowDim);
#endif
                
                // read 10x10 block
                u8 buf[10*10*4];
                glReadPixels(100, 100, 10, 10, GL_RGBA, GL_UNSIGNED_BYTE, buf);
                
                {
                    v2f32 botleft = V2F32(600.f, 100.f);
                    f32 size = 20.f;
                    
                    // outline
                    {
                        f32 outlineSize = 10.f*size + 2.f;
                        v2f32 p00 = SubV2F32(botleft, V2F32(2.f, 2.f));
                        v2f32 p11 = AddV2F32(botleft, V2F32(outlineSize, outlineSize));
                        DrawRects(&(Rect){ p00, p11, .c = V4F32(1, 1, 1, 1) }, 1, windowDim);
                    }
                    
                    // main grid
                    for (u32 y = 0; y < 10; ++y)
                    {
                        for (u32 x = 0; x < 10; ++x)
                        {
                            v2f32 p00 = AddV2F32(botleft, V2F32(x*size, y*size));
                            v2f32 p01 = AddV2F32(p00, V2F32(1*size, 0.f));
                            v2f32 p10 = AddV2F32(p00, V2F32(0.f, size));
                            v2f32 p11 = V2F32(p01.x, p10.y);
                            
                            u8* colorPtr = buf + 4*(x + 10*y);
                            v4f32 color = V4F32((f32)colorPtr[0]/255.f, (f32)colorPtr[1]/255.f,
                                                (f32)colorPtr[2]/255.f, (f32)colorPtr[3]/255.f);
                            DrawRects(&(Rect){ p00, p11, .c = color }, 1, windowDim);
                        }
                    }
                    
                    // "fine" geometry
                    Rect fineRects[ArrayCount(testRects)];
                    for (u64 i = 0; i < ArrayCount(fineRects); ++i)
                    {
                        fineRects[i].p0 = AddV2F32(botleft, ScaleV2F32(testRects[i].p0, size));
                        fineRects[i].p1 = AddV2F32(botleft, ScaleV2F32(testRects[i].p1, size));
                        fineRects[i].c  = V4F32(1.f, 0.f, 1.f, .25f);
                    }
                    
                    DrawRects(fineRects, ArrayCount(fineRects), windowDim);
                }
            }
            
            {
                u64 targetEnd = frameBegin + delta;
                u64 now = OSNowUS();
                endHistory[frameIdx % ArrayCount(endHistory)] = now;
                
                if (now < targetEnd)
                {
                    u64 leftover = targetEnd - now;
                    Sleep((DWORD)(leftover / 1000));
                    now = OSNowUS();
                }
                
                prevFrame = now - frameBegin;
                
                b8 good = targetEnd - now < delta/16;
                if (good)
                    frameBegin = targetEnd;
                else
                    frameBegin = now;
                
                badHistory[frameIdx % ArrayCount(badHistory)] = !good;
                ++frameIdx;
            }
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
