#ifndef MAUREN_VULKANRENDERER_H
#define MAUREN_VULKANRENDERER_H

#include "RendererPCH.h"

#include "Renderer.h"

#include "VulkanInstanceContext.h"
#include "VulkanSurfaceContext.h"
#include "VulkanDebugContext.h"
#include "VulkanDeviceContext.h"
#include "VulkanDescriptorContext.h"
#include "VulkanSwapchainContext.h"
#include "VulkanGraphicsPipelineContext.h"
#include "VulkanCommandPoolManager.h"

#include "VulkanBuffer.h"

#include "Assets/VulkanImage.h"

#include "DebugRenderer/DebugVertex.h"

// Sources
// https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples#swapchain-image-acquire-and-present
// https://blog.traverseresearch.nl/bindless-rendering-setup-afeb678d77fc
// DAE course - Graphics Programming 2

namespace MauEng
{
	struct CStaticMesh;
}

namespace MauRen
{
	class VulkanRenderer final : public Renderer
	{
	public:
		explicit VulkanRenderer(SDL_Window* pWindow, class DebugRenderer& debugRenderer);
		virtual ~VulkanRenderer() override = default;

		virtual void Init() override;
		virtual void Destroy() override;

		virtual void Render(glm::mat4 const& view, glm::mat4 const& proj) override;
		virtual void ResizeWindow() override;

		virtual void QueueDraw(glm::mat4 const& transformMat, MauEng::CStaticMesh const& mesh) override;
		virtual [[nodiscard]] uint32_t LoadOrGetMeshID(char const* path) override;

		VulkanRenderer(VulkanRenderer const&) = delete;
		VulkanRenderer(VulkanRenderer&&) = delete;
		VulkanRenderer& operator=(VulkanRenderer const&) = delete;
		VulkanRenderer& operator=(VulkanRenderer&&) = delete;

	private:
		// "reference" to the window
		SDL_Window* m_pWindow{ nullptr };

		class InternalDebugRenderer* m_DebugRenderer = nullptr;

		VulkanInstanceContext m_InstanceContext{};
		VulkanSurfaceContext m_SurfaceContext{};
		VulkanDebugContext m_DebugContext{};

		VulkanDescriptorContext m_DescriptorContext{};
		VulkanSwapchainContext m_SwapChainContext{};
		VulkanGraphicsPipelineContext* m_GraphicsPipeline{};

		VulkanCommandPoolManager m_CommandPoolManager{};

		// Signal that an image has been acquired from the swapchain and is ready for rendering
		std::vector<VkSemaphore> m_ImageAvailableSemaphores{};

		// Signal that rendering has finished and presentation can happen
		std::vector<VkSemaphore> m_RenderFinishedSemaphores{};

		// Fence to make sure only one frame is rendering at a time
		std::vector<VkFence> m_InFlightFences{};

		uint32_t m_CurrentFrame{ 0 };

		bool m_FramebufferResized{ false };

		struct alignas(16) UniformBufferObject final
		{
			glm::mat4 viewProj;
			glm::vec3 cameraPosition;
		};
		std::vector<VulkanMappedBuffer> m_MappedUniformBuffers{};

		VulkanBuffer m_DebugVertexBuffer{};
		VulkanBuffer m_DebugIndexBuffer{};

		std::array<VkClearValue, 2> static constexpr CLEAR_VALUES
		{
			VkClearValue{.color = { 0.0f, 0.0f, 0.0f, 1.f } },
			VkClearValue{.depthStencil = { 1.0f, 0 } }
		};

		uint32_t static constexpr COLOR_CLEAR_ID{ 0 };
		uint32_t static constexpr DEPTH_CLEAR_ID{ 1 };

		void CreateUniformBuffers();

		void CreateSyncObjects();

		void DrawFrame(glm::mat4 const& view, glm::mat4 const& proj);
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void UpdateUniformBuffer(uint32_t currentImage, glm::mat4 const& view, glm::mat4 const& proj);

		// Recreate the swapchain on e.g a window resize
		bool RecreateSwapchain();

		// Update the buffer for debug drawing
		void UpdateDebugVertexBuffer();

		void RenderDebug(VkCommandBuffer commandBuffer);
	};
}

#endif
