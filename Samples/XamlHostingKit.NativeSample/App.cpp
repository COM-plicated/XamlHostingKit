#include "pch.h"
//#include <xhk/xhk.h> // uncomment when using XamlHostingKit.Static[.DynCRT]

#include "App.h"
#include "MainPage.h"

using namespace winrt;
using namespace XamlHostingKit;
using namespace Windows::UI::Xaml;
using namespace Windows::Foundation;
using namespace Windows::ApplicationModel;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::ApplicationModel::Activation;

namespace winrt::XamlHostingKit::NativeSample::implementation
{
    /// <summary>
    /// Creates the singleton application object.  This is the first line of authored code
    /// executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    App::App()
    {
    #if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
        UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
            {
                if (IsDebuggerPresent())
                {
                    auto errorMessage = e.Message();
                    __debugbreak();
                }
            });
    #endif
    }

    /// <summary>
    /// Invoked when the application is launched normally by the end user.  Other entry points
    /// will be used such as when the application is launched to open a specific file.
    /// </summary>
    /// <param name="e">Details about the launch request and process.</param>
    void App::OnLaunched(LaunchActivatedEventArgs const& e)
    {
        auto window = XamlWindow::Current();
        window.Title(L"XamlHostingKit Native Sample");

        Frame frame;
        frame.Navigate(xaml_typename<NativeSample::MainPage>());
        window.Content(frame);

        window.Show();
    }
}

int WINAPI wWinMain(
    [[maybe_unused]] _In_ HINSTANCE hInstance,
    [[maybe_unused]] _In_opt_ HINSTANCE hPrevInstance,
    [[maybe_unused]] _In_ LPWSTR lpCmdLine,
    [[maybe_unused]] _In_ int nShowCmd
)
{
    winrt::init_apartment(apartment_type::single_threaded);

    XamlApplication::Start(
    [](auto&&)
    {
        winrt::make<NativeSample::implementation::App>();
    });

    return 0;
}