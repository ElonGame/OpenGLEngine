#ifndef BEZIERCURVE_H
#define BEZIERCURVE_H

#include <glm/glm.hpp>

class BezierCurve
{
public:
	BezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
	void SetControlPoints(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
	glm::vec3 GetPoint(float t);
	glm::vec3 GetTangent(float t);

private:
	glm::vec3 p0, p1, p2, p3;
};

#endif