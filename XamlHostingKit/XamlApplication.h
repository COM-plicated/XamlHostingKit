#pragma once

#include "XamlApplication.g.h"
#include <appmodel.h>
#include <wil/resource.h>
#include "Helpers.h"
#include "mrm_private.h"
#include <winrt/Windows.ApplicationModel.Resources.Core.h>
#include <threadpoolapiset.h>
#include <mutex>

namespace winrt::XamlHostingKit::implementation
{
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::ApplicationModel::Resources::Core;
    namespace mrm = ABI::Windows::ApplicationModel::Resources::Core;
    namespace warc = winrt::Windows::ApplicationModel::Resources::Core;

    struct XamlApplication
    {
    private:
        static const constexpr PCWSTR KNOWN_FILE_NAME = L"__XHK_EMBEDDED_RESOURCES__.pri";

        inline static bool s_hasStarted { false };
        inline static bool s_propsFilled { false };
        inline static bool s_usingDefaultResPri { false };
        inline static bool s_isWebViewAvailable { false };
        inline static bool s_mrmHookedSuccessfully { false };
        inline static wil::unique_handle s_priTempFile { nullptr };
        inline static std::wstring s_priFileName { };
        inline static winrt::XamlHostingKit::XamlWindow s_mainWindow { nullptr };
        inline static ResourceManager s_resourceManager { nullptr };
        inline static winrt::com_ptr<mrm::IMrtResourceManager> s_mrtManager { nullptr };
        inline static IVector<winrt::XamlHostingKit::XamlWindow> s_windows { winrt::multi_threaded_vector<winrt::XamlHostingKit::XamlWindow>() };
        inline static HRESULT(WINAPI* s_originalInitializeForCurrentApplication)(mrm::IMrtResourceManager* _this) { nullptr };
        inline static HRESULT(WINAPI* s_originalTryInitializeForCurrentApplication)(mrm::IMrtResourceManager2* _this) { nullptr };
        inline static std::once_flag s_appInitFlag { };
        inline static std::once_flag s_mrmHookFlag { };
        inline static std::mutex s_mainWindowMutex { };

        static HRESULT InitializeWebView();
        static void StartCommon(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback);

        static winrt::hstring FindSettingsPackageFullname();
        static FARPROC WINAPI GetProcAddressHook(HMODULE hModule, _In_ LPCSTR lpProcName);
        static BOOL WINAPI IsImmersiveProcessHook(void* unk);
        static LONG WINAPI GetCurrentPackageInfoHook(_In_ const UINT32 flags, _Inout_ UINT32* bufferLength, _Out_opt_ BYTE* buffer, _Out_opt_ UINT32* count);
        static LONG WINAPI AppPolicyGetWindowingModelHook(HANDLE processToken, AppPolicyWindowingModel* policy);
        static HRESULT WINAPI CreateAppxSecurityManagerHook(void* unk, IWebPlatformSecurityManager** ppManager);
        static HRESULT WINAPI CreateAppxProtocolClassFactoryHook(void* unk1, void* unk2, void* unk3, void* unk4, IClassFactory** ppFactory);
        static HANDLE WINAPI CreateFileWHook(_In_ LPCWSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile);
        static BOOL WINAPI GetFileAttributesExWHook(_In_ LPCWSTR lpFileName, _In_ GET_FILEEX_INFO_LEVELS fInfoLevelId, _Out_writes_bytes_(sizeof(WIN32_FILE_ATTRIBUTE_DATA)) LPVOID lpFileInformation);
        static HRESULT WINAPI InitializeForCurrentApplicationHook(mrm::IMrtResourceManager* _this);
        static HRESULT WINAPI TryInitializeForCurrentApplicationHook(mrm::IMrtResourceManager2* _this);
        static HRESULT WINAPI CoCreateInstanceHook(REFCLSID rclsid, ::IUnknown* pUnkOuter, DWORD dwClsContext, REFIID riid, void** ppv);
        static HRESULT InitializeMrmHooks();
        static void CreateResourceManager(PCWSTR priPath);
        static HRESULT WINAPI get_ViewsHook(void* _this, void** pViews);
        static HRESULT WINAPI get_MainViewHook(void* _this, void** pView);
        static HRESULT InitializeCoreApplicationHooks();
        inline static void WINAPI SubmitThreadpoolWorkHook([[maybe_unused]] PTP_WORK work) { };

    public:
        XamlApplication() = default;

        static bool IsWebViewAvailable();
        static ResourceManager ResourceManager();

        static winrt::XamlHostingKit::XamlWindow MainWindow();
        static winrt::Windows::Foundation::Collections::IVectorView<winrt::XamlHostingKit::XamlWindow> Windows();
        
        static void Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback);
        static void Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback, hstring const& priPath, bool shouldThrowOnHookFailure = true);
        static void Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback, winrt::Windows::Storage::Streams::IBuffer const& priBuffer);
        
        static winrt::XamlHostingKit::XamlWindow CreateWindow();
        static winrt::XamlHostingKit::XamlWindow CreateWindow(winrt::XamlHostingKit::WindowCreationOptions const& options);
        static winrt::XamlHostingKit::XamlWindow CreateWindow(winrt::XamlHostingKit::WindowCreationOptions const& options, winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& windowCallback);
        
        static void Close();

        static void RemoveWindow(winrt::XamlHostingKit::XamlWindow const& window);
    };
}

namespace winrt::XamlHostingKit::factory_implementation
{
    struct XamlApplication : XamlApplicationT<XamlApplication, implementation::XamlApplication>
    {
    };
}
