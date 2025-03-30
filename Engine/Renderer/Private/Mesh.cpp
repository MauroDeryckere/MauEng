#include "Mesh.h"

#include "Utils.h"

namespace MauRen
{
	Mesh::Mesh(std::filesystem::path const& filepath)
	{
		Utils::LoadModel(filepath, m_Vertices, m_Indices);
	}
}
