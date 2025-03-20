//#include "Transform.h"
//
//#define GLM_FORCE_RADIANS
//#define GLM_ENABLE_EXPERIMENTAL
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/quaternion.hpp>
//#include <glm/gtx/euler_angles.hpp>
//#include <glm/gtx/matrix_decompose.hpp>
//#include <glm/gtc/constants.hpp>
//
//Transform::Transform() {
//	_position = glm::vec3(0.0f, 0.0f, 0.0f);
//	_rotationQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
//	_scale = glm::vec3(1.0f, 1.0f, 1.0f);
//
//	_right = glm::vec3(1.0f, 0.0f, 0.0f);
//	_up = glm::vec3(0.0f, 1.0f, 0.0f);
//	_forward = glm::vec3(0.0f, 0.0f, 1.0f);
//
//	_model = glm::mat4(1.0f);
//	_worldToLocal = glm::inverse(_model);
//}
//
//void Transform::SetPosition(glm::vec3 newPos) {
//	_position = newPos;
//	_model[3] = glm::vec4(newPos.x, newPos.y, newPos.z, _model[3][3]);
//	_isPosDirty = true;
//}
//
//void Transform::SetRotation_Euler(glm::vec3 euler) {
//	_rotationQuat = glm::toQuat(glm::orientate3(euler));
//	_isRotDirty = true;
//}
//
//void Transform::SetRotation_Quat(glm::quat newRot) {
//	_rotationQuat = newRot;
//	_isRotDirty = true;
//}
//
//void Transform::SetRotation_Matrix(glm::mat4 newRotMatrix) {
//	_rotationQuat = glm::toQuat(newRotMatrix);
//	_isRotDirty = true;
//}
//
//void Transform::Rotate(float angleRadians, glm::vec3 axis) {
//	//SetRotation_Matrix(glm::rotate(_model, angleRadians, axis));
//
//}
//
//void Transform::Rotate(glm::quat rotation) {
//	SetRotation_Quat(rotation * _rotationQuat);
//}
//
//void Transform::Rotate(glm::vec3 rotation) {
//	_rotationQuat *= glm::toQuat(glm::orientate3(rotation));
//	_isRotDirty;
//}
//
//void Transform::SetScale(glm::vec3 newScale) {
//	_scale = newScale;
//	//_isDirty = true;
//}
//
// void Transform::setLocalToWorldMatrix(glm::mat4 modelMat) {
//	_model = modelMat;
//	glm::vec3 skew;
//	glm::vec4 perspective;
//	glm::decompose(modelMat, _scale, _rotationQuat, _position, skew, perspective);
//	computeBasisVectors();
//	recomputeWorldToLocal();
// }
//
// //should be called when _isDirty is true
//void Transform::computeModelMatrices() {
//	glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0), _position);
//	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0), _scale);;
//	_model = translateMatrix * scaleMatrix;
//	recomputeWorldToLocal();
//	_isPosDirty = false;
//}
//
//void Transform::recomputeWorldToLocal() {
//	_worldToLocal = glm::inverse(_model);
//}
//
////Call this each time any rotation is changed
//void Transform::computeBasisVectors() {
//	//_right = glm::normalize(glm::vec3(_rotationMat[0][0], _rotationMat[1][0], _rotationMat[2][0]));
//	//_up = glm::normalize(glm::vec3(_rotationMat[0][1], _rotationMat[1][1], _rotationMat[2][1]));
//	//
//	// 
//	// 
//	// 
//	// 
//	//_forward = glm::normalize(glm::vec3(_rotationMat[0][2], _rotationMat[1][2], _rotationMat[2][2]));
//	/*_right = glm::vec3(_rotationMat[0]);
//	_up = glm::vec3(_rotationMat[1]);
//	_forward = glm::vec3(_rotationMat[2]);*/
//}
//
//
//void Transform::RotateAtAxis(glm::vec3 rotationPos, float angleRadians, glm::vec3 rotationAxis) {
//	glm::mat4 rotPosLocalMat = glm::translate(glm::mat4(1), rotationPos);
//	glm::mat4 rotPosRotatedMat = glm::rotate(rotPosLocalMat, angleRadians, rotationAxis);
//
//	glm::mat4 invertedRotPosMat = glm::inverse(rotPosRotatedMat);
//
//	glm::vec4 translatedPos = rotPosLocalMat * glm::vec4(_position, 1.0);
//	glm::vec4 rotatedPos = rotPosRotatedMat * translatedPos;
//
//	glm::vec3 worldPos = invertedRotPosMat * rotatedPos;
//
//	_position = worldPos;
//	_rotationQuat = glm::toQuat(rotPosRotatedMat) * _rotationQuat;
//	computeModelMatrices();
//}
//
//
//void Transform::LookAt(glm::vec3 targetPosition, glm::vec3 upVector) {
//	Transform::LookAt(_position, targetPosition, upVector);
//}
//
//void Transform::LookAt(glm::vec3 eyePos, glm::vec3 targetPosition, glm::vec3 upVector) {
//	glm::vec3 direction = glm::normalize(glm::vec3(targetPosition - eyePos));
//	glm::vec3 rotationZ = direction;
//	glm::vec3 rotationX = glm::normalize(glm::cross(upVector, rotationZ));
//	glm::vec3 rotationY = glm::normalize(glm::cross(rotationZ, rotationX));
//
//	glm::mat4 rotation(
//		rotationX.x, rotationY.x, rotationZ.x, 0.0f,
//		rotationX.y, rotationY.y, rotationZ.y, 0.0f,
//		rotationX.z, rotationY.z, rotationZ.z, 0.0f, 
//		0.0f, 0.0f, 0.0f, 1.0f);
//
//	glm::mat4 pos(
//		1.0f, 0.0f, 0.0f, eyePos.x,
//		0.0f, 1.0f, 0.0f, eyePos.y,
//		0.0f, 0.0f, 1.0f, eyePos.z,
//		0.0f, 0.0f, 0.0f, 1.0f);
//
//	_position = eyePos;
//	_rotationQuat = glm::toQuat(rotation);
//
//	_model = pos * rotation * glm::scale(glm::mat4(1.0f), _scale);
//	recomputeWorldToLocal();
//
//	_right = rotationX;
//	_up = rotationY;
//	_forward = rotationZ;
//}
//
//glm::vec3 Transform::GetPosition() {
//	return _position;
//}
//
//glm::quat Transform::GetRotationQuat() {
//	return _rotationQuat;
//}
//
//glm::mat4 Transform::GetRotationMatrix() {
//	return glm::mat4(1.0f);
//}
//
//glm::vec3 Transform::GetScale() {
//	return _scale;
//}
//
//glm::vec3 Transform::getForward() {
//	return _forward;
//}
//
//glm::vec3 Transform::getRight() {
//	return _right;
//}
//
//glm::vec3 Transform::getUp() {
//	return _up;
//}
//
//glm::mat4 Transform::getLocalToWorldMatrix() {
//	if(_isPosDirty){
//		computeModelMatrices();
//	}
//	return _model;
//}
//
//glm::mat4 Transform::getWorldToLocalMatrix() {
//	if (_isPosDirty) {
//		computeModelMatrices();
//	}
//	return _worldToLocal;
//}
//
