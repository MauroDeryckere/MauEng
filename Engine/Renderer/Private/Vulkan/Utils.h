#ifndef MAUREN_UTILS_H
#define MAUREN_UTILS_H

#include "RendererPCH.h"
#include "Vertex.h"

namespace MauRen
{
	namespace Utils
	{
#pragma region Vertices
		static VkVertexInputBindingDescription GetVertexBindingDescription() noexcept
		{
			VkVertexInputBindingDescription bindingDescription{};

			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);

			// VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
			// VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> GetVertexAttributeDescriptions() noexcept
		{
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			return attributeDescriptions;
		}
#pragma endregion
	}
}

#endif // MAUREN_UTILS_H