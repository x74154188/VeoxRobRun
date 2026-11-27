#pragma once
#include <filesystem>
#include <memory>
namespace LicenseAuth {
class Controller;
// The task is joined before returning. Cancel invalidates even a late success.
bool ShowLoginDialog(std::shared_ptr<Controller> controller,
    const std::filesystem::path& savedKeyPath, bool chinese);
}
