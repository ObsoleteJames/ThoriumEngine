#pragma once

#include "Util/Types.h"
#include "Util/Array.h"
#include "Util/String.h"

#include <atomic>

typedef uint8 byte;

template<typename T>
using TAtomic = std::atomic<T>;

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

#define UTIL_DEPRECATED(...) [[deprecated(__VA_ARGS__)]]
