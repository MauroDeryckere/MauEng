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
    uint roughnessTextureID;
    uint metallicTextureID;
};

layout(set = 0, binding = 3) buffer readonly MaterialDataBuffer 
{
    MaterialData materials[];
};

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) flat in uint inMaterialIndex;


layout(location = 0) out vec4 outColor;

void main() 
{
    // Access the material data based on the materialID from PushConstants
    MaterialData material = materials[inMaterialIndex];

    // When the ID is 0xFFFFFFFF, we treat it as missing or invalid and return a zero vector.
    vec4 albedo = (material.albedoTextureID == 0xFFFFFFFF)
        ? vec4(0.0)
        : texture(sampler2D(TextureBuffer[material.albedoTextureID], globalSampler), fragTexCoord);

    vec4 normal = (material.normalTextureID == 0xFFFFFFFF)
        ? vec4(0.0)
        : texture(sampler2D(TextureBuffer[material.normalTextureID], globalSampler), fragTexCoord);
        
    vec4 roughness = (material.roughnessTextureID == 0xFFFFFFFF)
        ? vec4(0.0)
        : texture(sampler2D(TextureBuffer[material.roughnessTextureID], globalSampler), fragTexCoord);
        
    vec4 metallic = (material.metallicTextureID == 0xFFFFFFFF)
        ? vec4(0.0)
        : texture(sampler2D(TextureBuffer[material.metallicTextureID], globalSampler), fragTexCoord);


    vec3 n = normalize(normal.xyz);


    // TODO
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));  // Example light direction
    float NdotL = max(dot(n, lightDir), 0.0);  // Diffuse lighting calculation

    outColor = albedo * vec4(vec3(NdotL), 1.0);  // Color is modulated by diffuse lighting


    outColor = albedo;  // Color is modulated by diffuse lighting

    // outColor = texture(texSampler, fragTexCoord);
    // Debug colours
    // outColor = vec4(fragTexCoord, 0.0, 1.0);
}