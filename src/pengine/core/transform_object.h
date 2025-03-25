#ifndef TRANSFORM_OBJECT_H
#define TRANSFORM_OBJECT_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "transform.h"

class TransformObject {
public:
	Transform transform;
};

#endif