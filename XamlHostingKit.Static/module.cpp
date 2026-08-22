#include "pch.h"
#include "winrt/base.h"

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