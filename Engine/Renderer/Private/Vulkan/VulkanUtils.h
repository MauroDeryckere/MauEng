#ifndef MAUREN_VULKAN_UTILS_H
#define MAUREN_VULKAN_UTILS_H

#include "RendererPCH.h"
#include "Vertex.h"

namespace MauRen
{
	namespace VulkanUtils
	{
#pragma region Management
#pragma region Destroys
		inline bool SafeDestroy(VkDevice device, VkDescriptorPool& descriptorPool, VkAllocationCallbacks const* pAllocator)
		{
			if (descriptorPool == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroyDescriptorPool(device, descriptorPool, pAllocator);
			descriptorPool = VK_NULL_HANDLE;
			return true;
		}
		inline bool SafeDestroy(VkDevice device, VkDescriptorSetLayout& descriptorSet, VkAllocationCallbacks const* pAllocator)
		{
			if (descriptorSet == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroyDescriptorSetLayout(device, descriptorSet, pAllocator);
			descriptorSet = VK_NULL_HANDLE;
			return true;
		}
		inline bool SafeDestroy(VkDevice device, VkBuffer& buffer, VkAllocationCallbacks const* pAllocator)
		{
			if (buffer == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroyBuffer(device, buffer, pAllocator);
			buffer = VK_NULL_HANDLE;
			return true;
		}
		inline bool SafeDestroy(VkDevice device, VkDeviceMemory& deviceMemory, VkAllocationCallbacks const* pAllocator)
		{
			if (deviceMemory == VK_NULL_HANDLE)
			{
				return false;
			}

			vkFreeMemory(device, deviceMemory, pAllocator);
			deviceMemory = VK_NULL_HANDLE;
			return true;
		}
		inline bool SafeDestroy(VkDevice device, VkPipeline& pipeline, VkAllocationCallbacks const* pAllocator)
		{
			if (pipeline == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroyPipeline(device, pipeline, pAllocator);
			pipeline = VK_NULL_HANDLE;
			return true;
		}
		inline bool SafeDestroy(VkDevice device, VkPipelineLayout& pipelineLayout, VkAllocationCallbacks const* pAllocator)
		{
			if (pipelineLayout == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
			pipelineLayout = VK_NULL_HANDLE;
			return true;
		}
		inline bool SafeDestroy(VkDevice device, VkRenderPass& renderPass, VkAllocationCallbacks const* pAllocator)
		{
			if (renderPass == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroyRenderPass(device, renderPass, pAllocator);
			renderPass = VK_NULL_HANDLE;
			return true;
		}
		inline bool SafeDestroy(VkDevice device, VkShaderModule& shaderModule, VkAllocationCallbacks const* pAllocator)
		{
			if (shaderModule == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroyShaderModule(device, shaderModule, pAllocator);
			shaderModule = VK_NULL_HANDLE;
			return true;
		}
		inline bool SafeDestroy(VkDevice device, VkImage& image, VkAllocationCallbacks const* pAllocator)
		{
			if (image == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroyImage(device, image, pAllocator);
			image = VK_NULL_HANDLE;
			return true;
		}
		inline bool SafeDestroy(VkDevice device, VkImageView& imageView, VkAllocationCallbacks const* pAllocator)
		{
			if (imageView == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroyImageView(device, imageView, pAllocator);
			imageView = VK_NULL_HANDLE;
			return true;
		}

		inline bool SafeDestroy(VkDevice device, VkSwapchainKHR& swapchain, VkAllocationCallbacks const* pAllocator)
		{
			if (swapchain == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroySwapchainKHR(device, swapchain, pAllocator);
			swapchain = VK_NULL_HANDLE;
			return true;
		}

		inline bool SafeDestroy(VkDevice device, VkCommandPool& commandPool, VkAllocationCallbacks const* pAllocator)
		{
			if (commandPool == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroyCommandPool(device, commandPool, pAllocator);
			commandPool = VK_NULL_HANDLE;
			return true;
		}
		inline bool SafeDestroy(VkDevice device, VkFramebuffer& frameBuffer, VkAllocationCallbacks const* pAllocator)
		{
			if (frameBuffer == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroyFramebuffer(device, frameBuffer, pAllocator);
			frameBuffer = VK_NULL_HANDLE;
			return true;
		}
		inline bool SafeDestroy(VkDevice device, VkSampler& sampler, VkAllocationCallbacks const* pAllocator)
		{
			if (sampler == VK_NULL_HANDLE)
			{
				return false;
			}

			vkDestroySampler(device, sampler, pAllocator);
			sampler = VK_NULL_HANDLE;
			return true;
		}

#pragma endregion

		inline [[nodiscard]] uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
		{
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

			for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
			{
				if (typeFilter & (1 << i) and (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					return i;
				}
			}

			throw std::runtime_error("Failed to find suitable memory type!");
		}
#pragma endregion

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

		static std::array<VkVertexInputAttributeDescription, 3> GetVertexAttributeDescriptions() noexcept
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}
#pragma endregion
	}
}

#endif // MAUREN_VULKAN_UTILS_H