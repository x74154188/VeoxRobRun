#include <Windows.h>
#include <cstdio>
#include <filesystem>
#include "Security/LicenseAuth.h"

int wmain(int argc, wchar_t** argv)
{
    int checks = 0, failed = 0;
    auto check = [&](bool ok, const char* label) {
        ++checks;
        if (!ok) { ++failed; std::printf("FAIL: %s\n", label); }
    };
    check(!LicenseAuth::IsAuthorized(), "default deny before host load");
    if (argc != 2) return 2;
    const std::filesystem::path folder(argv[1]);
    check(!LicenseAuth::Initialize(folder / L"missing"), "missing component fails closed");
    check(!LicenseAuth::IsAuthorized(), "missing component cannot authorize");
    check(LicenseAuth::Initialize(folder), "native CLR bridge and real SDK load");
    check(LicenseAuth::Initialize(folder), "repeat initialization");
    check(!LicenseAuth::IsAuthorized(), "loaded bridge does not authorize");
    LicenseAuth::Shutdown();
    LicenseAuth::Shutdown();
    check(!LicenseAuth::IsAuthorized(), "shutdown remains unauthorized");
    std::printf("%s: native auth host; %d checks; %d failures\n", failed ? "FAIL" : "PASS", checks, failed);
    return failed ? 1 : 0;
}
