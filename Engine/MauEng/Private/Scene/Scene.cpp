#include "Scene/Scene.h"

namespace MauEng
{
	void Scene::OnRender() const
	{
		ME_PROFILE_FUNCTION()
		{
			ME_PROFILE_SCOPE("QUEUE DRAWS")
			auto group{ GetECSWorld().Group<CStaticMesh, CTransform>() };
			group.Each([](CStaticMesh const& m, CTransform const& t)
						{
							RENDERER.QueueDraw(t.mat, m);
						});
		}
	}

	Entity Scene::CreateEntity()
	{
		Entity ent{ m_ECSWorld.CreateEntity() };

		ent.AddComponent<CTransform>();

		return ent;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_ECSWorld.DestroyEntity(entity);
	}
}
