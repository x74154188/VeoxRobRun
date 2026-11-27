#include "pch.h"
#include "EspFont.h"
#include "../ImGUI/imgui.h"
#include <commdlg.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// Validate against the same parser used by imgui_draw.cpp before changing the
// live atlas. Internal linkage keeps this separate from ImGui's implementation.
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "../ImGUI/imstb_truetype.h"

#pragma comment(lib, "comdlg32.lib")

namespace EspFont {
namespace {
ImFont* g_font = nullptr;
std::vector<unsigned char> g_data;
std::string g_attemptedPath;
bool g_attempted = false;
const char* g_status = "Default ESP font";

std::uint32_t Read32(const std::vector<unsigned char>& bytes, std::size_t p)
{
    return (std::uint32_t(bytes[p]) << 24) | (std::uint32_t(bytes[p + 1]) << 16)
        | (std::uint32_t(bytes[p + 2]) << 8) | bytes[p + 3];
}

bool ValidFont(const std::vector<unsigned char>& bytes)
{
    if (bytes.size() < 12) return false;
    std::size_t offset = 0;
    if (Read32(bytes, 0) == 0x74746366) { // ttcf: use the first collection face
        const auto count = Read32(bytes, 8);
        if (!count || count > (bytes.size() - 12) / 4) return false;
        offset = Read32(bytes, 12);
    }
    if (offset > bytes.size() - 12) return false;
    const auto signature = Read32(bytes, offset);
    if (signature != 0x00010000 && signature != 0x4f54544f && signature != 0x74727565)
        return false; // TrueType, OpenType/CFF, Apple TrueType
    const auto count = (unsigned(bytes[offset + 4]) << 8) | bytes[offset + 5];
    if (!count || count > (bytes.size() - offset - 12) / 16) return false;
    for (unsigned i = 0; i < count; ++i) {
        const auto p = offset + 12 + i * 16;
        const std::size_t start = Read32(bytes, p + 8), length = Read32(bytes, p + 12);
        if (start > bytes.size() || length > bytes.size() - start) return false;
    }
    if (stbtt_GetFontOffsetForIndex(bytes.data(), 0) != static_cast<int>(offset)) return false;
    stbtt_fontinfo info{};
    return stbtt_InitFont(&info, bytes.data(), static_cast<int>(offset)) != 0;
}

std::vector<unsigned char> ReadFont(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return {};
    const auto size = file.tellg();
    if (size < 12 || size > 128 * 1024 * 1024) return {};
    std::vector<unsigned char> bytes(static_cast<std::size_t>(size));
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(bytes.data()), size) || !ValidFont(bytes)) return {};
    return bytes;
}

std::filesystem::path WindowsFonts()
{
    wchar_t path[MAX_PATH]{};
    const auto length = GetWindowsDirectoryW(path, MAX_PATH);
    return length && length < MAX_PATH ? std::filesystem::path(path) / L"Fonts" : std::filesystem::path();
}

std::filesystem::path Utf8Path(const std::string& value)
{
    return std::filesystem::path(std::u8string(value.begin(), value.end()));
}
}

bool Prepare(const std::string& utf8Path)
{
    // A picker may return during menu drawing. Never mutate atlas state there.
    if (ImGui::GetCurrentContext() && ImGui::GetIO().Fonts->Locked) return false;
    if (g_attempted && utf8Path == g_attemptedPath) return false;
    g_attempted = true;
    g_attemptedPath = utf8Path;
    g_data.clear();
    g_status = "Default ESP font";
    if (!utf8Path.empty()) {
        try { g_data = ReadFont(Utf8Path(utf8Path)); }
        catch (const std::exception&) { g_data.clear(); }
        g_status = g_data.empty() ? "Font load failed; using the default ESP font." : "ESP font loaded";
    }
    return true;
}

void RequestReload() { g_attempted = false; }

void AddToAtlas(ImFontAtlas& atlas)
{
    g_font = nullptr;
    if (g_data.empty()) return;
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false; // ImGui makes its own owned copy
    g_font = atlas.AddFontFromMemoryTTF(g_data.data(), static_cast<int>(g_data.size()),
        18.f, &cfg, atlas.GetGlyphRangesChineseFull());
    // Merge only missing glyphs, preserving the user's chosen Latin/CJK glyphs.
    const auto fonts = WindowsFonts();
    if (!fonts.empty()) {
        for (const auto* name : { L"simhei.ttf", L"msyh.ttc", L"simsun.ttc" }) {
            auto fallback = ReadFont(fonts / name);
            if (fallback.empty()) continue;
            ImFontConfig merge;
            merge.MergeMode = true;
            merge.FontDataOwnedByAtlas = false;
            atlas.AddFontFromMemoryTTF(fallback.data(), static_cast<int>(fallback.size()),
                18.f, &merge, atlas.GetGlyphRangesChineseFull());
            break;
        }
    }
}

ImFont* Get() { return g_font; }
const char* StatusKey() { return g_status; }

std::string PickFile(void* owner, bool chinese, const std::string& currentPath)
{
    wchar_t path[32768]{};
    std::wstring initial;
    try {
        initial = currentPath.empty() ? WindowsFonts().wstring()
            : Utf8Path(currentPath).parent_path().wstring();
    } catch (const std::exception&) { initial = WindowsFonts().wstring(); }
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = static_cast<HWND>(owner);
    ofn.lpstrFilter = chinese
        ? L"字体文件 (*.ttf;*.otf;*.ttc)\0*.ttf;*.otf;*.ttc\0所有文件 (*.*)\0*.*\0"
        : L"Font files (*.ttf;*.otf;*.ttc)\0*.ttf;*.otf;*.ttc\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = static_cast<DWORD>(std::size(path));
    ofn.lpstrTitle = chinese ? L"选择 ESP 绘制字体" : L"Choose ESP font";
    ofn.lpstrInitialDir = initial.empty() ? nullptr : initial.c_str();
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&ofn)) return {}; // cancel leaves config untouched
    const auto utf8 = std::filesystem::path(path).u8string();
    return std::string(utf8.begin(), utf8.end());
}

Scope::Scope()
{
    if (g_font) {
        ImGui::PushFont(g_font);
        pushed_ = true;
    }
}
Scope::~Scope() { if (pushed_) ImGui::PopFont(); }
}
