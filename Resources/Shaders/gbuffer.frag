#version 450

layout(set = 0, binding = 1) uniform sampler globalSampler;

layout(set = 0, binding = 2) uniform texture2D TextureBuffer[];

// 1 : 1 copy of material data on CPU
struct MaterialData
{
    vec4 baseColor;
    uint albedoTextureID;
    uint normalTextureID;
    uint roughnessTextureID;
    uint metallicTextureID;
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

void main()
{
    const MaterialData material = materials[inMaterialIndex];

    const vec4 albedo = texture(sampler2D(TextureBuffer[nonuniformEXT(material.albedoTextureID)], globalSampler), fragTexCoord);
    const vec4 normalTex = texture(sampler2D(TextureBuffer[nonuniformEXT(material.normalTextureID)], globalSampler), fragTexCoord);

    const vec3 bitangent = cross(inNormal, inTangent.xyz) * inTangent.w;
    const vec3 sampledNormal = normalize(normalTex.xyz * 2.0 - 1.0);
    const mat3 TBN = mat3(normalize(inTangent.xyz), normalize(bitangent), inNormal);
    const vec3 n = normalize(TBN * sampledNormal);

    outColor = vec4(fragTexCoord, 0.0, 1.0);
}