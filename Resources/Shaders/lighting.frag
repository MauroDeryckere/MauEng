#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 viewProj;
    vec3 cameraPos;
} ubo;

layout(location = 0) in vec4 inColor;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = inColor;
}