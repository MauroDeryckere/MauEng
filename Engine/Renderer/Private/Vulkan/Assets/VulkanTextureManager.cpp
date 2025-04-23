#include "RendererPCH.h"

#include "VulkanTextureManager.h"

#include "../VulkanCommandPoolManager.h"
#include "../VulkanDescriptorContext.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

		return it->second;
	}

	uint32_t VulkanTextureManager::LoadOrGetTexture(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, std::string const& textureName) noexcept
	{
		ME_PROFILE_FUNCTION()

		if (m_Textures.size() >= MAX_TEXTURES || textureName.empty())
		{
			return INVALID_TEXTURE_ID;
		}

		auto const it{ m_TextureIDMap.find(textureName) };
		if (it != end(m_TextureIDMap))
		{
			return it->second;
		}

		VulkanImage textureImage{ CreateTextureImage(cmdPoolManager, textureName)};
		descriptorContext.BindTexture(m_Textures.size(), textureImage.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_Textures.emplace_back(std::move(textureImage));
		m_TextureIDMap[textureName] = m_Textures.size() - 1;

		return m_Textures.size() - 1;
	}

	uint32_t VulkanTextureManager::LoadOrGetTexture(VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext, std::string const& textureName, EmbeddedTexture const& embTex) noexcept
	{
		ME_PROFILE_FUNCTION()

		if (m_Textures.size() >= MAX_TEXTURES || textureName.empty())
		{
			return INVALID_TEXTURE_ID;
		}

		auto const it = m_TextureIDMap.find(textureName);
		if (it != m_TextureIDMap.end())
		{
			return it->second;
		}

		VulkanImage textureImage{ CreateTextureImage(cmdPoolManager, embTex) };
		descriptorContext.BindTexture(m_Textures.size(), textureImage.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_Textures.emplace_back(std::move(textureImage));
		m_TextureIDMap[textureName] = m_Textures.size() - 1;

		return m_Textures.size() - 1;
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
		//samplerInfo.minLod = static_cast<float>(m_TextureImage.mipLevels / 2);
		//samplerInfo.maxLod = static_cast<float>(m_TextureImage.mipLevels);
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
		auto defaultWhiteTexture{ Create1x1Texture(cmdPoolManager, glm::vec4(1.0f)) };
		descriptorContext.BindTexture(m_Textures.size(), defaultWhiteTexture.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_Textures.emplace_back(std::move(defaultWhiteTexture));
		m_TextureIDMap["__DefaultWhite"] = m_Textures.size() - 1;

		// 1
		auto defaultGrayTexture{ Create1x1Texture(cmdPoolManager, glm::vec4(.5f)) };
		descriptorContext.BindTexture(m_Textures.size(), defaultGrayTexture.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		m_Textures.emplace_back(std::move(defaultGrayTexture));
		m_TextureIDMap["__DefaultGray"] = m_Textures.size() - 1;

		// 2
		auto defaultNormalTexture{ Create1x1Texture(cmdPoolManager, glm::vec4(0.5f, 0.5f, 1.0f, 1.0f)) };
		descriptorContext.BindTexture(m_Textures.size(), defaultNormalTexture.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_Textures.emplace_back(std::move(defaultNormalTexture));
		m_TextureIDMap["__DefaultNormal"] = m_Textures.size() - 1;

		// 3
		auto defaultBlackTexture{ Create1x1Texture(cmdPoolManager, glm::vec4(0.0f)) };
		descriptorContext.BindTexture(m_Textures.size(), defaultBlackTexture.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_Textures.emplace_back(std::move(defaultBlackTexture));
		m_TextureIDMap["__DefaultBlack"] = m_Textures.size() - 1;

		// 4
		auto invalidTexture{ Create1x1Texture(cmdPoolManager, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f)) };
		descriptorContext.BindTexture(m_Textures.size(), invalidTexture.imageViews[0], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		m_Textures.emplace_back(std::move(invalidTexture));
		m_TextureIDMap["__DefaultInvalid"] = m_Textures.size() - 1;
	}

	VulkanImage VulkanTextureManager::CreateTextureImage(VulkanCommandPoolManager& cmdPoolManager, std::string const& path)
	{
		ME_PROFILE_FUNCTION()

		ME_ASSERT(std::filesystem::exists(path));

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		int texWidth{};
		int texHeight{};
		int texChannels{};

		stbi_uc* const pixels{ stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha) };
		VkDeviceSize const imageSize{ static_cast<uint32_t>(texWidth * texHeight * 4) };

		if (!pixels or texWidth == 0 or texHeight == 0)
		{
			throw std::runtime_error("Failed to load texture image!");
		}

		VulkanBuffer stagingBuffer{ imageSize,
									 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		void* data;
		vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);

		stbi_image_free(pixels);

		VulkanImage texImage
		{
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			static_cast<uint32_t>(texWidth),
			static_cast<uint32_t>(texHeight),
			static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1
		};

		texImage.TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VulkanBuffer::CopyBufferToImage(cmdPoolManager, stagingBuffer.buffer, texImage.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		// is transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		texImage.GenerateMipmaps(cmdPoolManager);

		stagingBuffer.Destroy();

		texImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

		return texImage;
	}

	VulkanImage VulkanTextureManager::CreateTextureImage(VulkanCommandPoolManager& cmdPoolManager, EmbeddedTexture const& embTex)
	{
		ME_PROFILE_FUNCTION()

		ME_ASSERT(embTex.hash != std::string{ "INVALID" });

		auto const deviceContext{ VulkanDeviceContextManager::GetInstance().GetDeviceContext() };

		int texWidth{};
		int texHeight{};
		int texChannels{};

		stbi_uc* pixels{ nullptr };

		if (embTex.isCompressed)
		{
			// Load from memory like PNG/JPG
			pixels = stbi_load_from_memory(embTex.data.data(), static_cast<int>(embTex.data.size()), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		}
		else
		{
			// Already raw RGBA data, assume 4 channels
			texWidth = embTex.width;
			texHeight = embTex.height;
			texChannels = 4;
			pixels = new stbi_uc[embTex.data.size()];
			std::memcpy(pixels, embTex.data.data(), embTex.data.size());
		}

		if (!pixels or texWidth == 0 or texHeight == 0)
		{
			throw std::runtime_error("Failed to load embedded texture image!");
		}

		VkDeviceSize const imageSize{ static_cast<uint32_t>(texWidth * texHeight * 4) };

		VulkanBuffer stagingBuffer{ imageSize,
									 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		void* data;
		vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);

		if (embTex.isCompressed)
		{
			stbi_image_free(pixels);
		}
		else
		{
			delete[] pixels;
		}

		VulkanImage texImage
		{
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			static_cast<uint32_t>(texWidth),
			static_cast<uint32_t>(texHeight),
			static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1
		};

		texImage.TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VulkanBuffer::CopyBufferToImage(cmdPoolManager, stagingBuffer.buffer, texImage.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		// is transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		texImage.GenerateMipmaps(cmdPoolManager);

		stagingBuffer.Destroy();

		texImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

		return texImage;
	}

	VulkanImage VulkanTextureManager::Create1x1Texture(VulkanCommandPoolManager& cmdPoolManager, glm::vec4 const& color)
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
		vkMapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory, 0, imageSize, 0, &data);
		memcpy(data, &pixel, static_cast<size_t>(imageSize));
		vkUnmapMemory(deviceContext->GetLogicalDevice(), stagingBuffer.bufferMemory);


		VulkanImage texImage
		{
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			static_cast<uint32_t>(1),
			static_cast<uint32_t>(1),
			static_cast<uint32_t>(std::floor(std::log2(std::max(1, 1)))) + 1
		};


		texImage.TransitionImageLayout(cmdPoolManager, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VulkanBuffer::CopyBufferToImage(cmdPoolManager, stagingBuffer.buffer, texImage.image, static_cast<uint32_t>(1), static_cast<uint32_t>(1));
		// is transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

		texImage.GenerateMipmaps(cmdPoolManager);

		stagingBuffer.Destroy();

		texImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

		return texImage;
	}
}
