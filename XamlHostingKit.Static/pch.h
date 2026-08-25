#pragma once

#define NOMINMAX
#define WINRT_CONTAINED_STATIC_LIB

#define winrt xhk_winrt
#define wil   xhk_wil

#include <Windows.h>
#include <Unknwn.h>
#include <inspectable.h>

#undef CreateWindow
#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>