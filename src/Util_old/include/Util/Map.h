#pragma once

#include <map>
#include <unordered_map>

template<typename T1, typename T2>
using TMap = std::map<T1, T2>;

template<typename T1, typename T2>
using TUnorderedMap = std::unordered_map<T1, T2>;
