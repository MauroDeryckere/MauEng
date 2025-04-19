#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace MauRen
{
	void ModelLoader::LoadModel(const std::string& path)
	{
		Assimp::Importer importer;

		// And have it read the given file with some example postprocessing
		// Usually - if speed is not the most important aspect for you - you'll
		// probably to request more postprocessing than we do in this example.
		 aiScene const* scene{ importer.ReadFile(path,
												aiProcess_CalcTangentSpace |
												aiProcess_Triangulate |
												aiProcess_JoinIdenticalVertices |
												aiProcess_SortByPType | 
												aiProcess_OptimizeMeshes) };

		 if (nullptr == scene) 
         {
            //TODO
			 //DoTheErrorLogging(importer.GetErrorString());
			 return;
		 }

		 if (!scene || !scene->HasMeshes()) 
		 {
            //TODO
			 std::cerr << "Error loading model!" << std::endl;
			 return;
		 }

         for (unsigned int i{ 0 }; i < scene->mNumMeshes; ++i)
         {
             aiMesh* mesh = scene->mMeshes[i];

             // Process the mesh
             std::vector<Vertex> vertices{};
             std::vector<uint32_t> indices{};

             // Vertices
             for (unsigned int j{ 0 }; j < mesh->mNumVertices; ++j) 
             {
                 aiVector3D vertex = mesh->mVertices[j];
                // aiVector3D normal = mesh->mNormals[j];
                 aiColor4D* color = mesh->mColors[j];
                 aiVector3D* texcoord = mesh->mTextureCoords[j];
             	// Assuming you have a custom Vertex struct
                 Vertex const vert
                 {
                    .position = glm::vec3(vertex.x, vertex.y, vertex.z),
					.color = color ? glm::vec3(color->r, color->g, color->b) : glm::vec3(1.0f),
					.texCoord = texcoord ? glm::vec2(texcoord->x, texcoord->y) : glm::vec2(0.0f)
                 };

                 vertices.emplace_back(vert);
             }

             // Indices
             for (unsigned int j = 0; j < mesh->mNumFaces; j++) 
             {
                 aiFace face = mesh->mFaces[j];
                 for (unsigned int k = 0; k < face.mNumIndices; k++) 
                 {
                     indices.emplace_back(face.mIndices[k]);
                 }
             }

             // Optionally handle materials or textures for the mesh here.
             aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
             auto const matNamem = material->GetName();
             aiTexture diffuse;
			 material->Get(AI_MATKEY_COLOR_DIFFUSE,diffuse);

             // LoadMaterial in manager

             // static mesh mat IDs -> add material ID
			 // static mesh mesh IDS -> add mesh ID
         }
	}
}