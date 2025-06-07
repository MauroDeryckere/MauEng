#ifndef MAUENG_CLIGHT_H
#define MAUENG_CLIGHT_H

#include "RendererIdentifiers.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace MauEng
{
	enum class ELightType : uint32_t
	{
		DIRECTIONAL = 0,
		POINT = 1,
		COUNT
	};

	struct CLight final
	{
		ELightType type{ ELightType::DIRECTIONAL };

		bool isEnabled{ true };
		bool castShadows{ true };

		glm::vec3 lightColour{ 1.0f, 1.0f, 1.0f };
		// Directional light direction or position for other lighttypes
		glm::vec3 direction_position{ -1.0f, -1.0f, -1.0f };

		// lumen for point light, lux for directional light
		float lumen_lux{ 1.0f };

		uint32_t lightID{ MauRen::INVALID_LIGHT_ID };

		CLight();
	};
}

#endif