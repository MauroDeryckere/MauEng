#include "Components/CStaticMesh.h"

#include "InternalServiceLocator.h"

namespace MauEng
{
	CStaticMesh::CStaticMesh(char const* path)
	{
		meshID = RENDERER.LoadOrGetMeshID(path);
	}
}
