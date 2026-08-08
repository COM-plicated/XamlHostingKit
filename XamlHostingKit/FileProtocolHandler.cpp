#include "pch.h"
#include "FileProtocolHandler.h"
#include "Helpers.h"
#include "XamlConfig.h"

#include <shlwapi.h>
#include <wininet.h>
#include <wil/com.h>
#include <wil/result.h>

namespace winrt::XamlHostingKit::implementation
{
    std::wstring_view FileProtocolHandler::StripUrlDecorations(std::wstring_view url)
    {
        if (auto const cut = url.find_first_of(L"?#"); cut != std::wstring_view::npos) [[unlikely]]
        {
            url = url.substr(0, cut);
        }

        return url;
    }

    std::wstring FileProtocolHandler::UnescapeUrl(std::wstring_view const& value)
    {
        std::wstring buffer{ value };
        DWORD length = static_cast<DWORD>(buffer.size() + 1);
        buffer.resize(length);

        if (SUCCEEDED(UrlUnescapeW(buffer.data(), nullptr, &length, URL_UNESCAPE_INPLACE))) [[likely]]
        {
            buffer.resize(wcsnlen(buffer.c_str(), buffer.size()));
        }

        return buffer;
    }

    const bool FileProtocolHandler::IsWithin(std::filesystem::path const& root, std::filesystem::path const& candidate)
    {
        auto const rootString = root.native();
        auto const candidateString = candidate.native();

        return candidateString.size() >= rootString.size() &&
            CompareStringOrdinal(candidateString.c_str(),
                static_cast<int>(rootString.size()),
                rootString.c_str(),
                static_cast<int>(rootString.size()),
                TRUE) == CSTR_EQUAL &&
            candidateString.find(L"\\..\\") == std::wstring::npos;
    }

    std::filesystem::path FileProtocolHandler::ResolveLocalPath(std::wstring_view url)
    {
        auto trimmed = StripUrlDecorations(url);

        if (trimmed.size() >= ProtocolPrefix.size() &&
            CompareStringOrdinal(trimmed.data(),
                static_cast<int>(ProtocolPrefix.size()),
                ProtocolPrefix.data(),
                static_cast<int>(ProtocolPrefix.size()),
                TRUE) == CSTR_EQUAL) [[likely]]
        {
            trimmed.remove_prefix(ProtocolPrefix.size());
        }
        else
        {
            return { };
        }

        trimmed.remove_prefix(trimmed.starts_with(L"//") ? 2 : 0);
        trimmed.remove_prefix(trimmed.starts_with(L'/')  ? 1 : 0);

        auto converted = UnescapeUrl(trimmed);
        std::replace(converted.begin(), converted.end(), L'/', L'\\');

        std::filesystem::path path { converted };

        if (path.has_root_name()) [[unlikely]]
        {
            return XamlConfig::s_enableArbitraryPathsInMsAppxWeb ?
                path.lexically_normal() : std::filesystem::path { };
        }

        auto const root = Helpers::GetExecutableFolderPath().lexically_normal();
        auto const combined = (root / path.relative_path()).lexically_normal();

        if (!XamlConfig::s_enableArbitraryPathsInMsAppxWeb && !IsWithin(root, combined)) [[unlikely]]
        {
            return { };
        }

        return combined;
    }

    ::IUri* FileProtocolHandler::ConvertToFileProtocol(std::filesystem::path const& path)
    {
        ::IUri* pUri = nullptr;
        LOG_IF_FAILED(CreateUri(
            path.c_str(),
            Uri_CREATE_ALLOW_IMPLICIT_FILE_SCHEME | Uri_CREATE_NO_DECODE_EXTRA_INFO | Uri_CREATE_IE_SETTINGS,
            0,
            &pUri
        ));

        return pUri;
    }

    IFACEMETHODIMP FileProtocolHandler::Start(LPCWSTR szUrl, IInternetProtocolSink* pSink, IInternetBindInfo* pOIProtSink, DWORD grfPI, HANDLE_PTR dwReserved) noexcept
    {
        RETURN_HR_IF_NULL(E_INVALIDARG, szUrl);
        RETURN_HR_IF_NULL(E_INVALIDARG, pSink);

        auto path = ResolveLocalPath(szUrl);
        if (path.empty()) [[unlikely]]
        {
            return INET_E_RESOURCE_NOT_FOUND;
        }

        RETURN_HR(StartEx(ConvertToFileProtocol(path), pSink, pOIProtSink, grfPI, dwReserved));
    }

    IFACEMETHODIMP FileProtocolHandler::Continue(PROTOCOLDATA* pData) noexcept
    {
        RETURN_HR_IF_NULL(INET_E_DEFAULT_ACTION, m_fileProtocol);
        return m_fileProtocol->Continue(pData);
    }

    IFACEMETHODIMP FileProtocolHandler::Abort(HRESULT hrReason, DWORD dwOptions) noexcept
    {
        RETURN_HR_IF_NULL(INET_E_DEFAULT_ACTION, m_fileProtocol);
        return m_fileProtocol->Abort(hrReason, dwOptions);
    }

    IFACEMETHODIMP FileProtocolHandler::Terminate(DWORD dwOptions) noexcept
    {
        RETURN_HR_IF_NULL(INET_E_DEFAULT_ACTION, m_fileProtocol);
        return m_fileProtocol->Terminate(dwOptions);
    }

    IFACEMETHODIMP FileProtocolHandler::Suspend() noexcept
    {
        RETURN_HR_IF_NULL(INET_E_DEFAULT_ACTION, m_fileProtocol);
        return m_fileProtocol->Suspend();
    }

    IFACEMETHODIMP FileProtocolHandler::Resume() noexcept
    {
        RETURN_HR_IF_NULL(INET_E_DEFAULT_ACTION, m_fileProtocol);
        return m_fileProtocol->Resume();
    }

    IFACEMETHODIMP FileProtocolHandler::Read(void* pv, ULONG cb, ULONG* pcbRead) noexcept
    {
        RETURN_HR_IF_NULL(INET_E_DEFAULT_ACTION, m_fileProtocol);
        RETURN_HR(m_fileProtocol->Read(pv, cb, pcbRead));
    }

    IFACEMETHODIMP FileProtocolHandler::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) noexcept
    {
        RETURN_HR_IF_NULL(INET_E_DEFAULT_ACTION, m_fileProtocol);
        return m_fileProtocol->Seek(dlibMove, dwOrigin, plibNewPosition);
    }

    IFACEMETHODIMP FileProtocolHandler::LockRequest(DWORD dwOptions) noexcept
    {
        RETURN_HR_IF_NULL(INET_E_DEFAULT_ACTION, m_fileProtocol);
        return m_fileProtocol->LockRequest(dwOptions);
    }

    IFACEMETHODIMP FileProtocolHandler::UnlockRequest() noexcept
    {
        RETURN_HR_IF_NULL(INET_E_DEFAULT_ACTION, m_fileProtocol);
        return m_fileProtocol->UnlockRequest();
    }

    IFACEMETHODIMP FileProtocolHandler::StartEx(IUri* pUri, IInternetProtocolSink* pOIProtSink, IInternetBindInfo* pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved) noexcept
    {
        RETURN_HR_IF_NULL(E_INVALIDARG, pUri);
        RETURN_HR_IF_NULL(E_INVALIDARG, pOIProtSink);
        RETURN_IF_FAILED(UrlmonCreateInstance(CLSID_FileProtocol, nullptr, IID_PPV_ARGS(m_fileProtocol.put())));
        RETURN_HR_IF_NULL(E_FAIL, m_fileProtocol);

        wil::unique_bstr url;
        RETURN_IF_FAILED(pUri->GetAbsoluteUri(url.put()));

        auto sink = winrt::make<SinkWrapper>(pOIProtSink);
        auto path = ResolveLocalPath(url.get());
        if (path.empty()) [[unlikely]]
        {
            RETURN_HR(m_fileProtocol->StartEx(pUri, sink.get(), pOIBindInfo, grfPI, dwReserved));
        }

        RETURN_HR(m_fileProtocol->StartEx(ConvertToFileProtocol(path), sink.get(), pOIBindInfo, grfPI, dwReserved));
    }
}