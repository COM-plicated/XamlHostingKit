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

struct WindowCreationParameters
{
    int Left, Top, Width, Height;
    char TransparentBackground, IsCoreNavigationClient;
};

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

inline static const constexpr GUID Windows10_PlatformID = { 0x8e0f7a12, 0xbfb3, 0x4fe8, 0xb9a5, 0x48, 0xfd, 0x50, 0xa1, 0x5a, 0x9a };