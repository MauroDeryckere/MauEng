#include "Scene/Scene.h"

namespace MauEng
{
	Entity Scene::CreateEntity()
	{
		return m_ECSWorld.CreateEntity();
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_ECSWorld.DestroyEntity(entity);
	}
}
