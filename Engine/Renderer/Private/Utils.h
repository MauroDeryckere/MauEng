#ifndef MAUREN_UTILS_H
#define MAUREN_UTILS_H

#include <vector>
#include <filesystem>

namespace MauRen
{
	struct Vertex;

	namespace Utils
	{
		void LoadModel(std::filesystem::path const& filepath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	}
}

#endif