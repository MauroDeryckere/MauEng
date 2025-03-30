#ifndef MAUREN_MESHINSTANCE_H
#define MAUREN_MESHINSTANCE_H

#include <glm/mat4x4.hpp>
#include "Mesh.h"

namespace MauRen
{
	class MeshInstance final
	{
	public:
		MeshInstance(Mesh const& mesh);
		~MeshInstance() = default;

		void Draw() const;

		[[nodiscard]] uint32_t GetMeshID() const noexcept { return m_MeshID; }

		void SetModelMatrix(glm::mat4 const& modelMatrix) noexcept { m_ModelMatrix = modelMatrix; }
		[[nodiscard]] glm::mat4 const& GetModelMatrix() const noexcept { return m_ModelMatrix; }

		void Translate(glm::vec3 const& translation) noexcept;

		MeshInstance(MeshInstance const&) = default;
		MeshInstance(MeshInstance&&) = default;
		MeshInstance& operator=(MeshInstance const&) = default;
		MeshInstance& operator=(MeshInstance&&) = default;
	private:
		uint32_t m_MeshID { UINT32_MAX };
		glm::mat4 m_ModelMatrix{ 1.0f };

	};
}

#endif