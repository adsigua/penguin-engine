#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "TransformObject.h"

struct CameraUniformBufferOjbect {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class Camera : public TransformObject {
public:
	float fov;
	float nearPlane;
	float farPlane;
	float aspectRatio;

	Camera() {

	}

	~Camera() {

	}

	Camera(float fov, float aspectRatio, float nearPlane, float farPlane) {
		this->fov = fov;
		this->aspectRatio = aspectRatio;
		this->nearPlane = nearPlane;
		this->farPlane = farPlane;
	}

	glm::mat4 GetViewMatrix() {
		return glm::lookAt(transform.GetPosition(), transform.GetPosition()+transform.getForward(), transform.getUp());
	}

	glm::mat4 GetProjectionMatrix(bool invertY = true) {
		glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
		if (invertY) {
			projection[1][1] *= -1;
		}
		return projection;
	}

	CameraUniformBufferOjbect* GetUniformBufferObject() {
		_cameraBufferObject.view = GetViewMatrix();
		_cameraBufferObject.proj = GetProjectionMatrix();
		return &_cameraBufferObject;
	}

private:
	CameraUniformBufferOjbect _cameraBufferObject{};
};



#endif