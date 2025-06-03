#version 450

layout(set = 0, binding = 0, std140) uniform UniformBufferObject
{
    mat4 viewProj;
    mat4 invView;
    mat4 invProj;
    vec3 cameraPos;
    float _pad0; // Padding to align next vec2

    vec2 screenSize;
    vec2 _pad1; // Padding to align next uint

    uint numLights;
    uint _pad2;
    uint _pad3;
    uint _pad4;
} ubo;



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

void main()
{
    MeshInstanceData instance = instances[gl_InstanceIndex];
    mat4 model = instance.modelMatrix;

    gl_Position = ubo.viewProj * model * vec4(inPosition, 1.0);
}
