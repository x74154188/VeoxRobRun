#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Security/AuthController.h"
#include "Security/LicenseAuth.h"
#include "Security/LoginResource.h"
#include <atomic>
#include <cstdio>
#include <future>
#include <stdexcept>

using namespace LicenseAuth;
using BaoBao::Auth::AuthResponse;
namespace {
AuthResponse Ok()
{
    AuthResponse result;
    result.success = true;
    result.data.emplace();
    result.data->app_id = AppId;
    return result;
}
struct Stats { int stopped = 0; bool destroyed = false; };
struct Fake final : Client {
    std::shared_ptr<Stats> stats = std::make_shared<Stats>();
    AuthResponse version = Ok(), activation = Ok();
    bool valid = false, permit = true, throws = false, throwStart = false;
    int versions = 0, activations = 0, heartbeats = 0;
    std::string lastKey;
    std::function<void()> duringVersion, duringActivate;
    std::function<void(const AuthResponse&)> callback;
    ~Fake() override { stats->destroyed = true; }
    AuthResponse CheckVersion() override {
        ++versions;
        if (throws) throw std::runtime_error("test network error");
        if (duringVersion) duringVersion();
        return version;
    }
    AuthResponse Activate(const std::string& key) override {
        ++activations; lastKey = key;
        if (duringActivate) duringActivate();
        valid = activation.success && permit;
        return activation;
    }
    bool Validate() override { return valid && permit; }
    bool Active() override { return valid; }
    void Invalidate() override { valid = false; }
    void StartHeartbeat(const std::string&, std::function<void(const AuthResponse&)> cb) override {
        if (throwStart) throw std::runtime_error("heartbeat start failed");
        ++heartbeats; callback = std::move(cb);
    }
    void StopHeartbeat() override { ++stats->stopped; }
    void Emit(const AuthResponse& response) { valid = response.success && permit; callback(response); }
};
INT_PTR CALLBACK HiddenDialog(HWND, UINT, WPARAM, LPARAM) { return FALSE; }
}

int main()
{
    int checks = 0, failed = 0;
    auto check = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) { ++failed; std::printf("FAIL: %s\n", name); }
    };
    auto authenticate = [](Controller& controller, const std::string& key = "TEST-NOT-A-REAL-CARD") {
        return controller.Authenticate(controller.BeginAttempt(), key);
    };
    check(std::string(AppId) == "rob_run", "requested App ID retained");
    check(!LicenseAuth::IsAuthorized(), "native entry default deny without a client");
    LicenseAuth::Shutdown(); LicenseAuth::Shutdown();
    check(!LicenseAuth::IsAuthorized(), "native shutdown remains unauthorized");
    std::string reason;
    check(VersionPermits(Ok(), reason), "normal response policy");
    auto response = Ok(); response.success = false;
    check(!VersionPermits(response, reason), "server rejection");
    response = Ok(); response.data->app_id = "wrong-app";
    check(!VersionPermits(response, reason), "wrong app rejected");
    response = Ok(); response.data->version_check_enabled = true;
    for (const auto* minimum : { "1.0", "1.0.0", "1.0.0.0", "0.9.9" }) {
        response.data->min_version = minimum;
        check(VersionPermits(response, reason), "compatible minimum version");
    }
    for (const auto* minimum : { "2.0", "1.0.1", "invalid", "1..0", "1.0.", "1", "1.0.0.0.0", "1.42949672960" }) {
        response.data->min_version = minimum;
        check(!VersionPermits(response, reason), "newer or malformed minimum denied");
    }
    {
        auto service = std::make_unique<Fake>(); auto* f = service.get(); auto stats = f->stats;
        std::int64_t now = 0;
        {
            Controller controller(std::move(service), [&] { return now; });
            check(!controller.IsAuthorized(), "controller default deny");
            check(!authenticate(controller, " \t\r\n"), "empty key rejected");
            check(!authenticate(controller, std::string(2049, 'x')), "oversized key rejected");
            check(f->versions == 0 && f->activations == 0, "invalid input makes no SDK call");
            check(authenticate(controller, "  TEST-CARD \r\n"), "activation succeeds");
            check(f->lastKey == "TEST-CARD", "trimmed input");
            check(f->heartbeats == 1 && controller.IsAuthorized(), "heartbeat starts only after login");
            now = 60000; f->Emit(Ok());
            now = 140000;
            check(controller.IsAuthorized(), "heartbeat renews monotonic lease");
            response = Ok(); response.success = false; response.message = "revoked";
            f->Emit(response);
            check(!controller.IsAuthorized() && !f->valid, "rejected heartbeat revokes");
            f->Emit(Ok());
            check(!controller.IsAuthorized() && !f->valid, "late success cannot revive revoked session");
            check(authenticate(controller), "explicit re-login allowed");
            controller.Shutdown(); controller.Shutdown();
            check(!controller.IsAuthorized() && stats->stopped == 1, "shutdown idempotent");
            check(controller.BeginAttempt() == 0, "shutdown prevents new login");
        }
        check(stats->destroyed, "SDK adapter destroyed before controller state");
    }
    {
        auto service = std::make_unique<Fake>(); auto* f = service.get();
        f->version.success = false;
        Controller controller(std::move(service));
        check(!authenticate(controller) && f->activations == 0, "version refusal prevents activation");
        f->version = Ok(); f->activation.success = false;
        check(!authenticate(controller) && f->heartbeats == 0, "activation failure denies access");
        f->activation = Ok(); f->permit = false;
        check(!authenticate(controller), "SDK session validation required");
        f->permit = true; f->throws = true;
        check(!authenticate(controller) && !controller.IsAuthorized(), "SDK exception denies access");
        f->throws = false; f->throwStart = true;
        check(!authenticate(controller) && !controller.IsAuthorized(), "heartbeat startup exception denies access");
    }
    {
        auto service = std::make_unique<Fake>(); auto* f = service.get();
        Controller controller(std::move(service));
        auto attempt = controller.BeginAttempt(); controller.Cancel();
        check(!controller.Authenticate(attempt, "TEST") && f->versions == 0, "cancel queued attempt before thread starts");
        f->duringVersion = [&] { controller.Cancel(); };
        check(!authenticate(controller) && f->activations == 0, "cancel version request prevents activation");
        f->duringVersion = {}; f->duringActivate = [&] { controller.Cancel(); };
        check(!authenticate(controller) && !controller.IsAuthorized() && !f->valid, "cancel activation rejects late success");
    }
    {
        std::int64_t now = 0;
        auto service = std::make_unique<Fake>(); auto* f = service.get();
        Controller controller(std::move(service), [&] { return now; });
        check(authenticate(controller), "lease setup");
        now = LeaseMilliseconds;
        check(!controller.IsAuthorized() && !f->valid, "exact lease boundary expires");
        f->Emit(Ok());
        check(!controller.IsAuthorized() && !f->valid, "late heartbeat cannot revive expired lease");
        now = 0;
        check(!controller.IsAuthorized(), "clock rollback does not revive access");
        check(authenticate(controller), "new login after expiry");
        f->valid = false;
        check(!controller.IsAuthorized(), "SDK invalidation observed immediately");
    }
    {
        auto service = std::make_unique<Fake>(); auto* f = service.get();
        Controller controller(std::move(service));
        check(authenticate(controller), "stale callback setup");
        auto oldCallback = f->callback;
        controller.Cancel();
        check(authenticate(controller), "new attempt succeeds");
        oldCallback(Ok());
        check(controller.IsAuthorized(), "stale callback cannot mutate new attempt");
        response = Ok(); response.data->app_id = "other";
        f->Emit(response);
        check(!controller.IsAuthorized(), "cross-app heartbeat revoked");
    }
    {
        auto service = std::make_unique<Fake>(); auto* f = service.get();
        Controller controller(std::move(service));
        std::promise<void> entered, release;
        auto ready = release.get_future();
        f->duringActivate = [&] { entered.set_value(); ready.wait(); };
        const auto attempt = controller.BeginAttempt();
        auto work = std::async(std::launch::async, [&] { return controller.Authenticate(attempt, "TEST"); });
        entered.get_future().wait();
        controller.Cancel();
        check(!controller.IsAuthorized(), "UI cancel remains responsive during synchronous HTTP");
        release.set_value();
        check(!work.get() && !f->valid, "in-flight request success rejected after cancellation");
    }
    const HWND dialog = CreateDialogParamW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDD_LICENSE), nullptr, HiddenDialog, 0);
    check(dialog != nullptr, "native dialog resource loads without managed runtime");
    if (dialog) {
        check(!IsWindowVisible(dialog), "test dialog remains hidden");
        check((GetWindowLongPtrW(GetDlgItem(dialog, IDC_LICENSE_KEY), GWL_STYLE) & ES_PASSWORD) != 0, "native key edit is masked");
        check(GetDlgItem(dialog, IDOK) && GetDlgItem(dialog, IDCANCEL), "login and cancel controls exist");
        check(GetDlgItem(dialog, IDC_LICENSE_REMEMBER) && GetDlgItem(dialog, IDC_LICENSE_STATUS), "remember and status controls exist");
        check(DestroyWindow(dialog) != 0, "native test dialog closes");
    }
    std::printf("%s: native auth integration; %d checks; %d failures\n", failed ? "FAIL" : "PASS", checks, failed);
    return failed ? 1 : 0;
}
