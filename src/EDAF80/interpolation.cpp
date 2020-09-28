#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	// glm::vec2 res = glm::vec2(1*1 + x*-1, 1*0 + x*1);
	// auto const p_x = (res.x * p0 + res.y * p1);
	// return p_x;
	glm::vec2 x_2 = glm::vec2(1, x);
	glm::mat2 b = glm::mat2(1, -1, 0, 1);
	glm::mat3x2 p_vec = glm::mat3x2(p0.x, p1.x,
									p0.y, p1.y, 
									p0.z, p1.z);

	return x_2 * b * p_vec;
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{
	// glm::vec4 res = glm::vec4(
	// 						1.0f * 0.0f + x * -t + std::pow(x,2) * 2*t + std::pow(x, 3) * -t,
	// 						1.0f * 1 + x * 0.0f + std::pow(x,2) * (t-3) + std::pow(x,3) * (2-t),
	// 						1.0f * 0 + x * t + std::pow(x,2) * (3.0f - (2*t)) + std::pow(x,3) * (t-2),
	// 						1.0f * 0 + x * 0 + std::pow(x,2) * -t + std::pow(x,3) * t
	// 						);

	// auto const q_x = res.x * p0 + res.y * p1 + res.z * p2 + res.w * p3;
	// return q_x;
	glm::vec4 x_4 = glm::vec4(1.0f, x, x*x, x*x*x);
	glm::mat4 b = glm::mat4(0, -t, 2*t, -t,
							1, 0, t-3, 2-t,
							0, t, 3-2*t, t-2,
							0, 0, -t, t);

	glm::mat3x4 p_vec = glm::mat3x4(p0.x, p1.x, p2.x, p3.x, 
									p0.y, p1.y, p2.y, p3.y,
									p0.z, p1.z, p2.z, p3.z);

	return x_4 * b * p_vec;
}
