#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "TransformObject.h"

class Camera : public TransformObject {
public:
	float fov;
	float nearPlane;
	float farPlane;
	float aspectRatio;

	glm::mat4 GetViewMatrix() {
		return glm::lookAt(transform.GetPosition(), transform.GetPosition()+transform.getForward(), transform.getUp());
	}

	glm::mat4 GetProjectionMatrix() {
		return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	}
};

#endif