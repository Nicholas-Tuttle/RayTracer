#pragma once
#include <chrono>

namespace RayTracer
{
	class ElapsedTimer
	{
	public:
		ElapsedTimer() : last_poll_time(std::chrono::high_resolution_clock::now()) {}

		std::chrono::duration<double, std::milli> Poll()
		{
			auto now = std::chrono::high_resolution_clock::now();
			auto elapsed_time = now - last_poll_time;
			last_poll_time = now;
			return elapsed_time;
		}

	private:
		std::chrono::steady_clock::time_point last_poll_time;
	};
}

