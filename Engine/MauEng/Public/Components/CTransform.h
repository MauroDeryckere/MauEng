#ifndef MAUENG_CTRANSFORM_H
#define MAUENG_CTRANSFORM_H

#include "Math/Rotator.h"

namespace MauEng
{
	struct CTransform final
	{
        glm::vec3 translation{ };
        MauCor::Rotator rotation{ };
        glm::vec3 scale{ 1.0f };

		glm::mat4 mat{ 1.0f };

        bool isDirty{ false };

        void Translate(glm::vec3 const& t) noexcept
        {
            translation += t;
            isDirty = true;
        }

        void ResetTransformation() noexcept
        {
            translation = glm::vec3{ 0.0f };
            rotation = MauCor::Rotator{};
            scale = glm::vec3{ 1.0f };
            isDirty = true;
        }

        void Rotate(MauCor::Rotator const& rotator) noexcept
        {
            rotation = rotation * rotator;
            isDirty = true;
        }

        void Scale(glm::vec3 const& s) noexcept
        {
            scale *= s;
            isDirty = true;
        }

        void UpdateMatrix() noexcept
        {
            if (isDirty)
            {
                mat = glm::translate(glm::mat4(1.0f), translation);
                mat *= glm::toMat4(rotation.rotation);
                mat = glm::scale(mat, scale);
                isDirty = false; 
            }
        }

        [[nodiscard]] glm::mat4 GetMatrix() noexcept
        {
            UpdateMatrix();
            return mat;
        }
    };
}

#endif