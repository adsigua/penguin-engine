#ifndef PENGUIN_TIME
#define PENGUIN_TIME

#include <chrono>

namespace PenguinEngine {
	class Time {

	public:
		static void Init();

		static void Tick();

		static float getTime();

		static float getTimeInSec();

		static float getDeltaTime();

		Time() = delete;

	private:
		static std::chrono::steady_clock::time_point _startTime;
		static float _time;
		static float _prevTime;
		static float _deltaTime;
	};




}

#endif