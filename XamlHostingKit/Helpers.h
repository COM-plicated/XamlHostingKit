#pragma once

#include <Shlobj.h>
#include <appmodel.h>
#include <filesystem>
#include "Privates.h"
#include <wil/registry.h>
#include <wil/resource.h>
#include <ShellScalingApi.h>
#include <winrt/Windows.Storage.Streams.h>

#if USE_PATH_HASH_FOR_TEMP_PRI // defined in the vcxproj file
#include <bcrypt.h>
#pragma warning(push, 0)
#include <simdutf/simdutf.h>
#pragma warning(pop)
#endif

#define USE_FULL_CLASS_FACTORY false
#if USE_FULL_CLASS_FACTORY
#include <weakreference.h>
#endif

namespace winrt::XamlHostingKit
{
    static const auto UXThemeModule = wil::unique_hmodule(LoadLibraryW(L"uxtheme.dll"));
    static const auto User32Module = wil::unique_hmodule(LoadLibraryW(L"user32.dll"));
    static const auto IERTUtilModule = wil::unique_hmodule(LoadLibraryW(L"iertutil.dll"));
    static const auto AppCoreModule = wil::unique_hmodule(LoadLibraryW(L"kernel.appcore.dll"));
    static const auto WinUIModule = wil::unique_hmodule(LoadLibraryW(L"Windows.UI.dll"));
    static const auto XAMLModule = wil::unique_hmodule(LoadLibraryW(L"Windows.UI.Xaml.dll"));
    static const auto MrmModule = wil::unique_hmodule(LoadLibraryW(L"MrmCoreR.dll"));
    static const auto UrlMonModule = wil::unique_hmodule(LoadLibraryW(L"UrlMon.dll"));
    static const auto TWinAPICoreModule = wil::unique_hmodule(LoadLibraryW(L"twinapi.appcore.dll"));
    static const auto ThreadPoolModule = wil::unique_hmodule(LoadLibraryW(L"threadpoolwinrt.dll"));
    static const auto COMBaseModule = GetModuleHandleW(L"combase.dll");
    static const auto KernelBaseModule = GetModuleHandleW(L"kernelbase.dll");
    static const auto Kernel32Module = GetModuleHandleW(L"kernel32.dll");
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
    static const auto NtUserGetResizeDCompositionSynchronizationObject = reinterpret_cast<BOOL(WINAPI*)(HWND, HANDLE*)>(GetProcAddress(Win32UModule, "NtUserGetResizeDCompositionSynchronizationObject"));
    static const auto NtUserEnableResizeLayoutSynchronization = reinterpret_cast<BOOL(WINAPI*)(HWND, BOOL)>(GetProcAddress(Win32UModule, "NtUserEnableResizeLayoutSynchronization"));
    static const auto UrlmonCreateInstance = reinterpret_cast<HRESULT(WINAPI*)(REFCLSID, IUnknown*, REFIID, void**)>(GetProcAddress(UrlMonModule.get(), MAKEINTRESOURCEA(441)));
    static const auto GetCurrentPackageInfo2Method = reinterpret_cast<decltype(&GetCurrentPackageInfo2)>(GetProcAddress(KernelBaseModule, "GetCurrentPackageInfo2"));
    static const auto CreateXamlUIPresenter = reinterpret_cast<HRESULT(WINAPI*)(void*, void**)>(GetProcAddress(XAMLModule.get(), "CreateXamlUIPresenter"));
    static const auto RegisterPermanentUrlRedirectionPolicyManager = reinterpret_cast<HRESULT(WINAPI*)(IUrlRedirectionPolicyManager*)>(GetProcAddress(UrlMonModule.get(), MAKEINTRESOURCEA(560)));

    static const auto PersonalizeKey = wil::reg::open_unique_key(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize");

    #define NtCurrentPeb() (NtCurrentTeb()->ProcessEnvironmentBlock)

    EXTERN_C NTSTATUS RtlGetVersion(_Out_ PRTL_OSVERSIONINFOW lpVersionInformation);

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

#if USE_PATH_HASH_FOR_TEMP_PRI
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

        inline static std::wstring ToBase64(std::span<const std::byte> input)
        {
            auto inputSizeInBytes = input.size();
            auto size = simdutf::base64_length_from_binary(inputSizeInBytes, simdutf::base64_url);

            char* output = new char[size];
            simdutf::binary_to_base64((const char*)input.data(), inputSizeInBytes, output, simdutf::base64_url);

            char16_t* wideOutput = new char16_t[size];
            simdutf::convert_utf8_to_utf16(output, size, wideOutput);

            auto str = std::wstring((wchar_t*)wideOutput, size);

            delete[] output;
            delete[] wideOutput;

            return str;
        }

        inline static std::array<std::byte, 32> Sha256(std::span<const std::byte> data)
        {
            std::array<std::byte, 32> digest { };

            wil::unique_bcrypt_algorithm alg;
            THROW_IF_NTSTATUS_FAILED(::BCryptOpenAlgorithmProvider(
                alg.put(), BCRYPT_SHA256_ALGORITHM, nullptr, 0));

            DWORD hashLength = 0;
            ULONG written = 0;
            THROW_IF_NTSTATUS_FAILED(::BCryptGetProperty(
                alg.get(), BCRYPT_HASH_LENGTH,
                reinterpret_cast<PUCHAR>(&hashLength), sizeof(hashLength), &written, 0));

            THROW_HR_IF(E_UNEXPECTED, hashLength != digest.size());

            THROW_IF_NTSTATUS_FAILED(::BCryptHash(
                alg.get(),
                nullptr, 0,
                reinterpret_cast<PUCHAR>(const_cast<std::byte*>(data.data())),
                static_cast<ULONG>(data.size()),
                reinterpret_cast<PUCHAR>(digest.data()),
                static_cast<ULONG>(digest.size())));

            return digest;
        }
#endif

    public:

        inline static std::wstring PriTempFilePath { };

        inline static std::wstring CurrentPackageFamilyName = []() -> std::wstring
        {
            uint32_t length = 0;
            if (GetCurrentPackageFamilyName(&length, nullptr) == ERROR_INSUFFICIENT_BUFFER) [[unlikely]]
            {
                std::wstring pfn(length - 1, L'\0');
                LOG_IF_WIN32_ERROR(GetCurrentPackageFamilyName(&length, pfn.data()));
                return pfn;
            }

            return { };
        }();

        inline static uint32_t OSBuild = []() -> uint32_t
        {
            OSVERSIONINFOEXW osvi { };
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            if (SUCCEEDED_NTSTATUS_LOG(RtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi)))) [[likely]]
                return osvi.dwBuildNumber;

            return 0;
        }();

        inline static winrt::hstring const& GetExecutableName()
        {
            static winrt::hstring exeName = __GetExecutableName();
            return exeName;
        }

        inline static std::filesystem::path GetExecutableFolderPath()
        {
            return _executablePath.parent_path();
        }

        inline static const wchar_t* GetExecutablePath()
        {
            return _executablePath.c_str();
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
                auto isIntResource = IS_INTRESOURCE(Import);
                if ((pImportNameTable[j].u1.AddressOfData & IMAGE_ORDINAL_FLAG) != 0)
                {
                    if (!isIntResource)
                        continue;

                    if (((pImportNameTable[j].u1.Ordinal & ~IMAGE_ORDINAL_FLAG) == (uintptr_t)Import))
                    {
                        *pThunk = &pImportAddressTable[j];
                        return S_OK;
                    }

                    continue;
                }

                if (isIntResource)
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

#if USE_PATH_HASH_FOR_TEMP_PRI
            auto const& folderPath = GetExecutableFolderPath().wstring();
            PriTempFilePath = L"\\\\?\\" + (std::filesystem::path(path.get()) /
                ToBase64(Sha256({ (std::byte*)folderPath.data(), folderPath.length() * sizeof(wchar_t) })))
                .wstring() + std::to_wstring((uint32_t)GetCurrentProcessId());
#else
            PriTempFilePath = L"\\\\?\\" + (std::filesystem::path(path.get()) /
                std::format(L"{}.{}.xhk.pri", GetExecutableName(), (uint32_t)GetCurrentProcessId())).native();
#endif

            HANDLE handle;
            winrt::check_bool((handle = CreateFileW(PriTempFilePath.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                nullptr)) != INVALID_HANDLE_VALUE);

            DWORD bytesWritten;
            winrt::check_bool(WriteFile(handle, buffer.data(), static_cast<DWORD>(buffer.Length()), &bytesWritten, nullptr));

            return handle;
        }

        inline static bool EnableResizeSynchronization(HWND hwnd, bool enabled)
        {
            if (NtUserEnableResizeLayoutSynchronization)
                return NtUserEnableResizeLayoutSynchronization(hwnd, enabled);

            return false;
        }

        inline static HANDLE GetResizeSynchronizationObject(HWND hwnd)
        {
            if (!NtUserGetResizeDCompositionSynchronizationObject)
                return nullptr;

            HANDLE handle;
            if (!NtUserGetResizeDCompositionSynchronizationObject(hwnd, &handle))
                return nullptr;

            return handle;
        }
    };

    namespace detail
    {
#if !USE_FULL_CLASS_FACTORY
        template <typename D, typename... Interfaces>
        inline constexpr bool has_iid(winrt::implements<D, Interfaces...>*, const GUID& iid)
        {
#ifdef __INTELLISENSE__
            return (iid == __uuidof(::IUnknown)) || ((iid == __uuidof(Interfaces)) || ...);
#else
            return (iid == __uuidof(::IUnknown)) || ((iid == std::bit_cast<GUID>(winrt::guid_of<Interfaces>())) || ...);
#endif
        }
#else
        template <typename T>
        struct is_real_interface : std::true_type { };

        template <typename T>
        struct is_real_interface<winrt::cloaked<T>> : std::false_type { };

        template <>
        struct is_real_interface<winrt::composable> : std::false_type { };

        template <>
        struct is_real_interface<winrt::composing> : std::false_type { };

        template <>
        struct is_real_interface<winrt::static_lifetime> : std::false_type { };

        template <>
        struct is_real_interface<winrt::non_agile> : std::false_type { };

        template <>
        struct is_real_interface<winrt::no_weak_ref> : std::false_type { };

        template <>
        struct is_real_interface<winrt::no_module_lock> : std::false_type { };

        template <typename T>
        inline constexpr bool is_real_interface_v = is_real_interface<T>::value;

        template <typename... Interfaces>
        inline constexpr bool has_non_agile_v = (std::is_same_v<Interfaces, winrt::non_agile> || ...);

        template <typename... Interfaces>
        inline constexpr bool has_no_weak_ref_v = (std::is_same_v<Interfaces, winrt::no_weak_ref> || ...);

        template <typename D, typename... Interfaces>
        inline constexpr bool has_iid(winrt::implements<D, Interfaces...>*, const GUID& iid)
        {
            return (iid == __uuidof(::IUnknown) || iid == __uuidof(::IInspectable))
                || (!has_no_weak_ref_v<Interfaces...> && iid == __uuidof(::IWeakReferenceSource))
                || (!has_non_agile_v<Interfaces...> && (iid == __uuidof(::IAgileObject) || iid == __uuidof(::IMarshal)))
#ifdef __INTELLISENSE__
                || ((is_real_interface_v<Interfaces> && iid == __uuidof(Interfaces)) || ...);
#else
                || ((is_real_interface_v<Interfaces> && iid == std::bit_cast<GUID>(winrt::guid_of<Interfaces>())) || ...);
#endif
        }
#endif
    }

    template <typename T>
    inline constexpr bool has_iid(const GUID& iid)
    {
        return detail::has_iid(static_cast<T*>(nullptr), iid);
    }

    template <typename T>
    struct ClassFactory : winrt::implements<ClassFactory<T>, IClassFactory>
    {
    public:
        inline ClassFactory() = default;

        inline HRESULT WINAPI CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) noexcept override
        {
            RETURN_HR_IF(CLASS_E_NOAGGREGATION, pUnkOuter != nullptr);
            RETURN_HR_IF_NULL(E_POINTER, ppvObject);

            if (!has_iid<T>(riid))
            {
                *ppvObject = nullptr;
                return E_NOINTERFACE;
            }

            auto object = winrt::make<T>();
            RETURN_HR(object->QueryInterface(riid, ppvObject));
        }

        inline HRESULT WINAPI LockServer(BOOL fLock) noexcept override
        {
            if (fLock)
            {
                ++winrt::get_module_lock();
            }
            else
            {
                --winrt::get_module_lock();
            }

            return S_OK;
        }
    };
}