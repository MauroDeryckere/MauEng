#ifndef MAUREN_MESH_H
#define MAUREN_MESH_H

#include <filesystem>
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

		[[nodiscard]] std::vector<Vertex> const& GetVertices() const noexcept { return m_Vertices; }
		[[nodiscard]] std::vector<uint32_t> const& GetIndices() const noexcept { return m_Indices; }

		Mesh(Mesh const&) = default;
		Mesh(Mesh&&) = default;
		Mesh& operator=(Mesh const&) = default;
		Mesh& operator=(Mesh&&) = default;

	private:
		uint32_t m_MeshID{ UINT32_MAX };

		std::vector<Vertex> m_Vertices{};
		std::vector<uint32_t> m_Indices{};

		//TODO material info
	};
}

#endif