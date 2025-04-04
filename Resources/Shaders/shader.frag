#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
    uint albedoID;
    uint normalID;
    uint roughnessID;
    uint metallicID;
} pc;

layout(set = 0, binding = 1) uniform sampler globalSampler;
layout(set = 0, binding = 2) uniform texture2D bindlessTextures[];

//layout(set = 0, binding = 1) uniform sampler2D texSampler;  // This line is defining a combined image sampler

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
    // When the push constant ID is 0xFFFFFFFF, we treat it as missing and return a zero vector.
    vec4 albedo = (pc.albedoID == 0xFFFFFFFF)
        ? vec4(0.0)
        : texture(sampler2D(bindlessTextures[pc.albedoID], globalSampler), fragTexCoord);
        
    vec4 normal = (pc.normalID == 0xFFFFFFFF)
        ? vec4(0.0)
        : texture(sampler2D(bindlessTextures[pc.normalID], globalSampler), fragTexCoord);
        
    vec4 roughness = (pc.roughnessID == 0xFFFFFFFF)
        ? vec4(0.0)
        : texture(sampler2D(bindlessTextures[pc.roughnessID], globalSampler), fragTexCoord);
        
    vec4 metallic = (pc.metallicID == 0xFFFFFFFF)
        ? vec4(0.0)
        : texture(sampler2D(bindlessTextures[pc.metallicID], globalSampler), fragTexCoord);


    vec3 n = normalize(normal.xyz);


    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));  // Example light direction
    float NdotL = max(dot(n, lightDir), 0.0);  // Diffuse lighting calculation

    outColor = albedo * vec4(vec3(NdotL), 1.0);  // Color is modulated by diffuse lighting


    outColor = albedo;  // Color is modulated by diffuse lighting

    // outColor = texture(texSampler, fragTexCoord);
    // Debug colours
    // outColor = vec4(fragTexCoord, 0.0, 1.0);
}