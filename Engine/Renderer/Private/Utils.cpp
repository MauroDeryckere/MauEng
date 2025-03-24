#include "RendererPCH.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <unordered_map>
#include "Vertex.h"

namespace MauRen
{
    void Utils::LoadModel(std::filesystem::path const& path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
        vertices = {};
        indices = {};

		assert(std::filesystem::exists(path));

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str()))
        {
            throw std::runtime_error(warn + err);
        }

        size_t numVertices{ 0 };
        size_t numIndices{ 0 };
        for (const auto& shape : shapes)
        {
            numIndices += shape.mesh.indices.size();
            numVertices += shape.mesh.indices.size();
        }

        vertices.reserve(numVertices);
        indices.reserve(numIndices);

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) 
        {
            for (const auto& index : shape.mesh.indices) 
            {
                Vertex vertex{};

                vertex.position = 
                {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = 
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (not uniqueVertices.contains(vertex)) 
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.emplace_back(vertex);
                }

                indices.emplace_back(uniqueVertices[vertex]);
            }
        }

        vertices.shrink_to_fit();
        indices.shrink_to_fit();
	}
}


