#ifndef MAUREN_RENDERER_H
#define MAUREN_RENDERER_H

struct GLFWwindow;

namespace MauRen
{
	class Renderer
	{
	public:
		Renderer() = default;
		virtual ~Renderer() = default;

		virtual void Initialize(GLFWwindow* pWindow) = 0;

		virtual void Render() = 0;

		Renderer(Renderer const&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer const&) = delete;
		Renderer& operator=(Renderer&&) = delete;
	};

	class NullRenderer final : public Renderer
	{
	public:
		NullRenderer() = default;
		virtual ~NullRenderer() override = default;

		virtual void Initialize(GLFWwindow* pWindow) override {}

		virtual void Render() override {}

		NullRenderer(NullRenderer const&) = delete;
		NullRenderer(NullRenderer&&) = delete;
		NullRenderer& operator=(NullRenderer const&) = delete;
		NullRenderer& operator=(NullRenderer&&) = delete;
	};
}

#endif // MAUREN_RENDERER_H