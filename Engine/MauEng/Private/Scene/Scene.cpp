#include "Scene/Scene.h"

namespace MauEng
{
	void Scene::OnRender() const
	{
		ME_PROFILE_FUNCTION()

			//m_ECSWorld.ForEach<CStaticMesh, CTransform>(
			//	[&](ECS::EntityID id, CStaticMesh const& m, CTransform& t)
			//	{
			//		RENDERER.QueueDraw(t.GetTransformMatrix(), m);
			//	}
			//);
			//{
			//	ME_PROFILE_SCOPE("VIEW")

			//	auto&& view = m_ECSWorld.View<CStaticMesh, CTransform>();
			//	view.each([](auto it, CStaticMesh const& m, CTransform& t)
			//		{
			//			RENDERER.QueueDraw(t.GetTransformMatrix(), m);

			//		});
			//}

			{
				auto& reg = m_ECSWorld.Reg();
				auto group = reg.group<CStaticMesh, CTransform>();

				{
					ME_PROFILE_SCOPE("QUEUE DRAWS")

					// Iterate over the group to render each entity
					for (auto entity : group)
					{
						auto& mesh = group.get<CStaticMesh>(entity);
						auto& transform = group.get<CTransform>(entity);

						RENDERER.QueueDraw(transform.mat, mesh);
					}
				}


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
