#ifndef MAUENG_CSTATICMESH_H
#define MAUENG_CSTATICMESH_H

namespace MauEng
{
	struct CStaticMesh final
	{
		uint32_t meshID{ UINT32_MAX };
		uint32_t materialID{ UINT32_MAX };

		CStaticMesh() = default;
		CStaticMesh(char const* path);
	};
}

#endif