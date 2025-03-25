#pragma once
#ifndef PENGUIN_CONSTANTS_H
#define PENGUIN_CONSTANTS_H

namespace PenguinEngine {

	static const uint16_t SPAWN_COUNT = 2;
	static const float SPAWN_SIZE = 3.0f;
	static const float CAMERA_DISTANCE = 5.0f;

	static const float CAMERA_MOVE_SPEED = 4.0f;
	static const float CAMERA_FAST_SPEED_MULTIPLIER = 2.0f;
	static const float CAMERA_TURN_SPEED = 70.0f;
	static const float CAMERA_TURN_MIN_SCREEN_DELTA = 5.0f;

	static const int DEFAULT_WINDOW_WIDTH = 800;
	static const int DEFAULT_WINDOW_HEIGHT = 600;

	static const double PI = 3.141592653589793238462643383279502884L;

}
#endif