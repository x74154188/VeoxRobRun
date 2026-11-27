#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "ConsoleLogin.h"
#include "AuthController.h"
#include "ConsoleInput.h"
#include <cstdio>
#include <iostream>
#include <string>

namespace LicenseAuth {
namespace {
struct Key {
    std::string value;
    void Clear() { if (!value.empty()) SecureZeroMemory(value.data(), value.size()); value.clear(); }
    ~Key() { Clear(); }
};
std::string Trim(const std::string& value)
{
    const auto start = value.find_first_not_of(" \t\r\n");
    return start == std::string::npos ? std::string{} : value.substr(start, value.find_last_not_of(" \t\r\n") - start + 1);
}
bool ReadLine(std::istream& in, std::ostream& out, const char* prompt, std::string& value,
    const std::function<void(bool)>& inputMode = {}, const NativeLineReader& reader = {}, bool secret = false)
{
    out << prompt << std::flush;
    struct Mode {
        const std::function<void(bool)>& set;
        Mode(const std::function<void(bool)>& fn) : set(fn) { if (set) set(true); }
        ~Mode() { if (set) set(false); }
    } mode(inputMode);
    const bool read = reader ? reader(secret, value) : static_cast<bool>(std::getline(in, value));
    if (inputMode && !reader) out << '\n';
    return read;
}
void PrintStatus(Controller& controller, std::ostream& out)
{
    if (!controller.IsAuthorized()) { out << "当前授权：未登录或已失效\n"; return; }
    const auto expires = controller.ExpiresAt();
    out << "当前授权：有效\n到期时间：" << (expires.empty() ? "服务器未返回到期时间" : expires) << '\n';
}
bool Authenticate(Controller& controller, const std::string& key, bool activate, std::ostream& out)
{
    out << (activate ? "正在登录 / 激活…\n" : "正在在线验证…\n") << std::flush;
    const bool ok = controller.Authenticate(controller.BeginAttempt(), key, activate);
    out << controller.Message() << '\n';
    PrintStatus(controller, out);
    return ok && controller.IsAuthorized();
}
}

bool RunConsoleLogin(Controller& controller, const std::filesystem::path& path,
    std::istream& in, std::ostream& out, const std::function<void(bool)>& inputMode, const NativeLineReader& reader)
{
    Key remembered;
    try {
        if (BaoBao::Auth::SecureKeyStore::TryLoad(path, remembered.value) && !remembered.value.empty()) {
            out << "已读取加密保存的卡密。\n";
            if (!Authenticate(controller, remembered.value, false, out))
                out << "自动验证失败，请重新登录或解绑。\n";
        }
    } catch (...) { out << "读取已保存卡密失败，请手动登录。\n"; }

    for (;;) {
        out << "\n=== VeoxRobRun 授权控制台（rob_run）===\n"
            "1. 启动程序\n2. 验证卡密 / 查看到期时间\n3. 登录 / 激活卡密\n"
            "4. 解绑卡密\n5. 退出\n6. 清除本地记住的卡密\n";
        PrintStatus(controller, out);
        std::string choice;
        if (!ReadLine(in, out, "请选择 [1-6]：", choice, {}, reader)) { controller.Cancel(); return false; }
        choice = Trim(choice);
        if (choice == "1") {
            if (controller.IsAuthorized()) { out << "授权有效，正在启动程序…\n" << std::flush; return true; }
            out << "请先选择 3 输入卡密登录。\n";
        } else if (choice == "2" || choice == "3") {
            Key key;
            if (choice == "2" && !remembered.value.empty()) key.value = remembered.value;
            else {
                if (!ReadLine(in, out, "请输入卡密（Ctrl+V / Shift+Insert / 右键粘贴，回车确认）：", key.value, inputMode, reader, true)) { controller.Cancel(); return false; }
                key.value = Trim(key.value);
            }
            if (key.value.empty() || key.value.size() > 2048) { out << "请输入有效卡密。\n"; continue; }
            if (Authenticate(controller, key.value, choice == "3", out)) {
                remembered.Clear();
                remembered.value = key.value;
                try {
                    BaoBao::Auth::SecureKeyStore::Save(path, key.value);
                    out << "卡密已自动加密保存，下次启动将在线验证。\n";
                } catch (...) { out << "登录成功，但保存新卡密失败，请重试登录以保存。\n"; }
            }
        } else if (choice == "4") {
            Key key;
            std::string answer;
            if (!remembered.value.empty()) {
                if (!ReadLine(in, out, "使用记住的卡密解绑？[Y/n]：", answer, {}, reader)) { controller.Cancel(); return false; }
                answer = Trim(answer);
                if (answer.empty() || answer == "Y" || answer == "y") key.value = remembered.value;
            }
            if (key.value.empty()) {
                if (!ReadLine(in, out, "请输入要解绑的卡密（支持 Ctrl+V 粘贴）：", key.value, inputMode, reader, true)) { controller.Cancel(); return false; }
                key.value = Trim(key.value);
            }
            if (key.value.empty() || key.value.size() > 2048) { out << "请输入有效卡密。\n"; continue; }
            if (!ReadLine(in, out, "解绑将注销授权，服务器会扣除 3 小时时长。确认解绑？[y/N]：", answer, {}, reader)) {
                controller.Cancel(); return false;
            }
            answer = Trim(answer);
            if (answer != "y" && answer != "Y") { out << "已取消解绑。\n"; continue; }
            out << "正在解绑…\n" << std::flush;
            const auto result = controller.Unbind(key.value);
            out << result.message << '\n';
            if (result.data && !result.data->expires_at.empty()) out << "到期时间：" << result.data->expires_at << '\n';
            // Keep a different remembered card when unbinding a manually entered card.
            if (result.success && (remembered.value.empty() || key.value == remembered.value)) {
                std::error_code error;
                std::filesystem::remove(path, error);
                remembered.Clear();
                out << (error ? "解绑成功，但本地卡密缓存删除失败，请重试清除。\n" : "解绑成功，已清除本地卡密缓存。\n");
            }
        } else if (choice == "5" || choice == "0") {
            controller.Cancel(); return false;
        } else if (choice == "6") {
            std::error_code error;
            std::filesystem::remove(path, error);
            if (error) out << "清除本地卡密缓存失败，请重试。\n";
            else { remembered.Clear(); out << "本地卡密缓存已清除。\n"; }
        } else out << "无效选项，请输入 1 到 6。\n";
    }
}

bool ShowConsoleLogin(std::shared_ptr<Controller> controller, const std::filesystem::path& path)
{
    // WinMain already allocates the application console and redirects stdio.
    if (!GetConsoleWindow()) return false;
    SetConsoleTitleW(L"VeoxRobRun - License Console");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // Open the console devices directly; do not depend on redirected/stale CRT stdin.
    const HANDLE input = CreateFileW(L"CONIN$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    const HANDLE output = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    struct Handles {
        HANDLE input, output;
        ~Handles() { if (input != INVALID_HANDLE_VALUE) CloseHandle(input); if (output != INVALID_HANDLE_VALUE) CloseHandle(output); }
    } handles{input, output};
    if (input == INVALID_HANDLE_VALUE || output == INVALID_HANDLE_VALUE) return false;
    const NativeLineReader reader = [=](bool secret, std::string& value) {
        return ReadNativeConsoleLine(input, output, secret, value);
    };
    return RunConsoleLogin(*controller, path, std::cin, std::cout, {}, reader);
}
}
