#pragma once

#include <filesystem>

namespace winrt::XamlHostingKit
{
    class Helpers
    {
    private:

        inline static winrt::hstring const& __GetExecutableName()
        {
            wchar_t path[MAX_PATH];
            GetModuleFileNameW(GetModuleHandleW(nullptr), path, MAX_PATH);

            std::filesystem::path exePath(path);
            return winrt::hstring { exePath.stem().wstring() };
        }

    public:

        inline static winrt::hstring const& GetExecutableName()
        {
            static winrt::hstring const& exeName = __GetExecutableName();
            return exeName;
        }
    };
}