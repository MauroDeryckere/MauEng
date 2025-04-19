#include "doctest/doctest.h"
#include "Math/Rotator.h"
#include <glm/gtc/constants.hpp>

//Unit tests here are limited because glm is a heavily tested library we can rely on.

TEST_CASE("Test Rotator Construction with Pitch, Yaw, and Roll")
{
	MauCor::Rotator const rot(45.0f, 30.0f, 60.0f);
	glm::vec3 const testVec { 1.f, 0.f, 0.f };

	glm::vec3 const result{ rot * testVec };
	CHECK(result != testVec);
}

TEST_CASE("Test Rotator Multiplication")
{
    // 0.523598775 rad
    MauCor::Rotator const rot1{ 30.0f, 0.f, 0.f }; // Rotate 30 degrees around X-axis
    // 0.7853981625 rad
	MauCor::Rotator const rot2{ 0.f, 45.0f, 0.f }; // Rotate 45 degrees around Y-axis

    MauCor::Rotator const result{ rot1 * rot2 };

	// Rotate along the X-axis
    glm::vec3 const testVec { 1.f, 0.f, 0.f };
	// Apply the combined rotation
    glm::vec3 const rotatedVec{ result * testVec };

    glm::mat4 rotationMatrix = glm::mat4_cast(rot1.rotation) * glm::mat4_cast(rot2.rotation);
    glm::vec4 expectedVec = rotationMatrix * glm::vec4(testVec, 1.0f);

    CHECK(glm::epsilonEqual(rotatedVec.x, expectedVec.x, 0.001f));
    CHECK(glm::epsilonEqual(rotatedVec.y, expectedVec.y, 0.001f));
    CHECK(glm::epsilonEqual(rotatedVec.z, expectedVec.z, 0.001f));
}

TEST_CASE("Test simple rotation on Vector")
{
	// Rotate 90 degrees around the Y-axis
	MauCor::Rotator const rot{ 0.f, 90.f, 0.f };

	// Rotate along the X-axis
    glm::vec3 const testVec { 1.f, 0.f, 0.f };
	glm::vec3 const result{ rot * testVec };

    // After rotating 90 degrees around Y-axis, (1, 0, 0) should become (0, 0, -1)
    glm::vec3 const expectedResult { 0.f, 0.f, -1.f };

    CHECK(glm::all(glm::epsilonEqual(result, expectedResult, 0.001f)));
}

