#pragma once
#include <BaoBao/Auth.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace LicenseAuth {
inline constexpr char AppId[] = "rob_run";
inline constexpr char AppVersion[] = "1.0.0";
inline constexpr std::int64_t LeaseMilliseconds = 135000;

// Tests substitute this interface, never the production protocol or SDK source.
class Client {
public:
    virtual ~Client() = default;
    virtual BaoBao::Auth::AuthResponse CheckVersion() = 0;
    virtual BaoBao::Auth::AuthResponse Activate(const std::string& key) = 0;
    virtual bool Validate() = 0;
    virtual bool Active() = 0;
    virtual void Invalidate() = 0;
    virtual void StartHeartbeat(const std::string& key,
        std::function<void(const BaoBao::Auth::AuthResponse&)> callback) = 0;
    virtual void StopHeartbeat() = 0;
};

bool VersionPermits(const BaoBao::Auth::AuthResponse& response, std::string& reason);

class Controller {
public:
    explicit Controller(std::unique_ptr<Client> client,
        std::function<std::int64_t()> monotonicMilliseconds = {});
    ~Controller();
    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;
    std::uint64_t BeginAttempt(); // Call on UI thread before launching async work.
    bool Authenticate(std::uint64_t attempt, std::string key);
    bool IsAuthorized(); // Fast cached lease + SDK active flag; no HTTP/WMI here.
    void Cancel();
    void Shutdown(); // Called after login task and business workers have stopped.
    std::string Message() const;
private:
    bool Current(std::uint64_t attempt);
    void Revoke(std::uint64_t attempt, const std::string& message);
    void Heartbeat(std::uint64_t attempt, const BaoBao::Auth::AuthResponse& response);
    std::unique_ptr<Client> client_;
    std::function<std::int64_t()> now_;
    mutable std::mutex mutex_;
    std::mutex operationMutex_;
    std::uint64_t generation_ = 0;
    std::int64_t deadline_ = 0;
    bool active_ = false, stopped_ = false;
    std::string message_;
};
}
