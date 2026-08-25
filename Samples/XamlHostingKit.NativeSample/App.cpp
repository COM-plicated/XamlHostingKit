#include "pch.h"
#include <xhk/xhk.h>

#include "App.h"
#include "MainPage.h"

using namespace winrt;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

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
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    winrt::XamlHostingKit::XamlApplication::Start(
    [](auto&&)
    {
        winrt::make<winrt::XamlHostingKit::NativeSample::implementation::App>();
    });

    return 0;
}