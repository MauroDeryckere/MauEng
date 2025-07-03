#include "RendererPCH.h"

#include "VulkanTextureManager.h"

#include "../VulkanCommandPoolManager.h"
#include "../VulkanDescriptorContext.h"


#include "Assets/ImageLoader.h"
#include "Vulkan/VulkanMemoryAllocator.h"

namespace MauRen
{
	VulkanTextureManager::VulkanTextureManager()
	{
		CreateTextureSampler();
	}


	void VulkanTextureManager::InitializeTextures(VulkanCommandPoolManager& cmdPoolManager,
		VulkanDescriptorContext& descriptorContext)
	{
		CreateDefaultTextures(cmdPoolManager, descriptorContext);
	}

	VulkanTextureManager::~VulkanTextureManager()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		for (auto& texture : m_Textures)
		{
			texture.Destroy();
		}

		VulkanUtils::SafeDestroy(deviceContext->GetLogicalDevice(), m_TextureSampler, nullptr);
	}

	void VulkanTextureManager::PreDraw(uint32_t currentFrame)
	{

	}

	bool VulkanTextureManager::IsTextureLoaded(std::string const& textureName) const noexcept
	{
		return m_TextureIDMap.contains(textureName);
	}

	uint32_t VulkanTextureManager::GetTextureID(std::string const& textureName) const noexcept
	{
		auto const it{ m_TextureIDMap.find(textureName) };

		if (it == end(m_TextureIDMap))
		{
			return INVALID_TEXTURE_ID;
		}

		return it->second.textureID;
	}

	void VulkanTextureManager::UnloadTexture(uint32_t textureID) noexcept
	{
		ME_PROFILE_FUNCTION()
		if (textureID >= m_Textures.size())
				return;

		// Don't remove default textures
		if (textureID < 6)
			return;

		auto const it{ m_TextureID_PathMap.find(textureID) };
		if (it == end(m_TextureID_PathMap))
		{
			ME_LOG_ERROR(LogRenderer, "Trying to Unload texture ID: {}, but ID does not exist", textureID);
			return;
		}

		auto const& key{ it->second };
		auto const mapIt{ m_TextureIDMap.find(key) };
		ME_RENDERER_ASSERT(mapIt != end(m_TextureIDMap));

		mapIt->second.useCount--;
		if (mapIt->second.useCount == 0)
		{
			ME_LOG_INFO(LogRenderer, "Unloading texture: {} with ID: {}", key, textureID);

			// unload & erase everywhere
			m_Textures[textureID].Destroy();
			m_FreeTextureSlots.emplace_back(textureID);
			
			m_TextureID_PathMap.erase(it);
			m_TextureIDMap.erase(mapIt);
		}
	}

	uint32_t VulkanTextureManager::LoadOrGetTexture(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, std::string const& textureName, bool isNorm) noexcept
	{
		ME_PROFILE_FUNCTION()

		if (m_Textures.size() >= MAX_TEXTURES || textureName.empty())
		{
			return INVALID_TEXTURE_ID;
		}
		std::string cleanPath{ textureName };
		std::string const prefix{ "Resources/Models/" };
		if (cleanPath.starts_with(prefix))
		{
			cleanPath.erase(0, prefix.size());
		}

		auto const it{ m_TextureIDMap.find(cleanPath) };
		if (it != end(m_TextureIDMap))
		{
			it->second.useCount++;
			return it->second.textureID;
		}

		auto const ID
		{
			(m_FreeTextureSlots.empty() ? static_cast<uint32_t>(m_Textures.size()) : m_FreeTextureSlots.front())
		};

		VulkanImage textureImage{ CreateTextureImage(cmdPoolManager, textureName, isNorm)};
		descriptorContext.BindTexture(ID, textureImage.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		if (!m_FreeTextureSlots.empty())
		{
			m_Textures[ID] = (std::move(textureImage));
			m_FreeTextureSlots.pop_front();
		}
		else
		{
			m_Textures.emplace_back(std::move(textureImage));
		}

		m_TextureIDMap[cleanPath] = { ID, 1 };
		m_TextureID_PathMap[ID] = cleanPath;

		return ID;
	}

	uint32_t VulkanTextureManager::LoadOrGetTexture(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, std::string const& textureName, EmbeddedTexture const& embTex, bool isNorm) noexcept
	{
		ME_PROFILE_FUNCTION()

		if (m_Textures.size() >= MAX_TEXTURES || textureName.empty())
		{
			return INVALID_TEXTURE_ID;
		}
		std::string cleanPath{ textureName };
		std::string const prefix{ "Resources/Models/" };
		if (cleanPath.starts_with(prefix))
		{
			cleanPath.erase(0, prefix.size());
		}

		auto const it{ m_TextureIDMap.find(cleanPath) };
		if (it != m_TextureIDMap.end())
		{
			it->second.useCount++;
			return it->second.textureID;
		}

		auto const ID
		{
			(m_FreeTextureSlots.empty() ? static_cast<uint32_t>(m_Textures.size()) : m_FreeTextureSlots.front())
		};

		VulkanImage textureImage{ CreateTextureImage(cmdPoolManager, embTex, isNorm) };
		descriptorContext.BindTexture(ID, textureImage.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		if (!m_FreeTextureSlots.empty())
		{
			m_Textures[ID] = (std::move(textureImage));
			m_FreeTextureSlots.pop_front();
		}
		else
		{
			m_Textures.emplace_back(std::move(textureImage));
		}

		m_TextureIDMap[cleanPath] = { ID, 1 };
		m_TextureID_PathMap[ID] = cleanPath;

		return ID;
	}

	void VulkanTextureManager::CreateTextureSampler()
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		// If addressed outside of bounds, repeat (tileable texture)
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(deviceContext->GetPhysicalDevice(), &properties);

		// If we want to go for maximum quality, we can simply use the limit directly
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.f;
		samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
		samplerInfo.mipLodBias = 0.0f; // Optional

		if (vkCreateSampler(deviceContext->GetLogicalDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

	void VulkanTextureManager::CreateDefaultTextures(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext)
	{
		// 0
		auto defaultWhiteTexture{ Create1x1Texture(cmdPoolManager, glm::vec4(1.0f), false) };
		descriptorContext.BindTexture(static_cast<uint32_t>(std::size(m_Textures)), defaultWhiteTexture.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_Textures.emplace_back(std::move(defaultWhiteTexture));
		m_TextureIDMap["__DefaultWhite"] = { static_cast<uint32_t>(std::size(m_Textures) - 1) , 0};

		// 1
		auto defaultGrayTexture{ Create1x1Texture(cmdPoolManager, glm::vec4(.5f), false) };
		descriptorContext.BindTexture(static_cast<uint32_t>(std::size(m_Textures)), defaultGrayTexture.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_Textures.emplace_back(std::move(defaultGrayTexture));
		m_TextureIDMap["__DefaultGray"] = { static_cast<uint32_t>(std::size(m_Textures) - 1) , 0 };
		
		// 2
		auto defaultNormalTexture{ Create1x1Texture(cmdPoolManager, glm::vec4(0.5f, 0.5f, 1.0f, 1.0f), false) };
		descriptorContext.BindTexture(static_cast<uint32_t>(std::size(m_Textures)), defaultNormalTexture.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_Textures.emplace_back(std::move(defaultNormalTexture));
		m_TextureIDMap["__DefaultNormal"] = { static_cast<uint32_t>(std::size(m_Textures) - 1) , 0 };

		// 3
		auto defaultBlackTexture{ Create1x1Texture(cmdPoolManager, glm::vec4(0.0f), false) };
		descriptorContext.BindTexture(static_cast<uint32_t>(std::size(m_Textures)), defaultBlackTexture.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_Textures.emplace_back(std::move(defaultBlackTexture));
		m_TextureIDMap["__DefaultBlack"] = { static_cast<uint32_t>(std::size(m_Textures) - 1) , 0 };

		// 4
		auto defaultMetalnessTexture{ Create1x1Texture(cmdPoolManager, glm::vec4(1.0f, 1.0f, 0.f, 1.0f), false) };
		descriptorContext.BindTexture(static_cast<uint32_t>(std::size(m_Textures)), defaultMetalnessTexture.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_Textures.emplace_back(std::move(defaultMetalnessTexture));
		m_TextureIDMap["__DefaultMetalness"] = { static_cast<uint32_t>(std::size(m_Textures) - 1) , 0 };

		// 5
		auto invalidTexture{ Create1x1Texture(cmdPoolManager, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), false) };
		descriptorContext.BindTexture(static_cast<uint32_t>(std::size(m_Textures)), invalidTexture.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_Textures.emplace_back(std::move(invalidTexture));
		m_TextureIDMap["__DefaultInvalid"] = { static_cast<uint32_t>(std::size(m_Textures) - 1) , 0 };

	}

	VulkanImage VulkanTextureManager::CreateTextureImage(VulkanCommandPoolManager& cmdPoolManager, std::string const& path, bool isNorm)
	{
		ME_PROFILE_FUNCTION()

		ME_ASSERT(std::filesystem::exists(path));

		Image const img{ path };

		VkDeviceSize const imageSize{ static_cast<uint32_t>(img.width * img.height * 4) };


		VulkanBuffer stagingBuffer{ imageSize,
									 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		void* data;
		vmaMapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), stagingBuffer.alloc, &data);
		memcpy(data, img.pixels, static_cast<size_t>(imageSize));
		vmaUnmapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), stagingBuffer.alloc);

		VulkanImage texImage
		{
			(isNorm ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB),
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			static_cast<uint32_t>(img.width),
			static_cast<uint32_t>(img.height),
			static_cast<uint32_t>(std::floor(std::log2(std::max(img.width, img.height)))) + 1
		};

		texImage.TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE);
		VulkanBuffer::CopyBufferToImage(cmdPoolManager, stagingBuffer.buffer, texImage.image, static_cast<uint32_t>(img.width), static_cast<uint32_t>(img.height));
		// is transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		texImage.GenerateMipmaps(cmdPoolManager);

		stagingBuffer.Destroy();

		texImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

		return texImage;
	}

	VulkanImage VulkanTextureManager::CreateTextureImage(VulkanCommandPoolManager& cmdPoolManager, EmbeddedTexture const& embTex, bool isNorm)
	{
		ME_PROFILE_FUNCTION()

		ME_ASSERT(embTex.hash != std::string{ "INVALID" });

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		Image img{ embTex.data.data(),embTex.data.size(), embTex.isCompressed, 4 };

		if (not embTex.isCompressed)
		{
			img.width = embTex.width;
			img.height = embTex.height;
		}

		VkDeviceSize const imageSize{ static_cast<uint32_t>(img.width * img.height * 4) };

		VulkanBuffer stagingBuffer{ imageSize,
									 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		void* data;
		vmaMapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), stagingBuffer.alloc, &data);
		memcpy(data, img.pixels, static_cast<size_t>(imageSize));
		vmaUnmapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), stagingBuffer.alloc);

		VulkanImage texImage
		{
			isNorm ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			static_cast<uint32_t>(img.width),
			static_cast<uint32_t>(img.height),
			static_cast<uint32_t>(std::floor(std::log2(std::max(img.width, img.height)))) + 1
		};

		texImage.TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE);
		VulkanBuffer::CopyBufferToImage(cmdPoolManager, stagingBuffer.buffer, texImage.image, static_cast<uint32_t>(img.width), static_cast<uint32_t>(img.height));
		// is transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		texImage.GenerateMipmaps(cmdPoolManager);

		stagingBuffer.Destroy();

		texImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

		return texImage;
	}

	VulkanImage VulkanTextureManager::Create1x1Texture(VulkanCommandPoolManager& cmdPoolManager, glm::vec4 const& color, bool isNorm)
	{
		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		uint32_t pixel =
		(static_cast<uint8_t>(color.r * 255.0f) << 0)  |
		(static_cast<uint8_t>(color.g * 255.0f) << 8)  |
		(static_cast<uint8_t>(color.b * 255.0f) << 16) |
		(static_cast<uint8_t>(color.a * 255.0f) << 24);

		VkDeviceSize imageSize{ sizeof(uint32_t) };

		VulkanBuffer stagingBuffer{ imageSize,
									 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		void* data;
		vmaMapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), stagingBuffer.alloc, &data);
		memcpy(data, &pixel, static_cast<size_t>(imageSize));
		vmaUnmapMemory(VulkanMemoryAllocator::GetInstance().GetAllocator(), stagingBuffer.alloc);


		VulkanImage texImage
		{
			isNorm ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			static_cast<uint32_t>(1),
			static_cast<uint32_t>(1),
			static_cast<uint32_t>(std::floor(std::log2(std::max(1, 1)))) + 1
		};


		texImage.TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE);
		VulkanBuffer::CopyBufferToImage(cmdPoolManager, stagingBuffer.buffer, texImage.image, static_cast<uint32_t>(1), static_cast<uint32_t>(1));
		// is transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		texImage.GenerateMipmaps(cmdPoolManager);

		stagingBuffer.Destroy();

		texImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

		return texImage;
	}
}
