#define WIN32_LEAN_AND_MEAN
#include "ConsoleInput.h"
#include <algorithm>

namespace LicenseAuth {
namespace {
std::wstring ClipboardText()
{
    if (!OpenClipboard(nullptr)) return {};
    struct Close { ~Close() { CloseClipboard(); } } close;
    const HANDLE data = GetClipboardData(CF_UNICODETEXT);
    if (!data) return {};
    const auto* text = static_cast<const wchar_t*>(GlobalLock(data));
    if (!text) return {};
    struct Unlock { HANDLE data; ~Unlock() { GlobalUnlock(data); } } unlock{data};
    const auto capacity = (std::min)(GlobalSize(data) / sizeof(wchar_t), SIZE_T(65536));
    std::size_t size = 0;
    while (size < capacity && text[size]) ++size;
    return std::wstring(text, size);
}
void Write(HANDLE output, const std::wstring& text)
{
    DWORD count = 0;
    WriteConsoleW(output, text.data(), static_cast<DWORD>(text.size()), &count, nullptr);
}
}

bool ReadNativeConsoleLine(HANDLE input, HANDLE output, bool /*secret*/, std::string& value,
    const ClipboardReader& clipboard)
{
    value.clear();
    DWORD original = 0;
    if (!GetConsoleMode(input, &original)) return false;
    // Disable Quick Edit selection freezes, cooked input and VT escape conversion.
    // We perform editing/echo ourselves so Ctrl+V works even with host shortcuts off.
    const DWORD mode = (original | ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT)
        & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT
            | ENABLE_QUICK_EDIT_MODE | ENABLE_VIRTUAL_TERMINAL_INPUT);
    if (!SetConsoleMode(input, mode)) return false;
    struct Restore { HANDLE input; DWORD mode; ~Restore() { SetConsoleMode(input, mode); } } restore{input, original};
    std::wstring line;
    struct Wipe { std::wstring& line; ~Wipe() { if (!line.empty()) SecureZeroMemory(line.data(), line.size() * sizeof(wchar_t)); } } wipe{line};
    auto append = [&](wchar_t c) {
        if (c < L' ' || c == 0x7f || c == 0xfeff || line.size() >= 4096) return;
        line += c;
        Write(output, std::wstring(1, c));
    };
    auto backspace = [&] {
        if (line.empty()) return;
        line.back() = L'\0';
        line.pop_back();
        Write(output, L"\b \b");
    };
    auto paste = [&] {
        auto text = clipboard ? clipboard() : ClipboardText();
        // A multiline paste is one field, not a stream of subsequent menu commands.
        for (const auto c : text) { if (c == L'\r' || c == L'\n') break; append(c); }
        if (!text.empty()) SecureZeroMemory(text.data(), text.size() * sizeof(wchar_t));
    };
    bool rightDown = false;
    for (;;) {
        INPUT_RECORD event{};
        DWORD count = 0;
        if (!ReadConsoleInputW(input, &event, 1, &count) || !count) return false;
        if (event.EventType == MOUSE_EVENT) {
            const bool pressed = (event.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED) != 0;
            if (pressed && !rightDown && event.Event.MouseEvent.dwEventFlags == 0) paste();
            rightDown = pressed;
            continue;
        }
        if (event.EventType != KEY_EVENT || !event.Event.KeyEvent.bKeyDown) continue;
        const auto& key = event.Event.KeyEvent;
        const bool ctrl = (key.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
        const bool shift = (key.dwControlKeyState & SHIFT_PRESSED) != 0;
        if ((ctrl && key.wVirtualKeyCode == 'V') || key.uChar.UnicodeChar == 0x16
            || (shift && key.wVirtualKeyCode == VK_INSERT)) { paste(); continue; }
        if (key.wVirtualKeyCode == VK_ESCAPE || (ctrl && key.wVirtualKeyCode == 'C')) {
            Write(output, L"\r\n"); return false;
        }
        if (key.wVirtualKeyCode == VK_RETURN || key.uChar.UnicodeChar == L'\r') {
            Write(output, L"\r\n");
            if (line.empty()) return true;
            const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, line.data(), static_cast<int>(line.size()), nullptr, 0, nullptr, nullptr);
            if (size <= 0) return false;
            value.resize(size);
            return WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, line.data(), static_cast<int>(line.size()), value.data(), size, nullptr, nullptr) == size;
        }
        if ((ctrl && key.wVirtualKeyCode == 'U') || key.uChar.UnicodeChar == 0x15) {
            while (!line.empty()) backspace();
            continue;
        }
        const WORD repeat = (std::max)(WORD(1), key.wRepeatCount);
        for (WORD n = 0; n < repeat; ++n) {
            if (key.wVirtualKeyCode == VK_BACK || key.uChar.UnicodeChar == L'\b') backspace();
            else if (!ctrl || (key.dwControlKeyState & RIGHT_ALT_PRESSED)) append(key.uChar.UnicodeChar);
        }
    }
}
}
