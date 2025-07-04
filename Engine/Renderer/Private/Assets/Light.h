#ifndef MAUREN_LIGHT_H
#define MAUREN_LIGHT_H

#include <cstdint>
#include "RendererIdentifiers.h"
#include <glm/glm.hpp>

namespace MauRen
{
	// Light that's stored on the GPU
	struct alignas(16) Light final
	{
		glm::mat4 lightViewProj;

		// Direction for directional lights, position for point lights
		glm::vec3 direction_position{ 0.0f, -1.0f, 0.0f };
		// Light type: 0 = directional, 1 = point
		uint32_t type{ 0 };

		glm::vec3 color = glm::vec3(1.0f);
		// lux for directional light, lumen for point light
		float lumen_lux{ 1.0f };

		// Index into shadow texture array
		uint32_t shadowMapIndex{ INVALID_SHADOW_MAP_ID };
		int castsShadows{ 1 };

		// room for 8 more bytes, padding
	};
}

#endif