#include "random.h"

namespace penguin_engine {
	void Random::initialize() {
		std::srand(Time::getTimeStartSeed());
	}
}