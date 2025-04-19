#ifndef MAUENG_ENTITYID_H
#define MAUENG_ENTITYID_H

#include <cstdint>

namespace MauEng::ECS
{
	using EntityID = uint32_t;

	inline constexpr EntityID NULL_ENTITY_ID{ static_cast<EntityID>(-1) };
}

#endif
