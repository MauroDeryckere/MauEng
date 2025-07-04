#include "Scene/Scene.h"

#include "InternalServiceLocator.h"

namespace MauEng
{
	Scene::Scene()
	{
		m_ECSWorld.RegisterPreRemoveCallback<CStaticMesh>([](CStaticMesh const& mesh)
			{
				RENDERER.UnloadMesh(mesh.meshID);
			});
	}

	void Scene::Tick()
	{
		m_TimerManager.Tick();

		m_CameraManager.Tick();
		for (auto& p : INPUT_MANAGER.GetPlayers())
		{
			p->Tick();
		}
	}

	void Scene::OnRender() const
	{
		ME_PROFILE_FUNCTION()
		{
			{
				auto const view = GetECSWorld().View<CTransform>();
				ME_PROFILE_SCOPE("UPDATE MATRICES")
					view.Each([](CTransform& t){
						t.UpdateMatrix();
					}, std::execution::par_unseq);
			}
			{
				ME_PROFILE_SCOPE("QUEUE DRAWS")
				auto view{ GetECSWorld().View<CStaticMesh, CTransform>() };
				view.Each([](CStaticMesh const& m, CTransform const& t)
							{
								RENDERER.QueueDraw(t.mat, m);
							});
			}

			ME_CHECK(GetCameraManager().GetActiveCamera());
			RENDERER.PreLightQueue(GetCameraManager().GetActiveCamera()->GetProjectionMatrix() * GetCameraManager().GetActiveCamera()->GetViewMatrix());
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

	Entity Scene::CreateEntity(glm::vec3 const& pos)
	{
		Entity ent{ &m_ECSWorld, m_ECSWorld.CreateEntity() };

		ent.AddComponent<CTransform>(pos);

		return ent;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_ECSWorld.DestroyEntity(entity);
	}
}
