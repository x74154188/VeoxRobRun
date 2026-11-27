// Standalone ImGui/font-atlas test. No game, DMA device or overlay is started.
#include "pch.h"
#include "OverlayConfig.h"
#include "imgui.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#ifdef HAS_ESP_FONT
#include "EspFont.h"
#include <commdlg.h>
extern std::wstring g_pickerResult;
extern std::wstring g_pickerTitle;
extern DWORD g_pickerFlags;
extern void* g_pickerOwner;
#endif

int main()
{
    int failed = 0, checks = 0;
    auto check = [&](bool condition, const char* label) {
        ++checks;
        if (!condition) { ++failed; std::printf("FAIL: %s\n", label); }
    };
    OverlayConfig settings("Overlay");
    check(!settings.ChineseLanguage, "English default retained");
#ifndef HAS_ESP_FONT
    check(false, "ESP font file selection/atlas service missing");
    check(false, "ESP font path persistence missing");
#else
    check(settings.EspFontPath.empty(), "default uses standard ESP font");
    settings.EspFontPath = "C:\\字体目录\\测试字体.ttf";
    const auto saved = settings.ToJson();
    OverlayConfig restored("Overlay");
    restored.FromJson(json::parse(saved.dump()));
    check(restored.EspFontPath == settings.EspFontPath, "UTF-8 font path JSON round trip");
    restored.FromJson(json{{"Overlay", json::object()}});
    check(restored.EspFontPath.empty(), "old profile clears custom font");
    wchar_t windows[MAX_PATH]{};
    GetWindowsDirectoryW(windows, MAX_PATH);
    const auto fonts = std::filesystem::path(windows) / L"Fonts";
    const auto fixtures = std::filesystem::current_path() / L"字体测试";
    std::filesystem::create_directories(fixtures);
    const auto ttf = fixtures / L"自定义.ttf";
    std::filesystem::copy_file(fonts / L"arial.ttf", ttf, std::filesystem::copy_options::overwrite_existing);
    auto utf8 = [](const std::filesystem::path& p) { auto s = p.u8string(); return std::string(s.begin(), s.end()); };
    g_pickerResult = ttf.wstring();
    check(EspFont::PickFile((void*)0x1234, true, "") == utf8(ttf), "picker returns selected Unicode path");
    check(g_pickerTitle == L"选择 ESP 绘制字体", "Chinese picker title");
    check(g_pickerOwner == (void*)0x1234, "picker owner forwarded");
    constexpr DWORD flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    check((g_pickerFlags & flags) == flags, "picker uses existing files without changing working directory");
    g_pickerResult.clear();
    check(EspFont::PickFile(nullptr, false, utf8(ttf)).empty(), "picker cancel returns no change");
    check(g_pickerTitle == L"Choose ESP font", "English picker title");
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(800, 600);
    io.DeltaTime = 1.f / 60.f;
    auto build = [&]() {
        io.FontDefault = nullptr;
        io.Fonts->Clear();
        io.FontDefault = io.Fonts->AddFontDefault();
        EspFont::AddToAtlas(*io.Fonts);
        return io.Fonts->Build();
    };
    check(EspFont::Prepare(""), "initial empty preparation");
    check(build() && !EspFont::Get(), "empty path keeps menu/default atlas");
    check(!EspFont::Prepare(""), "unchanged path does not rebuild every frame");
    check(EspFont::Prepare(utf8(ttf)), "Unicode path prepared");
    check(build() && EspFont::Get(), "TTF rasterization");
    check(std::string(EspFont::StatusKey()) == "ESP font loaded", "loaded status");
    ImFont* custom = EspFont::Get();
    check(custom->FindGlyphNoFallback('A') != nullptr, "custom Latin glyph");
    check(custom->FindGlyphNoFallback(0x73a9) != nullptr, "Chinese fallback glyph");
    io.Fonts->SetTexID((ImTextureID)1);
    ImGui::NewFrame();
    ImFont* menuFont = ImGui::GetFont();
    {
        EspFont::Scope scope;
        check(ImGui::GetFont() == custom, "ESP scope selects custom font");
        check(!EspFont::Prepare(""), "locked atlas defers changes");
        ImGui::Begin("ESP test");
        ImGui::TextUnformatted("玩家 / 物品 / 载具 123 ABC");
        ImGui::End();
    }
    check(ImGui::GetFont() == menuFont, "ESP scope restores menu font");
    ImGui::Render();
    check(!EspFont::Prepare(utf8(ttf)), "same path skips reload");
    EspFont::RequestReload();
    check(EspFont::Prepare(utf8(ttf)) && build(), "explicit same-file reload");
    check(EspFont::Prepare(utf8(fonts / L"cambria.ttc")) && build() && EspFont::Get(), "TTC first-face rasterization");
    const auto invalid = fixtures / L"invalid.ttf";
    { std::ofstream out(invalid, std::ios::binary); out << "not a font file"; }
    check(EspFont::Prepare(utf8(invalid)) && build() && !EspFont::Get(), "invalid font falls back");
    check(std::string(EspFont::StatusKey()) == "Font load failed; using the default ESP font.", "failure status");
    // A valid SFNT signature with a table outside the file must be rejected.
    { unsigned char bytes[28] = {0,1,0,0,0,1}; bytes[20] = 0x7f;
      std::ofstream out(invalid, std::ios::binary); out.write((char*)bytes, sizeof(bytes)); }
    EspFont::RequestReload();
    check(EspFont::Prepare(utf8(invalid)) && build() && !EspFont::Get(), "truncated font table rejected");
    check(EspFont::Prepare(utf8(fixtures / L"missing.otf")) && build() && !EspFont::Get(), "missing file falls back");
    check(EspFont::Prepare(utf8(ttf)) && build() && EspFont::Get(), "valid font after failure");
    check(EspFont::Prepare("") && build() && !EspFont::Get(), "reset restores default");
    check(std::string(EspFont::StatusKey()) == "Default ESP font", "reset status");
    ImGui::DestroyContext();
#endif
    std::printf("%s: ESP font regression; %d checks; %d failures\n", failed ? "FAIL" : "PASS", checks, failed);
    return failed ? 1 : 0;
}
