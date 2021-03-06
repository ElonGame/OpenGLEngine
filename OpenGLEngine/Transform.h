#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm\glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Component.h"
#include "QuaternionUtil.h"

class Transform : public Component
{
public:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

	glm::vec3 GetRight();
	glm::vec3 GetUp();
	glm::vec3 GetForward();

};


#endif