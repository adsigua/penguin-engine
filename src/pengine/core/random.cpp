#include "random.h"

namespace PenguinEngine {
	void Random::initialize() {
		std::srand(PenguinEngine::Time::getTimeStartSeed());
	}
}