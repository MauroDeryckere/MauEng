#version 450

layout(set = 0, binding = 0, std140) uniform UniformBufferObject
{
    mat4 viewProj;
    mat4 invView;
    mat4 invProj;
    vec3 cameraPos;
    float _pad0; // Padding to align next vec2

    vec2 screenSize;
    vec2 _pad1; // Padding to align next uint

    uint numLights;
    uint _pad2;
    uint _pad3;
    uint _pad4;
} ubo;



layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() 
{
    gl_Position = ubo.viewProj * vec4(inPosition, 1.0);
    fragColor = inColor;
}