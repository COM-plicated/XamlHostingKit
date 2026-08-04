#pragma once

#include <filesystem>
#include "Privates.h"
#include <wil/registry.h>
#include <ShellScalingApi.h>

namespace winrt::XamlHostingKit
{
    static const auto UXThemeModule = LoadLibraryW(L"uxtheme.dll");
    static const auto User32Module = LoadLibraryW(L"user32.dll");
    static const auto IERTUtilModule = LoadLibraryW(L"iertutil.dll");
    static const auto WinUIModule = LoadLibraryW(L"Windows.UI.dll");
    static const auto COMBaseModule = GetModuleHandleW(L"combase.dll");

    static const auto IsDarkModeAllowedForWindow = reinterpret_cast<BOOL(WINAPI*)(HWND)>(GetProcAddress(UXThemeModule, MAKEINTRESOURCEA(137)));
    static const auto SetWindowCompositionAttribute = reinterpret_cast<BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*)>(GetProcAddress(User32Module, "SetWindowCompositionAttribute"));
    static const auto SetPreferredAppMode = reinterpret_cast<void(WINAPI*)(PreferredAppMode)>(GetProcAddress(UXThemeModule, MAKEINTRESOURCEA(135)));
    static const auto RefreshImmersiveColorPolicyState = reinterpret_cast<void(WINAPI*)()>(GetProcAddress(UXThemeModule, MAKEINTRESOURCEA(104)));
    static const auto AllowDarkModeForWindow = reinterpret_cast<void(WINAPI*)(HWND, BOOL)>(GetProcAddress(UXThemeModule, MAKEINTRESOURCEA(133)));
    static const auto IEConfiguration_SetBrowserAppProfile = reinterpret_cast<HRESULT(WINAPI*)(const wchar_t*, uint32_t, uint32_t)>(GetProcAddress(IERTUtilModule, MAKEINTRESOURCEA(797)));
    static const auto PrivateCreateCoreWindow = reinterpret_cast<HRESULT(WINAPI*)(CoreWindowType, const wchar_t*, int, int, int, int, uint32_t, HWND, REFGUID, void**)>(GetProcAddress(WinUIModule, MAKEINTRESOURCEA(1500)));
    static const auto CoSetASTATestMode = reinterpret_cast<void(WINAPI*)(ASTA_TEST_MODE_FLAGS)>(GetProcAddress(COMBaseModule, MAKEINTRESOURCEA(100)));
    static const auto GetDpiForWindowMethod = reinterpret_cast<decltype(&GetDpiForWindow)>(GetProcAddress(User32Module, "GetDpiForWindow"));

    static const auto PersonalizeKey = wil::reg::open_unique_key(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize");

    #define NtCurrentPeb() (NtCurrentTeb()->ProcessEnvironmentBlock)

    class Helpers
    {
    private:

        inline static intptr_t _switchContextOffset = 0;

        inline static winrt::hstring __GetExecutableName()
        {
            wchar_t path[MAX_PATH];
            GetModuleFileNameW(GetModuleHandleW(nullptr), path, MAX_PATH);

            std::filesystem::path exePath(path);
            return winrt::hstring { exePath.stem().wstring() };
        }

        inline static bool ShouldAppsUseDarkMode()
        {
            return !wil::reg::try_get_value_dword(PersonalizeKey.get(), L"AppsUseLightTheme").value_or(true);
        }

    public:

        inline static winrt::hstring const& GetExecutableName()
        {
            static winrt::hstring exeName = __GetExecutableName();
            return exeName;
        }

        inline static void EnsureTitleBarTheme(HWND hwnd)
        {
            if (IsDarkModeAllowedForWindow &&
                SetWindowCompositionAttribute)
            {
                bool isDarkMode = IsDarkModeAllowedForWindow(hwnd) && ShouldAppsUseDarkMode();

                WINDOWCOMPOSITIONATTRIBDATA data
                {
                    .Attrib = WCA_USEDARKMODECOLORS,
                    .pvData = &isDarkMode,
                    .cbData = sizeof(BOOL)
                };

                SetWindowCompositionAttribute(hwnd, &data);
            }
            else
            {
                LOG_HR_MSG(E_FAIL,
                    "Dark Mode functions cannot be found, "
                    "IsDarkModeAllowedForWindow = 0x%08llx, SetWindowCompositionAttribute = 0x%08llx.",
                    (uintptr_t)IsDarkModeAllowedForWindow, (uintptr_t)SetWindowCompositionAttribute);
            }
        }

        inline static void EnableDarkModeSupport(HWND hwnd)
        {
            if (SetPreferredAppMode &&
                RefreshImmersiveColorPolicyState &&
                AllowDarkModeForWindow)
            {
                SetPreferredAppMode(PreferredAppMode::AllowDark);
                RefreshImmersiveColorPolicyState();
                AllowDarkModeForWindow(hwnd, true);
            }
            else
            {
                LOG_HR_MSG(E_FAIL,
                    "Dark Mode functions cannot be found, "
                    "SetPreferredAppMode = 0x%08llx, RefreshImmersiveColorPolicyState = 0x%08llx, AllowDarkModeForWindow = 0x%08llx.",
                    (uintptr_t)SetPreferredAppMode, (uintptr_t)RefreshImmersiveColorPolicyState, (uintptr_t)AllowDarkModeForWindow);
            }
        }

        inline static const float GetDpiScaleForWindow(HWND hwnd)
        {
            if (GetDpiForWindowMethod)
                return GetDpiForWindowMethod(hwnd) / 96.0f;

            uint32_t dpi = 96;
            HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi, &dpi);

            return dpi / 96.0f;
        }

        inline static SWITCH_CONTEXT* GetSwitchContext(APPCOMPAT_EXE_DATA* pShim)
        {
            if (_switchContextOffset == 0)
            {
                if (Windows10_PlatformID == pShim->SwitchContext.Data.Platform)
                {
                    _switchContextOffset = (intptr_t)((byte*)&pShim->SwitchContext - (byte*)pShim);
                }
                else if (Windows10_PlatformID == ((APPCOMPAT_EXE_DATA_RS2*)pShim)->SwitchContext.Data.Platform)
                {
                    _switchContextOffset = (intptr_t)((byte*)&((APPCOMPAT_EXE_DATA_RS2*)pShim)->SwitchContext - (byte*)pShim);
                }
                else if (Windows10_PlatformID == ((APPCOMPAT_EXE_DATA_TH1*)pShim)->SwitchContext.Data.Platform)
                {
                    _switchContextOffset = (intptr_t)((byte*)&((APPCOMPAT_EXE_DATA_TH1*)pShim)->SwitchContext - (byte*)pShim);
                }
                /*else if (Windows10_PlatformID == ((APPCOMPAT_EXE_DATA_EIGHT*)pShim)->SwitchContext.Data.Platform)
                {
                    _switchContextOffset = (intptr_t)((byte*)&((APPCOMPAT_EXE_DATA_EIGHT*)pShim)->SwitchContext - (byte*)pShim);
                }*/
                else
                {
                    auto current = (byte*)pShim;
                    auto end = (byte*)&pShim[1] - sizeof(GUID);
                    auto offset = (intptr_t)((byte*)&pShim->SwitchContext.Data.Platform - (byte*)&pShim->SwitchContext);

                    while (current <= end)
                    {
                        if (Windows10_PlatformID == *(GUID*)current)
                        {
                            _switchContextOffset = (intptr_t)(current - (byte*)pShim) - offset;
                            break;
                        }

                        current++;
                    }
                }
            }

            if (_switchContextOffset != 0)
                return (SWITCH_CONTEXT*)((byte*)pShim + _switchContextOffset);

            return nullptr;
        }

        inline static SWITCH_CONTEXT_DATA* GetSwitchContextData()
        {
            auto appCompat = *(APPCOMPAT_EXE_DATA**)((intptr_t)NtCurrentPeb() + OFFSET_OF_SHIM_DATA);
            if (!appCompat) return nullptr;

            auto switchContext = GetSwitchContext(appCompat);
            return switchContext ? &switchContext->Data : nullptr;
        }

        inline static void* GetModuleEntryPoint(HMODULE Module)
        {
            if (!Module)
                return nullptr;

            auto dosHeader = (IMAGE_DOS_HEADER*)Module;
            auto ntHeaders = (IMAGE_NT_HEADERS*)((byte*)Module + dosHeader->e_lfanew);

            return (byte*)Module + ntHeaders->OptionalHeader.AddressOfEntryPoint;
        }

        // Modified versions of XWine1's patching functions that only handle the use cases of the library.
        // You can find generalized versions of these functions here:
        // https://github.com/ahmed605/MrmTool/blob/master/MrmTool/Common/PatchingHelper.cs
        // Under "#else // Generic version"

        // Originally written by @DaZombieKiller for the XWine1 project
        inline static HRESULT XWineFindImport(HMODULE Module,
                                              const char* Import,
                                              IMAGE_THUNK_DATA* pImportAddressTable,
                                              IMAGE_THUNK_DATA* pImportNameTable,
                                              IMAGE_THUNK_DATA** pThunk)
        {
            for (size_t j = 0; pImportNameTable[j].u1.AddressOfData > 0; j++)
            {
                if ((pImportNameTable[j].u1.AddressOfData & IMAGE_ORDINAL_FLAG) != 0)
                    continue;

                auto name = ((IMAGE_IMPORT_BY_NAME*)((byte*)Module + pImportNameTable[j].u1.AddressOfData))->Name;
                if (strcmp(name, Import) != 0)
                    continue;

                *pThunk = &pImportAddressTable[j];
                return S_OK;
            }

            *pThunk = nullptr;
            RETURN_HR(E_FAIL);
        }

        // Originally written by @DaZombieKiller for the XWine1 project
        inline static HRESULT XWineGetImport(HMODULE Module,
                                             HMODULE ImportModule,
                                             const char* Import,
                                             IMAGE_THUNK_DATA** pThunk)
        {
            if (ImportModule == nullptr)
                RETURN_HR(E_INVALIDARG);

            if (pThunk == nullptr)
                RETURN_HR(E_POINTER);

            if (Module == nullptr)
                Module = GetModuleHandleW(nullptr);

            auto dosHeader = (IMAGE_DOS_HEADER*)Module;
            auto ntHeaders = (IMAGE_NT_HEADERS*)((byte*)Module + dosHeader->e_lfanew);
            auto directory = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

            if (directory->VirtualAddress <= 0 || directory->Size <= 0)
                RETURN_HR(E_FAIL);

            auto peImports = (IMAGE_IMPORT_DESCRIPTOR*)((byte*)Module + directory->VirtualAddress);

            for (size_t i = 0; peImports[i].Name > 0; i++)
            {
                if (GetModuleHandleA((char*)((byte*)Module + peImports[i].Name)) != ImportModule)
                    continue;

                auto iatThunks = (IMAGE_THUNK_DATA*)((byte*)Module + peImports[i].FirstThunk);
                auto intThunks = (IMAGE_THUNK_DATA*)((byte*)Module + peImports[i].OriginalFirstThunk);

                if (SUCCEEDED(XWineFindImport(Module, Import, iatThunks, intThunks, pThunk)))
                    return S_OK;
            }

            auto delayDir = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT];
            if (delayDir->VirtualAddress > 0 && directory->Size > 0)
            {
                auto delayImports = (IMAGE_DELAYLOAD_DESCRIPTOR*)((byte*)Module + delayDir->VirtualAddress);

                for (size_t i = 0; delayImports[i].DllNameRVA > 0; i++)
                {
                    if (GetModuleHandleA((char*)((byte*)Module + delayImports[i].DllNameRVA)) != ImportModule)
                        continue;

                    auto iatThunks = (IMAGE_THUNK_DATA*)((byte*)Module + delayImports[i].ImportAddressTableRVA);
                    auto intThunks = (IMAGE_THUNK_DATA*)((byte*)Module + delayImports[i].ImportNameTableRVA);

                    if (SUCCEEDED(XWineFindImport(Module, Import, iatThunks, intThunks, pThunk)))
                        return S_OK;
                }
            }

            *pThunk = nullptr;
            RETURN_HR(E_FAIL);
        }

        // Originally written by @DaZombieKiller for the XWine1 project
        inline static HRESULT XWinePatchImport(HMODULE Module,
                                               HMODULE ImportModule,
                                               const char* Import,
                                               void* Function)
        {
            DWORD protect;
            PIMAGE_THUNK_DATA pThunk;
            RETURN_IF_FAILED(XWineGetImport(Module, ImportModule, Import, &pThunk));
            RETURN_LAST_ERROR_IF(!VirtualProtect(&pThunk->u1.Function, sizeof(ULONG_PTR), PAGE_READWRITE, &protect));
            pThunk->u1.Function = (ULONG_PTR)Function;
            RETURN_LAST_ERROR_IF(!VirtualProtect(&pThunk->u1.Function, sizeof(ULONG_PTR), protect, &protect));
            return S_OK;
        }
    };
}