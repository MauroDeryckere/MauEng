#ifndef MAUREN_LOADEDMODEL_H
#define MAUREN_LOADEDMODEL_H

#include <vector>

#include "BindlessData.h"
#include "Vertex.h"

namespace MauRen
{
	struct LoadedModel final
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<SubMeshData> subMeshes;
	};
}

#endif