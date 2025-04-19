#include <doctest/doctest.h>
#include "Components/CTransform.h"


TEST_CASE("CTransform Default Constructor")
{
	MauEng::CTransform transform;
    CHECK(transform.translation == glm::vec3{ 0.0f, 0.0f, 0.0f });
    CHECK(transform.rotation.rotation == glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f });
    CHECK(transform.scale == glm::vec3{ 1.0f, 1.0f, 1.0f });
}

TEST_CASE("CTransform Constructor with Parameters")
{
    glm::vec3 constexpr position(1.0f, 2.0f, 3.0f);
    glm::quat constexpr rotation(0.707f, 0.0f, 0.707f, 0.0f); // 90-degree rotation around Y-axis
    glm::vec3 constexpr scale(2.0f, 2.0f, 2.0f);

    MauEng::CTransform transform{ position, rotation, scale };

    CHECK(transform.translation == position);
    CHECK(transform.rotation.rotation == rotation);
    CHECK(transform.scale == scale);
}

TEST_CASE("CTransform Combine Transformations")
{
	MauEng::CTransform transform;

    glm::vec3 constexpr position{ 10.0f, 0.0f, 0.0f };
	// 90-degree rotation around Y-axis
    glm::quat constexpr rotation{ 0.707f, 0.0f, 0.707f, 0.0f };
    glm::vec3 constexpr scale{ 2.0f, 2.0f, 2.0f };

    transform.translation = position;
    transform.rotation.rotation = rotation;
    transform.scale = scale;

    glm::mat4 expectedMatrix = glm::translate(glm::mat4{ 1.0f }, position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4{ 1.0f }, scale);

    CHECK(transform.GetMatrix() == expectedMatrix);
}