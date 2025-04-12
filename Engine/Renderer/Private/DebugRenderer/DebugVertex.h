#ifndef MAUREN_DEBUGVERTEX_H
#define MAUREN_DEBUGVERTEX_H

#include <glm/glm.hpp>

namespace MauRen
{
	struct DebugVertex
	{
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 color;
	};
}

#endif