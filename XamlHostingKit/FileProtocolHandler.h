#pragma once

#include <urlmon.h>
#include <filesystem>
#include <wil/resource.h>
#include <winrt/Windows.Foundation.h>

namespace winrt::XamlHostingKit::implementation
{
    struct FileProtocolHandler : winrt::implements<FileProtocolHandler, IInternetProtocolEx, IInternetProtocol, IInternetProtocolRoot>
    {
    private:
        inline static const constexpr std::wstring_view ProtocolPrefix { L"ms-appx-web:" };

        winrt::com_ptr<IInternetProtocolEx> m_fileProtocol;

        static std::wstring_view StripUrlDecorations(std::wstring_view url);
        static std::wstring UnescapeUrl(std::wstring_view const& value);
        static const bool IsWithin(std::filesystem::path const& root, std::filesystem::path const& candidate);
        static std::filesystem::path ResolveLocalPath(std::wstring_view url);
        static ::IUri* ConvertToFileProtocol(std::filesystem::path const& path);

    public:
        FileProtocolHandler() noexcept = default;

        IFACEMETHODIMP Start(LPCWSTR szUrl, IInternetProtocolSink* pSink, IInternetBindInfo* pBindInfo, DWORD grfPI, HANDLE_PTR dwReserved) noexcept override;
        IFACEMETHODIMP Continue(PROTOCOLDATA* pProtocolData) noexcept override;
        IFACEMETHODIMP Abort(HRESULT hrReason, DWORD dwOptions) noexcept override;
        IFACEMETHODIMP Terminate(DWORD dwOptions) noexcept override;
        IFACEMETHODIMP Suspend() noexcept override;
        IFACEMETHODIMP Resume() noexcept override;

        IFACEMETHODIMP Read(void* pv, ULONG cb, ULONG* pcbRead) noexcept override;
        IFACEMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) noexcept override;
        IFACEMETHODIMP LockRequest(DWORD dwOptions) noexcept override;
        IFACEMETHODIMP UnlockRequest() noexcept override;

        IFACEMETHODIMP StartEx(IUri* pUri, IInternetProtocolSink* pOIProtSink, IInternetBindInfo* pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved) noexcept override;
    };

    struct SinkWrapper : winrt::implements<SinkWrapper, IInternetProtocolSink>
    {
    private:
        bool m_dataReported { };
        wil::unique_bstr mimeType;
        winrt::com_ptr<IInternetProtocolSink> m_sink;

    public:
        inline SinkWrapper(IInternetProtocolSink* sink) { m_sink.copy_from(sink); }

        inline IFACEMETHODIMP Switch(PROTOCOLDATA* pProtocolData) noexcept override
        {
            return m_sink->Switch(pProtocolData);
        }

        inline IFACEMETHODIMP ReportProgress(ULONG ulStatusCode, LPCWSTR szStatusText) noexcept override
        {
            if (ulStatusCode == BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE)
            {
                mimeType = wil::make_bstr(szStatusText);
            }

            RETURN_HR(m_sink->ReportProgress(ulStatusCode, szStatusText));
        }

        inline HRESULT OnResponse(uint32_t ulStatusCode, LPCWSTR response)
        {
            winrt::com_ptr<IServiceProvider> serviceProvider;
            RETURN_IF_FAILED(m_sink->QueryInterface(serviceProvider.put()));

            winrt::com_ptr<IHttpNegotiate> httpNegotiate;
            RETURN_IF_FAILED(serviceProvider->QueryService(__uuidof(IHttpNegotiate), IID_PPV_ARGS(httpNegotiate.put())));

            wil::unique_bstr additionalHeaders;
            RETURN_HR(httpNegotiate->OnResponse(ulStatusCode, response, nullptr, additionalHeaders.put()));
        }

        inline IFACEMETHODIMP ReportData(DWORD grfBSCF, ULONG ulProgress, ULONG ulProgressMax) noexcept override
        {
            if (!m_dataReported)
            {
                std::wstring header = mimeType.is_valid() ?
                    std::format(L"HTTP/1.1 200 OK\r\nContent-Type: {}\r\nContent-Length: {}\r\n\r\n", mimeType.get(), ulProgressMax) :
                    std::format(L"HTTP/1.1 200 OK\r\nContent-Length: {}\r\n\r\n", ulProgressMax);

                m_dataReported = true;
                RETURN_IF_FAILED(OnResponse(200, header.c_str()));
            }

            RETURN_HR(m_sink->ReportData(grfBSCF, ulProgress, ulProgressMax));
        }

        inline IFACEMETHODIMP ReportResult(HRESULT hrResult, DWORD dwError, LPCWSTR szResult) noexcept override
        {
            if (FAILED_LOG(hrResult))
            {
                RETURN_IF_FAILED(hrResult == INET_E_INVALID_REQUEST ?
                    OnResponse(405, L"HTTP/1.1 405 Method not allowed\r\n\r\n") :
                    OnResponse(500, L"HTTP/1.1 500 Error\r\n\r\n"));
            }

            RETURN_HR(m_sink->ReportResult(S_OK, dwError, szResult));
        }
    };
}