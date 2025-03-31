#ifndef MAUREN_VULKANTEXTURE_H
#define MAUREN_VULKANTEXTURE_H

#include "RendererPCH.h"
#include "VulkanImage.h"

namespace MauRen
{
	struct VulkanTexture final
	{

		VulkanImage textureImage{};

		void Initialize();
		void Destroy();
	};

	inline void VulkanTexture::Destroy()
	{
		textureImage.Destroy();
	}
}

#endif