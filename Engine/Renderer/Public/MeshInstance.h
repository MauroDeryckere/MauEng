#ifndef MAUREN_MESHINSTANCE_H
#define MAUREN_MESHINSTANCE_H

#include "Mesh.h"

#include "Math/Rotator.h"

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>

namespace MauRen
{
	class MeshInstance final
	{
	public:
		MeshInstance(Mesh const& mesh);
		~MeshInstance() = default;

		void Draw() const;

		[[nodiscard]] uint32_t GetMeshID() const noexcept { return m_MeshID; }
		[[nodiscard]] uint32_t GetMaterialID() const noexcept { return m_MaterialID; }

		void SetModelMatrix(glm::mat4 const& modelMatrix) noexcept { m_ModelMatrix = modelMatrix; }
		[[nodiscard]] glm::mat4 const& GetModelMatrix() const noexcept { return m_ModelMatrix; }

		void Translate(glm::vec3 const& translation) noexcept;

		void ResetTransformation() noexcept;

		void Rotate(MauCor::Rotator const& rotator) noexcept;

		void Scale(glm::vec3 const& scale) noexcept;

		MeshInstance(MeshInstance const&) = default;
		MeshInstance(MeshInstance&&) = default;
		MeshInstance& operator=(MeshInstance const&) = default;
		MeshInstance& operator=(MeshInstance&&) = default;
	private:
		uint32_t m_MeshID{ UINT32_MAX };
		uint32_t m_MaterialID { UINT32_MAX };

		glm::mat4 m_ModelMatrix{ 1.0f };
	};
}

#endif