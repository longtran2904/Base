
//~ long: OpenGL Types

typedef struct OGL_Renderer OGL_Renderer;
struct OGL_Renderer
{
    GLuint program;
    GLint uTransform;
    GLint uTexture;
    
    GLuint vao;
    GLuint vbo;
    GLuint fbo;
    
    GLuint   canvasTex;
    GLuint fallbackTex;
    
    v2i32 dim;
};

global OGL_Renderer oglRenderer;

//~ long: GLSL shaders

#define GLSL_UNPACK_COLOR(u32c, c0, c1, c2, c3) \
    "float " c0 " = ((" u32c " >>  0) & 0xFFu) / 255.0;\n"   \
    "float " c1 " = ((" u32c " >>  8) & 0xFFu) / 255.0;\n"   \
    "float " c2 " = ((" u32c " >> 16) & 0xFFu) / 255.0;\n"   \
    "float " c3 " = ((" u32c " >> 24) & 0xFFu) / 255.0;\n"

// NOTE(long): Uniform variables are set globally using glUniform* functions
// Attributes with 'layout(location = x)' are fed using glVertexAttribPointer
// Vertex shader outputs and fragment shader inputs are linked by matching names and types
// By default, each fragment input is interpolated from the corresponding vertex outputs
// You can disable interpolation with the 'flat' qualifier, which may improve performance
// Usually, you want to keep xy/uv non-flat so they get interpolated across primitives
// But I mark them flat and interpolate manually in the fragment shader
// I also manually convert screen-based rects to device coordinates using u_view_xform

global char* glsl_vshader =
"#version 330\n"
"uniform vec2 u_view_xform;\n"
"layout (location = 0) in vec2  v_template_p;\n"
"layout (location = 1) in vec4  v_rect;\n"
"layout (location = 2) in float v_radius;\n"
"layout (location = 3) in float v_thick;\n"
"layout (location = 4) in float v_theta;\n"
"layout (location = 5) in uint  v_c0;\n"
"layout (location = 6) in uint  v_c1;\n"
"layout (location = 7) in uint  v_flags;\n"
"layout (location = 8) in vec4  v_uv;\n"
"layout (location = 9) in vec4  v_clip;\n"

//- declare outputs
"flat out vec2  f_center;\n"
"flat out vec2  f_extents;\n"
"flat out float f_radius;\n"
"flat out float f_thick;\n"
"flat out float f_theta;\n"
"flat out vec4  f_c0;\n"
"flat out vec4  f_c1;\n"
"flat out uint  f_flags;\n"
"flat out vec4  f_uv;\n"
"flat out vec4  f_clip;\n"

"void main(){\n"
//- setup parameters
"vec2  center = (v_rect.zw + v_rect.xy) * 0.5;\n"
"vec2 extents = (v_rect.zw - v_rect.xy) * 0.5;\n"
"float stheta = sin(v_theta);\n"
"float ctheta = cos(v_theta);\n"
GLSL_UNPACK_COLOR("v_c0", "c00", "c01", "c02", "c03")
GLSL_UNPACK_COLOR("v_c1", "c10", "c11", "c12", "c13")

//- setup position
"vec2 q = center + extents * v_template_p;\n"
"if (v_template_p.x < 0) q.x = floor(q.x);\n"
"else                    q.x =  ceil(q.x);\n"
"if (v_template_p.y < 0) q.y = floor(q.y);\n"
"else                    q.y =  ceil(q.y);\n"
"vec2 qr = q - center;\n"
"vec2 pr = vec2(ctheta*qr.x - stheta*qr.y, stheta*qr.x + ctheta*qr.y);\n"
"vec2 p  = center + pr;\n"
"vec2 np = p * u_view_xform + vec2(-1.0, -1.0);\n"

//- fill outputs
"gl_Position = vec4(np, 0.0, 1.0);\n"
"f_center    = center;\n"
"f_extents   = extents;\n"
"f_radius    = v_radius;\n"
"f_thick     = v_thick;\n"
"f_theta     = v_theta;\n"
"f_c0        = vec4(c00, c01, c02, c03);\n"
"f_c1        = vec4(c10, c11, c12, c13);\n"
"f_flags     = v_flags;\n"
"f_uv        = v_uv;\n"
"f_clip      = v_clip;\n"
"}\n";

global char* glsl_fshader =
"#version 330\n"
"uniform sampler2D u_tex;\n"
"flat in vec2  f_center;\n"
"flat in vec2  f_extents;\n"
"flat in float f_radius;\n"
"flat in float f_thick;\n"
"flat in float f_theta;\n"
"flat in vec4  f_c0;\n"
"flat in vec4  f_c1;\n"
"flat in uint  f_flags;\n"
"flat in vec4  f_uv;\n"
"flat in vec4  f_clip;\n"
"in vec4 gl_FragCoord;\n"
"out vec4 out_color;\n"

"void main(){\n"
//- discard fragments outside clip rect
"if (((f_flags & 0x20u) != 0u) &&\n"
"    ((gl_FragCoord.x <  f_clip.x) ||\n"
"     (gl_FragCoord.y <  f_clip.y) ||\n"
"     (gl_FragCoord.x >= f_clip.z) ||\n"
"     (gl_FragCoord.y >= f_clip.w))) { discard; }\n"

//- apply rotation
"vec2 q = gl_FragCoord.xy - f_center;\n"
"float stheta = sin(f_theta);\n"
"float ctheta = cos(f_theta);\n"
"vec2 p = vec2(ctheta*q.x + stheta*q.y, -stheta*q.x + ctheta*q.y);\n"

//- clamp radius
"float half_short_side = min(f_extents.x, f_extents.y);\n"
"float rad = min(f_radius, half_short_side);\n"

//- modify radius for quadrant
"uint quadrant = uint(p.x < 0) + 2u * uint(p.y < 0);\n"
"if ((f_flags & (1u << quadrant)) != 0u) rad = 0;\n"

//- setup rectangle
"float r_b = -f_extents.y + rad;\n"
"float r_t = -r_b;\n"
"float r_l = -f_extents.x + rad;\n"
"float r_r = -r_l;\n"

//- calculate distance
"float r_in_x = max(r_l - p.x, p.x - r_r);\n"
"float r_in_y = max(r_b - p.y, p.y - r_t);\n"
"float r_in_max = max(r_in_x, r_in_y);\n"
"float r_in_dist = min(0, r_in_max);\n"

"vec2 r_ex_np = vec2(clamp(p.x, r_l, r_r), clamp(p.y, r_b, r_t));\n"
"vec2 r_ex_d  = p - r_ex_np;\n"
"float r_ex_dist = sqrt(r_ex_d.x*r_ex_d.x + r_ex_d.y*r_ex_d.y);\n"

"float dist = r_ex_dist + r_in_dist - rad;\n"
"float half_thick = f_thick*0.5;\n"
"if (half_thick > 0) dist = abs(dist + half_thick) - half_thick;\n"

//- calculate alpha
"float sdf_a_unclamped = (0.5 - dist);\n"
"float sdf_a = clamp(sdf_a_unclamped, 0, 1);\n"

//- texture sampling
// TODO(long): Sample all channels instead of using only the red channel as alpha
"float uv_xtu = (p.x + f_extents.x) / (2*f_extents.x);\n"
"float uv_ytu = (p.y + f_extents.y) / (2*f_extents.y);\n"
"float s = 1;\n"
"if (f_uv.z > 0){\n"
"  ivec2 texdim = textureSize(u_tex, 0);\n"
"  vec2 htex = vec2(0.5/texdim.x, 0.5/texdim.y);\n"
"  float uv_xu = f_uv.x + (f_uv.z - f_uv.x)*uv_xtu;\n"
"  float uv_yu = f_uv.y + (f_uv.w - f_uv.y)*uv_ytu;\n"
"  float uv_x = clamp(uv_xu, f_uv.x + htex.x, f_uv.z - htex.x);\n"
"  float uv_y = clamp(uv_yu, f_uv.y + htex.y, f_uv.w - htex.y);\n"
"  s = texture(u_tex, vec2(uv_x, uv_y)).r;\n"
"}\n"

//- color interpolation
"float color_tu = uv_ytu;\n"
"if ((f_flags&0x10u) != 0u) color_tu = uv_xtu;\n"
"float color_t = clamp(color_tu, 0, 1);\n"
"vec4 color = f_c0 + (f_c1 - f_c0)*color_t;\n"

//- final color
"out_color = vec4(color.rgb, sdf_a*color.a*s);\n"
"}\n";

//~ long: OpenGL Functions

function void R_Init(void)
{
    ScratchBegin(scratch);
    b32 error = 0;
    
    //- long: Make the "shared" dummy context current
    w32RenderWnd = w32CoreWnd;
    w32RenderDC = GetDC(w32RenderWnd);
    w32WglMakeCurrent(w32RenderDC, w32OpenGLContext);
    
    //- long: Shaders
    OGL_Shader vshader = OGL_MakeShader(scratch, glsl_vshader, GL_VERTEX_SHADER);
    OGL_Shader fshader = OGL_MakeShader(scratch, glsl_fshader, GL_FRAGMENT_SHADER);
    OGL_Shader program = OGL_MakeProgram(scratch, ArrayExpand(OGL_Shader, vshader, fshader));
    
    if (vshader.log.size) ErrorSet(error,   "Vertex Shader:\n%.*s\n", StrExpand(vshader.log));
    if (fshader.log.size) ErrorSet(error, "Fragment Shader:\n%.*s\n", StrExpand(fshader.log));
    if (program.log.size) ErrorSet(error,         "Program:\n%.*s\n", StrExpand(program.log));
    
    //- long: Uniform Locations
    GLint uTransform = -1;
    GLint uTexture = -1;
    if (!error)
    {
        uTransform = glGetUniformLocation(program.handle, "u_view_xform");
        uTexture   = glGetUniformLocation(program.handle, "u_tex");
        
        // NOTE(long): The locations can be -1 if the uniform variables are optimized out
        GLenum glErr = glGetError();
        if (glErr != 0)
            ErrorSet(error, "Failed to get uniform location: %u", glErr);
    }
    
    //- long: Vertex Array
    GLuint vao = 0;
    if (!error)
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        GLenum glErr = glGetError();
        if (glErr != 0 || vao == 0)
            ErrorSet(error, "Failed to get uniform location: %u", glErr);
    }
    
    //- long: Vertex Buffer
    GLuint vbo = 0;
    if (!error)
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        
        GLenum glErr = glGetError();
        if (glErr != 0 || vbo == 0)
            ErrorSet(error, "Failed to get uniform location: %u", glErr);
    }
    
    //- long: Vertex Attributes
    if (!error)
    {
        u64 quadOffset = sizeof(v2f32) * 6;
        
        glEnableVertexAttribArray(0);
        glVertexAttribDivisor(0, 0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(v2f32), 0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribDivisor(1, 1);
        glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, xy)));
        
        glEnableVertexAttribArray(2);
        glVertexAttribDivisor(2, 1);
        glVertexAttribPointer(2, 1, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, roundness)));
        
        glEnableVertexAttribArray(3);
        glVertexAttribDivisor(3, 1);
        glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, thickness)));
        
        glEnableVertexAttribArray(4);
        glVertexAttribDivisor(4, 1);
        glVertexAttribPointer(4, 1, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, theta)));
        
        glEnableVertexAttribArray(5);
        glVertexAttribDivisor(5, 1);
        glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, c[0])));
        
        glEnableVertexAttribArray(6);
        glVertexAttribDivisor(6, 1);
        glVertexAttribIPointer(6, 1, GL_UNSIGNED_INT, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, c[1])));
        
        glEnableVertexAttribArray(7);
        glVertexAttribDivisor(7, 1);
        glVertexAttribIPointer(7, 1, GL_UNSIGNED_INT, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, flags)));
        
        glEnableVertexAttribArray(8);
        glVertexAttribDivisor(8, 1);
        glVertexAttribPointer(8, 4, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, uv)));
        
        glEnableVertexAttribArray(9);
        glVertexAttribDivisor(9, 1);
        glVertexAttribPointer(9, 4, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, clip)));
    }
    
    //- long: Blend Mode
    if (!error)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        
        GLenum glErr = glGetError();
        if (glErr != 0)
            ErrorSet(error, "Failed to set the blend mode: %u", glErr);
    }
    
    //- long: Pack/Unpack Alignment
    if (!error)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        
        GLenum glErr = glGetError();
        if (glErr != 0)
            ErrorSet(error, "Failed to set the pixel pack mode: %u", glErr);
    }
    
    //- long: Fallback Texture
    GLuint fallbackTex = 0;
    {
        glGenTextures(1, &fallbackTex);
        glBindTexture(GL_TEXTURE_2D, fallbackTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 4, 4, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        GLenum glErr = glGetError();
        if (glErr != 0 || fallbackTex == 0)
            ErrorSet(error, "Failed to set the pixel pack mode: %u", glErr);
    }
    
    //- long: Main Framebuffer for SRGB
    GLuint fbo = 0;
    GLuint canvasTex = 0;
    if (!error)
    {
        glGenTextures(1, &canvasTex);
        glBindTexture(GL_TEXTURE_2D, canvasTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4096, 4096, 0, GL_RGB, GL_FLOAT, 0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, canvasTex, 0);
        
        if (glGetError() != 0 || fbo == 0)
            ErrorSet(error, "Failed to setup the framebuffer");
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    //- long: Clear GL errors
    {
        GLenum glErr = 0;
        while ((glErr = glGetError()))
            ErrorFmt("OpenGL Error: %u", glErr);
    }
    
    //- long: Clean up
    if (NEVER(error))
    {
        if (program.handle != 0)
            glDeleteProgram(program.handle);
        if (vao != 0)
            glDeleteVertexArrays(1, &vao);
        if (vbo != 0)
            glDeleteBuffers(1, &vbo);
        if (fbo != 0)
            glDeleteFramebuffers(1, &fbo);
        if (canvasTex != 0)
            glDeleteTextures(1, &canvasTex);
        if (fallbackTex != 0)
            glDeleteTextures(1, &fallbackTex);
    }
    
    else
    {
        oglRenderer.program = program.handle;
        oglRenderer.uTransform = uTransform;
        oglRenderer.uTexture = uTexture;
        oglRenderer.vao = vao;
        oglRenderer.vbo = vbo;
        oglRenderer.fbo = fbo;
        oglRenderer.canvasTex = canvasTex;
        oglRenderer.fallbackTex = fallbackTex;
    }
    
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
    
    glBindFramebuffer(GL_FRAMEBUFFER, oglRenderer.fbo);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

function void R_End(void)
{
    DeferBlock(glEnable(GL_FRAMEBUFFER_SRGB), glDisable(GL_FRAMEBUFFER_SRGB))
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, oglRenderer.fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        
        v2i32 dim = oglRenderer.dim;
        glBlitFramebuffer(0, 0, dim.x, dim.y, 0, 0, dim.x, dim.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    OGL_End();
}

function void R_Submit(R_QuadNode* first, u64 count, R_Texture* texturePtr)
{
    GLuint texture = (GLuint)IntFromPtr(texturePtr);
    if (!glIsTexture(texture))
        texture = oglRenderer.fallbackTex;
    
    // NOTE(long): Rendering in OpenGL means telling the driver where the data is and how to interpret it
    // glBufferData clears the current buffer storage and uploads new data
    // glVertexAttrib* specifies how to interpret the data and what to send to the shaders
    // The first uses the currently bound buffer, so glBindBuffer must be called first
    // The second is fixed per shader, so set it once per VAO (at startup) and bind it before drawing
    // Also remember to switch the current shader with glUseProgram and update all uniform variables
    
    // NOTE(long): We’re rendering in batches using instancing, so these 6 points of a quad are shared
    local v2f32 quadTris[6] = {
        {-1.f, +1.f}, {+1.f, +1.f}, {-1.f, -1.f},
        {+1.f, +1.f}, {-1.f, -1.f}, {+1.f, -1.f},
    };
    
    GLintptr cursor = 0;
    glBindBuffer(GL_ARRAY_BUFFER, oglRenderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadTris) + sizeof(R_Quad)*count, 0, GL_STREAM_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, cursor, sizeof(quadTris), quadTris);
    cursor += sizeof(quadTris);
    
    for (R_QuadNode* node = first; node; node = node->next)
    {
        u64 nodeSize = node->count * sizeof(R_Quad);
        glBufferSubData(GL_ARRAY_BUFFER, cursor, nodeSize, node->quads);
        cursor += nodeSize;
    }
    
    glUseProgram(oglRenderer.program);
    glUniform2f(oglRenderer.uTransform, 2.f/(f32)oglRenderer.dim.x, 2.f/(f32)oglRenderer.dim.y);
    glUniform1i(oglRenderer.uTexture, 0);
    
    // NOTE(long): We can't go bulk completely because changing textures still requires a call
    // There are a few pptions: texture array (using layers), texture atlas, or bindless textures
    // Bindless is what I want, but not well-supported across vendors/OSes
    // The atlas approach seems better suited for a higher-level user layer
    // That layer could automatically pack sprites and allocate new atlases at runtime
    // For this layer, a texture array seems best: each unique texture is a separate layer
    // With many textures, an atlas may still be needed, so the ideal approach combines the first two:
    // 1. Automatically pack each texture into an atlas (we could reused FNT_PackRect here)
    // 2. When an atlas is full, push it as a new layer
    // 3. When all layers are used, create a new batch
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glBindVertexArray(oglRenderer.vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)count);
    
    // NOTE(long): Here’s how to query GPU info in OpenGL
    // e.g. max layers/textures and supported extensions
#if 0
    {
        GLint maxLayers = 0;
        glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxLayers);
        GLint maxTextures = 0;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);
        
        String ext = StrFromCStr((u8*)glGetString(GL_EXTENSIONS));
        String ver = StrFromCStr((u8*)glGetString(GL_VERSION));
        
        if (StrContainsStr(ext, StrLit("GL_ARB_bindless_texture")))
            OutputDebugString("Bindless textures are supported.\n");
    }
#endif
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
