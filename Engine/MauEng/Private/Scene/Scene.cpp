#include "Scene/Scene.h"

namespace MauEng
{
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
