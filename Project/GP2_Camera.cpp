#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi

#include "GP2_Camera.h"

GP2_Camera::GP2_Camera()
{
}

GP2_Camera::~GP2_Camera()
{
}

glm::mat4 GP2_Camera::CreateCamera(float translate, glm::vec2 const& rotate)
{
	glm::mat4 Projection{ glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.f) };
	glm::mat4 View{ glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -translate)) };

	View = glm::rotate(View, rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
	View = glm::rotate(View, rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 Model{ glm::scale(glm::mat4(1.0f), glm::vec3(0.5f)) };

	return Projection * View * Model;
}
