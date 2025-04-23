#ifndef MAUREN_VERTEX_H
#define MAUREN_VERTEX_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace MauRen
{
	struct Vertex final
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec4 tangent; // .xyz = tangent vector, .w = handedness
		glm::vec2 texCoord;

		bool operator==(const Vertex& other) const
		{
			return position == other.position &&
				normal == other.normal &&
				tangent == other.tangent &&
				texCoord == other.texCoord;
		}

	};
}

namespace std
{
	template<> struct hash<MauRen::Vertex>
	{
		size_t operator()(MauRen::Vertex const& vertex) const noexcept
		{
			return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec2>()(vertex.texCoord) << 1)));
		}
	};
}

#endif // MAUREN_VERTEX_H