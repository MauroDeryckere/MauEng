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
	};
}

#endif
