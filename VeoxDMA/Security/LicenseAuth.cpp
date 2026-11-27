#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj.h>
#include "LicenseAuth.h"
#include "AuthController.h"
#include "LoginDialog.h"
#include <memory>
#include <mutex>
#include <stdexcept>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

namespace LicenseAuth {
namespace {
class NativeClient final : public Client {
    BaoBao::Auth::LicenseClient client_;
public:
    BaoBao::Auth::AuthResponse CheckVersion() override { return client_.CheckVersion(); }
    BaoBao::Auth::AuthResponse Activate(const std::string& key) override { return client_.ActivateKey(key); }
    bool Validate() override { return BaoBao::Auth::AuthSession::Validate(); }
    bool Active() override { return BaoBao::Auth::AuthSession::IsActive(); }
    void Invalidate() override { BaoBao::Auth::AuthSession::Invalidate(); }
    void StartHeartbeat(const std::string& key,
        std::function<void(const BaoBao::Auth::AuthResponse&)> callback) override {
        client_.StartHeartbeat(key, std::chrono::seconds(60), std::move(callback));
    }
    void StopHeartbeat() override { client_.StopHeartbeat(); }
};
std::mutex stateMutex;
std::shared_ptr<Controller> controller;

std::filesystem::path KeyPath()
{
    PWSTR folder = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &folder)))
        throw std::runtime_error("LocalAppData unavailable");
    const std::filesystem::path path = std::filesystem::path(folder) / L"VeoxRobRun" / L"rob_run" / L"license.dat";
    CoTaskMemFree(folder);
    return path;
}
}

bool Login(bool chinese)
{
    try {
        BaoBao::Auth::AppIdentity::Configure(AppId, AppVersion);
        auto candidate = std::make_shared<Controller>(std::make_unique<NativeClient>());
        const bool accepted = ShowLoginDialog(candidate, KeyPath(), chinese);
        if (!accepted || !candidate->IsAuthorized()) { candidate->Shutdown(); return false; }
        std::lock_guard<std::mutex> lock(stateMutex);
        controller = std::move(candidate);
        return true;
    } catch (...) {
        MessageBoxW(nullptr, chinese ? L"网络验证初始化失败。" : L"License verification initialization failed.",
            L"VeoxRobRun", MB_OK | MB_ICONERROR);
        return false;
    }
}
bool IsAuthorized()
{
    try {
        std::lock_guard<std::mutex> lock(stateMutex);
        return controller && controller->IsAuthorized();
    } catch (...) { return false; }
}
void Shutdown()
{
    std::shared_ptr<Controller> old;
    { std::lock_guard<std::mutex> lock(stateMutex); old.swap(controller); }
    if (old) old->Shutdown();
}
}
