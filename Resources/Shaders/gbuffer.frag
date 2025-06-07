#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 1) uniform sampler globalSampler;

layout(set = 0, binding = 2) uniform texture2D TextureBuffer[];

// 1 : 1 copy of material data on CPU
struct MaterialData
{
    vec4 baseColor;
    uint albedoTextureID;
    uint normalTextureID;
    uint metalnessTextureID;
};

layout(set = 0, binding = 3) buffer readonly MaterialDataBuffer
{
    MaterialData materials[];
};

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in uint inMaterialIndex;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec3 inNormal;

// GBuffer Out
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outNormal;
layout(location = 2) out vec4 outMetal;

void main()
{
    const MaterialData material = materials[inMaterialIndex];

    const vec3 albedo = texture(sampler2D(TextureBuffer[nonuniformEXT(material.albedoTextureID)], globalSampler), fragTexCoord).rgb;
    const vec4 normalTex = texture(sampler2D(TextureBuffer[nonuniformEXT(material.normalTextureID)], globalSampler), fragTexCoord);
    const vec3 metalRough = texture(sampler2D(TextureBuffer[nonuniformEXT(material.metalnessTextureID)], globalSampler), fragTexCoord).rgb;

    vec3 N = normalize(inNormal);
    vec3 T = normalize(inTangent.xyz);
    vec3 B = normalize(cross(N, T) * inTangent.w);
    // Could also input bitangent if necessary (speed things up)

    mat3 TBN = mat3(T, B, N);
    vec3 sampledNormal = normalize(normalTex.xyz * 2.0 - 1.0);
    vec3 n = normalize(TBN * sampledNormal);

    float nzSign = (n.z < 0.0 ? 0.0 : 1.0);
    vec3 mr = metalRough.rgb;

    outColor = vec4(albedo, 1.0);
    outNormal = n.xy * 0.5 + 0.5;
    outMetal = vec4(mr, nzSign);
}