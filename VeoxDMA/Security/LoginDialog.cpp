#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "LoginDialog.h"
#include "AuthController.h"
#include "LoginResource.h"
#include <chrono>
#include <future>
#include <string>

namespace LicenseAuth {
namespace {
std::wstring Wide(const std::string& text)
{
    const int n = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), nullptr, 0);
    if (n <= 0) return {};
    std::wstring wide(n, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), wide.data(), n);
    return wide;
}
std::string Utf8(const std::wstring& text)
{
    const int n = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (n <= 0) return {};
    std::string utf8(n, '\0');
    WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), utf8.data(), n, nullptr, nullptr);
    return utf8;
}
struct State {
    std::shared_ptr<Controller> controller;
    std::filesystem::path path;
    bool chinese = false, cancel = false, remember = false;
    std::future<bool> pending;
};
void EnableInputs(HWND window, bool enable)
{
    for (const int id : { IDOK, IDC_LICENSE_KEY, IDC_LICENSE_REMEMBER }) EnableWindow(GetDlgItem(window, id), enable);
}
void Cancel(HWND window, State& state)
{
    state.cancel = true;
    state.controller->Cancel();
    if (state.pending.valid()) {
        // The SDK's synchronous HTTP API has no cancellation token. Keep pumping
        // the dialog until it returns; a late success cannot authorize anything.
        EnableWindow(GetDlgItem(window, IDCANCEL), FALSE);
        SetDlgItemTextW(window, IDC_LICENSE_STATUS, state.chinese
            ? L"已取消，正在等待当前网络请求结束…" : L"Cancelled. Waiting for the current request to finish…");
    } else EndDialog(window, IDCANCEL);
}
INT_PTR Handle(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* state = reinterpret_cast<State*>(GetWindowLongPtrW(window, DWLP_USER));
    if (message == WM_INITDIALOG) {
        state = reinterpret_cast<State*>(lParam);
        SetWindowLongPtrW(window, DWLP_USER, reinterpret_cast<LONG_PTR>(state));
        if (state->chinese) {
            SetWindowTextW(window, L"VeoxRobRun · 网络验证");
            SetDlgItemTextW(window, IDC_LICENSE_LABEL, L"请输入卡密（应用：rob_run）");
            SetDlgItemTextW(window, IDC_LICENSE_REMEMBER, L"记住卡密（当前 Windows 用户加密保存）");
            SetDlgItemTextW(window, IDOK, L"登录 / 激活");
            SetDlgItemTextW(window, IDCANCEL, L"退出");
            SetDlgItemTextW(window, IDC_LICENSE_FOOTER, L"验证通过后启动 DMA；运行中每 60 秒验证。");
        }
        SendDlgItemMessageW(window, IDC_LICENSE_KEY, EM_SETLIMITTEXT, 512, 0);
        try {
            std::string saved;
            if (BaoBao::Auth::SecureKeyStore::TryLoad(state->path, saved)) {
                SetDlgItemTextW(window, IDC_LICENSE_KEY, Wide(saved).c_str());
                CheckDlgButton(window, IDC_LICENSE_REMEMBER, BST_CHECKED);
            }
        } catch (...) { }
        if (!SetTimer(window, 1, 100, nullptr)) { EndDialog(window, IDCANCEL); return TRUE; }
        return TRUE;
    }
    if (!state) return FALSE;
    if (message == WM_CLOSE) { Cancel(window, *state); return TRUE; }
    if (message == WM_COMMAND && LOWORD(wParam) == IDCANCEL) { Cancel(window, *state); return TRUE; }
    if (message == WM_COMMAND && LOWORD(wParam) == IDOK) {
        if (state->pending.valid() || state->cancel) return TRUE;
        wchar_t text[513]{};
        GetDlgItemTextW(window, IDC_LICENSE_KEY, text, static_cast<int>(std::size(text)));
        std::string key = Utf8(text);
        SecureZeroMemory(text, sizeof(text));
        const auto attempt = state->controller->BeginAttempt();
        state->remember = IsDlgButtonChecked(window, IDC_LICENSE_REMEMBER) == BST_CHECKED;
        EnableInputs(window, false);
        SetDlgItemTextW(window, IDC_LICENSE_STATUS, state->chinese ? L"正在连接验证服务器…" : L"Connecting to license server…");
        try {
            auto controller = state->controller;
            state->pending = std::async(std::launch::async, [controller, attempt, key = std::move(key)]() mutable {
                const bool ok = controller->Authenticate(attempt, key);
                // Key persistence stays on the UI thread after cancellation checks.
                if (!key.empty()) SecureZeroMemory(key.data(), key.size());
                return ok;
            });
        } catch (...) { state->controller->Cancel(); EnableInputs(window, true); }
        return TRUE;
    }
    if (message == WM_TIMER && wParam == 1 && state->pending.valid()
        && state->pending.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        bool ok = false;
        try { ok = state->pending.get(); } catch (...) { state->controller->Cancel(); }
        if (state->cancel) { EndDialog(window, IDCANCEL); return TRUE; }
        if (ok && state->controller->IsAuthorized()) {
            try {
                if (state->remember) {
                    wchar_t text[513]{};
                    GetDlgItemTextW(window, IDC_LICENSE_KEY, text, static_cast<int>(std::size(text)));
                    auto key = Utf8(text);
                    SecureZeroMemory(text, sizeof(text));
                    BaoBao::Auth::SecureKeyStore::Save(state->path, key);
                    if (!key.empty()) SecureZeroMemory(key.data(), key.size());
                } else {
                    std::error_code error;
                    std::filesystem::remove(state->path, error);
                    if (error) throw std::filesystem::filesystem_error("Delete saved key", state->path, error);
                }
            } catch (...) {
                MessageBoxW(window, state->chinese ? L"登录成功，但保存卡密设置失败。" : L"Signed in, but saving key preferences failed.", L"VeoxRobRun", MB_OK);
            }
            SetDlgItemTextW(window, IDC_LICENSE_KEY, L"");
            EndDialog(window, IDOK);
        } else {
            const auto reason = Wide(state->controller->Message());
            SetDlgItemTextW(window, IDC_LICENSE_STATUS, reason.empty()
                ? (state->chinese ? L"验证失败，请重试。" : L"Verification failed. Please retry.") : reason.c_str());
            EnableInputs(window, true);
        }
        return TRUE;
    }
    if (message == WM_DESTROY) { KillTimer(window, 1); return TRUE; }
    return FALSE;
}
INT_PTR CALLBACK Procedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    try { return Handle(window, message, wParam, lParam); }
    catch (...) {
        auto* state = reinterpret_cast<State*>(GetWindowLongPtrW(window, DWLP_USER));
        if (state) state->controller->Cancel();
        EndDialog(window, IDCANCEL);
        return TRUE;
    }
}
}
bool ShowLoginDialog(std::shared_ptr<Controller> controller, const std::filesystem::path& path, bool chinese)
{
    State state{ std::move(controller), path, chinese };
    const auto result = DialogBoxParamW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDD_LICENSE), nullptr,
        Procedure, reinterpret_cast<LPARAM>(&state));
    if (result != IDOK) state.controller->Cancel();
    // The future joins on destruction, so no task outlives the controller state.
    return result == IDOK && state.controller->IsAuthorized();
}
}
