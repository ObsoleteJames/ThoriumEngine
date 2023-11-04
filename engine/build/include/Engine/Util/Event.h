#pragma once

#include "Util/Core.h"

#include <functional>
#include <algorithm>
#include <map>
#include <type_traits>

template<typename... Args>
class TEvent
{
public:
	TEvent() = default;
	~TEvent() = default;

	size_t Bind(std::function<void(Args...)> const& func)
	{
		_outputs.insert(std::pair(_lastIndex++, func));
		return _lastIndex;
	}

	//template<typename T>
	//size_t Bind(T* obj, void(T::*func)(Args...))
	//{
	//	return Bind([=](Args... args) {
	//		(obj->*func)(args...);
	//	});
	//}

	//template<typename T>
	//size_t Bind(T* obj, void(T::*func)(Args...) const)
	//{
	//	return Bind([=](Args... args) {
	//		(obj->*func)(args...);
	//	});
	//}

	template<typename T, typename TFunc>
	size_t Bind(T* obj, TFunc func)
	{
		return Bind((size_t)obj, [=](Args... args) {
			if constexpr (std::is_member_function_pointer<TFunc>::value)
				(obj->*func)(args...);
			else
				func(args...);
		});
	}

	void Fire(Args... arg)
	{
		for (auto& func : _outputs)
		{
			func.second(arg...);
		}
	}

	void FireAtIndex(size_t index, Args... arg)
	{
		auto const& func = _outputs.find(index);
		if (func != _outputs.end())
			func->second(arg...);
	}

	bool Remove(size_t index)
	{
		auto p = _outputs.find(index - 1);
		if (p != _outputs.end())
		{
			_outputs.erase(p);
			return true;
		}
		return false;
	}

	template<typename T>
	void Remove(T* obj)
	{
		auto p = _outputs.find((size_t)obj);
		while (p != _outputs.end())
		{
			_outputs.erase(p);
			p = _outputs.find((size_t)obj);
		}
	}

	void RemoveAll()
	{
		_outputs.clear();
	}

private:
	size_t Bind(size_t index, std::function<void(Args...)> const& func)
	{
		_outputs.insert(std::make_pair(index, func));
		return index;
	}

private:
	std::multimap<size_t, std::function<void(Args...)>> _outputs;
	size_t _lastIndex;

};
