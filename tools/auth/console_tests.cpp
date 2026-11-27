#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Security/AuthController.h"
#include "Security/ConsoleLogin.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace LicenseAuth;
using BaoBao::Auth::AuthResponse;
namespace {
constexpr char TestCard[] = "OFFLINE-TEST-CARD-ONLY";
AuthResponse Ok() {
    AuthResponse r; r.success = true; r.message = "操作成功"; r.data.emplace();
    r.data->app_id = AppId; r.data->expires_at = "2030-12-31 23:59:59"; return r;
}
struct Fake final : Client {
    AuthResponse response = Ok(), unbind = Ok();
    int activations = 0, verifies = 0, unbinds = 0, beats = 0;
    bool active = false, throwUnbind = false;
    std::string key;
    std::function<void(const AuthResponse&)> callback;
    AuthResponse CheckVersion() override { return Ok(); }
    AuthResponse Activate(const std::string& k) override { ++activations; key = k; active = response.success; return response; }
    AuthResponse Verify(const std::string& k) override { ++verifies; key = k; active = response.success; return response; }
    AuthResponse Unbind(const std::string& k) override {
        ++unbinds; key = k;
        if (throwUnbind) throw std::runtime_error("offline transport error");
        active = false; return unbind;
    }
    bool Validate() override { return active; }
    bool Active() override { return active; }
    void Invalidate() override { active = false; }
    void StartHeartbeat(const std::string&, std::function<void(const AuthResponse&)> cb) override { ++beats; callback = std::move(cb); }
    void StopHeartbeat() override {}
};
}
int main() {
    int checks = 0, failures = 0, scenario = 0;
    auto check = [&](bool ok, const char* name) { ++checks; if (!ok) { ++failures; std::cout << "FAIL: " << name << '\n'; } };
    struct Result { bool start, active, exists; int activations, verifies, unbinds, beats, modes; std::string text, key, cached, raw; };
    const auto dir = std::filesystem::temp_directory_path() / (L"veox-console-tests-" + std::to_wstring(GetCurrentProcessId()));
    std::filesystem::create_directories(dir);
    auto run = [&](const std::string& script, bool saved = false, const std::function<void(Fake&)>& configure = {}, bool corrupt = false) {
        const auto path = dir / (std::to_wstring(++scenario) + L".dat");
        if (saved) BaoBao::Auth::SecureKeyStore::Save(path, TestCard);
        if (corrupt) { std::ofstream file(path, std::ios::binary); file << "INVALID-DPAPI"; }
        auto service = std::make_unique<Fake>(); auto* f = service.get(); if (configure) configure(*f);
        Controller controller(std::move(service)); std::istringstream in(script); std::ostringstream out;
        int modes = 0, balance = 0;
        const bool start = RunConsoleLogin(controller, path, in, out, [&](bool hidden) { ++modes; balance += hidden ? 1 : -1; });
        check(balance == 0, "key echo restored for every input path");
        std::string cached;
        try { BaoBao::Auth::SecureKeyStore::TryLoad(path, cached); } catch (...) {}
        std::ifstream file(path, std::ios::binary); std::string raw((std::istreambuf_iterator<char>(file)), {}); file.close();
        Result r{start, controller.IsAuthorized(), std::filesystem::exists(path), f->activations, f->verifies,
            f->unbinds, f->beats, modes, out.str(), f->key, cached, raw};
        std::filesystem::remove(path); return r;
    };
    const auto manual = run(std::string("3\n  ") + TestCard + "  \n1\n");
    check(manual.start && manual.active && manual.activations == 1 && manual.beats == 1, "manual console activation gates startup");
    check(manual.cached == TestCard && manual.key == TestCard, "successful card automatically saved after trim");
    check(manual.raw.find(TestCard) == std::string::npos && !manual.raw.empty(), "real DPAPI cache has no plaintext card");
    check(manual.text.find("2030-12-31 23:59:59") != std::string::npos, "server expiry displayed");
    check(manual.text.find(TestCard) == std::string::npos && manual.modes == 2, "card input hidden and absent from output");
    const auto remembered = run("1\n", true);
    check(remembered.start && remembered.verifies == 1 && remembered.activations == 0, "next launch verifies saved card without activation");
    check(remembered.key == TestCard, "remembered DPAPI card loaded correctly");
    const auto denied = run("1\n5\n", true, [](auto& f) { f.response.success = false; f.response.message = "已过期"; });
    check(!denied.start && !denied.active && denied.verifies == 1, "expired saved card cannot launch");
    check(denied.exists && denied.activations == 0 && denied.text.find("自动验证失败") != std::string::npos, "failed automatic verify leaves unbind option");
    const auto verify = run(std::string("2\n") + TestCard + "\n1\n");
    check(verify.start && verify.verifies == 1 && verify.activations == 0 && verify.cached == TestCard, "manual verify saves valid card");
    const auto status = run("2\n1\n", true);
    check(status.start && status.verifies == 2, "expiry menu refreshes online verification");
    const auto unbound = run("4\n\ny\n1\n5\n", true);
    check(!unbound.start && !unbound.active && unbound.unbinds == 1 && !unbound.exists, "successful unbind revokes and clears same cached card");
    const auto cancelled = run("4\n\nn\n1\n", true);
    check(cancelled.start && cancelled.unbinds == 0 && cancelled.exists, "cancelled unbind preserves authorization and cache");
    const auto failed = run("4\n\ny\n1\n5\n", true, [](auto& f) { f.unbind.success = false; f.unbind.message = "服务器拒绝"; });
    check(!failed.start && !failed.active && failed.unbinds == 1 && failed.exists, "failed unbind revokes locally but preserves cache for retry");
    const auto thrown = run("4\n\ny\n5\n", true, [](auto& f) { f.throwUnbind = true; });
    check(!thrown.start && thrown.exists && thrown.text.find("解绑请求异常") != std::string::npos, "unbind transport exception returns to console");
    const auto other = run("4\nn\nOTHER-OFFLINE-CARD\ny\n5\n", true);
    check(other.unbinds == 1 && other.cached == TestCard && other.key == "OTHER-OFFLINE-CARD", "unbinding another card preserves remembered card");
    const auto noCache = run("4\nOTHER-OFFLINE-CARD\ny\n5\n");
    check(noCache.unbinds == 1 && !noCache.exists, "manual unbind works without login or cache");
    const auto forget = run("6\n5\n", true);
    check(!forget.exists && forget.unbinds == 0, "forget removes cache without server unbind");
    const auto fresh = run("1\ninvalid\n5\n");
    check(!fresh.start && fresh.activations == 0 && fresh.text.find("请先选择 3") != std::string::npos, "unauthorized start and unknown menu handled");
    const auto invalid = run("3\n \n3\n" + std::string(2049, 'x') + "\n5\n");
    check(!invalid.start && invalid.activations == 0 && !invalid.exists, "empty and oversized inputs make no network calls");
    check(!run("").start && !run("3\n").start && !run("4\nCARD\n").start, "EOF exits at menu, card input and confirmation");
    const auto corrupt = run(std::string("3\n") + TestCard + "\n1\n", false, {}, true);
    check(corrupt.start && corrupt.cached == TestCard, "corrupt cache recovers with manual login");
    const auto missing = run(std::string("3\n") + TestCard + "\n1\n", false, [](auto& f) { f.response.data.reset(); });
    check(missing.start && missing.text.find("服务器未返回到期时间") != std::string::npos, "missing expiry is explicit, never invented");
    {
        auto service = std::make_unique<Fake>(); auto* f = service.get(); Controller c(std::move(service));
        check(c.Authenticate(c.BeginAttempt(), TestCard, false) && f->verifies == 1, "controller verify uses verify API");
        auto stale = f->callback; auto response = Ok(); response.data->expires_at = "2031-01-01 01:02:03";
        f->callback(response); check(c.ExpiresAt() == response.data->expires_at, "heartbeat updates server expiry");
        check(!c.Unbind(" ").success && f->unbinds == 0 && !c.IsAuthorized(), "invalid unbind rejected locally");
        stale(Ok()); check(!c.IsAuthorized(), "stale heartbeat cannot restore unbound session");
        check(c.Authenticate(c.BeginAttempt(), TestCard), "relogin after unbind");
        check(c.Unbind("  CARD  ").success && f->key == "CARD" && !c.IsAuthorized(), "unbind trims card and revokes");
        c.Shutdown(); check(!c.Unbind(TestCard).success && f->unbinds == 1, "shutdown blocks unbind calls");
    }
    std::filesystem::remove(dir);
    std::cout << (failures ? "FAIL" : "PASS") << ": console auth workflows; " << checks << " checks; " << failures << " failures\n";
    return failures ? 1 : 0;
}
