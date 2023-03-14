#pragma once

#include "../include/Util/Core.h"

#include <functional>
#include <algorithm>
#include <map>

template<typename... Args>
class TEvent
{
private:
	std::map<int, std::function<void(Args...)>> _outputs;
	int _lastIndex;

public:
	TEvent() = default;
	~TEvent() = default;

	int Bind(std::function<void(Args...)> const& func)
	{
		_outputs.insert(std::make_pair(_lastIndex++, func));
		return _lastIndex;
	}

	template<typename T>
	int Bind(T* obj, void(T::*func)(Args...))
	{
		return Bind([=](Args... args) {
			(obj->*func)(args...);
		});
	}

	template<typename T>
	int Bind(T* obj, void(T::*func)(Args...) const)
	{
		return Bind([=](Args... args) {
			(obj->*func)(args...);
		});
	}

	void Fire(Args... arg)
	{
		for (auto const& func : _outputs)
		{
			func.second(arg...);
		}
	}

	void FireAtIndex(int index, Args... arg)
	{
		auto const& func = _outputs.find(index);
		if (func != _outputs.end())
			func->second(arg...);
	}

	bool Remove(int index)
	{
		auto p = _outputs.find(index - 1);
		if (p != _outputs.end())
		{
			_outputs.erase(p);
			return true;
		}
		return false;
	}

	void RemoveAll()
	{
		_outputs.clear();
	}
};