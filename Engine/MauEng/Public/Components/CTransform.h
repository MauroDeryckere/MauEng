#ifndef MAUENG_CTRANSFORM_H
#define MAUENG_CTRANSFORM_H

#include "Math/Rotator.h"

namespace MauEng
{
	struct CTransform final
	{
        glm::vec3 position{ 0.0f };
        MauCor::Rotator rotation{ };
        glm::vec3 scale{ 1.f };

        glm::mat4 GetTransformMatrix() const
        {
            return glm::translate(glm::mat4{ 1.0f }, position) * glm::mat4_cast(rotation.rotation) * glm::scale(glm::mat4{ 1.0f }, scale);
        }
    };
}

#endif