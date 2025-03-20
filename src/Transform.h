#ifndef TRANSFORM_H
#define TRANSFORM_H

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/constants.hpp>

class Transform {
	glm::vec3 _position;
	glm::quat _rotationQuat;
	glm::vec3 _rotationEuler;
	glm::vec3 _scale;

	glm::vec3 _forward;
	glm::vec3 _up;
	glm::vec3 _right;

	glm::mat4 _model;
	glm::mat4 _worldToLocal;

	bool _recomputeBasis;
	bool _isRotDirty;
	bool _isScaleDirty;

public:
	Transform() {
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

	glm::vec3 GetPosition() {
		return _position;
	}

	void SetPosition(glm::vec3 newPos) {
		_position = newPos;
		_model[3] = glm::vec4(newPos.x, newPos.y, newPos.z, _model[3][3]);
		RecomputeWorldToLocal();
	}

	glm::quat GetRotationQuat() {
		return _rotationQuat;
	}

	glm::quat GetRotationeEuler() {
		return _rotationEuler;
	}

	void SetRotation_Euler(glm::vec3 euler) {
		_rotationEuler = euler;

		_rotationQuat = glm::toQuat(glm::yawPitchRoll(_rotationEuler.y, _rotationEuler.x, _rotationEuler.z));
		_isRotDirty = true;
		_recomputeBasis = true;
	}

	void SetRotation_Quat(glm::quat rotation) {
		_rotationQuat = rotation;
		_rotationEuler = glm::eulerAngles(_rotationQuat);
		_isRotDirty = true;
		_recomputeBasis = true;
	}

	//rotate by an angle with an axis of rotation
	void Rotate(float angleRadians, glm::vec3 axis) {
		_rotationQuat = glm::angleAxis(angleRadians, axis) * _rotationQuat;
		_rotationEuler = glm::eulerAngles(_rotationQuat);
		_isRotDirty = true;
		_recomputeBasis = true;
	}

	//rotate by quaternion
	void Rotate(glm::quat rotation) {
		_rotationQuat = rotation * _rotationQuat;
		_rotationEuler = glm::eulerAngles(_rotationQuat);
		_isRotDirty = true;
		_recomputeBasis = true;
	}

	//rotate by euler angles
	void Rotate(glm::vec3 eulerAngles) {
		_rotationQuat = glm::toQuat(glm::yawPitchRoll(_rotationEuler.y, _rotationEuler.x, _rotationEuler.z)) * _rotationQuat;
		_rotationEuler = glm::eulerAngles(_rotationQuat);
		_isRotDirty = true;
		_recomputeBasis = true;
	}

	glm::vec3 GetScale() {
		return _scale;
	}

	void SetScale(glm::vec3 scale) {
		_scale = scale;
		_isScaleDirty = true;
	}

	glm::vec3 getForward() {
		RecomputeBasisVectors();
		return _forward;
	}

	glm::vec3 getRight() {
		RecomputeBasisVectors();
		return _right;
	}

	glm::vec3 getUp() {
		RecomputeBasisVectors();
		return _up;
	}

	void RecomputeModelMatrices() {
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

	void RecomputeWorldToLocal() {
		_worldToLocal = glm::inverse(_model);
	}

	glm::mat4 getLocalToWorldMatrix() {
		RecomputeModelMatrices();
		return _model;
	}

	glm::mat4 getWorldToLocalMatrix() {
		RecomputeModelMatrices();
		return _worldToLocal;
	}

	void RecomputeBasisVectors() {
		if (!_recomputeBasis)
			return;
		glm::mat3 rotMat = glm::toMat3(_rotationQuat);
		_right = glm::vec3(rotMat[0]);
		_up = glm::vec3(rotMat[1]);
		_forward = glm::vec3(rotMat[2]);
		_recomputeBasis = false;
	}

	void LookAt(glm::vec3 position, glm::vec3 up) {
		glm::mat4 lookAtMat = glm::lookAt(glm::vec3(0.0f), _position - position, up);
		_rotationQuat = glm::toQuat(glm::transpose(lookAtMat));
		_rotationEuler = glm::eulerAngles(_rotationQuat);
		_isRotDirty = true;
		_recomputeBasis = true;
	}
};

#endif