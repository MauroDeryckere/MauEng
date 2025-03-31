#ifndef MAUGAM_GAMESCENE_H
#define MAUGAM_GAMESCENE_H

#include "MeshInstance.h"
#include "Scene/Scene.h"

namespace MauGam
{
	class GameScene final : public MauEng::Scene
	{
	public:
		virtual void OnLoad() override;
		virtual void Tick() override;
		virtual void OnRender() override;
	private:
		std::vector<MauRen::MeshInstance> m_Mehses{};
	};
}

#endif
