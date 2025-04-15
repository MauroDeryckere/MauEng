#pragma once

#include <glm/glm.hpp>

// (GPU-side resource - CPU copy)
// Per instance data
// Maps to all used data from buffers
struct alignas(16) MeshInstanceData final
{
    glm::mat4 modelMatrix;
    uint32_t meshIndex;     // Index into MeshData[]
    uint32_t materialIndex; // Index into MaterialData[]

    uint32_t flags;         // Flags for deletion or active status (E.g 0 = active, 1 = marked for deletion)
    uint32_t objectID;      // Optional: ID for selection/debug
};

// NOT USED IN SHADER CURRENTLY
//
// Per mesh data
struct alignas(16) MeshData final
{
    uint32_t firstIndex;   // First index in global index buffer
    uint32_t indexCount;    // Indices to draw
    int32_t vertexOffset;  // Offset into big VBO, added to indices
    uint32_t flags;         // Flags for deletion or active status (E.g 0 = active, 1 = marked for deletion)
};

// (GPU-side resource - CPU copy)
// Per material data
struct alignas(16) MaterialData final
{
    glm::vec4 baseColor{ 0, 0, 0, 1 };
    uint32_t albedoTextureID{ UINT32_MAX };
    uint32_t normalTextureID{ UINT32_MAX };
    uint32_t roughnessTextureID{ UINT32_MAX };
    uint32_t metallicTextureID{ UINT32_MAX };
    //...

   // uint32_t flags; // Flags for deletion or active status (0 = active, 1 = marked for deletion)
};

// (CPU prepares, GPU uses)
struct DrawCommand final
{
    uint32_t indexCount{ 0 };        // Number of indices for the draw call
    uint32_t instanceCount{ 0 };     // Number of instances to draw
    uint32_t firstIndex{ 0 };        // Starting index in the index buffer
    int32_t  vertexOffset{ 0 };      // Offset to add to the vertex indices
    uint32_t firstInstance{ 0 };     // Starting instance index
};


/*
DrawCommands 
-> GPU
  ->InstanceData
        |
        |--> meshID --> MeshData[] --> offset into large vertex/index buffer
        |
        |--> materialID --> MaterialData[]
                                    |
                                    |--> textureIndex --> bindless texture array[]
*/