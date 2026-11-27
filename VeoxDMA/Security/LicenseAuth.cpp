#include "pch.h"
#include "LicenseAuth.h"
#include "../../auth/native/nethost.h"
#include "../../auth/native/hostfxr.h"
#include "../../auth/native/coreclr_delegates.h"
#include <vector>

namespace LicenseAuth {
namespace {
// Keep the CLR and its native host loaded for the process lifetime.
HMODULE nethostModule = nullptr, hostfxrModule = nullptr;
component_entry_point_fn begin = nullptr, poll = nullptr, end = nullptr;
bool ready = false;
}

bool Initialize(const std::filesystem::path& directory)
{
    if (ready) return true;
    const auto folder = std::filesystem::absolute(directory) / L"auth";
    const auto assembly = folder / L"Veox.AuthBridge.dll";
    const auto runtimeConfig = folder / L"Veox.AuthBridge.runtimeconfig.json";
    if (!std::filesystem::exists(assembly) || !std::filesystem::exists(runtimeConfig)) return false;
    if (!nethostModule) nethostModule = LoadLibraryExW((folder / L"nethost.dll").c_str(), nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!nethostModule) return false;
    auto getPath = reinterpret_cast<decltype(&get_hostfxr_path)>(GetProcAddress(nethostModule, "get_hostfxr_path"));
    if (!getPath) return false;
    size_t size = 0;
    get_hostfxr_parameters parameters{ sizeof(parameters), assembly.c_str(), nullptr };
    getPath(nullptr, &size, &parameters);
    if (!size || size > 32768) return false;
    std::vector<wchar_t> hostPath(size);
    if (getPath(hostPath.data(), &size, &parameters) != 0) return false;
    if (!hostfxrModule) hostfxrModule = LoadLibraryExW(hostPath.data(), nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!hostfxrModule) return false;
    auto init = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>(GetProcAddress(hostfxrModule, "hostfxr_initialize_for_runtime_config"));
    auto getDelegate = reinterpret_cast<hostfxr_get_runtime_delegate_fn>(GetProcAddress(hostfxrModule, "hostfxr_get_runtime_delegate"));
    auto close = reinterpret_cast<hostfxr_close_fn>(GetProcAddress(hostfxrModule, "hostfxr_close"));
    if (!init || !getDelegate || !close) return false;
    hostfxr_handle context = nullptr;
    const int status = init(runtimeConfig.c_str(), nullptr, &context);
    if (status < 0 || !context) { if (context) close(context); return false; }
    load_assembly_and_get_function_pointer_fn load = nullptr;
    const int delegateStatus = getDelegate(context, hdt_load_assembly_and_get_function_pointer,
        reinterpret_cast<void**>(&load));
    close(context);
    if (delegateStatus != 0 || !load) return false;
    auto bind = [&](const wchar_t* name, component_entry_point_fn& target) {
        return load(assembly.c_str(), L"Veox.AuthBridge.EntryPoints, Veox.AuthBridge", name,
            nullptr, nullptr, reinterpret_cast<void**>(&target)) == 0 && target;
    };
    component_entry_point_fn probe = nullptr;
    ready = bind(L"Begin", begin) && bind(L"Poll", poll) && bind(L"End", end)
        && bind(L"Probe", probe) && probe(nullptr, 0) == 1;
    return ready;
}

bool Login(bool chinese)
{
    try {
        wchar_t path[32768]{};
        const DWORD length = GetModuleFileNameW(nullptr, path, static_cast<DWORD>(std::size(path)));
        if (!length || length == std::size(path) || !Initialize(std::filesystem::path(path).parent_path())) {
            MessageBoxW(nullptr, chinese
                ? L"网络验证组件加载失败。请保留程序旁的 auth 文件夹，并安装 x64 .NET 9 Desktop Runtime。"
                : L"License component failed to load. Keep the auth folder beside this executable and install x64 .NET 9 Desktop Runtime.",
                L"VeoxRobRun", MB_OK | MB_ICONERROR);
            return false;
        }
        int language = chinese ? 1 : 0;
        return begin(&language, sizeof(language)) == 1 && IsAuthorized();
    } catch (...) { return false; }
}

bool IsAuthorized()
{
    try { return ready && poll && poll(nullptr, 0) == 1; }
    catch (...) { return false; }
}

void Shutdown()
{
    try { if (ready && end) end(nullptr, 0); }
    catch (...) { }
}
}
