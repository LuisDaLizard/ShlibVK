#version 450 core

layout(location = 0) out vec4 oColor;

layout(location = 0) in vec2 fTexCoord;

void main()
{
    oColor = vec4(fTexCoord, 0, 1);
}