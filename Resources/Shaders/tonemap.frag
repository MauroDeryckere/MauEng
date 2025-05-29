#version 450

layout(location = 0) in vec2 inFragUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler globalSampler;
layout(set = 0, binding = 10) uniform texture2D hdriImage;

vec3 ACESFilm(vec3 x) 
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main()
{
    const vec3 hdrColor = texture(sampler2D(hdriImage, globalSampler), inFragUV).rgb;

    const vec3 mapped = ACESFilm(hdrColor);
    outColor = vec4(mapped, 1.0);
}