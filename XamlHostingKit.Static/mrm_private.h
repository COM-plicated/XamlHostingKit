// Sources:
// - https://github.com/GPUOpen-Tools/common_src_amdtoswrappers/blob/master/src/win32/osApplication.cpp
// - https://github.com/diversenok/NtUtilsLibrary/blob/master/Headers/Ntapi.appmodel.mrm.pas

#pragma once

#include <Windows.h>
#include <Unknwn.h>

namespace ABI::Windows::ApplicationModel::Resources::Core
{
    class DECLSPEC_UUID("DBCE7E40-7345-439D-B12C-114A11819A09") MrtResourceManager;

    MIDL_INTERFACE("130A2F65-2BE7-4309-9A58-A9052FF2B61C")
    IMrtResourceManager : public ::IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize() PURE;
        virtual HRESULT STDMETHODCALLTYPE InitializeForCurrentApplication() PURE;
        virtual HRESULT STDMETHODCALLTYPE InitializeForPackage(LPCWSTR) PURE;
        virtual HRESULT STDMETHODCALLTYPE InitializeForFile(LPCWSTR filePath) PURE;
        virtual HRESULT STDMETHODCALLTYPE GetMainResourceMap(const GUID&, void**) PURE;
        virtual HRESULT STDMETHODCALLTYPE GetResourceMap(LPCWSTR, const GUID&, void**) PURE;
        virtual HRESULT STDMETHODCALLTYPE GetDefaultContext(const GUID&, void**) PURE;
        virtual HRESULT STDMETHODCALLTYPE GetReference(const GUID&, void**) PURE;
        virtual HRESULT STDMETHODCALLTYPE IsResourceReference(LPCWSTR, BOOL*) PURE;
    };

    MIDL_INTERFACE("439DD7C9-0EEB-4715-BAA7-F0877694E616")
    IMrtResourceManager2 : public ::IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitializeForPackageFile(LPCWSTR, LPCWSTR) PURE;
        virtual HRESULT STDMETHODCALLTYPE TryInitializeForCurrentApplication() PURE;
        virtual HRESULT STDMETHODCALLTYPE InitializeForInboxApplication(LPCWSTR, LPCWSTR) PURE;
    };
}