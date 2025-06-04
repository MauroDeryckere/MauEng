#ifndef MAUGAM_DEMOSCENE_H
#define MAUGAM_DEMOSCENE_H

#include "Scene/Scene.h"

namespace MauGam
{
	class DemoScene final : public MauEng::Scene
	{
	public:
		DemoScene();
		virtual ~DemoScene() override = default;
		virtual void OnLoad() override;
		virtual void Tick() override;
		virtual void OnRender() const override;
	private:
		enum class EDemo : uint8_t
		{
			Sponza,
			Chess,
			FlightHelmet,
			InstanceTest,
			DebugRendering,
			COUNT
		};

		enum class ELightMode : uint8_t
		{
			PointOnly,
			DirOnly,
			PointAndDir,
			COUNT
		};

		EDemo m_Demo{ EDemo::DebugRendering };

		ELightMode m_LightMode{ ELightMode::PointAndDir };

		bool m_DebugRenderLight{ true };
		bool m_Rotate{ true };
		bool m_CastShadows{ false };

		enum class EDebugRenderMode : uint8_t
		{
			DRAW_LINES,
			DRAW_RECTS,
			DRAW_TRIANGLES,
			DRAW_ARROWS,
			DRAW_CIRCLES,
			DRAW_SPHERES,
			DRAW_CYL,
			DRAW_POLY,
			ALL,
			COUNT
		};
		EDebugRenderMode m_DebugRenderMode{ EDebugRenderMode::DRAW_LINES };

		void SetupInput();
		void HandleInput();
		void RenderDebugDemo() const;
	};
}

#endif
