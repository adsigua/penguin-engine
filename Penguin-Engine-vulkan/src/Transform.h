#ifndef TRANSFORM_H
#define TRANSFORM_H

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
//#include <glm/ext/quaternion_float.hpp>
//#include <glm/ext/quaternion_common.hpp>


class Transform {
	glm::vec3 _position;
	glm::quat _rotation;
	glm::vec3 _scale;

	glm::vec3 _forward;
	glm::vec3 _up;
	glm::vec3 _right;

	glm::mat4 _localToWorld;
	glm::mat4 _worldToLocal;
public:
	Transform();

	void SetPosition(glm::vec3);
	glm::vec3 GetPosition();

	void SetRotation_Euler(glm::vec3);
	void SetRotation_Quat(glm::quat);
	glm::quat GetRotation();

	void SetScale(glm::vec3);
	glm::vec3 GetScale();

	glm::vec3 getForward();
	glm::vec3 getRight();
	glm::vec3 getUp();
	void computeModelMatrices();

	glm::mat4 getLocalToWorldMatrix();
	glm::mat4 getWorldToLocalMatrix();

	void RotateAtAxis(glm::vec3 rotationPos, float angleRadians, glm::vec3 rotationAxis);

	void LookAt(glm::vec3 position, glm::vec3 upVector);
};

#endif