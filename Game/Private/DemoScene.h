#ifndef MAUGAM_DEMOSCENE_H
#define MAUGAM_DEMOSCENE_H

#include "Scene/Scene.h"

namespace MauGam
{
	DECLARE_LOG_CATEGORY_EXTERN(TestLogCategory)

	struct TestEvent
	{
		int i = 10;
	};

	class DemoScene final : public MauEng::Scene
	{
	public:
		DemoScene();
		virtual ~DemoScene() override = default;
		virtual void OnLoad() override;
		virtual void Tick() override;
		virtual void OnRender() const override;
	private:
		MauCor::Delegate<TestEvent> m_DelegateTest{};

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

		EDemo m_Demo{ EDemo::FlightHelmet };

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

		enum class ECamSettings : uint8_t
		{
			NoExposure,
			SUNNY16,
			INDOOR,
			CUSTOM,
			COUNT
		};
		ECamSettings m_CamSettings{ ECamSettings::SUNNY16 };

		void SetupInput();
		void HandleInput();
		void RenderDebugDemo() const;

		void OutputKeybinds();

		void OnDelegate(TestEvent const& event);
		void OnDelegateConst(TestEvent const& event) const;
	};
}

#endif
