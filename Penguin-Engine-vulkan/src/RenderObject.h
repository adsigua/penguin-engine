#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "TransformObject.h"

struct RenderObjectUniformBufferOjbect {
	alignas(16) glm::mat4 model;
};

class RenderObject : public TransformObject {
public:
	RenderObjectUniformBufferOjbect* GetUniformBufferObject() {
		_modelUniformBufferObject.model = transform.getLocalToWorldMatrix();
		return &_modelUniformBufferObject;
	}

	float rotOffset = 0.0f;
private:
	RenderObjectUniformBufferOjbect _modelUniformBufferObject;
};


#endif