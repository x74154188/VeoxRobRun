#include "AuthController.h"
#include <array>
#include <chrono>
#include <limits>
#include <stdexcept>

namespace LicenseAuth {
namespace {
bool ParseVersion(const std::string& text, std::array<unsigned, 4>& parts)
{
    parts.fill(0);
    unsigned index = 0;
    bool digit = false;
    for (const char c : text) {
        if (c == '.') {
            if (!digit || ++index == parts.size()) return false;
            digit = false;
        } else {
            if (c < '0' || c > '9') return false;
            auto& n = parts[index];
            if (n > (std::numeric_limits<unsigned>::max() - unsigned(c - '0')) / 10) return false;
            n = n * 10 + unsigned(c - '0');
            digit = true;
        }
    }
    return digit && index >= 1;
}
std::string Trim(std::string key)
{
    const auto first = key.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    return key.substr(first, key.find_last_not_of(" \t\r\n") - first + 1);
}
}

bool VersionPermits(const BaoBao::Auth::AuthResponse& response, std::string& reason)
{
    reason = response.message;
    if (!response.success) return false;
    if (response.data) {
        const auto& data = *response.data;
        if (data.app_id && !data.app_id->empty() && *data.app_id != AppId) {
            reason = "授权应用标识不匹配。";
            return false;
        }
        if (data.version_check_enabled.value_or(false) && data.min_version && !data.min_version->empty()) {
            std::array<unsigned, 4> current{}, minimum{};
            if (!ParseVersion(AppVersion, current) || !ParseVersion(*data.min_version, minimum) || current < minimum) {
                reason = "当前版本低于服务器要求，请更新程序。";
                return false;
            }
        }
    }
    return true;
}

Controller::Controller(std::unique_ptr<Client> client, std::function<std::int64_t()> clock)
    : client_(std::move(client)), now_(std::move(clock))
{
    if (!client_) throw std::invalid_argument("Authorization client is required");
    if (!now_) now_ = [] {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    };
}
Controller::~Controller() { Shutdown(); client_.reset(); }

std::uint64_t Controller::BeginAttempt()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (stopped_) return 0;
    active_ = false;
    message_.clear();
    client_->Invalidate();
    return ++generation_;
}
bool Controller::Current(std::uint64_t attempt)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return attempt && attempt == generation_ && !stopped_;
}
void Controller::Revoke(std::uint64_t attempt, const std::string& message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (attempt != generation_) return;
    active_ = false;
    message_ = message;
    client_->Invalidate();
}

bool Controller::Authenticate(std::uint64_t attempt, std::string key)
{
    std::lock_guard<std::mutex> operation(operationMutex_);
    if (!Current(attempt)) return false;
    key = Trim(std::move(key));
    if (key.empty() || key.size() > 2048) {
        Revoke(attempt, "请输入有效卡密。");
        return false;
    }
    try {
        std::string reason;
        if (!VersionPermits(client_->CheckVersion(), reason)) { Revoke(attempt, reason); return false; }
        if (!Current(attempt)) { client_->Invalidate(); return false; }
        const auto response = client_->Activate(key);
        if (!Current(attempt)) { client_->Invalidate(); return false; }
        if (!VersionPermits(response, reason) || !client_->Validate()) {
            Revoke(attempt, reason.empty() ? "授权验证失败。" : reason);
            return false;
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (attempt != generation_ || stopped_) { client_->Invalidate(); return false; }
            deadline_ = now_() + LeaseMilliseconds;
            message_ = response.message;
            active_ = true;
        }
        client_->StartHeartbeat(key, [this, attempt](const auto& result) { Heartbeat(attempt, result); });
        return IsAuthorized();
    } catch (...) {
        Revoke(attempt, "网络验证异常，请检查网络后重试。");
        return false;
    }
}

bool Controller::IsAuthorized()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!active_ || stopped_) return false;
    if (now_() < deadline_ && client_->Active()) return true;
    active_ = false;
    message_ = "授权已失效或验证超时，请重新登录。";
    client_->Invalidate();
    return false;
}
void Controller::Heartbeat(std::uint64_t attempt, const BaoBao::Auth::AuthResponse& response)
{
    try {
        std::string reason;
        const bool valid = VersionPermits(response, reason) && client_->Validate();
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopped_ || attempt != generation_) return;
        if (!active_ || now_() >= deadline_ || !valid) {
            active_ = false;
            message_ = reason.empty() ? "授权验证失败或超时，请重新登录。" : reason;
            client_->Invalidate();
            return;
        }
        deadline_ = now_() + LeaseMilliseconds;
    } catch (...) { Revoke(attempt, "授权心跳异常，请重新登录。"); }
}
void Controller::Cancel()
{
    std::lock_guard<std::mutex> lock(mutex_);
    ++generation_;
    active_ = false;
    message_ = "验证已取消。";
    client_->Invalidate();
}
void Controller::Shutdown()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopped_) return;
        stopped_ = true;
        ++generation_;
        active_ = false;
        client_->Invalidate();
    }
    // Never hold the controller mutex while joining an SDK callback thread.
    try { client_->StopHeartbeat(); } catch (...) { }
    // Client destruction joins its SDK worker before this controller dies.
}
std::string Controller::Message() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return message_;
}
}
