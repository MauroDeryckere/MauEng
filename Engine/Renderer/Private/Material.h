#ifndef MAUREN_MATERIAL_H
#define MAUREN_MATERIAL_H

#include <glm/glm.hpp>
#include <string>

namespace MauRen
{
    // Fo now we will only work with the diffuse texture, later support can be added for all other variables
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
        std::string specularTexture;
        std::string normalMap;
        std::string ambientTexture;

        std::string bumpMap;
        std::string displacementMap;
	};
}

#endif