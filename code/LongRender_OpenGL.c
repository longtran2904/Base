
//~ long: OpenGL Types

typedef struct OGL_Renderer OGL_Renderer;
struct OGL_Renderer
{
    GLuint program;
    GLint viewTransform;
    GLint mainTexture;
    
    GLuint vertexBuffer;
    GLuint fallbackTex;
    
    v2i32 dim;
};

global OGL_Renderer oglRenderer;

//~ long: GLSL shaders

global char* glsl_vshader =
"#version 330\n"
"uniform vec2 u_view_xform;\n"
"layout (location = 0) in vec2 v_pos_pattern;\n"
"layout (location = 1) in vec4 v_quad;\n"
"layout (location = 2) in vec4 v_uv;\n"
"layout (location = 3) in float v_radius;\n"
"layout (location = 4) in float v_thick;\n"
"layout (location = 5) in float v_soft;\n"
"layout (location = 6) in vec4 v_color0;\n"
"layout (location = 7) in vec4 v_color1;\n"
"flat out vec2 f_center;\n"
"flat out vec2 f_extent;\n"
"flat out float f_radius;\n"
"flat out float f_thick;\n"
"flat out float f_soft;\n"
"flat out vec4 f_color0;\n"
"flat out vec4 f_color1;\n"
"flat out float f_override_sample;\n"
"out float f_pos_pattern_y;\n"
"out vec2 f_uv;\n"
"void main(){\n"
// compute normalized pos
"vec2 center = (v_quad.xy + v_quad.zw)*0.5;\n"
"vec2 extent = (v_quad.zw - v_quad.xy)*0.5;\n"
"vec2 pos = center + extent * v_pos_pattern;\n"
"vec2 norm_pos = pos*u_view_xform + vec2(-1.0, +1.0);\n"
// compute uv coords
"vec2 uv_center = (v_uv.xy + v_uv.zw)*0.5;\n"
"vec2 uv_extent = (v_uv.zw - v_uv.xy)*0.5;\n"
"vec2 uv = uv_center + uv_extent*v_pos_pattern;\n"
"float override_sample = 0;\n"
"if (v_uv.z == 0) { override_sample = 1.0; }\n"
// fill outputs
"gl_Position = vec4(norm_pos, 0.0, 1.0);\n"
"f_center = center;\n"
"f_extent = extent;\n"
"f_radius = v_radius;\n"
"f_thick = v_thick;\n"
"f_soft = v_soft;\n"
"f_color0 = v_color0;\n"
"f_color1 = v_color1;\n"
"f_override_sample = override_sample;\n"
"f_pos_pattern_y = v_pos_pattern.y;\n"
"f_uv = uv;\n"
"}\n";

global char* glsl_fshader =
"#version 330\n"
"uniform sampler2D u_texture;\n"
"flat in vec2 f_center;\n"
"flat in vec2 f_extent;\n"
"flat in float f_radius;\n"
"flat in float f_thick;\n"
"flat in float f_soft;\n"
"flat in vec4 f_color0;\n"
"flat in vec4 f_color1;\n"
"flat in float f_override_sample;\n"
"layout(origin_upper_left) in vec4 gl_FragCoord;\n"
"in float f_pos_pattern_y;\n"
"in vec2 f_uv;\n"
"out vec4 out_color;\n"
"void main(){\n"
// setup pos
"vec2 pos = gl_FragCoord.xy;\n"
// setup params
"float r = f_radius;\n"
"float thick = f_thick;\n"
"float soft = f_soft;\n"
// calculate signed distance
"vec2 d2 = abs(pos - f_center) - f_extent + vec2(r, r) + vec2(soft, soft);\n"
"float d_neg =    min(max(d2.x, d2.y), 0);\n"
"float d_pos = length(max(d2, vec2(0, 0)));\n"
// apply radius
"float d = d_neg + d_pos - r;\n"
// distance response curve
"float half_thick = thick * 0.5;\n"
"float d_mir = abs(d + half_thick) - half_thick;\n"
"float epsilon = 0.01;\n"
"float m = smoothstep(soft + epsilon, -soft - epsilon, d_mir);\n"
// blend color
"float c_t = (f_pos_pattern_y + 1.0) * 0.5;\n"
"vec4 c_base = f_color0 + (f_color1 - f_color0) * c_t;\n"
// sample texture
"float sample = texture(u_texture, f_uv).r;\n"
"sample = max(sample, f_override_sample);\n"
"out_color = vec4(c_base.xyz * sample, c_base.w * m);\n"
"}\n";

//~ long: OpenGL Functions

function void R_Init(void)
{
    ScratchBegin(scratch);
    
    w32RenderWnd = w32CoreWnd;
    w32RenderDC = GetDC(w32RenderWnd);
    w32WglMakeCurrent(w32RenderDC, w32OpenGLContext);
    
    OGL_Shader vshader = OGL_MakeShader(scratch, glsl_vshader, GL_VERTEX_SHADER);
    OGL_Shader fshader = OGL_MakeShader(scratch, glsl_fshader, GL_FRAGMENT_SHADER);
    
    if (vshader.log.size) Outf(  "Vertex Shader:\n%s\n", (char*)vshader.log.str);
    if (fshader.log.size) Outf("Fragment Shader:\n%s\n", (char*)fshader.log.str);
    
    OGL_Shader program = OGL_MakeProgram(scratch, ArrayExpand(OGL_Shader, vshader, fshader));
    if (program.log.size) Outf("Program:\n%s\n", (char*)program.log.str);
    
    GLint viewTransform = glGetUniformLocation(program.handle, "u_view_xform");
    GLint   mainTexture = glGetUniformLocation(program.handle, "u_texture");
    
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint vertexBuffer = 0;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    GLuint texture = 0;
    {
        u8* bitmap = PushArray(scratch, u8, 16);
        for (u32 i = 0; i < 16; ++i)
            bitmap[i] = 0xFF;
        
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    oglRenderer.program = program.handle;
    oglRenderer.viewTransform = viewTransform;
    oglRenderer.mainTexture = mainTexture;
    
    oglRenderer.vertexBuffer = vertexBuffer;
    oglRenderer.fallbackTex = texture;
    
    //OGL_Free();
    ScratchEnd(scratch);
}

function void R_Begin(GFXWindow window)
{
    OGL_Begin(window);
    
    u32 w, h;
    if (GFXWindowGetInnerRect(window, 0, 0, &w, &h))
    {
        glViewport(0, 0, w, h);
        oglRenderer.dim = V2I32(w, h);
    }
    
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

function void R_End(void)
{
    OGL_End();
}

function void R_Submit(R_QuadNode* first, u64 count, R_Texture* texturePtr)
{
    // resolve texture
    GLuint texture = (GLuint)IntFromPtr(texturePtr);
    if (!glIsTexture(texture))
        texture = oglRenderer.fallbackTex;
    
    // set buffer size
    GLintptr cursor = 0;
    u64 size = sizeof(v2f32)*6 + sizeof(R_Quad)*count;
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_STREAM_DRAW);
    
    // set 6 points of a quad
    local v2f32 quadTriangles[6] = {
        {-1.f, +1.f}, {+1.f, +1.f}, {-1.f, -1.f},
        {+1.f, +1.f}, {-1.f, -1.f}, {+1.f, -1.f},
    };
    
    GLintptr triaOffset = cursor;
    glBufferSubData(GL_ARRAY_BUFFER, cursor, sizeof(quadTriangles), quadTriangles);
    cursor += sizeof(quadTriangles);
    
    // set quad data
    GLintptr quadOffset = cursor;
    for (R_QuadNode* node = first; node; node = node->next)
    {
        u64 nodeSize = node->count * sizeof(R_Quad);
        glBufferSubData(GL_ARRAY_BUFFER, cursor, nodeSize, node->quads);
        cursor += nodeSize;
    }
    
    // call GPU program
    glUseProgram(oglRenderer.program);
    glUniform2f(oglRenderer.viewTransform, 2.f/(f32)oglRenderer.dim.x, -2.f/(f32)oglRenderer.dim.y);
    glUniform1i(oglRenderer.mainTexture, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glEnableVertexAttribArray(0);
    glVertexAttribDivisor(0, 0);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(v2f32), PtrFromInt(triaOffset));
    
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, xy)));
    
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);
    glVertexAttribPointer(2, 4, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, uv)));
    
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, radius)));
    
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
    glVertexAttribPointer(4, 1, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, thickness)));
    
    glEnableVertexAttribArray(5);
    glVertexAttribDivisor(5, 1);
    glVertexAttribPointer(5, 1, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, softness)));
    
    glEnableVertexAttribArray(6);
    glVertexAttribDivisor(6, 1);
    glVertexAttribPointer(6, 4, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, c0)));
    
    glEnableVertexAttribArray(7);
    glVertexAttribDivisor(7, 1);
    glVertexAttribPointer(7, 4, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, c1)));
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)count);
}

function R_Texture* R_TextureCreate(u32 w, u32 h, void* data)
{
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    return PtrFromInt(texture);
}

function void R_TextureUpdate(R_Texture* texturePtr, r2i32 rect, void* data)
{
    GLuint texture = (GLuint)IntFromPtr(texturePtr);
    if (glIsTexture(texture))
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        v2i32 size = SubV2I32(rect.p1, rect.p0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x0, rect.y0, size.x, size.y, GL_RED, GL_UNSIGNED_BYTE, data);
    }
}

function void R_TextureDestroy(R_Texture* texturePtr)
{
    GLuint texture = (GLuint)IntFromPtr(texturePtr);
    if (glIsTexture(texture))
        glDeleteTextures(1, &texture);
}

function b32 R_TextureValid(R_Texture* texturePtr)
{
    GLuint texture = (GLuint)IntFromPtr(texturePtr);
    b32 result = glIsTexture(texture);
    return result;
}
