#ifndef MAUREN_VULKANDEBUGRENDERER_H
#define MAUREN_VULKANDEBUGRENDERER_H

#include "DebugRenderer.h"

namespace MauRen
{
	class VulkanDebugRenderer final : public DebugRenderer
	{
	public:
		explicit VulkanDebugRenderer();

	private:
		
		std::vector<glm::vec3> m_ActiveLines;
		uint32_t const MAX_LINES { 1'000 };
	};
}

#endif