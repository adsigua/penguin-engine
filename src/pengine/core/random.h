#pragma once
#ifndef PENGUIN_RANDOM_H
#define PENGUIN_RANDOM_H

#include <stdlib.h>     /* srand, rand */
#include <glm/glm.hpp>
#include "constants.h"
#include "time.h"

namespace PenguinEngine {
	class Random {
	public:
		static void initialize();

		/// <summary>
		/// Returns random float value from 0.0f - 1.0f (both inclusive)
		/// </summary>
		/// <param name="precision">sets precision of decimal values by number of zeroes. Defaults to '4' decimal places</param>
		/// <returns></returns>
		static float getRandomValue(int precision) {
			int precisionValue = (int)std::pow(10, precision);
			return (rand() % (precisionValue + 1)) / (float)precisionValue;
		}

		/// <summary>
		/// Returns random float value from 0.0f - 1.0f (both inclusive) with 6 decimal point accuracy. For other decimal accuracy use 'getRandomValue(int precision)' instead.
		/// </summary>
		/// <returns></returns>
		static float inline getRandomValue() {
			return (rand() % (1000001)) / 1000000.0f;
		}

		/// <summary>
		/// Returns random float value from min(inclusive) to max(inclusive)
		/// </summary>
		/// <param name="min"></param>
		/// <param name="max"></param>
		/// <returns></returns>
		static float getRandomFromRange(float min, float max) {
			return (getRandomValue() * (max - min)) + min;
		}

		/// <summary>
		/// Returns random int value from min(int inclusive) to max(int exclusive)
		/// </summary>
		/// <param name="min"></param>
		/// <param name="max"></param>
		/// <returns></returns>
		static int getRandomIntFromRange(int min, int max) {
			return (rand() % (max - min)) + min;
		}

		static glm::vec2 getRandomInUnitCircle() {
			return glm::vec2(getRandomValue(), getRandomValue());
		}

		static glm::vec3 getRandomInUnitSphere() {
			return glm::vec3(getRandomValue(), getRandomValue(), getRandomValue());
		}

		static float getRandomAngleRadians() {
			return getRandomValue() * (float)PenguinEngine::PI * 2.0f;
		}

		Random() = delete;
	};
}
#endif