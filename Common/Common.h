#pragma once

#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32)
#pragma warning (disable : 4100)
#include <Windows.h>
#endif

#include "Macros.h"

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <optional>

void logTSMessage(char const* format, ...);
