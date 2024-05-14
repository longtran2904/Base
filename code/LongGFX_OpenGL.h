/* date = July 31st 2022 2:09 pm */

#ifndef _LONG_O_S_G_L_H
#define _LONG_O_S_G_L_H

//~ NOTE(long): System OpenGL Functions

function b32 InitGL(void);
function b32 FreeGL(void);
function b32 EquipGLWindow(GFXWindow window);

function void BeginGLRender(GFXWindow window);
function void EndGLRender(void);

#endif //_LONG_O_S_G_L_H
