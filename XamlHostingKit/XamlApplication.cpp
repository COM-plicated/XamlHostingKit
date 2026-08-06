#include "pch.h"
#include "XamlApplication.h"
#include "XamlApplication.g.cpp"

#include "Features.h"
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Management.Deployment.h>
#include "XamlConfig.h"
#include "XamlWindow.h"
#include <detours/detours.h>

namespace winrt::XamlHostingKit::implementation
{
    using namespace winrt::Windows::UI::Xaml;
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::Management::Deployment;
    using namespace winrt::Windows::Foundation::Collections;

    winrt::XamlHostingKit::XamlWindow XamlApplication::MainWindow()
    {
        return s_mainWindow;
    }

    winrt::Windows::Foundation::Collections::IVectorView<winrt::XamlHostingKit::XamlWindow> XamlApplication::Windows()
    {
        return s_windows.GetView();
    }

    bool XamlApplication::IsWebViewAvailable()
    {
        if (!s_propsFilled) [[unlikely]]
        {
            throw hresult_illegal_method_call(L"IsWebViewAvailable can only be called after XamlApplication.Start().");
        }

        return s_isWebViewAvailable;
    }

    ResourceManager XamlApplication::ResourceManager()
    {
        if (!s_propsFilled) [[unlikely]]
        {
            throw hresult_illegal_method_call(L"ResourceManager can only be called after XamlApplication.Start().");
        }

        return s_resourceManager;
    }

    void XamlApplication::StartCommon(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback)
    {
        if (!initCallback) [[unlikely]]
        {
            throw hresult_invalid_argument(L"initCallback cannot be null.");
        }

        std::call_once(s_appInitFlag, []
        {
            LOG_IF_FAILED(InitializeCoreApplicationHooks());

            if (XamlConfig::s_enableWebView) [[likely]]
            {
                s_isWebViewAvailable = SUCCEEDED_LOG(InitializeWebView());
            }

            if (!TWinAPICoreModule || !ThreadPoolModule) [[unlikely]]
            {
                LOG_HR_MSG(S_FALSE, "Failed to load twinapi.appcore.dll and/or threadpoolwinrt.dll, some features might not be available.");
            }
        });

        s_propsFilled = true;

        if (Features::IsXamlRootAvailable) [[likely]]
        {
            initCallback(nullptr);
        }
        else
        {
            if (!CoSetASTATestMode) [[unlikely]]
            {
                throw hresult_error(E_FAIL, L"Failed to find CoSetASTATestMode function.");
            }

            if (auto data = Helpers::GetSwitchContextData()) [[likely]]
            {
                data->OsMaxVersionTested = 0x000a00004a610000; // Windows 10 2004, build 19041
            }
            else
            {
                LOG_HR_MSG(E_FAIL, "Failed to set MaxVersionTested, XAML will load wrong/base XAML resources version.");
            }

            CoSetASTATestMode(ROINITIALIZEASTA_ALLOWED);

            auto priv = winrt::get_activation_factory<Application, IFrameworkApplicationStaticsPrivate>();
            winrt::check_hresult(priv->StartInCoreWindowHostingMode({ .TransparentBackground = 1 }, winrt::get_abi(initCallback)));
        }

        auto window = winrt::make_self<XamlWindow>(WindowCreationOptions { }, true);

        s_hasStarted = true;
        s_mainWindow = *window.get();
        s_windows.Append(s_mainWindow);

        Application::Current().as<IApplicationOverrides>().OnLaunched(nullptr);

        window->RunMessageLoop();

        XamlHostingKit::XamlWindow nextWindow { nullptr };
        while (s_windows.Size() > 0 && (nextWindow = s_windows.GetAt(0)))
        {
            auto thread = GetWindowThreadProcessId((HWND)nextWindow.WindowHandle().Value, nullptr);
            wil::unique_handle hThread(OpenThread(SYNCHRONIZE, FALSE, thread));

            WaitForSingleObject(hThread.get(), INFINITE);
        }

        HMODULE edgeModule;
        HMODULE threadpoolModule;
        if (XamlConfig::s_enableWebView &&
           (edgeModule = GetModuleHandleW(L"edgehtml.dll")) &&
           (threadpoolModule = GetModuleHandleW(L"api-ms-win-core-threadpool-l1-2-0.dll"))) [[likely]]
        {
            LOG_IF_FAILED(Helpers::XWinePatchImport(edgeModule, threadpoolModule, "SubmitThreadpoolWork", &SubmitThreadpoolWorkHook));
        }
    }

    void XamlApplication::Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback, hstring const& priPath)
    {
        if (s_hasStarted) [[unlikely]]
        {
            throw hresult_illegal_method_call(L"XamlApplication.Start() can only be called once.");
        }

        s_propsFilled = false;

        if (priPath.empty()) [[unlikely]]
        {
            throw hresult_invalid_argument(L"priPath cannot be empty.");
        }

        CreateResourceManager(priPath.c_str());
        StartCommon(initCallback);
    }

    void XamlApplication::Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback, winrt::Windows::Storage::Streams::IBuffer const& priBuffer)
    {
        if (s_hasStarted) [[unlikely]]
        {
            throw hresult_illegal_method_call(L"XamlApplication.Start() can only be called once.");
        }

        s_propsFilled = false;

        if (!priBuffer) [[unlikely]]
        {
            throw hresult_invalid_argument(L"priBuffer cannot be null.");
        }

        std::call_once(s_mrmHookFlag, []
        {
            s_mrmHookedSuccessfully = SUCCEEDED_LOG(InitializeMrmHooks());
        });

        if (!s_mrmHookedSuccessfully) [[unlikely]]
        {
            throw hresult_error(E_FAIL, L"Failed to hook Modern Resource Manager (MRM) functions.");
        }

        s_priTempFile.reset(Helpers::CreateTempFileFromBuffer(priBuffer));

        CreateResourceManager(s_priFileName.c_str());
        StartCommon(initCallback);
    }

    void XamlApplication::Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback)
    {
        Start(initCallback, winrt::hstring { (Helpers::GetExecutableFolderPath() / L"resources.pri").wstring() });
    }

    winrt::XamlHostingKit::XamlWindow XamlApplication::CreateWindow()
    {
        throw hresult_not_implemented();
    }

    winrt::XamlHostingKit::XamlWindow XamlApplication::CreateWindow(winrt::XamlHostingKit::WindowCreationOptions const& options)
    {
        throw hresult_not_implemented();
    }

    winrt::XamlHostingKit::XamlWindow XamlApplication::CreateWindow(winrt::XamlHostingKit::WindowCreationOptions const& options, winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& windowCallback)
    {
        throw hresult_not_implemented();
    }

    void XamlApplication::Close()
    {
        for (auto i = s_windows.Size(); i-- > 0;)
        {
            if (auto window = s_windows.GetAt(i)) [[likely]]
            {
                window.Close();
            }
        }

        s_mainWindow = nullptr;
    }

    void XamlApplication::RemoveWindow(winrt::XamlHostingKit::XamlWindow const& window)
    {
        uint32_t idx = 0;
        if (s_windows.IndexOf(window, idx)) [[likely]]
        {
            s_windows.RemoveAt(idx);

            if (s_mainWindow == window)
            {
                if (s_windows.Size() > 0)
                {
                    s_mainWindow = s_windows.GetAt(0);
                }
                else
                {
                    s_mainWindow = nullptr;
                }
            }
        }
    }

    BOOL WINAPI XamlApplication::IsImmersiveProcessHook([[maybe_unused]] void* unk)
    {
        return TRUE;
    }

    FARPROC WINAPI XamlApplication::GetProcAddressHook(HMODULE hModule, _In_ LPCSTR lpProcName)
    {
        if (lpProcName && !IS_INTRESOURCE(lpProcName)) [[likely]]
        {
            if (strcmp(lpProcName, "IsImmersiveProcess") == 0)
            {
                return reinterpret_cast<FARPROC>(&IsImmersiveProcessHook);
            }
        }

        _Analysis_assume_(lpProcName != 0);
        return GetProcAddress(hModule, lpProcName);
    }

    LONG WINAPI XamlApplication::AppPolicyGetWindowingModelHook([[maybe_unused]] HANDLE processToken, AppPolicyWindowingModel* policy)
    {
        if (!policy) [[unlikely]]
        {
            return ERROR_INVALID_PARAMETER;
        }

        *policy = AppPolicyWindowingModel_None;
        return ERROR_SUCCESS;
    }

    winrt::hstring XamlApplication::FindSettingsPackageFullname()
    {
        auto manager = PackageManager();
        auto packages = manager.FindPackagesForUser({ }, L"windows.immersivecontrolpanel_cw5n1h2txyewy");
        
        IIterator<Package> first { nullptr };
        if (!packages || !(first = packages.First()) || !first.HasCurrent()) [[unlikely]]
        {
            LOG_HR_MSG(E_FAIL, "Failed to find Settings package, WebView might not work.");
            return { };
        }

        return first.Current().Id().FullName();
    }

    LONG WINAPI XamlApplication::GetCurrentPackageInfoHook(_In_ const UINT32 flags, _Inout_ UINT32* bufferLength, _Out_opt_ BYTE* buffer, _Out_opt_ UINT32* count)
    {
        auto static const settingsPFN = FindSettingsPackageFullname();

        if (!settingsPFN.empty()) [[likely]]
        {
            PACKAGE_INFO_REFERENCE pir { NULL };
            if (LOG_IF_WIN32_ERROR(OpenPackageInfoByFullName(settingsPFN.c_str(), 0, &pir)) == ERROR_SUCCESS) [[likely]]
            {
                auto result = LOG_IF_WIN32_ERROR(GetPackageInfo(pir, flags, bufferLength, buffer, count));
                LOG_IF_WIN32_ERROR(ClosePackageInfo(pir));
                return result;
            }

            LOG_HR_MSG(E_FAIL, "Failed to open Settings package, WebView might not work.");
        }

        return GetCurrentPackageInfo(flags, bufferLength, buffer, count);
    }

    HRESULT XamlApplication::InitializeWebView()
    {
        if (AppCoreModule &&
            XAMLModule &&
            IERTUtilModule &&
            IEConfiguration_SetBrowserAppProfile) [[likely]]
        {
            RETURN_IF_FAILED(Helpers::XWinePatchImport(XAMLModule.get(), AppCoreModule.get(), "AppPolicyGetWindowingModel", &AppPolicyGetWindowingModelHook));
            RETURN_IF_FAILED(Helpers::XWinePatchImport(XAMLModule.get(), AppCoreModule.get(), "GetCurrentPackageInfo", &GetCurrentPackageInfoHook));
            RETURN_IF_FAILED(Helpers::XWinePatchImport(IERTUtilModule.get(), KernelBaseModule, "GetProcAddress", &GetProcAddressHook));
            RETURN_IF_FAILED(IEConfiguration_SetBrowserAppProfile(L"MicrosoftEdge", 2, 0));
            return S_OK;
        }

        return E_FAIL;
    }

    HANDLE WINAPI XamlApplication::CreateFileWHook(_In_ LPCWSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile)
    {
        HANDLE handle;
        if (lpFileName &&
            s_priTempFile &&
            _wcsicmp(lpFileName, s_priFileName.c_str()) == 0 &&
            DuplicateHandle(GetCurrentProcess(),
                s_priTempFile.get(),
                GetCurrentProcess(),
                &handle,
                0, FALSE,
                DUPLICATE_SAME_ACCESS)) [[likely]]
        {
            return handle;
        }

        return CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }

    BOOL WINAPI XamlApplication::GetFileAttributesExWHook(_In_ LPCWSTR lpFileName, _In_ GET_FILEEX_INFO_LEVELS fInfoLevelId, _Out_writes_bytes_(sizeof(WIN32_FILE_ATTRIBUTE_DATA)) LPVOID lpFileInformation)
    {
        if (lpFileName &&
            !Helpers::PriTempFilePath.empty() &&
            _wcsicmp(lpFileName, s_priFileName.c_str()) == 0) [[likely]]
        {
            lpFileName = Helpers::PriTempFilePath.c_str();
        }

        return GetFileAttributesExW(lpFileName, fInfoLevelId, lpFileInformation);
    }

    HRESULT WINAPI XamlApplication::InitializeForCurrentApplicationHook(mrm::IMrtResourceManager* _this)
    {
        HMODULE mod;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)_ReturnAddress(), &mod);

        HRESULT hr;
        if (mod != XAMLModule.get() || !s_priTempFile ||
            FAILED_LOG(hr = _this->InitializeForFile(s_priFileName.c_str()))) [[unlikely]]
        {
            return s_originalInitializeForCurrentApplication(_this);
        }

        return hr;
    }

    HRESULT WINAPI XamlApplication::TryInitializeForCurrentApplicationHook(mrm::IMrtResourceManager2* _this)
    {
        HMODULE mod;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)_ReturnAddress(), &mod);

        HRESULT hr;
        winrt::com_ptr<mrm::IMrtResourceManager> manager;

        if (mod != XAMLModule.get() || !s_priTempFile ||
            FAILED_LOG(_this->QueryInterface(manager.put())) ||
            FAILED_LOG(hr = manager->InitializeForFile(s_priFileName.c_str()))) [[unlikely]]
        {
            return s_originalTryInitializeForCurrentApplication(_this);
        }

        return hr;
    }

    HRESULT XamlApplication::InitializeMrmHooks()
    {
        if (MrmModule) [[likely]]
        {
            com_ptr<mrm::IMrtResourceManager> resourceManager;
            com_ptr<mrm::IMrtResourceManager2> resourceManager2;
            RETURN_IF_FAILED(CoCreateInstance(__uuidof(mrm::MrtResourceManager), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(resourceManager.put())));
            RETURN_IF_FAILED(resourceManager->QueryInterface(resourceManager2.put()));

            auto vtbl = *reinterpret_cast<void***>(resourceManager.get());
            auto vtbl2 = *reinterpret_cast<void***>(resourceManager2.get());
            s_originalInitializeForCurrentApplication = reinterpret_cast<decltype(s_originalInitializeForCurrentApplication)>(vtbl[4]);
            s_originalTryInitializeForCurrentApplication = reinterpret_cast<decltype(s_originalTryInitializeForCurrentApplication)>(vtbl2[4]);

            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            DetourAttach(&(PVOID&)s_originalInitializeForCurrentApplication, InitializeForCurrentApplicationHook);
            DetourAttach(&(PVOID&)s_originalTryInitializeForCurrentApplication, TryInitializeForCurrentApplicationHook);
            RETURN_LAST_ERROR_IF(DetourTransactionCommit() != NO_ERROR);
            RETURN_IF_FAILED(Helpers::XWinePatchImport(MrmModule.get(), KernelBaseModule, "CreateFileW", &CreateFileWHook));
            RETURN_IF_FAILED(Helpers::XWinePatchImport(MrmModule.get(), KernelBaseModule, "GetFileAttributesExW", &GetFileAttributesExWHook));

            return S_OK;
        }

        return E_FAIL;
    }

    void XamlApplication::CreateResourceManager(PCWSTR priPath)
    {
        winrt::com_ptr<IResourceManagerStaticInternal> mgrStatics;
        if (!(mgrStatics = winrt::try_get_activation_factory<warc::ResourceManager, IResourceManagerStaticInternal>()) ||
            FAILED_LOG(mgrStatics->GetCurrentResourceManagerForSystemProfile(winrt::put_abi(s_resourceManager))) ||
            !s_resourceManager) [[unlikely]]
        {
            winrt::com_ptr<IResourceManagerStaticInternalOld> mgrStaticsOld;
            if (!(mgrStaticsOld = try_get_activation_factory<warc::ResourceManager, IResourceManagerStaticInternalOld>()) ||
                FAILED_LOG(mgrStatics->GetCurrentResourceManagerForSystemProfile(winrt::put_abi(s_resourceManager))) ||
                !s_resourceManager) [[unlikely]]
            {
                LOG_HR_MSG(E_FAIL, "Failed to create ResourceManager.");
                return;
            }
        }

        winrt::com_ptr<ISystemResourceManagerExtensions2> mgrEx;
        if (!s_resourceManager.try_as(mgrEx) || FAILED_LOG(mgrEx->LoadPriFileForSystemUse(priPath))) [[unlikely]]
        {
            LOG_HR_MSG(E_FAIL, "Failed to get load the PRI into the ResourceManager.");
            s_resourceManager = nullptr;
        }
    }

    HRESULT WINAPI XamlApplication::get_ViewsHook([[maybe_unused]] void* _this, void** pViews)
    {
        if (!pViews) [[unlikely]]
        {
            return E_POINTER;
        }

        if (s_windows.Size() > 0) [[likely]]
        {
            std::vector<CoreApplicationView> views;
            for (uint32_t i = 0; i < s_windows.Size(); ++i)
            {
                if (auto window = s_windows.GetAt(i)) [[likely]]
                {
                    views.push_back(window.CoreApplicationView());
                }
            }

            *pViews = winrt::detach_abi(winrt::single_threaded_vector(std::move(views)).GetView());
        }
        else
        {
            *pViews = winrt::detach_abi(winrt::single_threaded_vector<CoreApplicationView>().GetView());
        }

        return S_OK;
    }

    HRESULT WINAPI XamlApplication::get_MainViewHook([[maybe_unused]] void* _this, void** pView)
    {
        if (!pView) [[unlikely]]
        {
            return E_POINTER;
        }

        if (s_mainWindow) [[likely]]
        {
            winrt::copy_to_abi(s_mainWindow.CoreApplicationView(), *pView);
        }
        else
        {
            *pView = nullptr;
            return E_FAIL;
        }

        return S_OK;
    }

    HRESULT XamlApplication::InitializeCoreApplicationHooks()
    {
        auto imrsv = winrt::get_activation_factory<CoreApplication, ICoreImmersiveApplication>();
        auto vtbl = *reinterpret_cast<void***>(winrt::get_abi(imrsv));

        auto GetViews = vtbl[6];
        auto GetMainView = vtbl[8];

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)GetViews, get_ViewsHook);
        DetourAttach(&(PVOID&)GetMainView, get_MainViewHook);
        RETURN_LAST_ERROR_IF(DetourTransactionCommit() != NO_ERROR);

        return S_OK;
    }
}