#include "Scene/Scene.h"

namespace MauEng
{
	void Scene::OnRender() const
	{
		m_ECSWorld.ForEach<CStaticMesh>(
			[&](ECS::EntityID id, CStaticMesh const& m)
			{
				RENDERER.QueueDraw(m_ECSWorld.GetComponent<CTransform>(id).GetTransformMatrix(), m);
			}
		);
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
