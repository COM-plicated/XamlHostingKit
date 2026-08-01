#pragma once

#include "XamlApplication.g.h"

namespace winrt::XamlHostingKit::implementation
{
    struct XamlApplication : XamlApplicationT<XamlApplication>
    {
        XamlApplication() = default;

        static winrt::XamlHostingKit::XamlWindow MainWindow();
        static winrt::Windows::Foundation::Collections::IVectorView<winrt::XamlHostingKit::XamlWindow> Windows();
        
        static void Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback);
        static void Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback, hstring const& priPath);
        static void Start(winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& initCallback, winrt::Windows::Storage::Streams::IBuffer const& priBuffer);
        
        static winrt::XamlHostingKit::XamlWindow CreateWindow();
        static winrt::XamlHostingKit::XamlWindow CreateWindow(winrt::XamlHostingKit::WindowCreationOptions const& options);
        static winrt::XamlHostingKit::XamlWindow CreateWindow(winrt::XamlHostingKit::WindowCreationOptions const& options, winrt::Windows::UI::Xaml::ApplicationInitializationCallback const& windowCallback);
        
        static void Close();
    };
}

namespace winrt::XamlHostingKit::factory_implementation
{
    struct XamlApplication : XamlApplicationT<XamlApplication, implementation::XamlApplication>
    {
    };
}
