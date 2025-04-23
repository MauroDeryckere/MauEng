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

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in uint inMaterialIndex;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

vec3 g_LightDir = vec3(0.577f, -0.577f, 0.577f);

void main() 
{
    // Access the material data based on the materialID from PushConstants
    MaterialData material = materials[inMaterialIndex];

    // When the ID is 0xFFFFFFFF, we treat it as missing or invalid and return a zero vector.
    vec4 albedo = texture(sampler2D(TextureBuffer[material.albedoTextureID], globalSampler), fragTexCoord);
    vec4 normalTex = texture(sampler2D(TextureBuffer[material.normalTextureID], globalSampler), fragTexCoord);
        
    //vec4 roughness = texture(sampler2D(TextureBuffer[material.roughnessTextureID], globalSampler), fragTexCoord);
    //vec4 metallic = texture(sampler2D(TextureBuffer[material.metallicTextureID], globalSampler), fragTexCoord);

    vec3 bitangent = cross(inNormal, inTangent.xyz) * inTangent.w;
    vec3 sampledNormal = normalize(normalTex.xyz * 2.0 - 1.0);
    mat3 TBN = mat3(normalize(inTangent.xyz), normalize(bitangent), inNormal);
    vec3 n = normalize(TBN * sampledNormal);

    //vec3 lightDir = normalize(vec3(
    //    2.0 * fract(sin(gl_FragCoord.x * 12.9898 + gl_FragCoord.y * 78.233) * 43758.5453) - 1.0,
    //    2.0 * fract(sin(gl_FragCoord.x * 45.4768 + gl_FragCoord.y * 34.786) * 98765.4678) - 1.0,
    //    2.0 * fract(sin(gl_FragCoord.x * 78.9234 + gl_FragCoord.y * 29.1457) * 12345.6789) - 1.0
    //));

    //vec3 lightPosition = vec3(5.0, 10.0, 0.0);
    //vec3 fragPos = vec3(0.0, 0.0, 0.0);
    //vec3 lightDir = normalize(lightPosition - fragPos);
    
    float NdotL = max(dot(n, -g_LightDir), 0.0);

    outColor = albedo * vec4(vec3(NdotL), 1.0);

    //outColor = vec4(fragTexCoord, 0.0, 1.0);
}