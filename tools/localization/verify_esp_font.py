"""Run in VS Developer Command Prompt: python tools/localization/verify_esp_font.py."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-archive", type=Path)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    folder = args.output or Path(tempfile.mkdtemp(prefix="veox-font-test-"))
    folder.mkdir(parents=True, exist_ok=True)
    sources = ["VeoxDMA/Config/OverlayConfig.h", "VeoxDMA/UI/EspFont.cpp", "VeoxDMA/UI/EspFont.h"]
    if args.source_archive:
        with zipfile.ZipFile(args.source_archive) as archive:
            for source in sources:
                if source in archive.namelist():
                    dest = folder / source
                    dest.parent.mkdir(parents=True, exist_ok=True)
                    dest.write_bytes(archive.read(source))
    else:
        for source in sources:
            dest = folder / source
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT / source, dest)
    # Real ImGui, stb_truetype and JSON; only the unrelated DMA PCH is stubbed.
    shutil.copytree(ROOT / "VeoxDMA/ImGUI", folder / "VeoxDMA/ImGUI", dirs_exist_ok=True)
    (folder / "pch.h").write_text('''#pragma once
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <d2d1.h>
#include <string>
#include <vector>
#include <iterator>
#include "json.hpp"
using json = nlohmann::json;
#define LIT(x) x
inline D2D1::ColorF Colour(int r, int g, int b) { return D2D1::ColorF(r/255.f,g/255.f,b/255.f); }
''', encoding="utf-8")
    shutil.copy2(Path(__file__).with_name("esp_font_regression.cpp"), folder / "test.cpp")
    command = ["cl", "/nologo", "/EHsc", "/std:c++20", "/utf-8", "/W0", "/Fe:test.exe", "/I.",
               "/I" + str(ROOT / "Include"), "/IVeoxDMA/Config", "/IVeoxDMA/UI", "/IVeoxDMA/ImGUI", "test.cpp"]
    if (folder / "VeoxDMA/UI/EspFont.h").exists():
        # Run the actual picker implementation, intercepting only the OS modal
        # dialog call so automated tests never disrupt a user's desktop.
        (folder / "picker_shim.cpp").write_text('''#include "pch.h"
#include <commdlg.h>
std::wstring g_pickerResult, g_pickerTitle;
DWORD g_pickerFlags = 0;
void* g_pickerOwner = nullptr;
BOOL WINAPI TestOpenFileNameW(OPENFILENAMEW* ofn) {
    g_pickerTitle = ofn->lpstrTitle;
    g_pickerFlags = ofn->Flags;
    g_pickerOwner = ofn->hwndOwner;
    if (g_pickerResult.empty()) return FALSE;
    wcscpy_s(ofn->lpstrFile, ofn->nMaxFile, g_pickerResult.c_str());
    return TRUE;
}
#define GetOpenFileNameW TestOpenFileNameW
#include "VeoxDMA/UI/EspFont.cpp"
''', encoding="utf-8")
        command += ["/DHAS_ESP_FONT", "picker_shim.cpp"]
    command += ["VeoxDMA/ImGUI/" + name for name in ["imgui.cpp", "imgui_draw.cpp", "imgui_tables.cpp", "imgui_widgets.cpp"]]
    command += ["/link", "user32.lib", "gdi32.lib"]
    result = subprocess.run(command, cwd=folder, capture_output=True)
    (folder / "compile.log").write_bytes(result.stdout + result.stderr)
    if result.returncode:
        print(result.stdout.decode(errors="replace"))
        raise SystemExit(result.returncode)
    result = subprocess.run([str(folder / "test.exe")], cwd=folder, capture_output=True)
    (folder / "result.log").write_bytes(result.stdout + result.stderr)
    print(result.stdout.decode("utf-8", errors="replace"), end="")
    raise SystemExit(result.returncode)


if __name__ == "__main__":
    main()
