#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>

#include "TransformObject.h"
#include "Time.h"

struct CameraUniformBufferOjbect {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

const glm::mat4 flipMat = glm::mat4(
	-1.0f, 0.0f, 0.0f, 0.0f,		//col 0
	0.0f, 1.0f, 0.0f, 0.0f,		//col 1
	0.0f, 0.0f, -1.0f, 0.0f,	//col 2
	0.0f, 0.0f, 0.0f, 1.0f		//col 3
);

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

	void ResetCamera(bool faceNegativeZ = true) {
		transform.SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
		transform.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		if (faceNegativeZ) {
			transform.SetRotation_Euler(glm::vec3(0.0f, glm::radians(180.0f), 0.0f));
		}
		else {
			transform.SetRotation_Euler(glm::vec3(0.0f, 0.0f, 0.0f));
		}
	}

	glm::mat4 GetViewMatrix() {
		glm::mat4 glmView = glm::lookAt(transform.GetPosition(), transform.GetPosition() + transform.getForward(), transform.getUp());
		return glmView;
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
	

	std::string GetMatrixString(glm::mat4 mat) {
		return
			"\n{ " + std::to_string(mat[0][0]) + ", " + std::to_string(mat[1][0]) + ", " + std::to_string(mat[2][0]) + ", " + std::to_string(mat[3][0]) + "\n" +
			"  " + std::to_string(mat[0][1]) + ", " + std::to_string(mat[1][1]) + ", " + std::to_string(mat[2][1]) + ", " + std::to_string(mat[3][1]) + "\n" +
			"  " + std::to_string(mat[0][2]) + ", " + std::to_string(mat[1][2]) + ", " + std::to_string(mat[2][2]) + ", " + std::to_string(mat[3][2]) + "\n" +
			"  " + std::to_string(mat[0][3]) + ", " + std::to_string(mat[1][3]) + ", " + std::to_string(mat[2][3]) + ", " + std::to_string(mat[3][3]) + " }";
	}
};

#endif