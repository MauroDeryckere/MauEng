#ifndef MAUGAM_DEBUGDRAWINGSCENE_H
#define MAUGAM_DEBUGDRAWINGSCENE_H

#include "Scene/Scene.h"

namespace MauGam
{
	class DebugDrawingScene final : public MauEng::Scene
	{
	public:
		DebugDrawingScene();
		virtual ~DebugDrawingScene() override = default;
		virtual void OnLoad() override;
		virtual void Tick() override;
		virtual void OnRender() const override;
	private:
		void DrawDebug() const;
	};
}

#endif
