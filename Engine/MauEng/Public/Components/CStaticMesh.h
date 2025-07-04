#ifndef MAUENG_CSTATICMESH_H
#define MAUENG_CSTATICMESH_H

#include "RendererIdentifiers.h"

namespace MauEng
{
	struct CStaticMesh final
	{
		uint32_t meshID{ MauRen::INVALID_MESH_ID };

		CStaticMesh(char const* path);
		~CStaticMesh();

		CStaticMesh(CStaticMesh const&) = default;
		CStaticMesh(CStaticMesh&&) = default;
		CStaticMesh& operator=(CStaticMesh const&) = default;
		CStaticMesh& operator=(CStaticMesh&&) = default;
	};
}

#endif