#include "TokenInitializer.h"
#include "..\Utils\Utils.h"
#include <Shlwapi.h>
#include <windowsx.h> // для удобных макросов
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

    std::wstring user = promptInput(L"Enter your username:", L"Authentication");
    std::wstring token = promptInput(L"Enter your token:", L"Authentication");

    if (user.empty() || token.empty()) {
        MessageBoxW(NULL, L"Username or token was not provided.", L"Warning", MB_ICONWARNING);
        return;
    }

    std::wstring valueUserName = std::wstring(user.begin(), user.end());
    ini.SetValue(instance, L"userName", valueUserName.c_str());

    std::string tokenEncrypted = encrypt(wideToUtf8(token));
    std::string tokenHex = toHex(tokenEncrypted);
    std::wstring valueToken(tokenHex.begin(), tokenHex.end());  // Problem here
    ini.SetValue(instance, L"token", valueToken.c_str());

    if (ini.SaveFile(iniPath.c_str()) < 0) {
        MessageBoxW(NULL, L"Failed to save config file.", L"Error", MB_ICONERROR);
    }
}

INT_PTR CALLBACK InputDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static wchar_t* outBuffer = nullptr;
    switch (msg) {
        case WM_INITDIALOG:
            outBuffer = reinterpret_cast<wchar_t*>(lParam);
            SetDlgItemTextW(hwndDlg, 1001, L"");
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    GetDlgItemTextW(hwndDlg, 1001, outBuffer, 256);
                    EndDialog(hwndDlg, IDOK);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hwndDlg, IDCANCEL);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}

std::wstring TokenInitializer::promptInput(const std::wstring& message, const std::wstring& title) {
    const int bufSize = 256;
    wchar_t buffer[bufSize] = {};

    HINSTANCE hInstance = GetModuleHandleW(NULL);
    INT_PTR result = DialogBoxParamW(hInstance, MAKEINTRESOURCE(101), NULL, InputDlgProc, reinterpret_cast<LPARAM>(buffer));

    if (result == IDOK) {
        return buffer;
    }

    return L"";
}

} // namespace smax
