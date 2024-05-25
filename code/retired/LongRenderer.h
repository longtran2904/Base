/* date = July 28th 2022 4:33 pm */

#ifndef _LONG_RENDERER_H
#define _LONG_RENDERER_H

edef(CommandType)
{
    CommandType_DrawRect,
    CommandType_DrawLine,
    CommandType_DrawCircle;
    CommandType_DrawPoly,
    CommandType_DrawText,
    CommandType_DrawSprite,
    CommandType_DrawShader,
    
    CommandType_Count
};

typedef Color ShaderFunc(i32 x, i32 y, DrawTarget target, void* data);
sdef(Shader)
{
    ShaderFunc* shader;
    void* data;
};

edef(FilterType)
{
    FilterType_Point,
    FilterType_Billinear,
    FilterType_Trillinear,
    
    FilterType_Count
};

sdef(Command)
{
    CommandType type;
    i32 x, y ,w, h, lineSize;
    i32 pivotX, pivotY;
    f32 rotation;
    Color color;
    DrawTarget drawTarget;
    
    union
    {
        struct
        {
            Vector2* points;
            u32 pointCount;
        };
        
        struct
        {
            String str;
            Font font;
        };
        
        struct
        {
            i32 srcX, srcY, srcW, srcH;
            FilterType filterType;
            DrawTarget sprite;
        };
        
        struct
        {
            Shader shader;
        };
    };
};

edef(BufferType)
{
    BufferType_Command,
    BufferType_Frame,
    BufferType_Texture,
    BufferType_Vertex,
    BufferType_Index,
    BufferType_Handle,
    
    BufferType_Count
};

Handle CreateRenderBuffer(BufferType type, u32 size);
b32    ExecuteCommandBuffer(Handle handle);
Handle WaitForMultipleFences(Handle* objects, u32 count, b32 waitAll, f32 waitTime);
HANDLE GetPlatformHandle(Handle handle);

#endif //_LONG_RENDERER_H
