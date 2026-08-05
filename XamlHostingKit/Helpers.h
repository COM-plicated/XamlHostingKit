#pragma once

#include <filesystem>
#include "Privates.h"
#include <wil/registry.h>
#include <ShellScalingApi.h>
#include <Shlobj.h>
#include <winrt/Windows.Storage.Streams.h>

#pragma warning(push, 0)
#include <simdutf/simdutf.h>
#pragma warning(pop)

namespace winrt::XamlHostingKit
{
    static const auto UXThemeModule = wil::unique_hmodule(LoadLibraryW(L"uxtheme.dll"));
    static const auto User32Module = wil::unique_hmodule(LoadLibraryW(L"user32.dll"));
    static const auto IERTUtilModule = wil::unique_hmodule(LoadLibraryW(L"iertutil.dll"));
    static const auto AppCoreModule = wil::unique_hmodule(LoadLibraryW(L"kernel.appcore.dll"));
    static const auto WinUIModule = wil::unique_hmodule(LoadLibraryW(L"Windows.UI.dll"));
    static const auto XAMLModule = wil::unique_hmodule(LoadLibraryW(L"Windows.UI.Xaml.dll"));
    static const auto MrmModule = wil::unique_hmodule(LoadLibraryW(L"MrmCoreR.dll"));
    static const auto TWinAPICoreModule = wil::unique_hmodule(LoadLibraryW(L"twinapi.appcore.dll"));
    static const auto ThreadPoolModule = wil::unique_hmodule(LoadLibraryW(L"threadpoolwinrt.dll"));
    static const auto COMBaseModule = GetModuleHandleW(L"combase.dll");
    static const auto KernelBaseModule = GetModuleHandleW(L"kernelbase.dll");
    static const auto Win32UModule = GetModuleHandleW(L"win32u.dll");

    static const auto IsDarkModeAllowedForWindow = reinterpret_cast<BOOL(WINAPI*)(HWND)>(GetProcAddress(UXThemeModule.get(), MAKEINTRESOURCEA(137)));
    static const auto SetWindowCompositionAttribute = reinterpret_cast<BOOL(WINAPI*)(HWND, WINDOWCOMPOSITIONATTRIBDATA*)>(GetProcAddress(User32Module.get(), "SetWindowCompositionAttribute"));
    static const auto SetPreferredAppMode = reinterpret_cast<void(WINAPI*)(PreferredAppMode)>(GetProcAddress(UXThemeModule.get(), MAKEINTRESOURCEA(135)));
    static const auto RefreshImmersiveColorPolicyState = reinterpret_cast<void(WINAPI*)()>(GetProcAddress(UXThemeModule.get(), MAKEINTRESOURCEA(104)));
    static const auto AllowDarkModeForWindow = reinterpret_cast<void(WINAPI*)(HWND, BOOL)>(GetProcAddress(UXThemeModule.get(), MAKEINTRESOURCEA(133)));
    static const auto IEConfiguration_SetBrowserAppProfile = reinterpret_cast<HRESULT(WINAPI*)(const wchar_t*, uint32_t, uint32_t)>(GetProcAddress(IERTUtilModule.get(), MAKEINTRESOURCEA(797)));
    static const auto PrivateCreateCoreWindow = reinterpret_cast<HRESULT(WINAPI*)(CoreWindowType, const wchar_t*, int, int, int, int, uint32_t, HWND, REFGUID, void**)>(GetProcAddress(WinUIModule.get(), MAKEINTRESOURCEA(1500)));
    static const auto CoSetASTATestMode = reinterpret_cast<void(WINAPI*)(ASTA_TEST_MODE_FLAGS)>(GetProcAddress(COMBaseModule, MAKEINTRESOURCEA(100)));
    static const auto GetDpiForWindowMethod = reinterpret_cast<decltype(&GetDpiForWindow)>(GetProcAddress(User32Module.get(), "GetDpiForWindow"));
    static const auto NtUserRegisterTouchPadCapable = reinterpret_cast<BOOL(WINAPI*)(BOOL)>(GetProcAddress(Win32UModule, "NtUserRegisterTouchPadCapable"));
    static const auto RegisterTouchpadCapableWindowMethod = reinterpret_cast<BOOL(WINAPI*)(HWND, BOOL)>(GetProcAddress(User32Module.get(), MAKEINTRESOURCEA(2689)));
    static const auto RegisterTouchpadCapableThreadMethod = reinterpret_cast<BOOL(WINAPI*)(BOOL)>(GetProcAddress(User32Module.get(), MAKEINTRESOURCEA(2688)));

    static const auto PersonalizeKey = wil::reg::open_unique_key(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize");

    #define NtCurrentPeb() (NtCurrentTeb()->ProcessEnvironmentBlock)

    namespace wss = winrt::Windows::Storage::Streams;

    class Helpers
    {
    private:
        inline static intptr_t _switchContextOffset = 0;

        inline static std::filesystem::path __GetExecutablePath()
        {
            wchar_t path[MAX_PATH];
            GetModuleFileNameW(GetModuleHandleW(nullptr), path, MAX_PATH);
            return std::filesystem::path(path);
        }

        inline static std::filesystem::path _executablePath = __GetExecutablePath();

        inline static winrt::hstring __GetExecutableName()
        {
            return winrt::hstring { _executablePath.stem().wstring() };
        }

        inline static bool ShouldAppsUseDarkMode()
        {
            return !wil::reg::try_get_value_dword(PersonalizeKey.get(), L"AppsUseLightTheme").value_or(true);
        }

        inline static std::wstring ToBase64(std::wstring const& input)
        {
            auto inputSizeInBytes = input.length() * sizeof(wchar_t);
            auto size = simdutf::base64_length_from_binary(inputSizeInBytes, simdutf::base64_url);

            char* output = new char[size];
            simdutf::binary_to_base64((const char*)input.c_str(), inputSizeInBytes, output, simdutf::base64_url);

            char16_t* wideOutput = new char16_t[size];
            simdutf::convert_utf8_to_utf16(output, size, wideOutput);

            auto str = std::wstring((wchar_t*)wideOutput, size);

            delete[] output;
            delete[] wideOutput;

            return str;
        }

    public:

        inline static winrt::hstring const& GetExecutableName()
        {
            static winrt::hstring exeName = __GetExecutableName();
            return exeName;
        }

        inline static std::filesystem::path GetExecutableFolderPath()
        {
            return _executablePath.parent_path();
        }

        inline static void EnsureTitleBarTheme(HWND hwnd)
        {
            if (IsDarkModeAllowedForWindow &&
                SetWindowCompositionAttribute) [[likely]]
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
                AllowDarkModeForWindow) [[likely]]
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
            if (GetDpiForWindowMethod) [[likely]]
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
                if (Windows10_PlatformID == pShim->SwitchContext.Data.Platform) [[likely]]
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
                else if (Windows10_PlatformID == ((APPCOMPAT_EXE_DATA_EIGHT*)pShim)->SwitchContext.Data.Platform ||
                         WindowsBlue_PlatformID == ((APPCOMPAT_EXE_DATA_EIGHT*)pShim)->SwitchContext.Data.Platform ||
                         Windows8_PlatformID == ((APPCOMPAT_EXE_DATA_EIGHT*)pShim)->SwitchContext.Data.Platform)
                {
                    _switchContextOffset = (intptr_t)((byte*)&((APPCOMPAT_EXE_DATA_EIGHT*)pShim)->SwitchContext - (byte*)pShim);
                }
                else
                {
                    auto current = (byte*)pShim;
                    auto end = (byte*)&pShim[1] - sizeof(GUID);
                    auto offset = (intptr_t)((byte*)&pShim->SwitchContext.Data.Platform - (byte*)&pShim->SwitchContext);

                    while (current <= end)
                    {
                        if (Windows10_PlatformID == *(GUID*)current /*||
                            WindowsBlue_PlatformID == *(GUID*)current ||
                            Windows8_PlatformID == *(GUID*)current*/)
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
            if (!appCompat) [[unlikely]]
            {
                return nullptr;
            }

            auto switchContext = GetSwitchContext(appCompat);
            return switchContext ? &switchContext->Data : nullptr;
        }

        inline static void* GetModuleEntryPoint(HMODULE Module)
        {
            if (!Module) [[unlikely]]
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
            if (ImportModule == nullptr) [[unlikely]]
                RETURN_HR(E_INVALIDARG);

            if (pThunk == nullptr) [[unlikely]]
                RETURN_HR(E_POINTER);

            if (Module == nullptr) [[unlikely]]
                Module = GetModuleHandleW(nullptr);

            auto dosHeader = (IMAGE_DOS_HEADER*)Module;
            auto ntHeaders = (IMAGE_NT_HEADERS*)((byte*)Module + dosHeader->e_lfanew);
            auto directory = &ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

            if (directory->VirtualAddress <= 0 || directory->Size <= 0) [[unlikely]]
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
            if (delayDir->VirtualAddress > 0 && directory->Size > 0) [[likely]]
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

        inline static HANDLE CreateTempFileFromBuffer(wss::IBuffer const& buffer)
        {
            wil::unique_cotaskmem_string path { nullptr };
            if FAILED_LOG(SHGetKnownFolderPath(FOLDERID_LocalAppData,
                KF_FLAG_CREATE | KF_FLAG_FORCE_APPCONTAINER_REDIRECTION | KF_FLAG_FORCE_PACKAGE_REDIRECTION,
                nullptr,
                path.put()))
            {
                winrt::check_hresult(SHGetKnownFolderPath(FOLDERID_LocalAppData,
                    KF_FLAG_CREATE | KF_FLAG_FORCE_APPCONTAINER_REDIRECTION,
                    nullptr,
                    path.put()));
            }

            HANDLE handle;
            auto filePath = std::filesystem::path(path.get()) / ToBase64(GetExecutableFolderPath());
            winrt::check_pointer(handle = CreateFileW(filePath.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                nullptr));

            DWORD bytesWritten;
            winrt::check_bool(WriteFile(handle, buffer.data(), static_cast<DWORD>(buffer.Length()), &bytesWritten, nullptr));

            return handle;
        }
    };
}