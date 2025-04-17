#ifndef MAUENG_CTRANSFORM_H
#define MAUENG_CTRANSFORM_H

#include "Math/Rotator.h"

namespace MauEng
{
    //TODO dirty flag



	struct CTransform final
	{
        glm::vec3 position{ 0.0f };
        MauCor::Rotator rotation{ };
        glm::vec3 scale{ 1.f };

        [[nodiscard]] glm::mat4 GetTransformMatrix() const noexcept
        {
            return glm::translate(glm::mat4{ 1.0f }, position) * glm::mat4_cast(rotation.rotation) * glm::scale(glm::mat4{ 1.0f }, scale);
        }

        //void MeshInstance::Translate(glm::vec3 const& translation) noexcept
        //{
        //    m_ModelMatrix = glm::translate(m_ModelMatrix, translation);
        //}

        //void MeshInstance::ResetTransformation() noexcept
        //{
        //    m_ModelMatrix = glm::mat4{ 1.0f };
        //}

        //void MeshInstance::Rotate(MauCor::Rotator const& rotator) noexcept
        //{
        //    m_ModelMatrix *= glm::toMat4(rotator.rotation);
        //}

        //void MeshInstance::Scale(glm::vec3 const& scale) noexcept
        //{
        //    m_ModelMatrix = glm::scale(m_ModelMatrix, scale);
        //}

        void Translate(glm::vec3 const& translation) noexcept
        {
            position += translation;
        }
        void ResetTransformation() noexcept
        {
            position = glm::vec3{ 0.f };
            rotation = MauCor::Rotator{};
            scale = glm::vec3{ 1.f };
        }
        void Rotate(MauCor::Rotator const& rotator) noexcept
        {
            rotation = rotation * rotator;
        }
        void Scale(glm::vec3 const& s) noexcept
        {
            scale *= s;
        }
    };
}

#endif