#pragma once

#ifndef XHK_VERSION_H
#define XHK_VERSION_H

#define XHK_VERSION "0.2.0"

extern "C" char const* const XHK_version;
extern "C" char const* const __stdcall XHK_GetVersion();
extern "C" char const* const __stdcall XHK_GetVersionString();

#endif // XHK_VERSION_H