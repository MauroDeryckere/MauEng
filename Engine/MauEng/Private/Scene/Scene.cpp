#include "Scene/Scene.h"

#include "InternalServiceLocator.h"

namespace MauEng
{
	void Scene::OnRender() const
	{
		ME_PROFILE_FUNCTION()
		{
			//GetECSWorld(). ;
			{
				auto const view = GetECSWorld().View<CTransform>();
				ME_PROFILE_SCOPE("UPDATE MATRICES")
					view.Each([](CTransform& t){
						t.UpdateMatrix();
					}, std::execution::par_unseq);
			}
			{
				ME_PROFILE_SCOPE("QUEUE DRAWS")
				auto group{ GetECSWorld().Group<CStaticMesh, CTransform>() };
				group.Each([](CStaticMesh const& m, CTransform const& t)
							{
								RENDERER.QueueDraw(t.mat, m);
							});
			}

			RENDERER.PreLightQueue(GetCameraManager().GetActiveCamera().GetProjectionMatrix() * GetCameraManager().GetActiveCamera().GetViewMatrix());
			{
				ME_PROFILE_SCOPE("QUEUE LIGHTS")

				auto view{ GetECSWorld().View<CLight>() };
				view.Each([](CLight const& l)
					{
						RENDERER.QueueLight(l);
					});
			}
		}
	}

	void Scene::SetSceneAABBOverride(glm::vec3 const& min, glm::vec3 const& max)
	{
		RENDERER.SetSceneAABBOverride(min, max);
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
