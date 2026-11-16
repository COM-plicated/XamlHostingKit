#include "pch.h"
#include "winrt/base.h"
#include <roapi.h>
#include <detours/detours.h>
#include <wil/result.h>

bool __stdcall XHK_can_unload_now() noexcept;
void* __stdcall XHK_get_activation_factory([[maybe_unused]] std::wstring_view const& name);

extern "C" int32_t __stdcall XHK_CanUnloadNow() noexcept
{
    #pragma comment(linker, "/alternatename:XHK_CanUnloadNow=" __FUNCDNAME__)

    return XHK_can_unload_now() ? 0 : 1;
}

extern "C" int32_t __stdcall XHK_GetActivationFactory(void* classId, void** factory) noexcept try
{
    #pragma comment(linker, "/alternatename:XHK_GetActivationFactory=" __FUNCDNAME__)

    std::wstring_view const name { *reinterpret_cast<winrt::hstring*>(&classId) };
    *factory = XHK_get_activation_factory(name);

    if (*factory)
    {
        return 0;
    }

    return winrt::hresult_class_not_available(name).to_abi();
}
catch (...) { return winrt::to_hresult(); }

namespace xhk::detail
{
    // Copied from C++/WinRT
    static winrt::hresult cppwinrt_get_runtime_activation_factory_impl(winrt::param::hstring const& name, winrt::guid const& guid, void** result) noexcept
    {
        winrt::hresult hr = WINRT_IMPL_RoGetActivationFactory(*(void**)(&name), guid, result);

        if (hr == winrt::impl::error_not_initialized)
        {
            auto usage = reinterpret_cast<std::int32_t(__stdcall*)(void** cookie) noexcept>(WINRT_IMPL_GetProcAddress(winrt::impl::load_library(L"combase.dll"), "CoIncrementMTAUsage"));

            if (!usage)
            {
                return hr;
            }

            void* cookie;
            usage(&cookie);
            hr = WINRT_IMPL_RoGetActivationFactory(*(void**)(&name), guid, result);
        }

        if (hr == 0)
        {
            return 0;
        }

        winrt::com_ptr<IErrorInfo> error_info;
        WINRT_IMPL_GetErrorInfo(0, error_info.put_void());

        std::wstring path { static_cast<winrt::hstring const&>(name) };
        std::size_t count { };

        while (std::wstring::npos != (count = path.rfind('.')))
        {
            path.resize(count);
            path += L".dll";
            winrt::impl::library_handle library(winrt::impl::load_library(path.c_str()));
            path.resize(path.size() - 4);

            if (!library)
            {
                continue;
            }

            auto library_call = reinterpret_cast<std::int32_t(__stdcall*)(void* classId, void** factory)>(WINRT_IMPL_GetProcAddress(library.get(), "DllGetActivationFactory"));

            if (!library_call)
            {
                continue;
            }

            winrt::com_ptr<winrt::impl::abi_t<winrt::Windows::Foundation::IActivationFactory>> library_factory;

            if (0 != library_call(*(void**)(&name), library_factory.put_void()))
            {
                continue;
            }

            if (0 == library_factory.as(guid, result))
            {
                library.detach();
                return 0;
            }
        }

        WINRT_IMPL_SetErrorInfo(0, error_info.get());
        return hr;
    }

    extern "C" int32_t __stdcall xhk_activation_handler(void* classId, winrt::guid const& iid, void** factory, decltype(winrt_activation_handler) originalHandler) noexcept
    {
        if (originalHandler && originalHandler(classId, iid, factory) == 0)
        {
            return 0;
        }

        int32_t result = static_cast<int32_t>(0x80040111);
        auto id = reinterpret_cast<winrt::hstring*>(&classId);
        if ((result = cppwinrt_get_runtime_activation_factory_impl(*id, iid, factory)) == 0)
        {
            return 0;
        }

        winrt::com_ptr<::IUnknown> factoryUnk = nullptr;
        if (XHK_GetActivationFactory(classId, factoryUnk.put_void()) == 0)
        {
            return factoryUnk->QueryInterface(iid, factory);
        }

        return result;
    }

    static volatile bool s_hooksInstalled = false;
    static decltype(&RoGetActivationFactory) OriginalRoGetActivationFactory = nullptr;
    static decltype(&RoActivateInstance) OriginalRoActivateInstance = nullptr;

    static HRESULT WINAPI RoGetActivationFactoryHook(_In_ HSTRING activatableClassId, _In_ REFIID iid, _COM_Outptr_ void** factory)
    {
        HRESULT hr;
        if (SUCCEEDED(hr = OriginalRoGetActivationFactory(activatableClassId, iid, factory))) [[likely]]
        {
            return hr;
        }

        if (!activatableClassId || !factory) [[unlikely]]
        {
            return E_POINTER;
        }

        winrt::com_ptr<::IUnknown> factoryUnk = nullptr;
        RETURN_IF_FAILED((HRESULT)XHK_GetActivationFactory(activatableClassId, factoryUnk.put_void()));
        return factoryUnk->QueryInterface(iid, factory);
    }

    static HRESULT WINAPI RoActivateInstanceHook(_In_ HSTRING activatableClassId, _COM_Outptr_ IInspectable** instance)
    {
        HRESULT hr;
        if (SUCCEEDED(hr = OriginalRoActivateInstance(activatableClassId, instance))) [[likely]]
        {
            return hr;
        }

        if (!activatableClassId || !instance) [[unlikely]]
        {
            return E_POINTER;
        }

        winrt::com_ptr<::IActivationFactory> factory = nullptr;
        RETURN_IF_FAILED((HRESULT)XHK_GetActivationFactory(activatableClassId, factory.put_void()));
        return factory->ActivateInstance(instance);
    }

    extern "C" void __cdecl xhk_install_activation_hooks()
    {
        if (s_hooksInstalled) [[likely]]
        {
            return;
        }

        s_hooksInstalled = true;
        OriginalRoActivateInstance     = RoActivateInstance;
        OriginalRoGetActivationFactory = RoGetActivationFactory;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)OriginalRoGetActivationFactory, RoGetActivationFactoryHook);
        DetourAttach(&(PVOID&)OriginalRoActivateInstance, RoActivateInstanceHook);
        LOG_IF_WIN32_ERROR_MSG(DetourTransactionCommit(), "[FATAL] Failed to install activation hooks.");
    }
}