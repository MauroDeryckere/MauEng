#ifndef MAUREN_VULKANTEXTUREMANAGER_H
#define MAUREN_VULKANTEXTUREMANAGER_H

#include <unordered_map>

#include "VulkanTexture.h"

namespace MauRen
{
	class VulkanDescriptorContext;
	class VulkanCommandPoolManager;

	class VulkanTextureManager final
	{
	public:
		uint32_t const INVALID_TEXTURE_ID{ UINT32_MAX };

		VulkanTextureManager();
		~VulkanTextureManager();

		[[nodiscard]] bool IsTextureLoaded(std::string const& textureName) const noexcept;
		[[nodiscard]] uint32_t GetTextureID(std::string const& textureName) const noexcept;

		[[nodiscard]] uint32_t LoadOrGetTexture(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, std::string const& textureName) noexcept;

		[[nodiscard]] VkSampler GetTextureSampler() const noexcept { return m_TextureSampler; }

		VulkanTextureManager(VulkanTextureManager const&) = delete;
		VulkanTextureManager(VulkanTextureManager&&) = delete;
		VulkanTextureManager& operator=(VulkanTextureManager const&) = delete;
		VulkanTextureManager& operator=(VulkanTextureManager const&&) = delete;
	private:
		// Texture name, ID
		// ID maps directly to the ID in the vulkan buffer & should be reflected in the material
		std::unordered_map<std::string, uint32_t> m_TextureIDMap;

		// Vector of all the texture images - this is a 1:1 with the buffer on the GPU
		std::vector<VulkanTexture> m_Textures;

		// for now one global sampler is used.
		VkSampler m_TextureSampler{ VK_NULL_HANDLE };

		void CreateTextureSampler();

		VulkanImage CreateTextureImage(VulkanCommandPoolManager& cmdPoolManager, std::string const& path);
	};
}

#endif