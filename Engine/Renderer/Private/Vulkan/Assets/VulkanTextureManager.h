#ifndef MAUREN_VULKANTEXTUREMANAGER_H
#define MAUREN_VULKANTEXTUREMANAGER_H

#include <unordered_map>

#include "BindlessData.h"
#include "VulkanImage.h"
#include "Assets/Material.h"

namespace MauRen
{
	class VulkanDescriptorContext;
	class VulkanCommandPoolManager;

	class VulkanTextureManager final
	{
	public:
		VulkanTextureManager();
		void InitializeTextures(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext);

		~VulkanTextureManager();

		void PreDraw(uint32_t currentFrame);

		[[nodiscard]] bool IsTextureLoaded(std::string const& textureName) const noexcept;
		[[nodiscard]] uint32_t GetTextureID(std::string const& textureName) const noexcept;

		void UnloadTexture(uint32_t textureID) noexcept;
		[[nodiscard]] uint32_t LoadOrGetTexture(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, std::string const& textureName, bool isNorm) noexcept;
		[[nodiscard]] uint32_t LoadOrGetTexture(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, std::string const& textureName, EmbeddedTexture const& embTex, bool isNorm) noexcept;

		[[nodiscard]] VkSampler GetTextureSampler() const noexcept { return m_TextureSampler; }

		[[nodiscard]] std::unordered_map<std::string, LoadedTextureInfo>const& GetTextureMap() const noexcept { return m_TextureIDMap; }

		VulkanTextureManager(VulkanTextureManager const&) = delete;
		VulkanTextureManager(VulkanTextureManager&&) = delete;
		VulkanTextureManager& operator=(VulkanTextureManager const&) = delete;
		VulkanTextureManager& operator=(VulkanTextureManager const&&) = delete;
	private:
		// Texture name, ID
		// ID maps directly to the ID in the vulkan buffer & should be reflected in the material
		std::unordered_map<std::string, LoadedTextureInfo> m_TextureIDMap;
		std::unordered_map<uint32_t, std::string> m_TextureID_PathMap;

		// Vector of all the texture images - this is a 1:1 with the buffer on the GPU
		std::vector<VulkanImage> m_Textures;

		// for now one global sampler is used.
		VkSampler m_TextureSampler{ VK_NULL_HANDLE };

		std::deque<uint32_t> m_FreeTextureSlots;

		std::vector<std::pair<uint32_t, uint32_t>> m_TexturesToDestroyWhen3frames;

		void CreateTextureSampler();

		void CreateDefaultTextures(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext);

		[[nodiscard]] VulkanImage CreateTextureImage(VulkanCommandPoolManager& cmdPoolManager, std::string const& path, bool isNorm);
		[[nodiscard]] VulkanImage CreateTextureImage(VulkanCommandPoolManager& cmdPoolManager, EmbeddedTexture const& embTex, bool isNorm);

		[[nodiscard]] VulkanImage Create1x1Texture(VulkanCommandPoolManager& cmdPoolManager, glm::vec4 const& color, bool isNorm);
	};
}

#endif