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
			COUNT
		};

		EDemo m_Demo{ EDemo::Chess };

		void SetupInput();
		void HandleInput();

	};
}

#endif
