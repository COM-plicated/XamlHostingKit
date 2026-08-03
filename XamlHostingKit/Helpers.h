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

    class Helpers
    {
    private:

        inline static winrt::hstring const& __GetExecutableName()
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
            static winrt::hstring const& exeName = __GetExecutableName();
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
        }

        inline static uint32_t GetDpiForWindow(HWND hwnd)
        {
            if (GetDpiForWindowMethod)
                return GetDpiForWindowMethod(hwnd);

            HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            uint32_t dpi;
            GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpi, &dpi);

            return dpi;
        }
    };
}