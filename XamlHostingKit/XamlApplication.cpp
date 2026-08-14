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
#include <FileProtocolHandler.h>

namespace winrt::XamlHostingKit::implementation
{
    using namespace winrt::Windows::UI::Xaml;
    using namespace winrt::Windows::ApplicationModel;
    using namespace winrt::Windows::Management::Deployment;
    using namespace winrt::Windows::Foundation::Collections;

    winrt::XamlHostingKit::XamlWindow XamlApplication::MainWindow()
    {
        std::lock_guard lock(s_mainWindowMutex);
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

            auto priv = winrt::try_get_activation_factory<Application, IFrameworkApplicationStaticsPrivate>();
            if (!priv || FAILED_LOG(priv->StartInCoreWindowHostingMode({ .TransparentBackground = 1 }, winrt::get_abi(initCallback)))) [[unlikely]]
            {
                // we can reach here on 8.1 (and older)
                // we can also reach here if IFrameworkApplicationStaticsPrivate
                // exists but StartInCoreWindowHostingMode failed, in which case
                // this will also fail (probably).

                try
                {
                    if (Application::Current()) [[unlikely]]
                    {
                        goto MakeWindow;
                    }
                }
                catch (...) { }

                if (!CreateXamlUIPresenter) [[unlikely]]
                {
                    throw hresult_error(E_FAIL, L"Failed to find CreateXamlUIPresenter function, this function is needed to initialize DXamlCore.");
                }

                // There is a bug on older builds where GetReferenceTrackerManager (called on the Application class)
                // doesn't call DirectUI::ReferenceTrackerManager::EnsureInitialized, which ends up with an AV in
                // GetReferenceTrackerManager (called by .NET), to workaround that we call CreateXamlUIPresenter
                // which calls DirectUI::DXamlCore::InitializeImpl, which in turn calls
                // DirectUI::ReferenceTrackerManager::EnsureInitialized, which fixes the AV.
                CreateXamlUIPresenter(nullptr, nullptr);
                initCallback(nullptr);
            }
        }

MakeWindow:
        auto window = winrt::make_self<XamlWindow>(WindowCreationOptions { }, true);

        s_hasStarted = true;
        XamlConfig::s_isInitialized = true;

        {
            std::lock_guard lock(s_mainWindowMutex);
            s_mainWindow = *window.get();
            s_windows.Append(s_mainWindow);
        }

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

    void XamlApplication::Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback, hstring const& priPath, bool shouldThrowOnHookFailure)
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

        s_priFileName = priPath;

        std::call_once(s_mrmHookFlag, []
        {
            s_mrmHookedSuccessfully = SUCCEEDED_LOG(InitializeMrmHooks());
        });

        if (!s_mrmHookedSuccessfully) [[unlikely]]
        {
            s_priFileName = { };

            if (shouldThrowOnHookFailure) [[unlikely]]
            {
                throw hresult_error(E_FAIL, L"Failed to hook Modern Resource Manager (MRM) functions.");
            }
        }

        // note for the future: change this if anything else started to use 
        // shouldThrowOnHookFailure=false other than the default resources.pri path
        if (shouldThrowOnHookFailure) [[unlikely]]
        {
            s_usingDefaultResPri = false;
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

        s_usingDefaultResPri = false;
        s_priTempFile.reset(Helpers::CreateTempFileFromBuffer(priBuffer));
        Start(initCallback, winrt::hstring { (Helpers::GetExecutableFolderPath() / KNOWN_FILE_NAME).wstring() }, true);
    }

    void XamlApplication::Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback)
    {
        s_usingDefaultResPri = true;
        Start(initCallback, winrt::hstring { (Helpers::GetExecutableFolderPath() / L"resources.pri").wstring() }, false);
    }

    winrt::XamlHostingKit::XamlWindow XamlApplication::CreateWindow(winrt::XamlHostingKit::WindowCreationOptions const& options)
    {
        if (!s_hasStarted) [[unlikely]]
        {
            throw hresult_illegal_method_call(L"XamlApplication.CreateWindow() can only be called after XamlApplication.Start().");
        }

        com_ptr<XamlWindow> window = nullptr;
        std::exception_ptr exception = nullptr;
        wil::unique_event windowCreated { wil::EventOptions::ManualReset };

        std::thread([&]()
        {
            try
            {
                winrt::init_apartment(winrt::apartment_type::single_threaded);

                if (CoSetASTATestMode) [[likely]]
                {
                    CoSetASTATestMode(ROINITIALIZEASTA_ALLOWED);
                }

                window = winrt::make_self<XamlWindow>(options, false);
                s_windows.Append(*window.get());

                {
                    std::lock_guard lock(s_mainWindowMutex);
                    if (s_mainWindow == nullptr) [[unlikely]]
                    {
                        s_mainWindow = *window.get();
                    }
                }
            }
            catch (...)
            {
                exception = std::current_exception();
            }

            auto localWindow = window;
            windowCreated.SetEvent();

            if (localWindow) [[likely]]
            {
                localWindow->RunMessageLoop();
            }
        }).detach();

        windowCreated.wait();
        if (exception) [[unlikely]]
        {
            std::rethrow_exception(exception);
        }

        return *window.get();
    }

    winrt::XamlHostingKit::XamlWindow XamlApplication::CreateWindow()
    {
        return CreateWindow({ });
    }

    winrt::XamlHostingKit::XamlWindow XamlApplication::CreateWindow(winrt::XamlHostingKit::WindowCreationOptions const& options, winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& windowCallback)
    {
        auto window = CreateWindow(options);
        window.Dispatcher().RunAsync(CoreDispatcherPriority::Normal, [windowCallback]()
        {
            windowCallback(nullptr);
        });

        return window;
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

        std::lock_guard lock(s_mainWindowMutex);
        s_mainWindow = nullptr;
    }

    void XamlApplication::RemoveWindow(winrt::XamlHostingKit::XamlWindow const& window)
    {
        uint32_t idx = 0;
        if (s_windows.IndexOf(window, idx)) [[likely]]
        {
            s_windows.RemoveAt(idx);

            std::lock_guard lock(s_mainWindowMutex);
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

        IIterator<Package> first{ nullptr };
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
            PACKAGE_INFO_REFERENCE pir{ NULL };
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

    inline HRESULT WINAPI XamlApplication::CreateAppxSecurityManagerHook([[maybe_unused]] void* unk, IWebPlatformSecurityManager** ppManager)
    {
        if (!ppManager) [[unlikely]]
            return E_POINTER;

        *ppManager = winrt::make<DefaultWebPlatformSecurityManager>().detach();
        return S_OK;
    }

    HRESULT WINAPI XamlApplication::CreateAppxProtocolClassFactoryHook(
        [[maybe_unused]] void* unk1,
        [[maybe_unused]] void* unk2,
        [[maybe_unused]] void* unk3,
        [[maybe_unused]] void* unk4,
        IClassFactory** ppFactory)
    {
        if (!ppFactory) [[unlikely]]
        {
            return E_POINTER;
        }

        *ppFactory = winrt::make<ClassFactory<FileProtocolHandler>>().detach();
        return S_OK;
    }

    HRESULT XamlApplication::InitializeWebView()
    {
        if (AppCoreModule &&
            XAMLModule &&
            IERTUtilModule) [[likely]]
        {
            if (FAILED_LOG(Helpers::XWinePatchImport(XAMLModule.get(), AppCoreModule.get(), "AppPolicyGetWindowingModel", &AppPolicyGetWindowingModelHook))) [[unlikely]]
            {
                LOG_IF_FAILED(Helpers::XWinePatchImport(XAMLModule.get(), Kernel32Module, "AppPolicyGetWindowingModel", &AppPolicyGetWindowingModelHook));
            }

            if (Helpers::CurrentPackageFamilyName.empty()) [[likely]]
            {
                if (FAILED_LOG(Helpers::XWinePatchImport(XAMLModule.get(), AppCoreModule.get(), "GetCurrentPackageInfo", &GetCurrentPackageInfoHook))) [[unlikely]]
                {
                    RETURN_IF_FAILED(Helpers::XWinePatchImport(XAMLModule.get(), Kernel32Module, "GetCurrentPackageInfo", &GetCurrentPackageInfoHook));
                }
            }

            RETURN_IF_FAILED(Helpers::XWinePatchImport(XAMLModule.get(), UrlMonModule.get(), MAKEINTRESOURCEA(517), &CreateAppxSecurityManagerHook));
            
            if (FAILED_LOG(Helpers::XWinePatchImport(IERTUtilModule.get(), KernelBaseModule, "GetProcAddress", &GetProcAddressHook))) [[unlikely]]
            {
                RETURN_IF_FAILED(Helpers::XWinePatchImport(IERTUtilModule.get(), Kernel32Module, "GetProcAddress", &GetProcAddressHook));
            }
            
            if (IEConfiguration_SetBrowserAppProfile) [[likely]]
            {
                RETURN_IF_FAILED(IEConfiguration_SetBrowserAppProfile(L"MicrosoftEdge", 2, 0));
            }

            if (Helpers::OSBuild < 15063u && RegisterPermanentUrlRedirectionPolicyManager) [[unlikely]]
            {
                if (GetBrowserTransitionState) [[likely]]
                {
                    auto manager = winrt::make<DefaultUrlRedirectionPolicyManager>();
                    LOG_IF_FAILED(RegisterPermanentUrlRedirectionPolicyManager(manager.get()));
                }
                else if (LCIEGetRedirectionPolicyForURL2)
                {
                    auto manager = winrt::make<OldDefaultUrlRedirectionPolicyManager>();
                    LOG_IF_FAILED(RegisterPermanentUrlRedirectionPolicyManager(reinterpret_cast<IUrlRedirectionPolicyManager*>(manager.get())));
                }
                else if (LCIEGetRedirectionPolicyForURL)
                {
                    auto manager = winrt::make<OlderDefaultUrlRedirectionPolicyManager>();
                    LOG_IF_FAILED(RegisterPermanentUrlRedirectionPolicyManager(reinterpret_cast<IUrlRedirectionPolicyManager*>(manager.get())));
                    goto SetConfig;
                }
                else
                {
                SetConfig:
                    if (IEConfiguration_SetBool) [[likely]]
                    {
                        LOG_IF_FAILED(IEConfiguration_SetBool(0x10000035, true));
                    }
                }
            }

            if (XamlConfig::s_enableMsAppxWebProtocolSupport) [[unlikely]]
            {
                RETURN_IF_FAILED(Helpers::XWinePatchImport(XAMLModule.get(), UrlMonModule.get(), MAKEINTRESOURCEA(505), &CreateAppxProtocolClassFactoryHook));
            }

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
        if (s_usingDefaultResPri && Helpers::OSBuild >= 18362u &&
            SUCCEEDED_LOG(s_originalInitializeForCurrentApplication(_this))) [[likely]]
        {
            return S_OK;
        }

        HMODULE mod;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)_ReturnAddress(), &mod);
        bool isXaml = mod == XAMLModule.get();

        // TODO: should we compare using IUnknown instead?
        if (isXaml && _this == s_mrtManager.get()) [[likely]]
        {
            return S_OK;
        }

        HRESULT hr;
        if (!isXaml || s_priFileName.empty() ||
            FAILED_LOG(hr = _this->InitializeForFile(s_priFileName.c_str()))) [[unlikely]]
        {
            return s_originalInitializeForCurrentApplication(_this);
        }

        return hr;
    }

    HRESULT WINAPI XamlApplication::TryInitializeForCurrentApplicationHook(mrm::IMrtResourceManager2* _this)
    {
        if (s_usingDefaultResPri && Helpers::OSBuild >= 18362u &&
            SUCCEEDED_LOG(s_originalTryInitializeForCurrentApplication(_this))) [[likely]]
        {
            return S_OK;
        }

        HMODULE mod;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)_ReturnAddress(), &mod);
        bool isXaml = mod == XAMLModule.get();

        HRESULT hr;
        winrt::com_ptr<mrm::IMrtResourceManager> manager;
        auto queryFailed = FAILED_LOG(_this->QueryInterface(manager.put()));

        // TODO: should we compare using IUnknown instead?
        if (isXaml && !queryFailed && manager.get() == s_mrtManager.get()) [[likely]]
        {
            return S_OK;
        }

        if (!isXaml || s_priFileName.empty() || queryFailed ||
            FAILED_LOG(hr = manager->InitializeForFile(s_priFileName.c_str()))) [[unlikely]]
        {
            return s_originalTryInitializeForCurrentApplication(_this);
        }

        return hr;
    }

    HRESULT WINAPI XamlApplication::CoCreateInstanceHook(REFCLSID rclsid, ::IUnknown* pUnkOuter, DWORD dwClsContext, REFIID riid, void** ppv)
    {
        if (rclsid == __uuidof(mrm::MrtResourceManager))
        {
            if ((!s_usingDefaultResPri || Helpers::OSBuild < 18362u) && s_mrtManager && SUCCEEDED_LOG(s_mrtManager->QueryInterface(riid, ppv))) [[unlikely]]
            {
                return S_OK;
            }
        }

        return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
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
            RETURN_IF_FAILED(Helpers::XWinePatchImport(XAMLModule.get(), COMBaseModule, "CoCreateInstance", &CoCreateInstanceHook));

            return S_OK;
        }

        return E_FAIL;
    }

    void XamlApplication::CreateResourceManager(PCWSTR priPath)
    {
        if (!Helpers::CurrentPackageFamilyName.empty() && s_usingDefaultResPri) [[unlikely]]
        {
            try
            {
                if (s_resourceManager = warc::ResourceManager::Current()) [[likely]]
                    return;
            }
            catch (...) { }
        }

        winrt::com_ptr<IResourceManagerStaticInternal> mgrStatics;
        if (!(mgrStatics = winrt::try_get_activation_factory<warc::ResourceManager, IResourceManagerStaticInternal>()) ||
            FAILED_LOG(mgrStatics->GetCurrentResourceManagerForSystemProfile(winrt::put_abi(s_resourceManager))) ||
            !s_resourceManager) [[unlikely]]
        {
            winrt::com_ptr<IResourceManagerStaticInternalOld> mgrStaticsOld;
            if (!(mgrStaticsOld = try_get_activation_factory<warc::ResourceManager, IResourceManagerStaticInternalOld>()) ||
                FAILED_LOG(mgrStaticsOld->GetCurrentResourceManagerForSystemProfile(winrt::put_abi(s_resourceManager))) ||
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
        else
        {
            auto const filters8 = PACKAGE_FILTER_HEAD | PACKAGE_FILTER_DIRECT;
            auto const filters81 = filters8 | PACKAGE_FILTER_RESOURCE | PACKAGE_FILTER_BUNDLE;
            auto const filters1607 = filters81 | PACKAGE_FILTER_OPTIONAL;
            auto const filters2004 = filters1607 | PACKAGE_FILTER_STATIC | PACKAGE_FILTER_DYNAMIC;
            auto const filters11 = filters2004 | PACKAGE_FILTER_HOSTRUNTIME;

            uint32_t count = 0;
            uint32_t length = 0;
            uint32_t filters = 0;
            std::unique_ptr<BYTE[]> buffer;
            PACKAGE_INFO* packageInfo = nullptr;
            if (GetCurrentPackageInfo2Method) [[likely]]
            {
                if (GetCurrentPackageInfo2Method(filters = filters11, PackagePathType_Effective, &length, nullptr, &count) == ERROR_INSUFFICIENT_BUFFER /*||
                    GetCurrentPackageInfo2Method(filters = filters2004, PackagePathType_Effective, &length, nullptr, &count) == ERROR_INSUFFICIENT_BUFFER ||
                    GetCurrentPackageInfo2Method(filters = filters1607, PackagePathType_Effective, &length, nullptr, &count) == ERROR_INSUFFICIENT_BUFFER*/) [[likely]]
                {
                    buffer = std::make_unique<BYTE[]>(length);
                    packageInfo = reinterpret_cast<PACKAGE_INFO*>(buffer.get());
                    if (LOG_IF_WIN32_ERROR(GetCurrentPackageInfo2Method(filters, PackagePathType_Effective, &length, buffer.get(), &count)) != ERROR_SUCCESS) [[unlikely]]
                    {
                        LOG_HR_MSG(E_FAIL, "Failed to get package info for the current process.");
                        return;
                    }
                }
            }

            if (!packageInfo) [[unlikely]]
            {
                count = 0;
                length = 0;
                filters = 0;
                if (GetCurrentPackageInfo(filters = filters1607, &length, nullptr, &count) == ERROR_INSUFFICIENT_BUFFER /*||
                    GetCurrentPackageInfo(filters = filters81, &length, nullptr, &count) == ERROR_INSUFFICIENT_BUFFER     ||
                    GetCurrentPackageInfo(filters = filters8, &length, nullptr, &count) == ERROR_INSUFFICIENT_BUFFER*/) [[likely]]
                {
                    buffer = std::make_unique<BYTE[]>(length);
                    packageInfo = reinterpret_cast<PACKAGE_INFO*>(buffer.get());
                    if (LOG_IF_WIN32_ERROR(GetCurrentPackageInfo(filters, &length, buffer.get(), &count)) != ERROR_SUCCESS) [[unlikely]]
                    {
                        LOG_HR_MSG(E_FAIL, "Failed to get package info for the current process.");
                        return;
                    }
                }
            }

            for (uint32_t i = 0; i < count; ++i)
            {
                if (packageInfo[i].path) [[likely]]
                {
                    if (!Helpers::CurrentPackageFamilyName.empty() &&
                        _wcsicmp(packageInfo[i].packageFamilyName, Helpers::CurrentPackageFamilyName.c_str()) == 0) [[unlikely]]
                    {
                        continue;
                    }

                    auto path = std::filesystem::path { packageInfo[i].path };
                    path /= L"resources.pri";

                    LOG_IF_FAILED(mgrEx->LoadPriFileForSystemUse(path.c_str()));
                }
            }

            winrt::com_ptr<ISystemResourceManagerExtensions> ext;
            if (!LOG_HR_IF(E_NOINTERFACE, !s_resourceManager.try_as(ext))) [[likely]]
            {
                winrt::com_ptr<::IInspectable> mgri;
                if (SUCCEEDED_LOG(ext->GetMrtResourceManagerForResourceManager(mgri.put()))) [[likely]]
                {
                    LOG_IF_FAILED(mgri->QueryInterface(s_mrtManager.put()));
                }
            }
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

        std::lock_guard lock(s_mainWindowMutex);
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