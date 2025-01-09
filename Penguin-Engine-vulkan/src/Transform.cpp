#include "Transform.h"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

Transform::Transform() {
	_position = glm::vec3(0, 0, 0);
	_rotation = glm::quat(1, 0, 0, 0);
	_scale = glm::vec3(1, 1, 1);

	_right = glm::vec3(1.0, 0.0, 0.0);
	_up = glm::vec3(0.0, -1.0, 0.0);
	_forward = glm::vec3(0.0, 0.0, 1.0);

	_localToWorld = glm::mat4(1);
	_worldToLocal = glm::inverse(_localToWorld);
}

void Transform::SetPosition(glm::vec3 newPos) {
	_position = newPos;
	computeModelMatrices();
}

void Transform::SetRotation_Euler(glm::vec3 euler) {
	_rotation = glm::toQuat(glm::orientate3(euler));
	computeModelMatrices();
}

void Transform::SetRotation_Quat(glm::quat newRot) {
	_rotation = newRot;
	computeModelMatrices();
}

void Transform::SetScale(glm::vec3 newScale) {
	_scale = newScale;
	computeModelMatrices();
}

glm::vec3 Transform::GetPosition() {
	return _position;
}

glm::quat Transform::GetRotation() {
	return _rotation;
}

glm::vec3 Transform::GetScale () {
	return _scale;
}

glm::vec3 Transform::getForward() {
	return _forward;
}

glm::vec3 Transform::getRight() {
	return _right;
}

glm::vec3 Transform::getUp() {
	return _up;
}

glm::mat4 Transform::getLocalToWorldMatrix() {
	return _localToWorld;
}

glm::mat4 Transform::getWorldToLocalMatrix() {
	return _worldToLocal;
}

void Transform::computeModelMatrices() {
	glm::mat4 baseMatrix = glm::translate(glm::mat4(1.0), _position);
	baseMatrix = glm::toMat4(_rotation) * baseMatrix;
	_localToWorld = glm::scale(baseMatrix, _scale);

	_right = _rotation * glm::vec3(1.0, 0.0, 0.0);
	_up = _rotation * glm::vec3(0.0, 1.0, 0.0);
	_forward = _rotation * glm::vec3(0.0, 0.0, 1.0);

	_worldToLocal = glm::inverse(_localToWorld);
}

void RotateAtAxis(glm::vec3 rotationPos, float angleRadians, glm::vec3 rotationAxis) {



}


void Transform::LookAt(glm::vec3 targetPosition, glm::vec3 upVector) {
	glm::vec3 targetDir = glm::normalize(targetPosition - _position);
	//glm::vec3 lookAtRight = glm::normalize(glm::cross(targetDir, upVector));
	//glm::vec3 lookAtUp = glm::normalize(glm::cross(lookAtRight, targetDir));
	//glm::mat3 lookAt = glm::lookAt(_position, targetPosition, upVector);


	_rotation = glm::rotation(glm::vec3(0, 0, 1.0f), targetDir) * glm::rotation(glm::vec3(0, 1.0f, 0), upVector);
	computeModelMatrices();
}
