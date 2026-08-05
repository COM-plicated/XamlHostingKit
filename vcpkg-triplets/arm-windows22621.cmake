set(VCPKG_TARGET_ARCHITECTURE arm)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_CMAKE_SYSTEM_VERSION "10.0.22621.0" CACHE STRING "" FORCE)
set(CMAKE_SYSTEM_VERSION "10.0.22621.0" CACHE STRING "" FORCE)
set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.22621.0" CACHE STRING "" FORCE)

if(DEFINED ENV{WindowsSdkDir} AND EXISTS "$ENV{WindowsSdkDir}")
  set(_win_sdk_kits_root "$ENV{WindowsSdkDir}")
else()
  get_filename_component(_win_sdk_kits_root
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]"
    ABSOLUTE)
  if(NOT _win_sdk_kits_root OR NOT EXISTS "${_win_sdk_kits_root}")
    message(FATAL_ERROR "Could not locate Windows SDK root.")
  endif()
  file(TO_NATIVE_PATH "${_win_sdk_kits_root}" _win_sdk_kits_root)
  
endif()

set(ENV{WindowsSdkDir}        "${_win_sdk_kits_root}")
set(ENV{WindowsSDKVersion}    "${VCPKG_CMAKE_SYSTEM_VERSION}\\")
set(ENV{WindowsSDKLibVersion} "${VCPKG_CMAKE_SYSTEM_VERSION}\\")
set(ENV{WindowsSdkVerBinPath} "${_win_sdk_kits_root}bin\\${VCPKG_CMAKE_SYSTEM_VERSION}\\")
set(ENV{WindowsLibPath}       "${_win_sdk_kits_root}UnionMetadata\\${VCPKG_CMAKE_SYSTEM_VERSION}\\")

set(_orig_include "$ENV{INCLUDE}")
set(_orig_lib     "$ENV{LIB}")

set(ENV{INCLUDE}
  "${_win_sdk_kits_root}Include\\${VCPKG_CMAKE_SYSTEM_VERSION}\\ucrt;${_win_sdk_kits_root}Include\\${VCPKG_CMAKE_SYSTEM_VERSION}\\shared;${_win_sdk_kits_root}Include\\${VCPKG_CMAKE_SYSTEM_VERSION}\\um;${_win_sdk_kits_root}Include\\${VCPKG_CMAKE_SYSTEM_VERSION}\\winrt;${_win_sdk_kits_root}Include\\${VCPKG_CMAKE_SYSTEM_VERSION}\\cppwinrt;${_orig_include}")

set(ENV{LIB}
  "${_win_sdk_kits_root}Lib\\${VCPKG_CMAKE_SYSTEM_VERSION}\\ucrt\\${VCPKG_TARGET_ARCHITECTURE};${_win_sdk_kits_root}Lib\\${VCPKG_CMAKE_SYSTEM_VERSION}\\um\\${VCPKG_TARGET_ARCHITECTURE};${_orig_lib}")
  
list(APPEND VCPKG_KEEP_ENV_VARS "WindowsSDKVersion" "WindowsSDKLibVersion" "WindowsSdkVerBinPath" "WindowsSdkVerBinPath" "WindowsLibPath")
set(VCPKG_ENV_PASSTHROUGH "LIB;INCLUDE;PATH;CPATH;VARSDIR;WindowsSDKVersion;WindowsSDKLibVersion;WindowsSdkVerBinPath;WindowsSdkVerBinPath;WindowsLibPath")
set(VCPKG_LOAD_VCVARS_ENV OFF CACHE BOOL "" FORCE)