#ifndef MAUREN_VULKANDESCRIPTORCONTEXT_H
#define MAUREN_VULKANDESCRIPTORCONTEXT_H

#include <variant>

#include "RendererPCH.h"
#include "VulkanBuffer.h"

#include "Assets/BindlessData.h"

namespace MauRen
{
	// Currently only supports one layout, more layouts is more optimal;
	// but to get everything working now I put everything is the single layout

	//TODO descriptor update queue

	class VulkanDescriptorContext final
	{
	public:
		explicit VulkanDescriptorContext() = default;
		~VulkanDescriptorContext() = default;

		void Initialize() {}
		void Destroy();

		void ProcessDescriptorUpdateQueue(uint32_t frame) noexcept;

		[[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout() const noexcept { return m_DescriptorSetLayout; }
		[[nodiscard]] std::vector<VkDescriptorSet> const& GetDescriptorSets() const noexcept { return m_DescriptorSets; }
		[[nodiscard]] VkDescriptorPool GetDescriptorPool() const noexcept { return m_DescriptorPool; }
		[[nodiscard]] VkDescriptorPool GetImGUIPool() const noexcept { return m_ImGUIPool; }

		void BindTexture(uint32_t destLocation, VkImageView imageView, VkImageLayout imageLayout);
		void BindMaterialBuffer(VkDescriptorBufferInfo bufferInfo, uint32_t frame);
		void BindLightBuffer(VkDescriptorBufferInfo bufferInfo, uint32_t frame);

		void BindShadowMap(uint32_t destLocation, VkImageView imageView, VkImageLayout imageLayout);

		void BindShadowMapSampler(VkSampler sampler);

		void BindMeshInstanceDataBuffer(VkDescriptorBufferInfo bufferInfo, uint32_t frame);

		void CreateDescriptorSetLayout();
		void CreateDescriptorPool();

		//TODO rework this function
		void CreateDescriptorSets(std::vector<VulkanBuffer> const& bufferInfoBuffers, VkDeviceSize offset, VkDeviceSize range, VkSampler sampler,
								  std::vector<VulkanBuffer> const& bufferInfoBuffersCamSettings, VkDeviceSize offsetCamSettings, VkDeviceSize rangeCamSettings);

		VulkanDescriptorContext(VulkanDescriptorContext const&) = delete;
		VulkanDescriptorContext(VulkanDescriptorContext&&) = delete;
		VulkanDescriptorContext& operator=(VulkanDescriptorContext const&) = delete;
		VulkanDescriptorContext& operator=(VulkanDescriptorContext&&) = delete;

		[[nodiscard]] uint32_t GetLightBufferSlot() const noexcept { return LIGHT_BUFFER_SLOT; }

	private:
		VkDescriptorSetLayout m_DescriptorSetLayout{ VK_NULL_HANDLE };
		VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };

		VkDescriptorPool m_ImGUIPool{ VK_NULL_HANDLE };

		// !
		// common practice to rank from "least updated" descriptor set(index 0) to "most frequent updated"
		// descriptor set(index N)!This way, we can avoid rebinding as much as possible!
		std::vector<VkDescriptorSet> m_DescriptorSets{};

		uint32_t const UBO_BINDING_SLOT{ 0 };
		uint32_t const SAMPLER_BINDING_SLOT{ 1 };
		uint32_t const TEXTURE_BINDING_SLOT{ 2 };
		uint32_t const MATERIAL_DATA_BINDING_SLOT{ 3 };

		// currently unused!; may use in future
		uint32_t const MESH_DATA_BINDING_SLOT{ 4 };
		uint32_t const MESH_INSTANCE_DATA_BINDING_SLOT{ 5 };

		uint32_t const GBUFFER_COLOR_SLOT{ 6 };
		uint32_t const GBUFFER_NORMAL_SLOT{ 7 };
		uint32_t const GBUFFER_METAL_SLOT{ 8 };
		uint32_t const GBUFFER_DEPTH_SLOT{ 9 };

		uint32_t const HDRI_COLOR_SLOT{ 10 };

		uint32_t const SHADOW_MAPS_SLOT{ 11 };
		uint32_t const LIGHT_BUFFER_SLOT{ 12 };
		uint32_t const SHADOW_MAP_SAMPLER{ 13 };

		uint32_t const CAM_SETTINGS_SLOT{ 14 };

		struct DescriptorSetUpdate final
		{
			enum class EType : uint8_t
			{
				Buffer,
				Image,
				TexelBuffer
			};

			// Depending on the type, one of these will be used
			std::variant<
				std::vector<VkDescriptorBufferInfo>,
				std::vector<VkDescriptorImageInfo>,
				std::vector<VkBufferView>
			>descriptorData;
			
			VkDescriptorSet targetSet;


			uint32_t dstBinding;
			uint32_t dstArrayElement;
			VkDescriptorType descriptorType;

			EType type;

			// Factory methods to create DescriptorSetUpdate easily
			[[nodiscard]] static DescriptorSetUpdate CreateBufferUpdate(
				VkDescriptorSet target,
				uint32_t binding,
				uint32_t arrayElement,
				VkDescriptorType descriptorType,
				std::vector<VkDescriptorBufferInfo>&& bufferInfos)
			{
				return DescriptorSetUpdate
				{
					.descriptorData =std::move(bufferInfos),
					.targetSet = target,
					.dstBinding = binding,
					.dstArrayElement = arrayElement,
					.descriptorType = descriptorType,
					.type = EType::Buffer
				};
			}

			[[nodiscard]] static DescriptorSetUpdate CreateImageUpdate(
				VkDescriptorSet target,
				uint32_t binding,
				uint32_t arrayElement,
				VkDescriptorType descriptorType,
				std::vector<VkDescriptorImageInfo>&& imageInfos)
			{
				return DescriptorSetUpdate
				{
					.descriptorData = std::move(imageInfos),
					.targetSet = target,
					.dstBinding = binding,
					.dstArrayElement = arrayElement,
					.descriptorType = descriptorType,
					.type = EType::Image
				};
			}

			[[nodiscard]] static DescriptorSetUpdate CreateTexelBufferUpdate(
				VkDescriptorSet target,
				uint32_t binding,
				uint32_t arrayElement,
				VkDescriptorType descriptorType,
				std::vector<VkBufferView>&& texelBuffers)
			{
				return DescriptorSetUpdate
				{
					.descriptorData =std::move(texelBuffers),
					.targetSet = target,
					.dstBinding = binding,
					.dstArrayElement = arrayElement,
					.descriptorType = descriptorType,
					.type = EType::TexelBuffer
				};
			}
		};

		std::array<std::vector<DescriptorSetUpdate>, MAX_FRAMES_IN_FLIGHT> m_DescriptorSetUpdates{};
	};
}

#endif