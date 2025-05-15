#include "TokenInitializer.h"
#include "..\Utils\Utils.h"
#include <Shlwapi.h>
#include <windowsx.h>
#pragma comment(lib, "Shlwapi.lib")

namespace smax {

void TokenInitializer::initializeToken(const std::wstring& iniPath) {
    CSimpleIniW ini;
    ini.SetUnicode();
    if (ini.LoadFile(iniPath.c_str()) < 0) {
        MessageBoxW(NULL, L"Failed to load config file.", L"Error", MB_ICONERROR);
        return;
    }

    const wchar_t* instance = ini.GetValue(L"Settings", L"instance", NULL);
    if (!instance) return;

    const wchar_t* tokenValue = ini.GetValue(instance, L"token", NULL);
    if (!tokenValue || wcscmp(tokenValue, L"-init-") != 0) return;

    auto [user, token] = promptInput();
    if (user.empty() || token.empty()) {
        MessageBoxW(NULL, L"Username or token was not provided.", L"Warning", MB_ICONWARNING);
        return;
    }

    if (user.empty() || token.empty()) {
        MessageBoxW(NULL, L"Username or token was not provided.", L"Warning", MB_ICONWARNING);
        return;
    }

    std::wstring valueUserName = std::wstring(user.begin(), user.end());
    ini.SetValue(instance, L"userName", valueUserName.c_str());

    std::string tokenEncrypted = encrypt(wideToUtf8(token));
    std::string tokenHex = toHex(tokenEncrypted);
    std::wstring valueToken(tokenHex.begin(), tokenHex.end());
    ini.SetValue(instance, L"token", valueToken.c_str());

    if (ini.SaveFile(iniPath.c_str()) < 0) {
        MessageBoxW(NULL, L"Failed to save config file.", L"Error", MB_ICONERROR);
    }
}

INT_PTR CALLBACK InputDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static InputData* input = nullptr;
    switch (msg) {
        case WM_INITDIALOG:
            input = reinterpret_cast<InputData*>(lParam);
            SetDlgItemTextW(hwndDlg, 1001, L"");
            SetDlgItemTextW(hwndDlg, 1002, L"");
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK: {
                    wchar_t userBuf[256], tokenBuf[256];
                    GetDlgItemTextW(hwndDlg, 1001, userBuf, 256);
                    GetDlgItemTextW(hwndDlg, 1002, tokenBuf, 256);
                    if (wcslen(userBuf) == 0 || wcslen(tokenBuf) == 0) {
                        MessageBoxW(hwndDlg, L"Both username and token must be provided.", L"Warning", MB_ICONWARNING);
                        return TRUE;
                    }
                    wcscpy_s(input->username, 256, userBuf);
                    wcscpy_s(input->token, 256, tokenBuf);
                    EndDialog(hwndDlg, IDOK);
                    return TRUE;
                }
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

std::pair<std::wstring, std::wstring> TokenInitializer::promptInput() {
    const int bufSize = 256;
    wchar_t usernameBuf[bufSize] = {};
    wchar_t tokenBuf[bufSize] = {};
    InputData data = { usernameBuf, tokenBuf };

    HINSTANCE hInstance = GetModuleHandleW(NULL);
    INT_PTR result = DialogBoxParamW(hInstance, MAKEINTRESOURCE(101), NULL, InputDlgProc, reinterpret_cast<LPARAM>(&data));

    if (result == IDOK) {
        return { usernameBuf, tokenBuf };
    }

    return { L"", L"" };
}

} // namespace smax
