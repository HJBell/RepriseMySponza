#pragma once

#include <glm/glm.hpp>
#include <sponza/sponza_fwd.hpp>

namespace Utils
{
	glm::vec3 SponzaToGLMVec3(const sponza::Vector3& v);
	glm::mat4 SponzaMat3ToGLMMat4(const sponza::Matrix4x3& m);
}


