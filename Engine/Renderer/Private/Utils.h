#ifndef MAUREN_UTILS_H
#define MAUREN_UTILS_H

#include <vector>
#include <filesystem>

namespace MauRen
{
	struct Vertex;
	struct Material;

	namespace Utils
	{
		// Absolute path to the material directory
		[[nodiscard]] std::string GetAbsoluteMaterialPath() noexcept;

		// Loads a model and the material the model uses if valid and not loaded yet
		// Only supports a single material for now! 
		void LoadModel(std::filesystem::path const& filepath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, Material& mat);
	}
}

#endif