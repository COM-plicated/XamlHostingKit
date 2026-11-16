#pragma once

#ifndef XHK_H
#define XHK_H

#include "xhk_version.h"

#ifndef __cplusplus
#define XHK_NO_CPPWINRT
#undef  XHK_WRL_HELPERS
#endif

#ifdef XHK_NO_CPPWINRT
#define XHK_NO_ACTIVATION_HANDLER
#define XHK_NO_ACTIVATION_HELPERS
#undef XHK_OVERRIDE_ACTIVATION_HANDLER
#endif

#ifdef XHK_USE_ACTIVATION_HOOKS
#undef XHK_NO_ACTIVATION_HOOKS
#define XHK_OVERRIDE_ACTIVATION_HANDLER
#endif

#if defined(XHK_NO_ACTIVATION_HANDLER) && !defined(XHK_USE_ACTIVATION_HOOKS)
#undef XHK_OVERRIDE_ACTIVATION_HANDLER
#endif

#undef XHK_ACTIVATION_OVERRIDE_USES_HOOKS
#undef XHK_ACTIVATION_OVERRIDE_USES_CPPWINRT_HANDLER

#ifdef XHK_OVERRIDE_ACTIVATION_HANDLER

#ifdef XHK_USE_ACTIVATION_HOOKS
#define XHK_ACTIVATION_OVERRIDE_USES_HOOKS 1
#else
#define XHK_ACTIVATION_OVERRIDE_USES_CPPWINRT_HANDLER 1
#endif

#endif // XHK_OVERRIDE_ACTIVATION_HANDLER

#ifndef XHK_NAMESPACE
#define XHK_NAMESPACE xhk
#endif

#ifdef XHK_WRL_HELPERS
#include <Windows.h>
#include <activation.h>
#include <wrl.h>
#endif // XHK_WRL_HELPERS

#if !defined(XHK_NO_CPPWINRT) && \
    !(defined(XHK_NO_ACTIVATION_HANDLER) && \
      defined(XHK_NO_ACTIVATION_HELPERS))

#include <Unknwn.h>
#include <winrt/Windows.Foundation.h>

#endif

#if !defined(XHK_NO_ACTIVATION_HANDLER) && defined(XHK_USE_ACTUAL_CPPWINRT_FACTORY_IMPL)

#if defined(__has_include)

#if __has_include(<version>)
#include <version>
#endif

#endif // __has_include

#if defined(__cpp_lib_atomic_ref) && __cpp_lib_atomic_ref >= 201806L
#include <atomic>
#define XHK_ATOMIC_REF_EXISTS
#endif

#endif // !defined(XHK_NO_ACTIVATION_HANDLER) && defined(XHK_USE_ACTUAL_CPPWINRT_FACTORY_IMPL)

#ifdef __cplusplus
extern "C"
{
#endif

int __stdcall XHK_CanUnloadNow(void);
int __stdcall XHK_GetActivationFactory(void* classId, void** factory);

#ifdef __cplusplus
}
#endif

#ifndef XHK_NO_ACTIVATION_HELPERS

namespace XHK_NAMESPACE
{
    template <typename Class, typename Interface = winrt::Windows::Foundation::IActivationFactory>
    inline auto get_activation_factory()
    {
        const winrt::hstring classId = winrt::hstring { winrt::name_of<Class>() };
        winrt::Windows::Foundation::IActivationFactory factory { nullptr };
        winrt::check_hresult(XHK_GetActivationFactory(winrt::get_abi(classId), reinterpret_cast<void**>(winrt::put_abi(factory))));

        if constexpr (std::is_same_v<Interface, winrt::Windows::Foundation::IActivationFactory>)
        {
            return factory;
        }
        else
        {
            return factory.as<Interface>();
        }
    }

    template <typename Class, typename Interface = winrt::Windows::Foundation::IActivationFactory>
    inline Interface try_get_activation_factory() noexcept
    {
        const winrt::hstring classId = winrt::hstring { winrt::name_of<Class>() };
        winrt::Windows::Foundation::IActivationFactory factory { nullptr };

        if (SUCCEEDED(XHK_GetActivationFactory(winrt::get_abi(classId), reinterpret_cast<void**>(winrt::put_abi(factory)))))
        {
            if constexpr (std::is_same_v<Interface, winrt::Windows::Foundation::IActivationFactory>)
            {
                return factory;
            }
            else
            {
                return factory.try_as<Interface>();
            }
        }
        
        return nullptr;
    }

    template <typename Class>
    inline auto activate_instance()
    {
        auto factory = get_activation_factory<Class>();
        return factory.template ActivateInstance<Class>();
    }

    template <typename Class>
    inline Class try_activate_instance() noexcept
    {
        try
        {
            return activate_instance<Class>();
        }
        catch (...)
        {
            return nullptr;
        }
    }

    template <typename Class>
    inline auto make()
    {
        return activate_instance<Class>();
    }
}

#endif // XHK_NO_ACTIVATION_HELPERS

#ifndef XHK_NO_ACTIVATION_HANDLER

inline int32_t __stdcall xhk_activation_handler(void* classId, winrt::guid const& iid, void** factory) noexcept;

inline decltype(winrt_activation_handler) xhk_original_handler = nullptr;

extern "C" inline void __cdecl xhk_install_activation_handler() noexcept
{
    if (winrt_activation_handler != xhk_activation_handler)
    {
        xhk_original_handler = winrt_activation_handler;
        winrt_activation_handler = xhk_activation_handler;
    }
}

#ifndef XHK_USE_ACTUAL_CPPWINRT_FACTORY_IMPL

namespace XHK_NAMESPACE::detail
{
    extern "C" int32_t __stdcall xhk_activation_handler(void* classId, winrt::guid const& iid, void** factory, decltype(winrt_activation_handler) originalHandler) noexcept;
}

#else

inline winrt::slim_mutex xhk_activation_handler_lock;

#endif // XHK_USE_ACTUAL_CPPWINRT_FACTORY_IMPL

inline int32_t __stdcall xhk_activation_handler(void* classId, winrt::guid const& iid, void** factory) noexcept
{
#ifndef XHK_USE_ACTUAL_CPPWINRT_FACTORY_IMPL

    return ::XHK_NAMESPACE::detail::xhk_activation_handler(classId, iid, factory, xhk_original_handler);

#else

    if (xhk_original_handler && xhk_original_handler(classId, iid, factory) == 0)
    {
        return 0;
    }

    int32_t result = static_cast<int32_t>(0x80040111);
    auto id = reinterpret_cast<winrt::hstring*>(&classId);

    {
        winrt::slim_lock_guard const lock { xhk_activation_handler_lock };

    #ifdef XHK_ATOMIC_REF_EXISTS
        std::atomic_ref<decltype(winrt_activation_handler)> handler_ref(winrt_activation_handler);
        handler_ref.store(nullptr, std::memory_order_release);
    #else
        winrt_activation_handler = nullptr;
    #endif

        result = winrt::impl::get_runtime_activation_factory_impl<false>(*id, iid, factory);

    #ifdef XHK_ATOMIC_REF_EXISTS
        handler_ref.store(xhk_activation_handler, std::memory_order_release);
    #else
        winrt_activation_handler = xhk_activation_handler;
    #endif

        if (result == 0)
        {
            return 0;
        }
    }

    winrt::com_ptr<::IUnknown> factoryUnk = nullptr;
    if (XHK_GetActivationFactory(classId, factoryUnk.put_void()) == 0)
    {
        return factoryUnk->QueryInterface(iid, factory);
    }

    return result;

#endif // XHK_USE_ACTUAL_CPPWINRT_FACTORY_IMPL
}

#endif // XHK_NO_ACTIVATION_HANDLER

#ifndef XHK_NO_ACTIVATION_HOOKS

#ifdef __cplusplus
extern "C"
{
#endif

    void __cdecl xhk_install_activation_hooks(void);

#ifdef __cplusplus
}
#endif

#endif // XHK_NO_ACTIVATION_HOOKS

#ifdef XHK_USE_ACTIVATION_HOOKS

#undef __xhk_activation_installation_function
#define __xhk_activation_installation_function xhk_install_activation_hooks

#else

#undef __xhk_activation_installation_function
#define __xhk_activation_installation_function xhk_install_activation_handler

#endif // XHK_USE_ACTIVATION_HOOKS

#ifdef XHK_OVERRIDE_ACTIVATION_HANDLER

#if defined(_MSC_VER)

#pragma section(".CRT$XCT", read)

#ifdef __cplusplus
extern "C"
{
#endif

__declspec(allocate(".CRT$XCT")) __declspec(selectany)
void(__cdecl* xhk_handler_init)(void) = __xhk_activation_installation_function;

#ifdef __cplusplus
}
#endif

#ifdef _M_IX86
#pragma comment(linker, "/include:_xhk_handler_init")
#else
#pragma comment(linker, "/include:xhk_handler_init")
#endif

#elif defined(__clang__) || defined(__GNUC__)
__attribute__((constructor(101))) static void xhk_handler_init(void) { __xhk_activation_installation_function(); }
#endif

#endif // XHK_OVERRIDE_ACTIVATION_HANDLER
#undef __xhk_activation_installation_function

#ifdef XHK_WRL_HELPERS

namespace XHK_NAMESPACE
{
    template<typename T>
    inline HRESULT GetActivationFactory(_In_ HSTRING activatableClassId, _Out_ T** factory)
    {
        if (!activatableClassId || !factory) { return E_POINTER; }
        *factory = nullptr;

        HRESULT hr;
        ::Microsoft::WRL::ComPtr<::IActivationFactory> actFactory;
        if (SUCCEEDED(hr = XHK_GetActivationFactory(activatableClassId, reinterpret_cast<void**>(actFactory.GetAddressOf()))))
        {
            hr = actFactory.CopyTo(factory);
        }

        return hr;
    }

    template<typename T>
    inline HRESULT GetActivationFactory(_In_ HSTRING activatableClassId, _Out_ ::Microsoft::WRL::Details::ComPtrRef<T> factory)
    {
        return ::XHK_NAMESPACE::GetActivationFactory(activatableClassId, factory.ReleaseAndGetAddressOf());
    }

    template<typename T>
    inline HRESULT ActivateInstance(_In_ HSTRING activatableClassId, _Out_ T** instance)
    {
        if (!activatableClassId || !instance) { return E_POINTER; }
        *instance = nullptr;

        HRESULT hr;
        ::Microsoft::WRL::ComPtr<::IActivationFactory> factory;
        if (SUCCEEDED(hr = ::XHK_NAMESPACE::GetActivationFactory(activatableClassId, &factory)))
        {
            ::Microsoft::WRL::ComPtr<::IInspectable> inst;
            if (SUCCEEDED(hr = factory->ActivateInstance(inst.GetAddressOf())))
            {
                hr = inst->QueryInterface(instance);
            }
        }

        return hr;
    }

    template<typename T>
    inline HRESULT ActivateInstance(_In_ HSTRING activatableClassId, _Out_ ::Microsoft::WRL::Details::ComPtrRef<T> instance)
    {
        return ::XHK_NAMESPACE::ActivateInstance(activatableClassId, instance.ReleaseAndGetAddressOf());
    }
}

#endif // XHK_WRL_HELPERS
#endif // XHK_H