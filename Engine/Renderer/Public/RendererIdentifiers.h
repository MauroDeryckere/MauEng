#ifndef MAUREN_RENDERERIDENTIFIERS_H
#define MAUREN_RENDERERIDENTIFIERS_H

namespace MauRen
{
	uint32_t constexpr INVALID_MESH_ID{ UINT32_MAX };

	uint32_t constexpr INVALID_DIFFUSE_TEXTURE_ID{ 0 };
	uint32_t constexpr INVALID_SPECULAR_TEXTURE_ID{ 1 };
	uint32_t constexpr INVALID_NORMAL_TEXTURE_ID{ 2 };
	uint32_t constexpr INVALID_AMBIENT_TEXTURE_ID{ 3 };

	uint32_t constexpr INVALID_TEXTURE_ID{ 4 };

	uint32_t constexpr INVALID_MATERIAL_ID{ 0 };

	uint32_t constexpr INVALID_DRAW_COMMAND{ UINT32_MAX };
}

#endif