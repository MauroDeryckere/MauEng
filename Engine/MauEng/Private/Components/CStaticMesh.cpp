#include "Components/CStaticMesh.h"

#include "InternalServiceLocator.h"
#include "MeshInstance.h"
namespace MauEng
{
	CStaticMesh::CStaticMesh(char const* path)
	{
		auto const& data{ RENDERER.LoadOrGetMeshData(path) };

		meshID = data.m_MeshID;
		materialID = data.m_MaterialID;

		ME_ASSERT(meshID != UINT32_MAX);
		ME_ASSERT(materialID != UINT32_MAX);
	}
}
