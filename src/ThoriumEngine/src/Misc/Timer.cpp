
#include "Timer.h"

FTimer::FTimer()
{
	Begin();
}

void FTimer::Begin()
{
	_start = std::chrono::high_resolution_clock::now();
}

void FTimer::Stop()
{
	_end = std::chrono::high_resolution_clock::now();
}

size_t FTimer::GetMicroSeconds()
{
	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::microseconds;

	auto diff = duration_cast<microseconds>(_end - _start);

	return diff.count();
}

double FTimer::GetMiliseconds()
{
	using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::microseconds;

	auto diff = duration_cast<microseconds>(_end - _start);

	double r = ((double)diff.count() / 1000.0);
	return r;
}

double FTimer::GetSeconds()
{
	return GetMiliseconds() / 1000.0;
}
