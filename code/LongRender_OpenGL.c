
//~ long: OpenGL Types

typedef struct OGL_Renderer OGL_Renderer;
struct OGL_Renderer
{
    b32 initialized;
    
    GLuint program;
    GLint viewTransform;
    GLint texture;
    
    GLuint fb;
    GLuint vb;
    GLuint fallbackTex;
    GLuint canvasTex;
    
    v2i32 dim;
};

global OGL_Renderer oglRenderer;

//~ long: OpenGL Functions

function void R_Init(void)
{
    ScratchBegin(scratch);
    
    //w32RenderWnd = w32_core_window;
    w32RenderDC = GetDC(w32RenderWnd);
    w32WglMakeCurrent(w32RenderDC, w32OpenGLContext);
    
    //OGL_Shader vshader
    ScratchEnd(scratch);
}

function void R_Begin(GFXWindow window)
{
    OGL_Begin(window);
    u32 w, h;
    if (GFXWindowGetInnerRect(window, 0, 0, &w, &h))
        glViewport(0, 0, w, h);
}

function void R_End(void)
{
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
    glUniform1i(oglRenderer.texture, 0);
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
    glVertexAttribPointer(5, 4, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, c0)));
    
    glEnableVertexAttribArray(6);
    glVertexAttribDivisor(6, 1);
    glVertexAttribPointer(6, 4, GL_FLOAT, false, sizeof(R_Quad), PtrFromInt(quadOffset + OffsetOf(R_Quad, c1)));
    
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 4);
}

function R_Texture* R_TextureCreate(void* data, u32 w, u32 h);
function void       R_TextureDestroy(R_Texture* texture);
function b32        R_TextureValid(R_Texture* texture);
