#pragma once
#include <filesystem>
#include <memory>
#include <functional>
#include <iosfwd>
#include <string>
namespace LicenseAuth {
class Controller;
using NativeLineReader = std::function<bool(bool cardField, std::string& value)>;
// The stream entry point is shared by the real console and offline tests.
bool RunConsoleLogin(Controller& controller, const std::filesystem::path& savedKeyPath,
    std::istream& input, std::ostream& output,
    const std::function<void(bool)>& keyInputMode = {}, const NativeLineReader& nativeReader = {});
// Interactive console authorization and key lifecycle menu.
bool ShowConsoleLogin(std::shared_ptr<Controller> controller,
    const std::filesystem::path& savedKeyPath);
}
