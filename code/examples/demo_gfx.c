#include "Base.h"
#include "Base.c"

#include "LongGFX.h"
#include "LongGFX_Win32.c"
#include "LongGFX_OpenGL.h"
#include "Win32_OpenGL.c"
#include "LongGFX_D3D11.h"
#include "Win32_D3D11.c"

enum
{
    Renderer_GL,
    Renderer_D3D,
};

function void WindowResizeHandler(GFXWindow window, u32 width, u32 height)
{
    UNUSED(width);
    UNUSED(height);
    
    u32 renderer = (u32)IntFromPtr(GFXWindowGetUserData(window));
    switch (renderer)
    {
        case Renderer_GL:
        {
            BeginGLRender(window);
            
            GFXFlags flags = GFXWindowGetFlags(window);
            if (flags & FLAG_WINDOW_MAXIMIZED)
                glClearColor(0.f, 1.f, 1.f, 1.f);
            else if (flags & FLAG_WINDOW_MINIMIZED)
                glClearColor(1.f, 0.f, 1.f, 1.f);
            else if (flags & FLAG_MODE_FULLSCREEN)
                glClearColor(1.f, 1.f, 0.f, 1.f);
            else if (window % 2 == 0)
                glClearColor(0, 0, 0, 1);
            else
                glClearColor(1.0f, 1.0f, 1.0f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            GL_Float flts[4];
            glGetFloatv(GL_VIEWPORT, flts);
            
#if 0
            glBegin(GL_TRIANGLES);
            glColor3f (1, 1, 1);
            glVertex3f(-1, -1, 0);
            glVertex3f(1, -1, 0);
            glVertex3f(0, 1, 0);
            glEnd();
#endif
            
            EndGLRender();
        } break;
        
        case Renderer_D3D:
        {
            ID3D11RenderTargetView* view = BeginD3D11Render(window);
            ID3D11DeviceContext_ClearRenderTargetView(ctx, view, MidnightBlue);
            ID3D11DeviceContext_OMSetRenderTargets(ctx, 1, &view, 0);
            EndD3D11Render(view);
        } break;
    }
}

#define TEST_WINDOW_COUNT 1

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
            InitGL();
            //InitD3D11();
            
#if 0
            ErrorFmt("Test Error 1");
            ErrorFmt("Test Error 2");
#endif
        }
        
        //GFXSetResizeFunc(WindowResizeHandler);
        
        GFXWindow windows[TEST_WINDOW_COUNT] = {0};
        for (u32 i = 0; i < TEST_WINDOW_COUNT; ++i)
        {
            i32 w = 1200, h = 800;
            i32 isGL = /*i % 2*/1;
            
            w = w * (i + 1);
            h = h * (i + 1);
            
            GFXErrorBlock(scratch, 1)
            {
                windows[i] = GFXCreateWindowEx(StrPushf(scratch, "Window: %u", i), CW_USEDEFAULT, CW_USEDEFAULT, w, h);
                if (isGL)
                    EquipGLWindow(windows[i]);
                else
                    EquipD3D11Window(windows[i]);
            }
            
            GFXWindowSetUserData(windows[i], PtrFromInt(isGL ? Renderer_GL : Renderer_D3D));
            GFXShowWindow(windows[i]);
            
            if (i == 3)
            {
                i32 x, y;
                GFXWindowGetInnerRect(windows[i-1], &x, &y, &w, &h);
                GFXWindowSetOuterRect(windows[i], x, y, w, h);
            }
        }
        
        u32 count = 0;
        for (TempArena temp = TempBegin(scratch); ; TempEnd(temp))
        {
            if (!GFXPeekInput())
                break;
            
            DeferBlock(BeginGLRender(windows[0]), EndGLRender())
            {
                char* glsl_vshader =
                    "#version 330\n"
                    "uniform vec2 u_view_xform;\n"
                    "layout (location = 0) in vec2 v_pos_pattern;\n"
                    "layout (location = 1) in vec4 v_quad;\n"
                    "layout (location = 2) in vec4 v_uv;\n"
                    "layout (location = 3) in float v_radius;\n"
                    "layout (location = 4) in float v_thick;\n"
                    "layout (location = 5) in vec4 v_color0;\n"
                    "layout (location = 6) in vec4 v_color1;\n"
                    "out vec2 f_center;\n"
                    "out vec2 f_extent;\n"
                    "out vec2 f_pos;\n"
                    "out float f_radius;\n"
                    "out float f_thick;\n"
                    "out vec4 f_color0;\n"
                    "out vec4 f_color1;\n"
                    "out float f_pos_pattern_y;\n"
                    "out vec2 f_uv;\n"
                    "void main(){\n"
                    "vec2 center = (v_quad.xy + v_quad.zw)*0.5;\n"
                    "vec2 extent = (v_quad.zw - v_quad.xy)*0.5;\n"
                    "vec2 pos = center + extent * v_pos_pattern;\n"
                    "vec2 norm_pos = pos*u_view_xform + vec2(-1.0, +1.0);\n"
                    "vec2 uv_center = (v_uv.xy + v_uv.zw)*0.5;\n"
                    "vec2 uv_extent = (v_uv.zw - v_uv.xy)*0.5;\n"
                    "gl_Position = vec4(norm_pos, 0.0, 1.0);\n"
                    "f_pos = pos;\n"
                    "f_center = center;\n"
                    "f_extent = extent;\n"
                    "f_radius = v_radius;\n"
                    "f_thick = v_thick;\n"
                    "f_color0 = v_color0;\n"
                    "f_color1 = v_color1;\n"
                    "f_pos_pattern_y = v_pos_pattern.y;\n"
                    "f_uv = uv_center + uv_extent*v_pos_pattern;\n"
                    "}\n";
                
                char* glsl_fshader =
                    "#version 330\n"
                    "uniform sampler2D u_texture;\n"
                    "in vec2 f_center;\n"
                    "in vec2 f_extent;\n"
                    "in vec2 f_pos;\n"
                    "in float f_radius;\n"
                    "in float f_thick;\n"
                    "in vec4 f_color0;\n"
                    "in vec4 f_color1;\n"
                    "in float f_pos_pattern_y;\n"
                    "in vec2 f_uv;\n"
                    "out vec4 out_color;\n"
                    "void main(){\n"
                    // setup params
                    "float r = f_radius;\n"
                    "float thick = f_thick;\n"
                    "float soft = 1.0;\n"
                    // calculate signed distance
                    "vec2 d2 = abs(f_pos - f_center) - f_extent + vec2(r, r) + vec2(soft, soft);\n"
                    "float d_neg =    min(max(d2.x, d2.y), 0);\n"
                    "float d_pos = length(max(d2, vec2(0, 0)));\n"
                    // apply radius
                    "float d = d_neg + d_pos - r;\n"
                    // distance response curve
                    "float half_thick = thick * 0.5;\n"
                    "float d_mir = abs(d + half_thick) - half_thick;\n"
                    "float m = smoothstep(soft, -soft, d_mir);\n"
                    // blend color
                    "float c_t = (f_pos_pattern_y + 1.0) * 0.5;\n"
                    "vec4 c_base = f_color0 + (f_color1 - f_color0) * c_t;\n"
                    // sample texture
                    "float sample = texture(u_texture, f_uv).r;\n"
                    "out_color = vec4(c_base.xyz * sample, c_base.w * m);\n"
                    "}\n";
                
                GL_Enum error = glGetError();
                DEBUG(error);
                
                GL_Shader vshader = GL_MakeShader(scratch, glsl_vshader, GL_VERTEX_SHADER);
                GL_Shader fshader = GL_MakeShader(scratch, glsl_fshader, GL_FRAGMENT_SHADER);
                
                if (vshader.log.size) Outf(  "Vertex Shader:\n%s\n", (char*)vshader.log.str);
                if (fshader.log.size) Outf("Fragment Shader:\n%s\n", (char*)fshader.log.str);
                
                GL_Uint program = 0;
                if (vshader.handle && fshader.handle)
                {
                    GL_Shader glProgram = GL_MakeProgram(scratch, ArrayExpand(GL_Shader, vshader, fshader));
                    if (glProgram.log.size) Outf("Program:\n%s\n", (char*)glProgram.log.str);
                    program = glProgram.handle;
                }
                
                if (program)
                {
                    GL_Uint vao = 0;
                    glGenVertexArrays(1, &vao);
                    glBindVertexArray(vao);
                }
                
                if (program)
                {
                    GL_Uint vertexBuffer = 0;
                    glGenBuffers(1, &vertexBuffer);
                    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
                }
                
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_BLEND);
                
                GL_Uint texture = 0;
                {
                    u8* bitmap = PushArrayNZ(scratch, u8, 1024*1024);
                    
                    for (u32 y = 0; y < 1024; ++y)
                    {
                        for (u32 x = 0; x < 1024; ++x)
                        {
                            u32 p = ((x/40) + (y/40)) % 4;
                            p *= 85;
                            bitmap[x + y*1024] = (u8)p;
                        }
                    }
                    
                    glGenTextures(1, &texture);
                    glBindTexture(GL_TEXTURE_2D, texture);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024, 1024, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
                    
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }
                
                glClearColor(1.f, 0.f, 1.f, 1.f);
                glClear(GL_COLOR_BUFFER_BIT);
                
                f32 data[] = {
                    // [0]: v_pos_pattern
                    -1, +1,
                    +1, +1,
                    -1, -1,
                    +1, +1,
                    -1, -1,
                    +1, -1,
                    
                    // [1]: four quad specifiers
                    200.f, 200.f, 300.f, 300.f,
                    0.f/1024.f, 0.f/1024.f, 100.f/1024.f, 100.f/1024.f,
                    10.f, 5.f,
                    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f,
                    
                    100.f, 400.f, 400.f, 500.f,
                    0.f/1024.f, 0.f/1024.f, 300.f/1024.f, 100.f/1024.f,
                    20.f, 10000.f,
                    1.f, 1.f, 1.f, 1.f, 0.f, 0.f, 0.f, 1.f,
                    
                    600.f, 100.f, 700.f, 300.f,
                    0.f/1024.f, 0.f/1024.f, 100.f/1024.f, 200.f/1024.f,
                    0.f, 10000.f,
                    1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 0.f,
                    
                    600.f, 400.f, 800.f, 700.f,
                    0.f/1024.f, 0.f/1024.f, 200.f/1024.f, 300.f/1024.f,
                    5.f, 10000.f,
                    1.f, .2f, 0.f, 1.f, 1.f, 0.f, .2f, 1.f,
                };
                
                GL_Size quadStride = 18;
                glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STREAM_DRAW);
                glUseProgram(program);
                
                u32 width, height;
                if (GFXWindowGetOuterRect(windows[0], 0, 0, &width, &height))
                {
                    glViewport(0, 0, width, height);
                    GL_Int viewXform = glGetUniformLocation(program, "u_view_xform");
                    glUniform2f(viewXform, 2.f/width, -2.f/height);
                }
                
                GL_Int u_texture = glGetUniformLocation(program, "u_texture");
                glUniform1i(u_texture, 0);
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, texture);
                
                glEnableVertexAttribArray(0);
                glVertexAttribDivisor(0, 0);
                glVertexAttribPointer(0, 2, GL_FLOAT, false, 2*sizeof(f32), 0);
                
                glEnableVertexAttribArray(1);
                glVertexAttribDivisor(1, 1);
                glVertexAttribPointer(1, 4, GL_FLOAT, false, quadStride*sizeof(f32), PtrFromInt((12+0)*sizeof(f32)));
                
                glEnableVertexAttribArray(2);
                glVertexAttribDivisor(2, 1);
                glVertexAttribPointer(2, 4, GL_FLOAT, false, quadStride*sizeof(f32), PtrFromInt((12+4)*sizeof(f32)));
                
                glEnableVertexAttribArray(3);
                glVertexAttribDivisor(3, 1);
                glVertexAttribPointer(3, 1, GL_FLOAT, false, quadStride*sizeof(f32), PtrFromInt((12+8)*sizeof(f32)));
                
                glEnableVertexAttribArray(4);
                glVertexAttribDivisor(4, 1);
                glVertexAttribPointer(4, 1, GL_FLOAT, false, quadStride*sizeof(f32), PtrFromInt((12+9)*sizeof(f32)));
                
                glEnableVertexAttribArray(5);
                glVertexAttribDivisor(5, 1);
                glVertexAttribPointer(5, 4, GL_FLOAT, false, quadStride*sizeof(f32), PtrFromInt((12+10)*sizeof(f32)));
                
                glEnableVertexAttribArray(6);
                glVertexAttribDivisor(6, 1);
                glVertexAttribPointer(6, 4, GL_FLOAT, false, quadStride*sizeof(f32), PtrFromInt((12+14)*sizeof(f32)));
                
                glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 4);
            }
            
            count++;
#define TEST_TOGGLE_TIMER 150
            if (count % TEST_TOGGLE_TIMER == 0)
            {
#define TEST_VISIBILITY 0
#define TEST_OUTERINNER 0
#define TEST_FULLSCREEN 0
                
#define TEST_RESIZABLE 0
#define TEST_UNFOCUSED 0
#define TEST_MINMAX    0
                
                if (TEST_VISIBILITY)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-1];
                    if (GFXWindowIsValid(window))
                    {
                        b32 visible = count % (TEST_TOGGLE_TIMER * 2);
                        GFXWindowSetFlags(window, FLAG_WINDOW_HIDDEN, !visible);
                        if (visible)
                            GFXWindowSetTitle(window, StrPushf(scratch, "Toggle Windows: %u", count / 100));
                        visible = !visible;
                    }
                }
                
                if (TEST_OUTERINNER)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-2];
                    if (GFXWindowIsValid(window))
                    {
                        i32 x, y, w, h;
                        if (count % (TEST_TOGGLE_TIMER * 2))
                        {
                            i32 innerX, innerY, innerW, innerH;
                            
                            GFXWindowGetOuterRect(window, &x, &y, &w, &h);
                            GFXWindowGetInnerRect(window, &innerX, &innerY, &innerW, &innerH);
                            
                            GFXWindowSetInnerRect(window, x, y, w, h);
                            GFXWindowGetInnerRect(window, &innerX, &innerY, &innerW, &innerH);
                        }
                        else
                        {
                            GFXWindowGetInnerRect(window, &x, &y, &w, &h);
                            GFXWindowSetOuterRect(window, x, y, w, h);
                        }
                    }
                }
                
                if (TEST_FULLSCREEN)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-3];
                    if (GFXWindowIsValid(window))
                    {
                        if (count % (TEST_TOGGLE_TIMER * 2))
                        {
                            GFXWindowSetFlags(window, FLAG_WINDOW_MAXIMIZED, 0);
                            GFXWindowSetFlags(window, FLAG_MODE_FULLSCREEN, 1);
                        }
                        else
                        {
                            GFXWindowSetFlags(window, FLAG_MODE_FULLSCREEN, 0);
                            GFXWindowSetFlags(window, FLAG_WINDOW_MAXIMIZED, 1);
                        }
                    }
                }
                
                if (TEST_RESIZABLE)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-4];
                    if (GFXWindowIsValid(window))
                        GFXWindowSetFlags(window, FLAG_WINDOW_RESIZABLE, count % (TEST_TOGGLE_TIMER * 2));
                }
                
                if (TEST_MINMAX)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-5];
                    if (GFXWindowIsValid(window))
                    {
                        u32 loop = (count % (TEST_TOGGLE_TIMER * 4)) / TEST_TOGGLE_TIMER;
                        switch (loop)
                        {
                            case 2:
                            case 0: GFXWindowSetFlags(window, FLAG_WINDOW_MINIMIZED|FLAG_WINDOW_MAXIMIZED, 0); break;
                            case 1: GFXWindowSetFlags(window, FLAG_WINDOW_MAXIMIZED, 1); break;
                            case 3: GFXWindowSetFlags(window, FLAG_WINDOW_MINIMIZED, 1); break;
                        }
                    }
                }
                
                if (TEST_UNFOCUSED)
                {
                    GFXWindow window = windows[TEST_WINDOW_COUNT-6];
                    if (GFXWindowIsValid(window))
                        GFXWindowSetFlags(window, FLAG_WINDOW_UNFOCUSED, count % (TEST_TOGGLE_TIMER * 2) == 0);
                }
            }
            
            u32 activeWindowCount = 0;
            for (u32 i = 0; i < TEST_WINDOW_COUNT; ++i)
            {
                if (GFXWindowIsValid(windows[i]))
                {
                    activeWindowCount++;
                    //WindowResizeHandler(windows[i], 0, 0);
                }
            }
            
            if (!activeWindowCount)
                break;
        }
        
        GFXErrorBlock(scratch, 1)
        {
            //FreeD3D11();
            FreeGL();
            InitGL();
            FreeGL();
        }
    }
    
    return 0;
}
