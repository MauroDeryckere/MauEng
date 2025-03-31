#ifndef MAUREN_VULKANMATERIAL_H
#define MAUREN_VULKANMATERIAL_H

#include "RendererPCH.h"

namespace MauRen
{
	struct VulkanMaterial final
	{
		// Only supporting this for now
		uint32_t albedoTexture{ UINT32_MAX };
	};
}

#endif