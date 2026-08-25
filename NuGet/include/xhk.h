#pragma once

#ifndef XHK_H
#define XHK_H

#include <mutex>
#include <version>
#include <Unknwn.h>
#include <winrt/Windows.Foundation.h>
#include "xhk_version.h"

#if defined(__cpp_lib_atomic_ref) && __cpp_lib_atomic_ref >= 201806L
#include <atomic>
#define XHK_ATOMIC_REF_EXISTS
#endif

extern "C" int32_t __stdcall XHK_CanUnloadNow() noexcept;
extern "C" int32_t __stdcall XHK_GetActivationFactory(void* classId, void** factory) noexcept;

#ifndef XHK_NO_ACTIVATION_HANDLER_OVERRIDE

static int32_t __stdcall xhk_activation_handler(void* classId, winrt::guid const& iid, void** factory) noexcept;

inline static decltype(winrt_activation_handler) xhk_original_handler =
    (winrt_activation_handler != xhk_activation_handler) ?
        winrt_activation_handler :
        nullptr;

extern "C" __declspec(selectany) decltype(winrt_activation_handler) xhk_activation_handler_ptr =
    (xhk_original_handler != xhk_activation_handler) ? // this check is pointless but it ensures that xhk_original_handler is initialized before winrt_activation_handler is set
        (winrt_activation_handler = xhk_activation_handler) :
        nullptr;

#if defined(_MSC_VER)
#ifdef _M_IX86
#pragma comment(linker, "/include:_xhk_activation_handler_ptr")
#else
#pragma comment(linker, "/include:xhk_activation_handler_ptr")
#endif
#endif

static inline std::mutex xhk_activation_handler_mutex;
static inline int32_t __stdcall xhk_activation_handler(void* classId, winrt::guid const& iid, void** factory) noexcept
{
    if (xhk_original_handler && xhk_original_handler(classId, iid, factory) == 0)
    {
        return 0;
    }

    auto id = reinterpret_cast<winrt::hstring*>(&classId);

    {
        std::lock_guard lock(xhk_activation_handler_mutex);

    #ifdef XHK_ATOMIC_REF_EXISTS
        std::atomic_ref<decltype(winrt_activation_handler)> handler_ref(winrt_activation_handler);
        handler_ref.store(nullptr, std::memory_order_release);
    #else
        winrt_activation_handler = nullptr;
    #endif

        auto result = winrt::impl::get_runtime_activation_factory_impl<false>(*id, iid, factory);

    #ifdef XHK_ATOMIC_REF_EXISTS
        handler_ref.store(xhk_activation_handler, std::memory_order_release);
    #else
        winrt_activation_handler = xhk_activation_handler;
    #endif

        if (result == 0)
        {
            return reinterpret_cast<::IUnknown*>(*factory)->QueryInterface(iid, factory);
        }
    }

    if (XHK_GetActivationFactory(classId, factory) == 0)
    {
        return reinterpret_cast<::IUnknown*>(*factory)->QueryInterface(iid, factory);
    }

    return winrt::hresult_class_not_available(*id).to_abi();
}

#endif // XHK_NO_ACTIVATION_HANDLER_OVERRIDE
#endif // XHK_H