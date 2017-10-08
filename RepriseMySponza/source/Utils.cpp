#include "Utils.hpp"

glm::vec3 Utils::SponzaToGLMVec3(const sponza::Vector3& v)
{
	return glm::vec3(v.x, v.y, v.z);
}

glm::mat4 Utils::SponzaMat3ToGLMMat4(const sponza::Matrix4x3& m)
{
	return glm::mat4(m.m00, m.m01, m.m02, 0,
		m.m10, m.m11, m.m12, 0,
		m.m20, m.m21, m.m22, 0,
		m.m30, m.m31, m.m32, 1);
}