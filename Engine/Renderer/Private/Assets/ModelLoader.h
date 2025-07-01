#ifndef MAUREN_MODELLOADER_H
#define MAUREN_MODELLOADER_H

#include "LoadedModel.h"
#include "Assets/Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace MauRen
{
	class VulkanDescriptorContext;
	class VulkanCommandPoolManager;

	class ModelLoader
	{
	public:
		ModelLoader() = default;
		~ModelLoader() = default;

		ModelLoader(ModelLoader const&) = delete;
		ModelLoader(ModelLoader&&) = delete;
		ModelLoader& operator=(ModelLoader const&) = delete;
		ModelLoader& operator=(ModelLoader const&&) = delete;
		/**
		 * load assimp by file
		 * -> this files contains one big static mesh
		 * -> split up in submeshes 
		 * -> these submeshes combind == static mesh
		 *		For rendering: the submesh is treated as a unique mesh
		 */
		[[nodiscard]] static LoadedModel LoadModel(std::string const& path, VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext) noexcept;

	private:
		[[nodiscard]] static Material ExtractMaterial(std::string const& path, aiMaterial const* material, aiScene const* scene);
		[[nodiscard]] static EmbeddedTexture ExtractEmbeddedTexture(aiTexture const* texture);
		[[nodiscard]] static std::string HashEmbeddedTexture(aiTexture const* texture) noexcept;

		static void ProcessMesh(
			aiMesh const* mesh,
			aiScene const* scene,
			aiMatrix4x4 const& transform,
			LoadedModel& model,
			VulkanCommandPoolManager& cmdPoolManager,
			VulkanDescriptorContext& descriptorContext,
			std::string const& path);

		static void ProcessNode(
			aiNode const* node,
			aiScene const* scene,
			aiMatrix4x4 const& parentTransform,
			LoadedModel& model,
			VulkanCommandPoolManager& cmdPoolManager,
			VulkanDescriptorContext& descriptorContext,
			std::string const& path);


	};
}

#endif