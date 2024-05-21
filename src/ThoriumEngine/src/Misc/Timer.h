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
#if __GNUC__
	std::chrono::system_clock::time_point _start;
	std::chrono::system_clock::time_point _end;
#else
	std::chrono::time_point<std::chrono::steady_clock> _start;
	std::chrono::time_point<std::chrono::steady_clock> _end;
#endif
};
