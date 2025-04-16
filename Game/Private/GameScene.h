#ifndef MAUGAM_GAMESCENE_H
#define MAUGAM_GAMESCENE_H

#include "MeshInstance.h"
#include "Scene/Scene.h"

namespace MauGam
{
	class GameScene final : public MauEng::Scene
	{
	public:
		GameScene();
		virtual ~GameScene() override = default;
		virtual void OnLoad() override;
		virtual void Tick() override;
		virtual void OnRender() const override;
	private:
		std::vector<MauRen::MeshInstance> m_Mehses{};
	};
}

#endif
