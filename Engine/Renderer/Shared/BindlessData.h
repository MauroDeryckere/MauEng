#ifndef MAUREN_BINDLESS_DATA_H
#define MAUREN_BINDLESS_DATA_H

#include <glm/glm.hpp>

#include "RendererIdentifiers.h"

/*
DrawCommands
-> offset into large vertex/index buffer
-> GPU
FirstInstance + instance Count -> InstanceData [gl_InstanceID]
-> InstanceData[]
        | ** For now we are directly storing each submesh's materialID in the instance data, later on we may move to a sep SubMeshData buffer.
        |
        |--> SubMeshData[subMeshID]
                |
                |--> indexCount
                |--> firstIndex
                |--> vertexOffset
                |--> materialID
                        |
                        |--> MaterialData[materialID]
                                |
                                |--> textureIndex --> bindless texture array[]
*/

namespace MauRen
{
    struct LoadedMeshes_PathInfo
    {
        uint32_t loadedMeshesID{ INVALID_MESH_ID };
        uint32_t useCount{ 0 };
    };


    // (GPU-side resource - CPU copy)
	// Per instance data
	// Maps to all used data from buffers
    struct alignas(16) MeshInstanceData final
    {
        glm::mat4 modelMatrix;
        uint32_t subMeshID;     // Index into SubMeshData[]
        uint32_t materialID;  // Material for this submesh

        // TODO
        uint32_t flags;         // Flags for deletion or active status (E.g 0 = active, 1 = marked for deletion)
        uint32_t objectID;      // Optional: ID for selection/debug
    };

    // Per mesh data - on CPU only currently
    struct MeshData final
    {
        uint32_t firstSubMesh;
		uint32_t subMeshCount;

        uint32_t meshID;    // Easier to link back to the array that way (todo - could probably remove this)
        uint32_t flags;     // Unused for now (todo)
    };

	// SubMesh data - on CPU onnly currently
    struct SubMeshData final
    {
        uint32_t indexCount;
        uint32_t firstIndex;
        int32_t  vertexOffset;

		uint32_t materialID;   // Material for this submesh
    };

    // (GPU-side resource - CPU copy)
    // Per material data
    struct alignas(16) MaterialData final
    {
        glm::vec4 baseColor{ 0, 0, 0, 1 };         // not used for now
        uint32_t albedoTextureID{ INVALID_DIFFUSE_TEXTURE_ID };
        uint32_t normalTextureID{ INVALID_NORMAL_TEXTURE_ID };
        uint32_t metallicTextureID{ INVALID_METALNESS_TEXTURE_ID };
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
}

#endif