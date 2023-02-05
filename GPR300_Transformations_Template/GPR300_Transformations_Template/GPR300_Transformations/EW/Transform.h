#pragma once
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


struct Transform
{
	glm::vec3 pos;
	glm::quat rotQ;
	glm::vec3 rotE;
	glm::vec3 scale;
	glm::mat4 getModelMatrix();

};