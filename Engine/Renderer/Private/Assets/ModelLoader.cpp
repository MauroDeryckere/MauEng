#include "ModelLoader.h"


#include "Vulkan/Assets/VulkanMaterialManager.h"
#include "Material.h"

#include <string>
#include <cstdint>
#include <vector>
#include <functional> // for std::hash



namespace MauRen
{
	LoadedModel ModelLoader::LoadModel(std::string const& path, VulkanCommandPoolManager& cmdPoolManager, VulkanDescriptorContext& descriptorContext) noexcept
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

		aiMatrix4x4 const rootTransform{ scene->mRootNode->mTransformation };

        for (unsigned i{ 0 }; i < scene->mNumMeshes; ++i)
		{
			aiMesh const* const mesh{ scene->mMeshes[i] };
			
			uint32_t const vertexOffset{ static_cast<uint32_t>(model.vertices.size()) };
			uint32_t const indexOffset{ static_cast<uint32_t>(model.indices.size()) };

		    for (unsigned j{ 0 }; j < mesh->mNumVertices; ++j) 
		    {
				aiVector3D const vertex{ rootTransform * mesh->mVertices[j] };

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
					texCoord = glm::vec2{ tex.x, 1 - tex.y };
				}
				else
				{
					ME_LOG_ERROR(MauCor::LogCategory::Renderer, "No textcoords");
				}

				Vertex const vert
				{
					.position = glm::vec3{ vertex.x, vertex.y, vertex.z },
					.normal = glm::vec3{ mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z },
					.tangent = mesh->HasTangentsAndBitangents()
								? glm::vec4{ mesh->mTangents[j].x, mesh->mTangents[j].y, mesh->mTangents[j].z, 1.0f }
								: glm::vec4{ 0.0f },
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

			uint32_t matID{ INVALID_MATERIAL_ID };

			// Material loading (done via material manager)
			aiMaterial const* const material{ scene->mMaterials[mesh->mMaterialIndex] };
			aiString matName;
			auto result{ material->Get(AI_MATKEY_NAME, matName) };
			ME_ASSERT(mesh->mMaterialIndex < scene->mNumMaterials);
			ME_ASSERT(result == AI_SUCCESS);
			auto& matManager{ VulkanMaterialManager::GetInstance() };

			std::string const matStr{ matName.C_Str() };
			auto const getMat{ matManager.GetMaterial(matStr) };
			if (getMat.first)
			{
				matID = getMat.second;
			}
			else
			{
				Material const extractedMat{ ExtractMaterial(path, material, scene) };
				matID = matManager.LoadOrGetMaterial(cmdPoolManager, descriptorContext, extractedMat);
			}

			model.subMeshes.emplace_back(
				SubMeshData
				{
					.indexCount = indexCount,
					.firstIndex = indexOffset,
					.vertexOffset = static_cast<int32_t>(vertexOffset),
					.materialID = matID
				});
		}

		return model;
	}

	Material ModelLoader::ExtractMaterial(std::string const&path, aiMaterial const* material, aiScene const* scene)
	{
		std::filesystem::path modelPath = path;
		std::filesystem::path modelDir = modelPath.parent_path();

		Material mat{};

		aiString name;
		if (material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
			mat.name = name.C_Str();

		aiColor3D color(0.f, 0.f, 0.f);

		// Diffuse color
		if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
			mat.diffuseColor = { color.r, color.g, color.b };

		// Specular color
		if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
			mat.specularColor = { color.r, color.g, color.b };

		// Ambient color
		if (material->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
			mat.ambientColor = { color.r, color.g, color.b };

		// Emissive color
		if (material->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS)
			mat.emissiveColor = { color.r, color.g, color.b };

		// Transparency
		float transparency{ 1.0f };
		if (material->Get(AI_MATKEY_OPACITY, transparency) == AI_SUCCESS)
			mat.transparency = transparency;

		// Shininess
		float shininess{ 0.0f };
		if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
			mat.shininess = shininess;

		// Refraction index
		float ior{ 1.0f };
		if (material->Get(AI_MATKEY_REFRACTI, ior) == AI_SUCCESS)
			mat.refractionIndex = ior;

		// Illumination model
		int illum{ 0 };
		if (material->Get(AI_MATKEY_SHADING_MODEL, illum) == AI_SUCCESS)
			mat.illuminationModel = illum;

		// Texture paths
		aiString texPath;
		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) 
		{
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
			{
				if (texPath.C_Str()[0] == '*')
				{
					int const texIndex{ atoi(texPath.C_Str() + 1) };
					const aiTexture* tex{ scene->mTextures[texIndex] };
					mat.embDiffuse = ExtractEmbeddedTexture(tex);
				}
				else
				{
					mat.diffuseTexture = (modelDir / texPath.C_Str()).string();
				}
			}
		}


		if (material->GetTexture(aiTextureType_SPECULAR, 0, &texPath) == AI_SUCCESS)
			mat.specularTexture = texPath.C_Str();

		if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			if (material->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS)
			{
				if (texPath.C_Str()[0] == '*')
				{
					int const texIndex{ atoi(texPath.C_Str() + 1) };
					const aiTexture* tex{ scene->mTextures[texIndex] };
					mat.embNormal = ExtractEmbeddedTexture(tex);
				}
				else
				{
					mat.normalMap = (modelDir / texPath.C_Str()).string();
				}
			}
		}

		if (material->GetTexture(aiTextureType_AMBIENT, 0, &texPath) == AI_SUCCESS)
			mat.ambientTexture = texPath.C_Str();

		//if (material->GetTexture(aiTextureType_DISPLACEMENT, 0, &texPath) == AI_SUCCESS)
		//	mat.displacementMap = texPath.C_Str();

		return mat;
	}

	EmbeddedTexture ModelLoader::ExtractEmbeddedTexture(aiTexture const* texture)
	{
		EmbeddedTexture t{};
		t.hash = HashEmbeddedTexture(texture);
		t.formatHint = texture->achFormatHint;

		if (0 == texture->mHeight)
		{
			// Compressed (e.g. PNG)
			t.data.assign(reinterpret_cast<uint8_t*>(texture->pcData),
				reinterpret_cast<uint8_t*>(texture->pcData) + texture->mWidth);

			t.isCompressed = true;
		}
		else
		{
			// Uncompressed BGRA
			size_t const dataSize{ texture->mWidth * texture->mHeight * sizeof(aiTexel) };
			t.data.assign(reinterpret_cast<uint8_t*>(texture->pcData),
				reinterpret_cast<uint8_t*>(texture->pcData) + dataSize);

			t.width = texture->mWidth;
			t.height = texture->mHeight;

			t.isCompressed = false;
		}

		return t;
	}

	std::string ModelLoader::HashEmbeddedTexture(aiTexture const* texture) noexcept
	{
		if (!texture) return { "INVALID EMBEDDED TEXTURE" };

		uint8_t const* data{ nullptr };
		size_t size{ 0 };

		if (texture->mHeight == 0) 
		{
			// Compressed texture (e.g., PNG, JPEG)
			data = reinterpret_cast<const uint8_t*>(texture->pcData);
			size = texture->mWidth;
		}
		else 
		{
			// Uncompressed texture (raw texels: BGRA)
			data = reinterpret_cast<const uint8_t*>(texture->pcData);
			size = texture->mWidth * texture->mHeight * sizeof(aiTexel);
		}

		// Simple 64-bit FNV-1a hash
		uint64_t hash = 14695981039346656037ull;
		for (size_t i = 0; i < size; ++i) 
		{
			hash ^= data[i];
			hash *= 1099511628211ull;
		}


		// Convert to hex string
		char buffer[17];
		snprintf(buffer, sizeof(buffer), "%016llx", static_cast<unsigned long long>(hash));
		return std::string{ buffer };
	}
}
