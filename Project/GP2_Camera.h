#pragma once
class GP2_Camera
{
public:
	GP2_Camera();
	~GP2_Camera();

	glm::mat4 CreateCamera(float translate, glm::vec2 const& rotate);

private:

};
