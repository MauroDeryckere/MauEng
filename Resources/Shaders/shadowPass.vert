#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 inPosition;

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

struct Light
{
    mat4 viewProj;

    // Direction for directional lights, position for point lights
    vec3 direction_position;
    // Light type: 0 = directional, 1 = point
    uint type;

    vec3 color;
    float intensity;

    // Index into shadow texture array
    uint shadowMapIndex;
    int castsShadows;
};

// Mesh instance data
layout(set = 0, binding = 5) buffer readonly MeshInstanceDataBuffer
{
    MeshInstanceData instances[];
};

layout(set = 0, binding = 12) buffer readonly LightDataBuffer
{
    Light lights[];
};

layout(push_constant) uniform PushConstants {
    uint lightIndex; // Which light we’re rendering the shadow map for
} pc;


void main()
{
    MeshInstanceData instance = instances[gl_InstanceIndex];
	gl_Position = lights[pc.lightIndex].viewProj * instance.modelMatrix * vec4(inPosition, 1.0);
}