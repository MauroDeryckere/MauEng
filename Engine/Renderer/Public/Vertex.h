#ifndef MAUREN_VERTEX_H
#define MAUREN_VERTEX_H

#include "RendererPCH.h"
#include <glm/glm.hpp>

namespace MauRen
{
	struct Vertex final
	{
		glm::vec2 position;
		glm::vec3 color;
	};
}

#endif // MAUREN_VERTEX_H