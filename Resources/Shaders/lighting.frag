#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 viewProj;
    vec3 cameraPos;
} ubo;

layout(set = 0, binding = 1) uniform sampler globalSampler;
layout(set = 0, binding = 6) uniform texture2D gAlbedo;
layout(set = 0, binding = 7) uniform texture2D gNormal;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 albedo = texture(sampler2D(gAlbedo, globalSampler), fragUV).rgb;

    outColor = vec4(albedo, 1.0);
}