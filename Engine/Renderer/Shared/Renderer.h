#ifndef MAUREN_RENDERER_H
#define MAUREN_RENDERER_H

namespace MauEng
{
	class Camera;
}

namespace MauEng
{
	struct CLight;
	struct CStaticMesh;
}

struct SDL_Window;

namespace MauRen
{
	class Renderer
	{
	public:
		virtual ~Renderer() = default;

		virtual void Init() = 0;
		virtual void InitImGUI() = 0;
		virtual void Destroy() = 0;
		virtual void DestroyImGUI() = 0;

		virtual void BeginImGUIFrame() = 0;
		virtual void Render(MauEng::Camera const* cam) = 0;
		virtual void EndImGUIFrame() = 0;

		virtual void ResizeWindow() = 0;
		virtual void QueueDraw(glm::mat4 const& transformMat, MauEng::CStaticMesh const& mesh) = 0;
		virtual void UnloadMesh(uint32_t meshID) = 0;
		virtual [[nodiscard]] uint32_t LoadOrGetMeshID(char const* path) = 0;

		virtual void SetSceneAABBOverride(glm::vec3 const& min, glm::vec3 const& max) = 0;
		virtual void PreLightQueue(glm::mat4 const& viewProj) = 0;
		virtual uint32_t CreateLight() = 0;
		virtual void QueueLight(MauEng::CLight const& light) = 0;

		virtual [[nodiscard]] std::pair<std::unordered_map<std::string, struct LoadedMeshes_PathInfo> const&, std::vector<struct MeshData>const&> GetRendererMeshInfo() = 0;
		virtual [[nodiscard]] struct MaterialRendererInfo GetMaterialRendererInfo() const noexcept = 0;
		virtual [[nodiscard]] std::unordered_map<std::string, struct LoadedTextureInfo>const& GetTextureMap() const noexcept = 0;


		Renderer(Renderer const&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer const&) = delete;
		Renderer& operator=(Renderer&&) = delete;
																			
	protected:
		explicit Renderer(SDL_Window* pWindow, class DebugRenderer&) {}
	};
}

#endif // MAUREN_RENDERER_H