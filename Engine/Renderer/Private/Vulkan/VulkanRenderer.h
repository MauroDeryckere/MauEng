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
#include "../../../MauEng/Public/Components/CLight.h"

#include "Assets/VulkanImage.h"

#include "DebugRenderer/DebugVertex.h"

// Sources
// https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples#swapchain-image-acquire-and-present
// https://blog.traverseresearch.nl/bindless-rendering-setup-afeb678d77fc
// DAE course - Graphics Programming 2

namespace MauEng
{
	class Camera;
}

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
		virtual void InitImGUI() override;
		virtual void Destroy() override;
		virtual void DestroyImGUI() override;

		virtual void BeginImGUIFrame() override;
		virtual void Render(MauEng::Camera const* cam) override;
		virtual void EndImGUIFrame() override;

		virtual void ResizeWindow() override;

		virtual uint32_t CreateLight() override;

		virtual void SetSceneAABBOverride(glm::vec3 const& min, glm::vec3 const& max) override;
		virtual void PreLightQueue(glm::mat4 const& viewProj) override;
		virtual void QueueLight(MauEng::CLight const& light) override;
		virtual void QueueDraw(glm::mat4 const& transformMat, MauEng::CStaticMesh const& mesh) override;
		virtual [[nodiscard]] uint32_t LoadOrGetMeshID(char const* path) override;

		virtual [[nodiscard]] std::pair<std::unordered_map<std::string, struct LoadedMeshes_PathInfo> const&, std::vector<struct MeshData>const&> GetRendererMeshInfo() override;

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
		VulkanGraphicsPipelineContext m_GraphicsPipelineContext{};

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
			alignas(16) glm::mat4 viewProj;
			alignas(16) glm::mat4 invView;
			alignas(16) glm::mat4 invProj;
			alignas(16) glm::vec3 cameraPosition;
			alignas(16) glm::vec2 screenSize;

			alignas(16) uint32_t numLights;

			// Room for 2 more floats (padding) e.g time,...
			//float padding02;
			//float padding03;
		};
		std::vector<VulkanMappedBuffer> m_MappedUniformBuffers{};

		struct alignas(16) CamSettingsUBO final
		{
			float aperture;
			float ISO;
			float shutterSpeed;
			float exposureOverride;

			uint32_t mapper;
			uint32_t isAutoExposure;
			uint32_t enableExposure;
		};

		std::vector<VulkanMappedBuffer> m_CamSettingsMappedUniformBuffers{};

		VulkanMappedBuffer m_DebugVertexBuffer{};
		VulkanMappedBuffer m_DebugIndexBuffer{};

		std::array<VkClearValue, 2> static constexpr CLEAR_VALUES
		{
			VkClearValue{.color = { 0, 0, 0, 0 } },
			VkClearValue{.depthStencil = { 1.0f, 0 } }
		};

		uint32_t static constexpr COLOR_CLEAR_ID{ 0 };
		uint32_t static constexpr DEPTH_CLEAR_ID{ 1 };

		std::array<glm::vec2, 6> static constexpr m_QuadVertices
		{
			glm::vec2(-1.0f, -1.0f), // Bottom-left
			glm::vec2(1.0f, -1.0f), // Bottom-right
			glm::vec2(-1.0f,  1.0f), // Top-left

			glm::vec2(-1.0f,  1.0f), // Top-left
			glm::vec2(1.0f, -1.0f), // Bottom-right
			glm::vec2(1.0f,  1.0f)  // Top-right
		}; 
		VulkanBuffer m_QuadVertexBuffer{};

		void CreateUniformBuffers();

		void CreateSyncObjects();

		// Before command buffer recording, update descriptor set, ...
		void PreDraw(MauEng::Camera const* cam, uint32_t image);

		void DrawFrame(MauEng::Camera const* cam);
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, glm::mat4 const& viewProj);
		void UpdateUniformBuffer(uint32_t currentImage, glm::mat4 const& view, glm::mat4 const& proj);
		void UpdateCamSettings(MauEng::Camera const* cam, uint32_t currentImage);

		// Recreate the swapchain on e.g a window resize
		bool RecreateSwapchain();

		// Update the buffer for debug drawing
		void UpdateDebugVertexBuffer();

		void RenderDebug(VkCommandBuffer commandBuffer, bool isPrepass);
	};
}

#endif
