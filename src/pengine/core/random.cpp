#include "random.h"
#include "constants.h"
#include "time.h"

namespace penguin_engine {
	void Random::initialize() {
		std::srand(Time::getTimeStartSeed());
	}
	
	float Random::getRandomValue(int precision) {
		int precisionValue = (int)std::pow(10, precision);
		return (rand() % (precisionValue + 1)) / (float)precisionValue;
	}
	/// <summary>
	/// Returns random float value from min(inclusive) to max(inclusive)
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	float Random::getRandomFromRange(float min, float max) {
		return (getRandomValue() * (max - min)) + min;
	}

	/// <summary>
	/// Returns random int value from min(int inclusive) to max(int exclusive)
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	int Random::getRandomIntFromRange(int min, int max) {
		return (rand() % (max - min)) + min;
	}

	glm::vec2 Random::getRandomInUnitCircle() {
		return glm::vec2(getRandomValue(), getRandomValue());
	}

	glm::vec3 Random::getRandomInUnitSphere() {
		return glm::vec3(getRandomValue(), getRandomValue(), getRandomValue());
	}

	float Random::getRandomAngleRadians() {
		return getRandomValue() * (float)constants::PI * 2.0f;
	}
}