#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

layout(location = 0) out vec2 fTexCoord;

layout(binding = 0) uniform UniformBuffers
{
    mat4 uProjection;
    mat4 uView;
} Uniforms;

void main()
{
    fTexCoord = aTexCoord;
    gl_Position = transpose(Uniforms.uProjection) * transpose(Uniforms.uView) * vec4(aPosition, 1);
}