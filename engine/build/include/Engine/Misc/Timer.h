#pragma once

#include "EngineCore.h"
#include <chrono>

/**
 * std timer abstraction
 */
class ENGINE_API FTimer
{
public:
	FTimer();

	void Begin();
	void Stop();

	size_t GetMicroSeconds();
	double GetMiliseconds();
	double GetSeconds();

private:
	std::chrono::time_point<std::chrono::steady_clock> _start;
	std::chrono::time_point<std::chrono::steady_clock> _end;
};
