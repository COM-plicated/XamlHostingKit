#include "pch.h"
#include "version.h"

extern "C"
char const* const XHK_version = "Xaml Hosting Kit version: " XHK_VERSION;

#if defined(_MSC_VER)
#ifdef _M_IX86
#pragma comment(linker, "/include:_XHK_version")
#else
#pragma comment(linker, "/include:XHK_version")
#endif
#endif

extern "C"
char const* const
WINAPI XHK_GetVersion()
{
    return XHK_VERSION;
}

extern "C"
char const* const
WINAPI XHK_GetVersionString()
{
    return XHK_version;
}