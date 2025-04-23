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

	struct Material final
	{
        std::string name {"DefaultMaterial"};

        glm::vec3 diffuseColor { 1.0f };
        glm::vec3 specularColor { 0.0f };
        glm::vec3 ambientColor {0.f };
        glm::vec3 emissiveColor {0.f};

        float transparency {0.f};
        float shininess {0.f};
        float refractionIndex {1.f};
        int illuminationModel{1};

        std::string diffuseTexture{ "__DefaultWhite" };
        EmbeddedTexture embDiffuse{};
        std::string specularTexture{ "__DefaultGray" };
        EmbeddedTexture embSpecular{};
        std::string normalMap{ "__DefaultNormal" };
        EmbeddedTexture embNormal{};
        std::string ambientTexture{ "__DefaultGray" };
        EmbeddedTexture embAmbient{};

        std::string displacementMap{"NONE"};
	};
}

#endif