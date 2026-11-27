#pragma once
namespace LicenseAuth {
bool Login(bool chinese);
bool IsAuthorized();
void Shutdown();
class Lifetime {
public:
    Lifetime() = default;
    ~Lifetime() { Shutdown(); }
    Lifetime(const Lifetime&) = delete;
    Lifetime& operator=(const Lifetime&) = delete;
};
}
