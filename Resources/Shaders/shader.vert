#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 proj;
} ubo;

// 		glm::vec3 position;
//      glm::vec3 normal;
//      glm::vec4 tangent; // .xyz = tangent vector, .w = handedness
//      glm::vec2 texCoord;

struct MeshInstanceData
{
    mat4 modelMatrix;
    uint meshIndex;     // Index into MeshData[]
    uint materialIndex; // Index into MaterialData[]

    uint flags;         // Flags for deletion or active status (E.g 0 = active, 1 = marked for deletion) - TODO
    uint objectID;      // Optional: ID for selection/debug - TODO
};   

// Mesh instance data
layout(set = 0, binding = 5) buffer readonly MeshInstanceDataBuffer 
{
    MeshInstanceData instances[];
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec2 outFragTexCoord;
layout(location = 1) out flat uint outMaterialIndex;
layout(location = 2) out vec4 outTangent;
layout(location = 3) out vec3 outNormal;

void main() 
{
    mat4 model = instances[gl_InstanceIndex].modelMatrix;
    gl_Position = ubo.proj * ubo.view * model * vec4(inPosition, 1.0);
    //sub id = instances[gl] 
    // SubMeshData submesh = submeshes[gl_PrimitiveID];

    mat3 normalMatrix = transpose(inverse(mat3(model)));

    outTangent = vec4(normalMatrix * inTangent.xyz, inTangent.w);
    outNormal = normalize(normalMatrix * inNormal);

    outFragTexCoord = inTexCoord;

    outMaterialIndex = instances[gl_InstanceIndex].materialIndex;
}