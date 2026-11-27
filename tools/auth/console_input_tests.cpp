#define WIN32_LEAN_AND_MEAN
#include "Security/ConsoleInput.h"
#include <cstdio>
#include <vector>
#include <future>

using namespace LicenseAuth;
namespace {
INPUT_RECORD Key(WORD vk, wchar_t c = 0, DWORD flags = 0, bool down = true, WORD repeat = 1) {
    INPUT_RECORD r{}; r.EventType = KEY_EVENT;
    r.Event.KeyEvent = { BOOL(down), repeat, vk, 0, {}, flags };
    r.Event.KeyEvent.uChar.UnicodeChar = c; return r;
}
std::vector<INPUT_RECORD> Text(const std::wstring& text) {
    std::vector<INPUT_RECORD> result;
    for (auto c : text) result.push_back(Key(0,c));
    return result;
}
}
int main() {
    int checks = 0, failures = 0;
    auto check = [&](bool ok, const char* name) { ++checks; if (!ok) { ++failures; std::printf("FAIL: %s\n",name); } };
    const HANDLE input = CreateFileW(L"CONIN$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);
    const HANDLE output = CreateConsoleScreenBuffer(GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,CONSOLE_TEXTMODE_BUFFER,nullptr);
    check(input != INVALID_HANDLE_VALUE && output != INVALID_HANDLE_VALUE,"isolated native console handles");
    if (input == INVALID_HANDLE_VALUE || output == INVALID_HANDLE_VALUE) return 1;
    DWORD original = 0; GetConsoleMode(input,&original);
    // Reproduce hosts whose cooked/echo/Quick Edit/VT settings are inconsistent.
    const DWORD awkward = (original | ENABLE_LINE_INPUT | ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS | ENABLE_VIRTUAL_TERMINAL_INPUT) & ~ENABLE_ECHO_INPUT;
    check(SetConsoleMode(input,awkward) != FALSE,"baseline awkward console mode set");
    struct Result { bool read; std::string text; std::wstring echo; int pastes; };
    auto run = [&](std::vector<INPUT_RECORD> events, const std::wstring& paste = L"", bool secret = true) {
        FlushConsoleInputBuffer(input);
        DWORD count = 0;
        FillConsoleOutputCharacterW(output,L' ',4096,{0,0},&count); SetConsoleCursorPosition(output,{0,0});
        events.push_back(Key(VK_RETURN,L'\r'));
        // Inject after the reader sets raw mode, just as real typing arrives.
        auto writer = std::async(std::launch::async, [&] {
            for (int n=0;n<2000;++n) {
                DWORD mode=0; GetConsoleMode(input,&mode);
                if (!(mode & ENABLE_LINE_INPUT)) break;
                Sleep(1);
            }
            DWORD written=0;
            return WriteConsoleInputW(input,events.data(),static_cast<DWORD>(events.size()),&written) && written==events.size();
        });
        std::string text; int pastes = 0;
        const bool read = ReadNativeConsoleLine(input,output,secret,text,[&]{ ++pastes; return paste; });
        check(writer.get(),"real console events queued after raw mode enabled");
        DWORD mode = 0; GetConsoleMode(input,&mode); check(mode==awkward,"original input mode restored");
        std::wstring echo(200,L'\0'); ReadConsoleOutputCharacterW(output,echo.data(),static_cast<DWORD>(echo.size()),{0,0},&count); echo.resize(count);
        return Result{read,text,echo,pastes};
    };
    auto typed=run(Text(L"AB12-xyz"));
    check(typed.read && typed.text=="AB12-xyz","typed card accepted via native console, not stdin");
    check(typed.echo.substr(0,8)==L"AB12-xyz","typed card echoed in plaintext");
    auto ctrl=run({Key('V',0x16,LEFT_CTRL_PRESSED)},L"PASTED-CARD");
    check(ctrl.read && ctrl.text=="PASTED-CARD" && ctrl.pastes==1,"Ctrl+V paste handled explicitly");
    check(ctrl.echo.substr(0,11)==L"PASTED-CARD","pasted card echoed in plaintext");
    auto rightCtrl=run({Key('V',0,RIGHT_CTRL_PRESSED)},L"RIGHT-CTRL");
    check(rightCtrl.text=="RIGHT-CTRL","right Ctrl+V accepted");
    auto shift=run({Key(VK_INSERT,0,SHIFT_PRESSED)},L"SHIFT-INSERT");
    check(shift.text=="SHIFT-INSERT","Shift+Insert paste accepted");
    INPUT_RECORD mouse{}; mouse.EventType=MOUSE_EVENT; mouse.Event.MouseEvent.dwButtonState=RIGHTMOST_BUTTON_PRESSED;
    auto right=run({mouse,mouse},L"RIGHT-CLICK");
    check(right.text=="RIGHT-CLICK" && right.pastes==1,"right-click paste, no duplicate on held mouse");
    auto mixed=Text(L"PREFIX-"); mixed.push_back(Key('V',0x16,LEFT_CTRL_PRESSED));
    auto suffix=Text(L"-TAIL"); mixed.insert(mixed.end(),suffix.begin(),suffix.end());
    check(run(mixed,L"MIDDLE").text=="PREFIX-MIDDLE-TAIL","typing and paste combine correctly");
    auto edit=Text(L"WRONG"); for(int n=0;n<5;++n)edit.push_back(Key(VK_BACK,L'\b'));
    edit.push_back(Key('V',0x16,LEFT_CTRL_PRESSED));
    check(run(edit,L"FIXED").text=="FIXED","backspace followed by paste works");
    auto clear=Text(L"WRONG"); clear.push_back(Key('U',0x15,LEFT_CTRL_PRESSED)); clear.push_back(Key('V',0x16,LEFT_CTRL_PRESSED));
    check(run(clear,L"CLEARED").text=="CLEARED","Ctrl+U clears input");
    check(run({Key('V',0x16,LEFT_CTRL_PRESSED)},L"ONLY-FIRST\r\n5\r\n").text=="ONLY-FIRST","multiline paste cannot become menu commands");
    check(run({Key('V',0x16,LEFT_CTRL_PRESSED)},L"\ufeffCARD").text=="CARD","clipboard BOM ignored");
    check(run({Key('V',0x16,LEFT_CTRL_PRESSED)},L"中文卡密").text=="中文卡密","clipboard Unicode converts to UTF-8");
    check(run({Key('V',0x16,LEFT_CTRL_PRESSED)},L"").text.empty(),"empty clipboard is recoverable");
    auto released=Text(L"A"); released.push_back(Key('B',L'B',0,false)); released.push_back(Key('C',L'C',0,true,3));
    check(run(released).text=="ACCC","key-up ignored and repeated key-down accepted");
    check(run(Text(L"3"),L"",false).echo.substr(0,1)==L"3","menu digits echoed normally");
    check(!run({Key(VK_ESCAPE)}).read,"Escape cancels field");
    check(!run({Key('C',3,LEFT_CTRL_PRESSED)}).read,"Ctrl+C cancels without waiting on CRT");
    std::string invalid;
    check(!ReadNativeConsoleLine(INVALID_HANDLE_VALUE,output,true,invalid),"invalid console handle returns an error");
    SetConsoleMode(input,original); CloseHandle(output); CloseHandle(input);
    std::printf("%s: native console input/paste; %d checks; %d failures\n",failures?"FAIL":"PASS",checks,failures);
    return failures ? 1 : 0;
}
