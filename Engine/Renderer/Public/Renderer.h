#ifndef MAUREN_RENDERER_H
#define MAUREN_RENDERER_H

#include "Mesh.h"
#include "MeshInstance.h"

struct GLFWwindow;

namespace MauRen
{
	class Renderer
	{
	public:
		explicit Renderer(GLFWwindow* pWindow) { }
		virtual ~Renderer() = default;

		virtual void Render() = 0;

		virtual void ResizeWindow() = 0;

		// Upload model to the GPU
		virtual void UpLoadModel(Mesh& mesh) = 0;


		Renderer(Renderer const&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer const&) = delete;
		Renderer& operator=(Renderer&&) = delete;
	};

	class NullRenderer final : public Renderer
	{
	public:
		explicit NullRenderer(GLFWwindow* pWindow) :
		Renderer{ pWindow }
		{}

		virtual ~NullRenderer() override = default;

		virtual void Render() override {}
		virtual void ResizeWindow() override {}

		virtual void UpLoadModel(Mesh&) override {}

		NullRenderer(NullRenderer const&) = delete;
		NullRenderer(NullRenderer&&) = delete;
		NullRenderer& operator=(NullRenderer const&) = delete;
		NullRenderer& operator=(NullRenderer&&) = delete;
	};
}

#endif // MAUREN_RENDERER_H