#ifndef MAUREN_MESH_H
#define MAUREN_MESH_H

#include <filesystem>
#include "Material.h"
#include "Vertex.h"

namespace MauRen
{
	class MeshInstance;

	class Mesh final
	{
	public:
		explicit Mesh(std::filesystem::path const& filepath);
		~Mesh() = default;

		[[nodiscard]] uint32_t GetMeshID() const noexcept { return m_MeshID; }
		void SetMeshID(uint32_t ID) noexcept
		{
			if (m_MeshID == UINT32_MAX)
			{
				m_MeshID = ID;
			}	
		}
		[[nodiscard]] uint32_t GetMaterialID() const noexcept { return m_MaterialID; }
		void SetMaterialID(uint32_t ID) noexcept
		{
			if (m_MaterialID == UINT32_MAX)
			{
				m_MaterialID = ID;
			}
		}

		[[nodiscard]] std::vector<Vertex> const& GetVertices() const noexcept { return m_Vertices; }
		[[nodiscard]] std::vector<uint32_t> const& GetIndices() const noexcept { return m_Indices; }

		[[nodiscard]] Material const& GetMaterial() const noexcept { return m_Material; }

		Mesh(Mesh const&) = default;
		Mesh(Mesh&&) = default;
		Mesh& operator=(Mesh const&) = default;
		Mesh& operator=(Mesh&&) = default;

	private:
		uint32_t m_MeshID{ UINT32_MAX };
		uint32_t m_MaterialID{ UINT32_MAX };

		Material m_Material{};

		std::vector<Vertex> m_Vertices{};
		std::vector<uint32_t> m_Indices{};
	};
}

#endif