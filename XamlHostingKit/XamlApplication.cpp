#include "pch.h"
#include "XamlApplication.h"
#include "XamlApplication.g.cpp"

namespace winrt::XamlHostingKit::implementation
{
    winrt::XamlHostingKit::XamlWindow XamlApplication::MainWindow()
    {
        throw hresult_not_implemented();
    }

    winrt::Windows::Foundation::Collections::IVectorView<winrt::XamlHostingKit::XamlWindow> XamlApplication::Windows()
    {
        throw hresult_not_implemented();
    }

    void XamlApplication::Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback)
    {
        throw hresult_not_implemented();
    }

    void XamlApplication::Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback, hstring const& priPath)
    {
        throw hresult_not_implemented();
    }

    void XamlApplication::Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback, winrt::Windows::Storage::Streams::IBuffer const& priBuffer)
    {
        throw hresult_not_implemented();
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
        throw hresult_not_implemented();
    }

    bool XamlApplication::IsWebViewAvailable()
    {
        throw hresult_not_implemented();
    }

    void XamlApplication::RemoveWindow(winrt::XamlHostingKit::XamlWindow const& window)
    {
        throw hresult_not_implemented();
    }
}
