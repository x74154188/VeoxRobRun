#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <functional>
#include <string>

namespace LicenseAuth {
using ClipboardReader = std::function<std::wstring()>;
// Native key events, independent of CRT stdin/iostream state and host shortcuts.
// Card characters are echoed in plaintext; Ctrl+V, Shift+Insert and right-click paste.
bool ReadNativeConsoleLine(HANDLE input, HANDLE output, bool secret, std::string& value,
    const ClipboardReader& clipboard = {});
}
