#include "Time.h"

namespace PenguinEngine {
	std::chrono::steady_clock::time_point Time::_startTime = std::chrono::high_resolution_clock::now();;
	float Time::_time = 0;
	float Time::_prevTime = 0;
	float Time::_deltaTime = 0;

	void Time::Init() {
		_startTime = std::chrono::high_resolution_clock::now();
		_time = std::chrono::duration<float, std::chrono::seconds::period>(_startTime - _startTime).count();
	}

	void Time::Tick() {
		_prevTime = _time;
		auto currentTime = std::chrono::high_resolution_clock::now();
		_time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _startTime).count();
		_deltaTime = _time - _prevTime;
	}

	float Time::getTime() {
		return _time;
	}

	float Time::getDeltaTime() {
		return _deltaTime;
	}
}