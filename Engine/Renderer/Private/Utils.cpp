#include "RendererPCH.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <unordered_map>
#include "Vertex.h"
#include "Material.h"

namespace MauRen
{
    std::string Utils::GetAbsoluteMaterialPath() noexcept
    {
        auto path{ (std::filesystem::absolute("Resources/Materials")).string() };
        return path;
    }

    void Utils::LoadModel(std::filesystem::path const& path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Material& mat)
	{
        vertices = {};
        indices = {};

		ME_RENDERER_VERIFY(std::filesystem::exists(path));

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string const matPath { GetAbsoluteMaterialPath() };
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), matPath.c_str()))
        {
            throw std::runtime_error(warn + err);
        }
        else
        {
	        if (!warn.empty())
	        {
				LOGGER.Log(MauCor::LogPriority::Warn, MauCor::LogCategory::Renderer, "Obj loader: {}", warn);
	        }
	        if (!err.empty())
	        {
                LOGGER.Log(MauCor::LogPriority::Error, MauCor::LogCategory::Renderer, "Obj loader: {}", err);
	        }
        }

        for (const auto& fileMat : materials)
        {
            if constexpr (DEBUG_OUT_MAT)
            {
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Material Info:");
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "--------------------------------");
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Material Name: {}", fileMat.name);

                // Diffuse color
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Diffuse Color (kd): ({}, {}, {})", fileMat.diffuse[0], fileMat.diffuse[1], fileMat.diffuse[2]);

                // Specular color
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Specular Color (Ks): ({}, {}, {})", fileMat.specular[0], fileMat.specular[1], fileMat.specular[2]);

                // Ambient color
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Ambient Color (Ka): ({}, {}, {})", fileMat.ambient[0], fileMat.ambient[1], fileMat.ambient[2]);

                // Emissive color (Ke)
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Emissive Color (Ke): ({}, {}, {})", fileMat.emission[0], fileMat.emission[1], fileMat.emission[2]);

                // Transparency (d or Tr)
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Transparency (d or Tr): {}", fileMat.dissolve);
 
                // Shininess (Ns)
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Shininess (Ns): {}", fileMat.shininess);

                // Refraction index (Ni)
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Refraction Index (Ni):{} ", fileMat.ior);

                // Illumination model (illum)
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Illumination Model (illum): {}", fileMat.illum);

                // Texture names
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Diffuse Texture: {}", (fileMat.diffuse_texname.empty() ? "None" : fileMat.diffuse_texname));
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Specular Texture: {}", (fileMat.specular_texname.empty() ? "None" : fileMat.specular_texname));
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Normal Map: {}", (fileMat.normal_texname.empty() ? "None" : fileMat.normal_texname));
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Ambient Texture: {}", (fileMat.ambient_texname.empty() ? "None" : fileMat.ambient_texname));
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Bump Map: {}", (fileMat.bump_texname.empty() ? "None" : fileMat.bump_texname));
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Displacement Map: {}", (fileMat.displacement_texname.empty() ? "None" : fileMat.displacement_texname));
                LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "--------------------------------");
            }

            mat.name = fileMat.name;

            // Diffuse color
            mat.diffuseColor[0] = fileMat.diffuse[0];
            mat.diffuseColor[1] = fileMat.diffuse[1];
            mat.diffuseColor[2] = fileMat.diffuse[2];

            // Specular color
            mat.specularColor[0] = fileMat.specular[0];
            mat.specularColor[1] = fileMat.specular[1];
            mat.specularColor[2] = fileMat.specular[2];

            // Ambient color
            mat.ambientColor[0] = fileMat.ambient[0];
            mat.ambientColor[1] = fileMat.ambient[1];
            mat.ambientColor[2] = fileMat.ambient[2];

            // Emissive color (Ke)
            mat.emissiveColor[0] = fileMat.emission[0];
            mat.emissiveColor[1] = fileMat.emission[1];
            mat.emissiveColor[2] = fileMat.emission[2];

            // Transparency (d or Tr)
            mat.transparency = fileMat.dissolve;

            // Shininess (Ns)
            mat.shininess = fileMat.shininess;

            // Refraction index (Ni)
            mat.refractionIndex = fileMat.ior;

            // Illumination model (illum)
            mat.illuminationModel = fileMat.illum;

            // Texture names
            mat.diffuseTexture = fileMat.diffuse_texname;
            mat.specularTexture = fileMat.specular_texname;
            mat.normalMap = fileMat.normal_texname;
            mat.ambientTexture = fileMat.ambient_texname;
            mat.bumpMap = fileMat.bump_texname;
            mat.displacementMap = fileMat.displacement_texname;
        }

        size_t numVertices{ 0 };
        size_t numIndices{ 0 };
        for (const auto& shape : shapes)
        {
            numIndices += shape.mesh.indices.size();
            numVertices += shape.mesh.num_face_vertices.size();
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
        LOGGER.Log(MauCor::LogPriority::Trace, MauCor::LogCategory::Renderer, "Model: {}; indices: {}; vertices: {})", path.string(), indices.size(), vertices.size());

        vertices.shrink_to_fit();
        indices.shrink_to_fit();
	}
}


