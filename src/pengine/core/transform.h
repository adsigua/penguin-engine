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
	Transform();

	glm::vec3 GetPosition() {
		return _position;
	}

	void SetPosition(glm::vec3 newPos);

	glm::quat GetRotationQuat() {
		return _rotationQuat;
	}

	glm::quat GetRotationeEuler() {
		return _rotationEuler;
	}

	void SetRotation_Euler(glm::vec3 euler);

	void SetRotation_Quat(glm::quat rotation);

	//rotate by an angle with an axis of rotation
	void Rotate(float angleRadians, glm::vec3 axis);

	//rotate by quaternion
	void Rotate(glm::quat rotation);

	//rotate by euler angles
	void Rotate(glm::vec3 eulerAngles);

	glm::vec3 GetScale() {
		return _scale;
	}

	void SetScale(glm::vec3 scale);

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

	void RecomputeModelMatrices();

	void RecomputeWorldToLocal();

	glm::mat4 getLocalToWorldMatrix() {
		RecomputeModelMatrices();
		return _model;
	}

	glm::mat4 getWorldToLocalMatrix() {
		RecomputeModelMatrices();
		return _worldToLocal;
	}

	void RecomputeBasisVectors();

	void LookAt(glm::vec3 position, glm::vec3 up);
};

#endif