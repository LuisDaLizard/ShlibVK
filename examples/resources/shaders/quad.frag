#version 450 core

layout(location = 0) out vec4 oColor;

layout(binding = 1) uniform sampler2D uTexture;

layout(location = 0) in vec2 fTexCoord;

void main()
{
    oColor = texture(uTexture, fTexCoord);
}