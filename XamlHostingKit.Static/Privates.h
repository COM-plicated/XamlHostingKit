#pragma once

#include <winternl.h>

enum WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_USEDARKMODECOLORS = 26,
    WCA_CORNER_STYLE = 27,
    WCA_PART_COLOR = 28,
    WCA_DISABLE_MOVESIZE_FEEDBACK = 29,
    WCA_SYSTEMBACKDROP_TYPE = 30,
    WCA_SET_TAGGED_WINDOW_RECT = 31,
    WCA_CLEAR_TAGGED_WINDOW_RECT = 32,
    WCA_LAST = 33,
};

struct WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    void* pvData;
    unsigned int cbData;
};

enum ACCENT_STATE
{
    ACCENT_DISABLED,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
};

struct ACCENT_POLICY
{
    ACCENT_STATE AccentState;
    ULONG AccentFlags;
    ULONG GradientColor;
    ULONG AnimationId;
};

enum class PreferredAppMode
{
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};

enum CoreWindowType
{
    IMMERSIVE_BODY = 0,
    IMMERSIVE_DOCK,
    IMMERSIVE_HOSTED,
    IMMERSIVE_TEST,
    IMMERSIVE_BODY_ACTIVE,
    IMMERSIVE_DOCK_ACTIVE,
    NOT_IMMERSIVE
};

enum ASTA_TEST_MODE_FLAGS : unsigned int
{
    NONE = 0x0,
    RO_INIT_SINGLETHREADED_CREATES_ASTAS = 0x1,
    GIT_LIFETIME_EXTENSION_ENABLED = 0x2,
    ROINITIALIZEASTA_ALLOWED = 0x4,
};

DEFINE_ENUM_FLAG_OPERATORS(ASTA_TEST_MODE_FLAGS);

struct WindowCreationParameters
{
    int Left, Top, Width, Height;
    char TransparentBackground, IsCoreNavigationClient;
};

// References: https://gist.github.com/diversenok/930600b5aec5e8d15664662b9176a691, https://ntdoc.m417z.com/peb

struct SWITCH_CONTEXT_ATTRIBUTE
{
    uint64_t ContextUpdateCounter;
    BOOL AllowContextUpdate;
    BOOL EnableTrace;
    uint64_t EtwHandle;
};

struct SWITCH_CONTEXT_DATA
{
    uint64_t OsMaxVersionTested;
    uint32_t TargetPlatform;
    uint64_t ContextMinimum;
    GUID Platform;
    GUID MinPlatform;
    uint32_t ContextSource;
    uint32_t ElementCount;
    GUID Elements[48];
};

struct SWITCH_CONTEXT
{
    SWITCH_CONTEXT_ATTRIBUTE Attribute;
    SWITCH_CONTEXT_DATA Data;
};

struct SDBQUERYRESULT
{
    uint32_t Exes[16];
    uint32_t ExeFlags[16];
    uint32_t Layers[8];
    uint32_t LayerFlags;
    uint32_t AppHelp;
    uint32_t ExeCount;
    uint32_t LayerCount;
    GUID ID;
    uint32_t ExtraFlags;
    uint32_t CustomSDBMap;
    GUID DB[16];
};

struct APPCOMPAT_EXE_DATA_EIGHT
{
    uint16_t ShimEngine[MAX_PATH];
    uint32_t Size;
    uint32_t Magic;
    uint16_t ExeType;
    SDBQUERYRESULT SdbQueryResult;
    char DbgLogChannels[1024];
    SWITCH_CONTEXT SwitchContext; // uint64_t[128]
};

struct APPCOMPAT_EXE_DATA_TH1
{
    uint16_t ShimEngine[MAX_PATH];
    uint32_t Size;
    uint32_t Magic;
    uint16_t ExeType;
    SDBQUERYRESULT SdbQueryResult;
    char DbgLogChannels[1024];
    SWITCH_CONTEXT SwitchContext; // uint64_t[128]
};

struct APPCOMPAT_EXE_DATA_RS2
{
    uint32_t Size;
    uint32_t Magic;
    BOOL LoadShimEngine;
    uint16_t ExeType;
    SDBQUERYRESULT SdbQueryResult;
    char DbgLogChannels[1024];
    SWITCH_CONTEXT SwitchContext;
};

struct APPCOMPAT_EXE_DATA
{
    uint64_t Reserved[65];
    uint32_t Size;
    uint32_t Magic;
    BOOL LoadShimEngine;
    uint16_t ExeType;
    SDBQUERYRESULT SdbQueryResult;
    char DbgLogChannels[1024];
    SWITCH_CONTEXT SwitchContext;
};

inline static const constexpr uintptr_t OffsetOfAppCompatShimData()
{
    auto const constexpr padding = sizeof(void*) - sizeof(uint32_t);
    return (uintptr_t)(FIELD_OFFSET(PEB, SessionId) + sizeof(uint32_t) + padding + (2 * sizeof(uint64_t)));
    //                                              + SessionId        + Padding + (AppCompatFlags + AppCompatFlagsUser)
}

inline static const constexpr auto OFFSET_OF_SHIM_DATA = OffsetOfAppCompatShimData();

inline static const constexpr GUID Windows8_PlatformID = { 0x4a2f28e3, 0x53b9, 0x4441, { 0xba, 0x9c, 0xd6, 0x9d, 0x4a, 0x4a, 0x6e, 0x38 } };
inline static const constexpr GUID WindowsBlue_PlatformID = { 0x1f676c76, 0x80e1, 0x4239, { 0x95, 0xbb, 0x83, 0xd0, 0xf6, 0xd0, 0xda, 0x78 } };
inline static const constexpr GUID Windows10_PlatformID = { 0x8e0f7a12, 0xbfb3, 0x4fe8, { 0xb9, 0xa5, 0x48, 0xfd, 0x50, 0xa1, 0x5a, 0x9a } };

MIDL_INTERFACE("6090202d-2843-4ba5-9b0d-fc88eecd9ce5")
ICoreApplicationPrivate2 : public ::IInspectable
{
    STDMETHOD(__stub0)() PURE;
    STDMETHOD(__stub1)() PURE;
    STDMETHOD(CreateNonImmersiveView)(void** ppView) PURE;
};

MIDL_INTERFACE("c45f3f8c-61e6-4f9a-be88-fe4fe6e64f5f")
IFrameworkApplicationStaticsPrivate : public ::IInspectable
{
    STDMETHOD(StartInCoreWindowHostingMode)(WindowCreationParameters windowParams, void* callback) PURE;
};

MIDL_INTERFACE("4a8eac58-b652-459d-8de1-239471e8b22b")
IResourceManagerStaticInternal : public ::IInspectable
{
    STDMETHOD(__stub0)() PURE;
    STDMETHOD(GetCurrentResourceManagerForSystemProfile)(void** ppResourceManager) PURE;
};

MIDL_INTERFACE("7d9da47a-8bc7-49d3-97aa-f7db06049172")
IResourceManagerStaticInternalOld : public ::IInspectable
{
    STDMETHOD(__stub0)() PURE;
    STDMETHOD(GetCurrentResourceManagerForSystemProfile)(void** ppResourceManager) PURE;
};

MIDL_INTERFACE("c408a1f1-3ede-41e9-9a38-c203678c2df7")
ISystemResourceManagerExtensions : public ::IInspectable
{
    STDMETHOD(__stub0)() PURE;
    STDMETHOD(GetMrtResourceManagerForResourceManager)(::IInspectable** ppManager) PURE;
};

MIDL_INTERFACE("8c25e859-1042-4da0-9232-bf2aa8ff3726")
ISystemResourceManagerExtensions2 : public ::IInspectable
{
    STDMETHOD(LoadPriFileForSystemUse)(PCWSTR path) PURE;
};

MIDL_INTERFACE("06636c29-5a17-458d-8ea2-2422d997a922")
IWindowPrivate : public ::IInspectable
{
    STDMETHOD(get_TransparentBackground)(BOOL* pValue) PURE;
    STDMETHOD(put_TransparentBackground)(BOOL value) PURE;
    STDMETHOD(_stub8)() PURE;
    STDMETHOD(_stub9)() PURE;
    STDMETHOD(_stub10)() PURE;
    STDMETHOD(_stub11)() PURE;
    STDMETHOD(_stub12)() PURE;
    STDMETHOD(_stub13)() PURE;
    STDMETHOD(SetSynchronizationInfo)(UINT64 syncObj, UINT64 hwnd) PURE;
};

MIDL_INTERFACE("b3ab45d8-6a4e-4e76-a00d-32d4643a9f1a")
IFrameworkApplicationPrivate : public ::IInspectable
{
    STDMETHOD(_stub6)() PURE;
    STDMETHOD(_stub7)() PURE;
    STDMETHOD(_stub8)() PURE;
    STDMETHOD(_stub9)() PURE;
    STDMETHOD(_stub10)() PURE;
    STDMETHOD(SetSynchronizationWindow)(UINT64 hwnd) PURE;
};

MIDL_INTERFACE("b3ab45d8-6a4e-4e76-a00d-32d4643a9f1a")
IFrameworkApplicationPrivateOld : public ::IInspectable
{
    STDMETHOD(_stub6)() PURE;
    STDMETHOD(_stub7)() PURE;
    STDMETHOD(_stub8)() PURE;
    STDMETHOD(SetSynchronizationWindow)(UINT64 hwnd) PURE;
};

MIDL_INTERFACE("49d33aad-f985-4b70-97a0-28eceb6523bf")
IWebPlatformSecurityManager : public ::IUnknown
{
    STDMETHOD(MapUrlToZone)(::IUri*, unsigned int*, unsigned int, wchar_t**, unsigned int*) PURE;
    STDMETHOD(ProcessUrlAction)(::IUri*, unsigned int, unsigned __int8*, unsigned int, unsigned __int8*, unsigned int, unsigned int, unsigned int*) PURE;
    STDMETHOD(GetSecurityId)(::IUri*, unsigned __int8*, unsigned int*) PURE;
    STDMETHOD(QueryCustomPolicy)(::IUri*, const GUID*, unsigned __int8**, unsigned int*, unsigned __int8*, unsigned int) PURE;
};

MIDL_INTERFACE("e4dfc97c-9bef-4803-8ce1-0f980df7c847")
IWebPlatformPermanentSecurityManager : public IWebPlatformSecurityManager
{
    STDMETHOD(GetZoneActionPolicyEx)(unsigned int, unsigned int, unsigned __int8*, unsigned int, URLZONEREG, unsigned int) PURE;
};

struct DefaultWebPlatformSecurityManager : winrt::implements<DefaultWebPlatformSecurityManager, IWebPlatformSecurityManager, IWebPlatformPermanentSecurityManager>
{
    DefaultWebPlatformSecurityManager() = default;
    inline STDMETHODIMP MapUrlToZone(::IUri*, unsigned int*, unsigned int, wchar_t**, unsigned int*) override { return INET_E_DEFAULT_ACTION; }
    inline STDMETHODIMP ProcessUrlAction(::IUri*, unsigned int, unsigned __int8*, unsigned int, unsigned __int8*, unsigned int, unsigned int, unsigned int*) override { return E_FAIL; }
    inline STDMETHODIMP GetSecurityId(::IUri*, unsigned __int8*, unsigned int*) override { return INET_E_DEFAULT_ACTION; }
    inline STDMETHODIMP QueryCustomPolicy(::IUri*, const GUID*, unsigned __int8**, unsigned int*, unsigned __int8*, unsigned int) override { return INET_E_DEFAULT_ACTION; }
    inline STDMETHODIMP GetZoneActionPolicyEx(unsigned int, unsigned int, unsigned __int8*, unsigned int, URLZONEREG, unsigned int) override { return INET_E_DEFAULT_ACTION; }
};

MIDL_INTERFACE("00000000-0000-0000-C000-000000000046")
IUrlRedirectionPolicyManager : public ::IUnknown
{
    STDMETHOD(IsRedirectToBrokerNeeded)(LPCWSTR, void*) PURE;
    STDMETHOD(GetBrowserTransitionState)(LPCWSTR, void*, void*) PURE;
};

MIDL_INTERFACE("00000000-0000-0000-C000-000000000046")
IUrlRedirectionPolicyManagerOld : public ::IUnknown
{
    STDMETHOD(LCIEGetRedirectionPolicyForURL)(LPCWSTR, int, int, unsigned int, unsigned int*, int*) PURE;
    STDMETHOD(IsRedirectToBrokerNeeded)(LPCWSTR, void*) PURE;
    STDMETHOD(LCIEGetRedirectionPolicyForURL2)(LPCWSTR, int, int, unsigned int, unsigned int*, int*, LPCWSTR*) PURE;
};

MIDL_INTERFACE("00000000-0000-0000-C000-000000000046")
IUrlRedirectionPolicyManagerOlder : public ::IUnknown
{
    STDMETHOD(LCIEGetRedirectionPolicyForURL)(LPCWSTR, int, int, unsigned int, unsigned int*, int*) PURE;
    STDMETHOD(IsRedirectToBrokerNeeded)(LPCWSTR, void*) PURE;
};

struct DefaultUrlRedirectionPolicyManager : winrt::implements<DefaultUrlRedirectionPolicyManager, IUrlRedirectionPolicyManager>
{
    DefaultUrlRedirectionPolicyManager() = default;
    inline STDMETHODIMP IsRedirectToBrokerNeeded(LPCWSTR, void*) override { return S_FALSE; }
    inline STDMETHODIMP GetBrowserTransitionState(LPCWSTR, void*, void*) override { return E_NOTIMPL; }
};

struct OldDefaultUrlRedirectionPolicyManager : winrt::implements<OldDefaultUrlRedirectionPolicyManager, IUrlRedirectionPolicyManagerOld>
{
    OldDefaultUrlRedirectionPolicyManager() = default;
    inline STDMETHODIMP IsRedirectToBrokerNeeded(LPCWSTR, void*) override { return S_FALSE; }
    inline STDMETHODIMP LCIEGetRedirectionPolicyForURL(LPCWSTR, int, int, unsigned int, unsigned int*, int*) override { return E_NOTIMPL; }
    inline STDMETHODIMP LCIEGetRedirectionPolicyForURL2(LPCWSTR, int, int, unsigned int, unsigned int*, int*, LPCWSTR*) override { return E_NOTIMPL; }
};

struct OlderDefaultUrlRedirectionPolicyManager : winrt::implements<OlderDefaultUrlRedirectionPolicyManager, IUrlRedirectionPolicyManagerOlder>
{
    OlderDefaultUrlRedirectionPolicyManager() = default;
    inline STDMETHODIMP IsRedirectToBrokerNeeded(LPCWSTR, void*) override { return S_FALSE; }
    inline STDMETHODIMP LCIEGetRedirectionPolicyForURL(LPCWSTR, int, int, unsigned int, unsigned int*, int*) override { return E_NOTIMPL; }
};