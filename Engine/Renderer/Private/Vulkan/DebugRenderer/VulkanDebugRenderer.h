#ifndef MAUREN_VULKANDEBUGRENDERER_H
#define MAUREN_VULKANDEBUGRENDERER_H

#include "DebugRenderer.h"

namespace MauRen
{
	class VulkanDebugRenderer final : public DebugRenderer
	{
	public:
		explicit VulkanDebugRenderer(Renderer& pRenderer);
	private:
	};
}

#endif