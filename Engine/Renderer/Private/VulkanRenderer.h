#ifndef MAUREN_VULKANRENDERER_H
#define MAUREN_VULKANRENDERER_H

#include "RendererPCH.h"

#include "Renderer.h"

#include "VulkanInstanceContext.h"
#include "VulkanSurfaceContext.h"
#include "VulkanDebugContext.h"
#include "VulkanDeviceContext.h"
#include "VulkanSwapchainContext.h"
#include "VulkanGraphicsPipeline.h"

namespace MauRen
{
	class VulkanRenderer final : public Renderer
	{
	public:
		explicit VulkanRenderer(GLFWwindow* pWindow);
		virtual ~VulkanRenderer() override;

		virtual void Render() override;


		VulkanRenderer(VulkanRenderer const&) = delete;
		VulkanRenderer(VulkanRenderer&&) = delete;
		VulkanRenderer& operator=(VulkanRenderer const&) = delete;
		VulkanRenderer& operator=(VulkanRenderer&&) = delete;

	private:
		std::unique_ptr<VulkanInstanceContext> m_InstanceContext;
		std::unique_ptr<VulkanSurfaceContext> m_SurfaceContext;
		std::unique_ptr<VulkanDebugContext> m_DebugContext;
		std::unique_ptr<VulkanDeviceContext> m_DeviceContext;
		std::unique_ptr<VulkanSwapchainContext> m_SwapChainContext;
		std::unique_ptr<VulkanGraphicsPipeline> m_GraphicsPipeline;

		// no unique ptr owned contxt for this currently, can be moved if the logic grows a lot
		std::vector<VkFramebuffer> m_SwapChainFramebuffers;

		// no unique ptr owned contxt for this currently, can be moved if the logic grows a lot
		VkCommandPool m_CommandPool;
		// Automatically freed when their pool is destroyed
		VkCommandBuffer m_CommandBuffer;

		// Signal that an image has been acquired from the swapchain and is ready for rendering
		VkSemaphore m_ImageAvailableSemaphore;
		
		// Signal that rendering has finished and presentation can happen
		VkSemaphore m_RenderFinishedSemaphore;

		// Fence to make sure only one frame is rendering at a time
		VkFence m_InFlightFence;

		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateCommandBuffer();

		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

		void CreateSyncObjects();

		void DrawFrame();

	};
}

#endif
