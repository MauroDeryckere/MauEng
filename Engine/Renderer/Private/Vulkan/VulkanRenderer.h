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
#include "VulkanGraphicsPipeline.h"
#include "VulkanCommandPoolManager.h"

#include "VulkanBuffer.h"
#include "VulkanMesh.h"

#include "VulkanImage.h"

// Sources
// https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples#swapchain-image-acquire-and-present
// https://blog.traverseresearch.nl/bindless-rendering-setup-afeb678d77fc
// DAE course - Graphics Programming 2

namespace MauRen
{
	class VulkanRenderer final : public Renderer
	{
	public:
		explicit VulkanRenderer(GLFWwindow* pWindow);
		virtual ~VulkanRenderer() override;

		virtual void Render() override;
		virtual void ResizeWindow() override;

		virtual void UpLoadModel(Mesh& mesh) override;


		VulkanRenderer(VulkanRenderer const&) = delete;
		VulkanRenderer(VulkanRenderer&&) = delete;
		VulkanRenderer& operator=(VulkanRenderer const&) = delete;
		VulkanRenderer& operator=(VulkanRenderer&&) = delete;

	private:
		// "reference" to the window
		GLFWwindow* m_pWindow{ nullptr };

		VulkanInstanceContext m_InstanceContext{};
		VulkanSurfaceContext m_SurfaceContext{};
		VulkanDebugContext m_DebugContext{};

		VulkanDescriptorContext m_DescriptorContext{};
		VulkanSwapchainContext m_SwapChainContext{};
		VulkanGraphicsPipeline m_GraphicsPipeline{};

		VulkanCommandPoolManager m_CommandPoolManager{};

		// Signal that an image has been acquired from the swapchain and is ready for rendering
		std::vector<VkSemaphore> m_ImageAvailableSemaphores{};

		// Signal that rendering has finished and presentation can happen
		std::vector<VkSemaphore> m_RenderFinishedSemaphores{};

		// Fence to make sure only one frame is rendering at a time
		std::vector<VkFence> m_InFlightFences{};

		uint32_t m_CurrentFrame{ 0 };

		bool m_FramebufferResized{ false };

#pragma region BindlessSetupTODO
		//TODO
		// Using a fixed size for now to test the system, until moving on to use the allocator
		const VkDeviceSize MAX_VERTEX_BUFFER_SIZE{ 64 * 1024 * 1024 }; // 64MB
		const VkDeviceSize MAX_INDEX_BUFFER_SIZE{ 32 * 1024 * 1024 };  // 32MB
		const VkDeviceSize MAX_INSTANCE_BUFFER_SIZE{ 16 * 1024 * 1024 }; // 16MB
		//const VkDeviceSize MAX_MESH_DATA_SIZE = 1024 * sizeof(MeshData); // 1024 meshes
		//const VkDeviceSize MAX_DRAW_COMMANDS = 1024 * sizeof(DrawCommand); // 1024 draw calls

		VulkanBuffer m_GlobalVertexBuffer{};
		VulkanBuffer m_GlobalIndexBuffer{};
		VulkanBuffer m_InstanceDataBuffer{};  // Holds per-instance data
#pragma endregion

		std::vector<VulkanMesh> m_Meshes;

		struct UniformBufferObject final
		{
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};
		std::vector<VulkanMappedBuffer> m_MappedUniformBuffers{};

		VulkanImage m_TextureImage{};
		VulkanImage m_TextureImage2{};

		// Should be managedin e.g a texturemanager
		VkSampler m_TextureSampler{ VK_NULL_HANDLE };

		void CreateTextureImage();

		void CreateUniformBuffers();

		void CreateSyncObjects();

		void DrawFrame();
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void UpdateUniformBuffer(uint32_t currentImage);

		void CreateTextureSampler();

		// Recreate the swapchain on e.g a window resize
		void RecreateSwapchain();

		//TODO
		void CreateGlobalBuffers();
	};
}

#endif
