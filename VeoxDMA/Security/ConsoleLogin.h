#pragma once
#include <filesystem>
#include <memory>
#include <functional>
#include <iosfwd>
namespace LicenseAuth {
class Controller;
// The stream entry point is shared by the real console and offline tests.
bool RunConsoleLogin(Controller& controller, const std::filesystem::path& savedKeyPath,
    std::istream& input, std::ostream& output,
    const std::function<void(bool)>& keyInputMode = {});
// Interactive console authorization and key lifecycle menu.
bool ShowConsoleLogin(std::shared_ptr<Controller> controller,
    const std::filesystem::path& savedKeyPath);
}
