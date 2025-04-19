#ifndef MAUGAM_ECSTESTSCENE_H
#define MAUGAM_ECSTESTSCENE_H

#include "Scene/Scene.h"

namespace MauGam
{
	class ECSTestScene final : public MauEng::Scene
	{
	public:
		ECSTestScene();
		virtual ~ECSTestScene() override = default;
		virtual void OnLoad() override;
		virtual void Tick() override;
	private:

	};
}

#endif
