#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in uint inMaterialIndex;

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


void main()
{
    const MaterialData material = materials[inMaterialIndex];

    const vec4 albedo = texture(sampler2D(TextureBuffer[nonuniformEXT(material.albedoTextureID)], globalSampler), fragTexCoord);

    if (albedo.a < 0.95f)
        discard;
}