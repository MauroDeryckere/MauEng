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
			glm::quat pitchRotation{ glm::angleAxis(glm::radians(pitch), glm::vec3{1, 0, 0}) };
			glm::quat yawRotation{ glm::angleAxis(glm::radians(yaw), glm::vec3{0, 1, 0}) };
			glm::quat rollRotation{ glm::angleAxis(glm::radians(roll), glm::vec3{0, 0, 1}) };

			rotation = glm::normalize(rollRotation * yawRotation * pitchRotation);
		}
	};
}

#endif	