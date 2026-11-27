#pragma once
#include <filesystem>

namespace LicenseAuth {
// Loading dependencies never grants authorization. Login alone can establish it.
bool Initialize(const std::filesystem::path& executableDirectory);
bool Login(bool chinese);
bool IsAuthorized();
void Shutdown();

class Lifetime {
public:
    ~Lifetime() { Shutdown(); }
    Lifetime() = default;
    Lifetime(const Lifetime&) = delete;
    Lifetime& operator=(const Lifetime&) = delete;
};
}
