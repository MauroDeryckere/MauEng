#ifndef MAUCOR_ROTATOR_H
#define MAUCOR_ROTATOR_H

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/vec3.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/gtx/quaternion.hpp"

namespace MauCor
{
	struct Rotator final
	{
		glm::quat rotation{ 1.f, 0.f, 0.f, 0.f };

		constexpr Rotator() = default;
		Rotator(float pitch, float yaw = 0.f, float roll = 0.f)
		{
			glm::quat const pitchRotation{ glm::angleAxis(glm::radians(pitch), glm::vec3{1, 0, 0}) };
			glm::quat const yawRotation{ glm::angleAxis(glm::radians(yaw), glm::vec3{0, 1, 0}) };
			glm::quat const rollRotation{ glm::angleAxis(glm::radians(roll), glm::vec3{0, 0, 1}) };

			rotation = glm::normalize(rollRotation * yawRotation * pitchRotation);
		}

		explicit Rotator(glm::quat const& quat) : rotation { quat } { }
	};

	inline Rotator operator*(MauCor::Rotator const& lhs, MauCor::Rotator const& rhs) noexcept
	{
		// Multiply two quaternions (rotation concatenation)
		return MauCor::Rotator{ lhs.rotation * rhs.rotation };
	}

	inline glm::vec3 operator*(MauCor::Rotator const& rot, glm::vec3 const& vec) noexcept
	{
		// Apply the rotation to the vector using glm::rotate
		return glm::rotate(rot.rotation, vec);
	}
}

#endif	