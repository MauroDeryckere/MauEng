#ifndef MAUENG_CTRANSFORM_H
#define MAUENG_CTRANSFORM_H

#include "Math/Rotator.h"

namespace MauEng
{
	struct CTransform final
	{
		glm::mat4 mat{ 1.0f };

        void Translate(glm::vec3 const& translation) noexcept
        {
            mat = glm::translate(mat, translation);
        }
        void ResetTransformation() noexcept
        {
            mat = glm::mat4{ 1.0f };
        }
        void Rotate(MauCor::Rotator const& rotator) noexcept
        {
            mat *= glm::toMat4(rotator.rotation);
        }
        void Scale(glm::vec3 const& s) noexcept
        {
            mat = glm::scale(mat, s);
        }
    };
}

#endif