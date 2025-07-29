#pragma once

#include <stdlib.h>     /* srand, rand */
#include <glm/glm.hpp>

namespace penguin_engine {
	class Random {
	public:
		static void initialize();

		/// <summary>
		/// Returns random float value from 0.0f - 1.0f (both inclusive)
		/// </summary>
		/// <param name="precision">sets precision of decimal values by number of zeroes. Defaults to '4' decimal places</param>
		/// <returns></returns>
		static float getRandomValue(int precision);

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
		static float getRandomFromRange(float min, float max);

		/// <summary>
		/// Returns random int value from min(int inclusive) to max(int exclusive)
		/// </summary>
		/// <param name="min"></param>
		/// <param name="max"></param>
		/// <returns></returns>
		static int getRandomIntFromRange(int min, int max);

		static glm::vec2 getRandomInUnitCircle();

		static glm::vec3 getRandomInUnitSphere();

		static float getRandomAngleRadians();

		Random() = delete;
	};
}
