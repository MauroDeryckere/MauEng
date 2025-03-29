#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
    uint albedoID;
    uint normalID;
    uint roughnessID;
    uint metallicID;
} pc;

layout(set = 0, binding = 1) uniform sampler2D bindlessTextures[];

//layout(set = 0, binding = 1) uniform sampler2D texSampler;  // This line is defining a combined image sampler

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec4 albedo = texture(bindlessTextures[pc.albedoID], fragTexCoord);
    vec4 normal = texture(bindlessTextures[pc.normalID], fragTexCoord);
    vec4 roughness = texture(bindlessTextures[pc.roughnessID], fragTexCoord);
    vec4 metallic = texture(bindlessTextures[pc.metallicID], fragTexCoord);

    outColor = albedo * vec4(fragColor, 1.0);

    // outColor = texture(texSampler, fragTexCoord);
    // Debug colours
    // outColor = vec4(fragTexCoord, 0.0, 1.0);
}