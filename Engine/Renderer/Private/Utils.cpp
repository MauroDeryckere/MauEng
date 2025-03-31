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
        auto path{ (std::filesystem::absolute("Materials")).string() };
        return path;
    }

    void Utils::LoadModel(std::filesystem::path const& path, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Material& mat)
	{
        vertices = {};
        indices = {};

		assert(std::filesystem::exists(path));

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
            std::cout << warn << err << "\n";
        }

        for (const auto& fileMat : materials)
        {
            if constexpr (DEBUG_OUT_MAT)
            {
                std::cout << "Material Info:\n";
                std::cout << "--------------------------------\n";
                std::cout << "Material Name: " << fileMat.name << "\n";

                // Diffuse color
                std::cout << "Diffuse Color (Kd): ("
                    << fileMat.diffuse[0] << ", "
                    << fileMat.diffuse[1] << ", "
                    << fileMat.diffuse[2] << ")\n";

                // Specular color
                std::cout << "Specular Color (Ks): ("
                    << fileMat.specular[0] << ", "
                    << fileMat.specular[1] << ", "
                    << fileMat.specular[2] << ")\n";

                // Ambient color
                std::cout << "Ambient Color (Ka): ("
                    << fileMat.ambient[0] << ", "
                    << fileMat.ambient[1] << ", "
                    << fileMat.ambient[2] << ")\n";

                // Emissive color (Ke)
                std::cout << "Emissive Color (Ke): ("
                    << fileMat.emission[0] << ", "
                    << fileMat.emission[1] << ", "
                    << fileMat.emission[2] << ")\n";

                // Transparency (d or Tr)
                std::cout << "Transparency (d or Tr): "
                    << fileMat.dissolve << "\n";

                // Shininess (Ns)
                std::cout << "Shininess (Ns): " << mat.shininess << "\n";

                // Refraction index (Ni)
                std::cout << "Refraction Index (Ni): " << fileMat.ior << "\n";

                // Illumination model (illum)
                std::cout << "Illumination Model (illum): " << fileMat.illum << "\n";

                // Texture names
                std::cout << "Diffuse Texture: " << (fileMat.diffuse_texname.empty() ? "None" : fileMat.diffuse_texname) << "\n";
                std::cout << "Specular Texture: " << (fileMat.specular_texname.empty() ? "None" : fileMat.specular_texname) << "\n";
                std::cout << "Normal Map: " << (fileMat.normal_texname.empty() ? "None" : fileMat.normal_texname) << "\n";
                std::cout << "Ambient Texture: " << (fileMat.ambient_texname.empty() ? "None" : fileMat.ambient_texname) << "\n";
                std::cout << "Bump Map: " << (fileMat.bump_texname.empty() ? "None" : fileMat.bump_texname) << "\n";
                std::cout << "Displacement Map: " << (fileMat.displacement_texname.empty() ? "None" : fileMat.displacement_texname) << "\n";
                std::cout << "--------------------------------\n";
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


