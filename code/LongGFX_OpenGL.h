/* date = July 31st 2022 2:09 pm */

#ifndef _LONG_O_S_G_L_H
#define _LONG_O_S_G_L_H

//~ long: OGL System Functions

function b32 OGL_Init(void);
function b32 OGL_Free(void);
function b32 OGL_WindowEquip(GFXWindow window);

function void OGL_Begin(GFXWindow window);
function void OGL_End(void);

//~ long: OGL Helper Functions

typedef struct OGL_Shader OGL_Shader;
struct OGL_Shader
{
    GLuint handle;
    String log;
};

function OGL_Shader OGL_MakeShader(Arena* arena, char* src, GLenum type);
function OGL_Shader OGL_MakeProgram(Arena* arena, OGL_Shader* shaders, u64 count);

#endif //_LONG_O_S_G_L_H
