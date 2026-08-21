#pragma once

#define NOMINMAX

#include <Windows.h>
#include <Unknwn.h>
#include <inspectable.h>

#undef CreateWindow
#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>