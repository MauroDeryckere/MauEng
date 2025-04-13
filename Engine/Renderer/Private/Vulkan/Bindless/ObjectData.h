#pragma once

#include <glm/glm.hpp>

// (CPU prepares, GPU uses)
// Per instance data
// Maps to all used data from buffers
struct alignas(16) InstanceData final
{
    glm::mat4 modelMatrix;
    uint32_t meshIndex;     // Index into MeshData[]
    uint32_t materialIndex; // Index into MaterialData[]

    uint32_t objectID;      // Optional: ID for selection/debug
    uint32_t padding;       // Padding to 80 bytes - can be used for features in future
};

// (GPU-side resource)
// Per mesh data
struct alignas(16) MeshData final
{
    uint32_t indexOffset;   // First index in global index buffer
    uint32_t indexCount;    // Indices to draw
    uint32_t vertexOffset;  // Offset into big VBO, added to indices
    uint32_t flags;         // Flags for deletion or active status (E.g 0 = active, 1 = marked for deletion)
};

// TODO
// - is it optimal at all to split up per texture as sep buffer & all unique ids?
// or just have a global material buffer

// (GPU-side resource)
// Per material data
struct alignas(16) MaterialData final
{
    glm::vec4 baseColor;
    uint32_t albedoTextureIndex;
    uint32_t metallicRoughnessTextureIndex;
    //...

    uint32_t flags; // Flags for deletion or active status (0 = active, 1 = marked for deletion)
};

// (CPU prepares, GPU uses)
// CPU side only
struct DrawCommand final
{
    uint32_t indexCount;    // Number of indices for the draw call
    uint32_t instanceCount; // Number of instances to draw
    uint32_t firstIndex;    // Starting index in the index buffer
    int32_t  vertexOffset;  // Offset to add to the vertex indices
    uint32_t firstInstance; // Starting instance index
};


/*
DrawCommand
->
[ InstanceData[] ]  <- 1 per instance/draw
    |
    |--> meshIndex --> [ MeshData[] ] --> offset into large vertex/index buffer
    |
    |--> materialIndex --> [ MaterialData[] ]
                                |
                                |--> textureIndex --> [ bindless texture array ]
*/