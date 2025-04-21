#ifndef MAUREN_MODELLOADER_H
#define MAUREN_MODELLOADER_H

#include "LoadedModel.h"

namespace MauRen
{
	class ModelLoader
	{
	public:
		ModelLoader() = default;
		~ModelLoader() = default;

		ModelLoader(ModelLoader const&) = delete;
		ModelLoader(ModelLoader&&) = delete;
		ModelLoader& operator=(ModelLoader const&) = delete;
		ModelLoader& operator=(ModelLoader const&&) = delete;
		/*
		 * load assimp by file
		 * -> this files contains one big static mesh
		 * -> split up in submeshes 
		 * -> these submeshes combind == static mesh
		 *		For rendering: the submesh is treated as a unique mesh
		 */
		static LoadedModel LoadModel(std::string const& path) noexcept;
	private:
	};
}

#endif