#include "time.h"

namespace PenguinEngine {
	std::chrono::steady_clock::time_point Time::_startTime = std::chrono::high_resolution_clock::now();;
	float Time::_time = 0;
	float Time::_prevTime = 0;
	float Time::_deltaTime = 0;

	void Time::initialize() {
		_startTime = std::chrono::high_resolution_clock::now();
		_time = std::chrono::duration<float, std::chrono::milliseconds::period>(_startTime - _startTime).count();
	}

	void Time::tick() {
		_prevTime = _time;
		auto currentTime = std::chrono::high_resolution_clock::now();
		_time = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - _startTime).count();
		_deltaTime = _time - _prevTime;
	}

	unsigned Time::getTimeStartSeed() {
		return _startTime.time_since_epoch().count();
	}

	float Time::getTime() {
		return _time;
	}

	float Time::getTimeInSec() {
		return _time / 1000;
	}

	float Time::getDeltaTime() {
		return _deltaTime / 1000;
	}


	/* periodic update timer
		static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - startTime).count();
        if ((int)glm::floor(time) % 2000 == 0) {
			//update every two seconds
        }
	*/
}