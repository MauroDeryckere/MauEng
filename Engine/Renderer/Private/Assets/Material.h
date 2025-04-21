#ifndef MAUREN_MATERIAL_H
#define MAUREN_MATERIAL_H

#include <glm/glm.hpp>
#include <string>

namespace MauRen
{
    struct EmbeddedTexture final
    {
        std::vector<uint8_t> data{};
        std::string formatHint{"NONE"};     // e.g., "png", "jpg"


        std::string hash{"INVALID"};        // Used to deduplicate

        int width{ 0 };                         // Only used if uncompressed
        int height{ 0 };
        bool isCompressed{ true };              // True = use stbi_load_from_memory

        operator bool() const noexcept { return !data.empty(); }
    };

    // For now we will only work with the diffuse texture, later support can be added for all other variables
	struct Material final
	{
        std::string name;

        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        glm::vec3 ambientColor;
        glm::vec3 emissiveColor;

        float transparency;
        float shininess;
        float refractionIndex;
        int illuminationModel;

        std::string diffuseTexture;
        EmbeddedTexture embDiffuse;
        std::string specularTexture;
        EmbeddedTexture embSpecular;
        std::string normalMap;
        EmbeddedTexture embNormal;
        std::string ambientTexture;
        EmbeddedTexture embAmbient;

        std::string bumpMap;
        std::string displacementMap;
	};
}

#endif