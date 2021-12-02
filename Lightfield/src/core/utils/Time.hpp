#pragma once

#include <chrono>

class Time
{

private:
	Time() : appStart(std::chrono::high_resolution_clock::now()), lastFrame(appStart) {}
	~Time() = default;
	ROF_DELETE(Time);

public:
	static inline Time& Get()
	{
		static Time instance;
		return instance;
	}
	inline void Mark()
	{
		const auto oldFrame = lastFrame;
		lastFrame = std::chrono::steady_clock::now();

		deltaTime = std::chrono::duration<float>(lastFrame - oldFrame).count();
		totalTime = std::chrono::duration<float>(lastFrame - appStart).count();
	}

public:
	float deltaTime;
	float totalTime;

private:
	const std::chrono::high_resolution_clock::time_point appStart;
	std::chrono::high_resolution_clock::time_point lastFrame;
};