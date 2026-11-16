#pragma once

#ifndef XHK_VERSION_H
#define XHK_VERSION_H

#define XHK_VERSION "0.2.0"

#ifdef __cplusplus
extern "C"
{
#endif

extern char const* const XHK_version;
char const* const __stdcall XHK_GetVersion(void);
char const* const __stdcall XHK_GetVersionString(void);

#ifdef __cplusplus
}
#endif

#endif // XHK_VERSION_H