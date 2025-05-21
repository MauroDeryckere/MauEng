#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 viewProj;
    vec3 cameraPos;
} ubo;

layout(set = 0, binding = 1) uniform sampler globalSampler;

layout(set = 0, binding = 6) uniform texture2D gAlbedo;
layout(set = 0, binding = 7) uniform texture2D gNormal;
layout(set = 0, binding = 8) uniform texture2D gMetal;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 albedo = texture(sampler2D(gAlbedo, globalSampler), fragUV).rgb;
    vec2 packedNormalXY = texture(sampler2D(gNormal, globalSampler), fragUV).xy;
	// Reconstruct normal from packed normal
    vec2 nXY = packedNormalXY * 2.0 - 1.0;
    float nZ = sqrt(max(0.0, 1.0 - dot(nXY, nXY)));
    vec3 normal = normalize(vec3(nXY, nZ));

    vec3 lightDir = normalize(vec3(-1.0, -1.0, -1.0));
    vec3 lightColor = vec3(1.0, 0.95, 0.9);

    float diffuse = max(dot(normal, normalize(-lightDir)), 0.0);
    vec3 lighting = lightColor * diffuse;
    vec3 finalColor = albedo * lighting;

    outColor = vec4(finalColor, 1.0);
}