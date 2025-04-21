#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace MauRen
{
	LoadedModel ModelLoader::LoadModel(std::string const& path) noexcept
	{
		Assimp::Importer importer;

		LoadedModel model;

		// And have it read the given file with some example postprocessing
		// Usually - if speed is not the most important aspect for you - you'll
		// probably to request more postprocessing than we do in this example.
		aiScene const* scene{ importer.ReadFile(path,
												aiProcess_Triangulate |
												aiProcess_GenNormals |
												aiProcess_JoinIdenticalVertices |
												aiProcess_ImproveCacheLocality |
												aiProcess_CalcTangentSpace |
												aiProcess_LimitBoneWeights |
												aiProcess_ValidateDataStructure |
												aiProcess_RemoveRedundantMaterials |
												aiProcess_OptimizeGraph |
												aiProcess_OptimizeMeshes) };
												// AI_SCENE_FLAGS_NON_VERBOSE_FORMAT

		if (!scene || !scene->HasMeshes()) 
		{
			ME_LOG_ERROR(MauCor::LogCategory::Renderer, "Error loading model! {}", importer.GetErrorString());
			return model;
		}

        for (unsigned i{ 0 }; i < scene->mNumMeshes; ++i)
		{
			aiMesh const* const mesh{ scene->mMeshes[i] };

			uint32_t const vertexOffset{ static_cast<uint32_t>(model.vertices.size()) };
			uint32_t const indexOffset{ static_cast<uint32_t>(model.indices.size()) };

		    for (unsigned j{ 0 }; j < mesh->mNumVertices; ++j) 
		    {
				aiVector3D const vertex{ mesh->mVertices[j] };

				//For now just support channel 0
				glm::vec3 color{ 1.0f };
				if (mesh->HasVertexColors(0))
				{
					aiColor4D const& col { mesh->mColors[0][j] };
					color = glm::vec3(col.r, col.g, col.b);
				}

				//For now just support channel 0
				glm::vec2 texCoord{ 0.0f };
				if (mesh->HasTextureCoords(0))
				{
					aiVector3D const& tex{ mesh->mTextureCoords[0][j] };
					texCoord = glm::vec2{ tex.x, tex.y };
				}

				Vertex const vert
				{
					.position = glm::vec3{ vertex.x, vertex.y, vertex.z },
					.color = color,
					.texCoord = texCoord
		        };

				model.vertices.emplace_back(vert);
		    }

			uint32_t indexCount{ 0 };
			for (unsigned j{ 0 }; j < mesh->mNumFaces; ++j)
		    {
		        aiFace const& face{ mesh->mFaces[j] };
				for (unsigned k{ 0 }; k < face.mNumIndices; ++k)
		        {
					model.indices.emplace_back(face.mIndices[k]);
		        }

				indexCount += face.mNumIndices;
		    }

			model.subMeshes.emplace_back(
				SubMeshData
				{
					.indexCount = indexCount,
					.firstIndex = indexOffset,
					.vertexOffset = static_cast<int32_t>(vertexOffset),
					.materialID = 0  // placeholder, material integration can come later
				});

		    // Optionally handle materials or textures for the mesh here.
			aiMaterial const* const material{ scene->mMaterials[mesh->mMaterialIndex] };
			aiString matName;
			material->Get(AI_MATKEY_NAME, matName);

			aiString baseColorPath;
			if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &baseColorPath))
			{
				std::string const texturePath{ baseColorPath.C_Str() };
				// Load as base color
			}

		    // LoadMaterial in manager
			// emplace matID if not existing
			//loadedMesh.emplace_back(MeshInstance{ i, vertices, indices, matName.C_Str() });
		}

		return model;
	}
}