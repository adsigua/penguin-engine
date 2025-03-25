#include "transform.h"

Transform::Transform() {
	_position = glm::vec3(0.0f, 0.0f, 0.0f);
	_rotationQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	_rotationEuler = glm::vec3(0.0f, 0.0f, 0.0f);
	_scale = glm::vec3(1.0f, 1.0f, 1.0f);

	_right = glm::vec3(1.0f, 0.0f, 0.0f);
	_up = glm::vec3(0.0f, 1.0f, 0.0f);
	_forward = glm::vec3(0.0f, 0.0f, 1.0f);

	_model = glm::mat4(1.0f);
	_worldToLocal = glm::mat4(1.0f);

	_isRotDirty = false;
	_isScaleDirty = false;
	_recomputeBasis = false;
}

void Transform::SetPosition(glm::vec3 newPos) {
	_position = newPos;
	_model[3] = glm::vec4(newPos.x, newPos.y, newPos.z, _model[3][3]);
	RecomputeWorldToLocal();
}

void Transform::SetRotation_Euler(glm::vec3 euler) {
	_rotationEuler = euler;

	_rotationQuat = glm::toQuat(glm::yawPitchRoll(_rotationEuler.y, _rotationEuler.x, _rotationEuler.z));
	_isRotDirty = true;
	_recomputeBasis = true;
}

void Transform::SetRotation_Quat(glm::quat rotation) {
	_rotationQuat = rotation;
	_rotationEuler = glm::eulerAngles(_rotationQuat);
	_isRotDirty = true;
	_recomputeBasis = true;
}

void Transform::Rotate(float angleRadians, glm::vec3 axis) {
	_rotationQuat = glm::angleAxis(angleRadians, axis) * _rotationQuat;
	_rotationEuler = glm::eulerAngles(_rotationQuat);
	_isRotDirty = true;
	_recomputeBasis = true;
}

//rotate by quaternion
void Transform::Rotate(glm::quat rotation) {
	_rotationQuat = rotation * _rotationQuat;
	_rotationEuler = glm::eulerAngles(_rotationQuat);
	_isRotDirty = true;
	_recomputeBasis = true;
}

//rotate by euler angles
void Transform::Rotate(glm::vec3 eulerAngles) {
	_rotationQuat = glm::toQuat(glm::yawPitchRoll(_rotationEuler.y, _rotationEuler.x, _rotationEuler.z)) * _rotationQuat;
	_rotationEuler = glm::eulerAngles(_rotationQuat);
	_isRotDirty = true;
	_recomputeBasis = true;
}


void Transform::SetScale(glm::vec3 scale) {
	_scale = scale;
	_isScaleDirty = true;
}

void Transform::RecomputeModelMatrices() {
	if (!_isScaleDirty && !_isRotDirty) {
		return;
	}
	glm::mat4 translationMat = glm::translate(glm::mat4(1.0f), _position);
	glm::mat4 rotMat = glm::toMat4(_rotationQuat);
	glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), _scale);
	_model = translationMat * rotMat * scaleMat;
	RecomputeWorldToLocal();
	RecomputeBasisVectors();
	_isRotDirty = false;
	_isScaleDirty = false;
}

void Transform::RecomputeWorldToLocal() {
	_worldToLocal = glm::inverse(_model);
}


void Transform::RecomputeBasisVectors() {
	if (!_recomputeBasis)
		return;
	glm::mat3 rotMat = glm::toMat3(_rotationQuat);
	_right = glm::vec3(rotMat[0]);
	_up = glm::vec3(rotMat[1]);
	_forward = glm::vec3(rotMat[2]);
	_recomputeBasis = false;
}

void Transform::LookAt(glm::vec3 position, glm::vec3 up) {
	glm::mat4 lookAtMat = glm::lookAt(glm::vec3(0.0f), _position - position, up);
	_rotationQuat = glm::toQuat(glm::transpose(lookAtMat));
	_rotationEuler = glm::eulerAngles(_rotationQuat);
	_isRotDirty = true;
	_recomputeBasis = true;
}

