#include "camera.h"

namespace PenguinEngine {
	Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane) {
		this->fov = fov;
		this->aspectRatio = aspectRatio;
		this->nearPlane = nearPlane;
		this->farPlane = farPlane;
	}

	void Camera::ResetCamera() {
		transform.SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
		transform.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		transform.SetRotation_Euler(glm::vec3(0.0f, 0.0f, 0.0f));
	}

	glm::mat4 Camera::GetViewMatrix() {
		glm::mat4 glmView = glm::lookAt(transform.GetPosition(), transform.GetPosition() + transform.getForward(), transform.getUp());
		return glmView;
	}

	glm::mat4 Camera::GetProjectionMatrix(bool invertY) {
		glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
		if (invertY) {
			projection[1][1] *= -1;
		}
		return projection;
	}

	glm::vec4 Camera::GetScreenSpaceViewPos(double x, double y) {
		float xNdc0 = (2.0f * x / Window::getWindowWidth()) - 1.0f, yNdc0 = 1.0f - (2.0f * y / Window::getWidowHeight());
		glm::vec4 csPos = glm::vec4(xNdc0, yNdc0, -1.0f, 1.0f);
		glm::mat4 invProj = glm::inverse(GetProjectionMatrix(false));
		glm::vec4 vPos = (invProj * csPos) / csPos.w;

		return vPos;
	}

	/*glm::vec4 GetScreenSpaceWorldPos(double x, double y, float distance = 1.0f) {
		float xNdc0 = (2.0f * x / windowSize.x) - 1.0f, yNdc0 = 1.0f - (2.0f * y / windowSize.y);
		glm::vec4 csPos = glm::vec4(xNdc0, yNdc0, -1.0f, 1.0f);
		glm::mat4 invProj = glm::inverse(GetProjectionMatrix(false));
		glm::vec4 vPos = (invProj * csPos) / csPos.w;

		glm::vec4 worldPos = GetScreenSpaceViewPos
		return vPos;
	}*/

	void Camera::moveCamera(glm::vec3 dir, float speedMult) {
		glm::vec3 camPos = transform.GetPosition();
		camPos += dir * CAMERA_MOVE_SPEED * speedMult * PenguinEngine::Time::getDeltaTime();
		transform.SetPosition(camPos);
	}

	void Camera::rotateCamera(glm::vec2 dir) {
		static glm::vec2 prevDir = glm::vec2(0.0f);

		glm::vec3 camEuler = glm::eulerAngles(transform.GetRotationQuat());
		float turnAngle = glm::radians(CAMERA_TURN_SPEED) * PenguinEngine::Time::getDeltaTime();

		if (dir.x != 0) {
			transform.Rotate(turnAngle * dir.x, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (dir.y != 0) {
			transform.Rotate(turnAngle * dir.y, transform.getRight());
		}
	}
}