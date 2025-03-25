#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>

#include "transform_object.h"
#include "time.h"
#include "constants.h"
#include "window.h"

namespace PenguinEngine {
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

	Camera(float fov, float aspectRatio, float nearPlane, float farPlane);

	void ResetCamera();

	glm::mat4 GetViewMatrix();

	glm::mat4 GetProjectionMatrix(bool invertY = true);

	glm::vec4 GetScreenSpaceViewPos(double x, double y);

	//glm::vec4 GetScreenSpaceWorldPos(double x, double y, float distance = 1.0f)

	void moveCamera(glm::vec3 dir, float speedMult);

	void rotateCamera(glm::vec2 dir);


};
}

#endif