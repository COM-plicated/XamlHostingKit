# XamlHostingKit

XamlHostingKit is a System XAML (aka UWP XAML aka Windows XAML) hosting library for Desktop applications with smooth resizing, WebView support, backwards compatibility down to Windows 8.1, and more!

## Installation

A NuGet package is in the works.

## Features

### Broad Windows Version Support

#### Windows 11

<img width="1920" height="1080" alt="Windows 11" src="https://github.com/user-attachments/assets/333bdf8d-6825-4ce4-94b5-1ef00f9580bd" />

#### Windows 10 RTM (Build 10240)

<img width="1727" height="1014" alt="Windows 10 RTM" src="https://github.com/user-attachments/assets/9cd3b161-4323-47f5-aad9-ad0e022a6506" />

#### Windows 8.1 (Build 9600)

<img width="1660" height="1044" alt="Windows 8.1" src="https://github.com/user-attachments/assets/16e77ded-fa61-4dd2-9ce9-546a12a5b6d2" />

### Smooth Resizing Support (Build 15063+) *[Enabled By Default -> `XamlConfig.EnableSmoothResize`]*

https://github.com/user-attachments/assets/cb654547-d3b7-442a-848c-250bab53d760

### WebView (Edge Legacy) Support *[Enabled By Default -> `XamlConfig.EnableWebView`]*

<img width="1424" height="776" alt="WebView (Edge Legacy)" src="https://github.com/user-attachments/assets/f3acfd8b-f15a-4cec-b192-31a8dc1852de" />

The video seen in images above is also playing inside a WebView.

> [!NOTE]
> - WebView2 is also supported.
> - WebView uses **Immersive Internet Explorer** (**Trident**) when running on Windows 8.1 instead of **Edge Legacy**.

### Other Miscellaneous Features

- [Touchpad Awareness](https://learn.microsoft.com/en-us/windows/win32/input-precisiontouchpad/precision-touchpad-portal) support (on all supported builds, including Windows 8.1) ***[Enabled By Default -> `XamlConfig.EnableTouchpadAwareness`]***
- `ms-appx-web:///` protocol support ***[Disabled By Default -> `XamlConfig.EnableMsAppxWebProtocolSupport`]***
- Arbitrary paths support for the `ms-appx-web:///` protocol ***[Disabled By Default -> `XamlConfig.EnableArbitraryPathsInMsAppxWeb`]***
- Removal of the GDI redirection layer, which allows performant/efficient window transparency ***[Enabled By Default -> `XamlConfig.DisableRedirectionLayer`]***

## Samples

You can find a simple XamlHostingKit C# sample [here](https://github.com/COM-plicated/XamlHostingKit/tree/master/Samples/XamlHostingKit.ManagedSample).

We will be adding more samples in the future.

## Notes

> [!WARNING]
> The `Windows.UI.Composition` API is currently not usable on **Build 14393 (Windows 10 Anniversary Update: 1607)** and older unless the process is running under **AppContainer**, we are working on a solution.
