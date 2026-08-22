#include "pch.h"
#include "version.h"

extern "C"
char const* const XHK_version = "Xaml Hosting Kit version: " XHK_VERSION;

#if defined(_MSC_VER)
#ifdef _M_IX86
#pragma comment(linker, "/include:_XHK_version")
#pragma comment(linker, "/alternatename:XHK_version=_XHK_version")
#else
#pragma comment(linker, "/include:XHK_version")
#endif
#endif

extern "C"
char const* const
WINAPI XHK_GetVersion()
{
    #pragma comment(linker, "/alternatename:XHK_GetVersion=" __FUNCDNAME__)

    return XHK_VERSION;
}

extern "C"
char const* const
WINAPI XHK_GetVersionString()
{
    #pragma comment(linker, "/alternatename:XHK_GetVersionString=" __FUNCDNAME__)

    return XHK_version;
}