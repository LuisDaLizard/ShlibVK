#version 450

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;

layout(location = 0) out vec3 fColor;

void main()
{
    gl_Position = vec4(aPosition, 0, 1);
    fColor = aColor;
}