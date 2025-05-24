#include "ConfigInitializer.h"
#include <Shlwapi.h>
#include <windowsx.h>

#pragma comment(lib, "Shlwapi.lib")

namespace smax {


INT_PTR CALLBACK FullInputDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    static InputFullData* input = nullptr;

    switch (msg) {
        case WM_INITDIALOG:
            input = reinterpret_cast<InputFullData*>(lParam);
            SetDlgItemTextW(hwndDlg, 1001, input->hostname);
            SetDlgItemTextW(hwndDlg, 1002, input->tenant);
            SetDlgItemTextW(hwndDlg, 1003, input->period);
            SetDlgItemTextW(hwndDlg, 1004, input->username);
            SetDlgItemTextW(hwndDlg, 1005, input->token);

            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK: {
                    GetDlgItemTextW(hwndDlg, 1001, input->hostname, 256);
                    GetDlgItemTextW(hwndDlg, 1002, input->tenant, 256);
                    GetDlgItemTextW(hwndDlg, 1003, input->period, 256);
                    GetDlgItemTextW(hwndDlg, 1004, input->username, 256);
                    GetDlgItemTextW(hwndDlg, 1005, input->token, 256);

                    if (wcslen(input->hostname) == 0 || wcslen(input->tenant) == 0 ||
                        wcslen(input->period) == 0 || wcslen(input->username) == 0 ||
                        wcslen(input->token) == 0) {
                        MessageBoxW(hwndDlg, L"All fields are required.", L"Warning", MB_OK | MB_ICONWARNING);
                        return TRUE;
                    }

                    int periodValue = _wtoi(input->period);

                    if (periodValue < 60) {
                        periodValue = 60;
                        _itow_s(periodValue, input->period, 256, 10);
                    }

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

void ConfigInitializer::initializeToken(const std::wstring& iniPath, EncryptFunc encryptFunc, WideToUtf8Func wideToUtf8Func) {
    if (!PathFileExistsW(iniPath.c_str())) {
        if (!generateINI(iniPath, encryptFunc)) {
            MessageBoxW(NULL, L"Configuration was not created.", L"Warning", MB_ICONWARNING);
        }
        return;
    }

    processINI(iniPath, encryptFunc);
}

bool ConfigInitializer::generateINI(const std::wstring& iniPath, EncryptFunc encryptFunc) {
    HINSTANCE hInstance = GetModuleHandleW(NULL);
    InputFullData data = {};
    wcscpy_s(data.period, L"60");

    INT_PTR result = DialogBoxParamW(hInstance, MAKEINTRESOURCE(102), NULL, FullInputDlgProc, reinterpret_cast<LPARAM>(&data));
    if (result != IDOK) return false;

    CSimpleIniW ini;
    ini.SetUnicode();

    ini.SetValue(L"Settings", L"instance", data.tenant);
    ini.SetValue(L"Settings", L"period", data.period);

    ini.SetValue(data.tenant, L"hostname", data.hostname);
    ini.SetValue(data.tenant, L"tenantId", data.tenant);
    ini.SetValue(data.tenant, L"userName", encryptFunc(data.username).c_str());
    ini.SetValue(data.tenant, L"token", encryptFunc(data.token).c_str());

    return ini.SaveFile(iniPath.c_str()) >= 0;
}

void ConfigInitializer::UpdateINI(const std::wstring& iniPath, DecryptFunc decryptFunc, EncryptFunc encryptFunc, Utf8ToWideFunc utf8ToWideFunc) {
    HINSTANCE hInstance = GetModuleHandleW(NULL);
    CSimpleIniW ini;
    ini.SetUnicode();

    InputFullData data = {};

    if (!PathFileExistsW(iniPath.c_str())) {
        generateINI(iniPath, encryptFunc);
        return;
    }

    ini.SetSpaces(true);
    if (ini.LoadFile(iniPath.c_str()) < 0) {
        MessageBoxW(NULL, L"Failed to load config file.", L"Error", MB_ICONERROR);
        return;
    }

    const wchar_t* tenant = ini.GetValue(L"Settings", L"instance", L"");
    const wchar_t* period = ini.GetValue(L"Settings", L"period", L"");
    const wchar_t* hostname = ini.GetValue(tenant, L"hostname", L"");
    const wchar_t* userName = ini.GetValue(tenant, L"userName", L"");
    const wchar_t* token = ini.GetValue(tenant, L"token", L"");

    std::wstring userName_encrypted_wstr(userName);
    std::wstring token_encrypted_wstr(token);
  
    std::wstring decryptedUser = utf8ToWideFunc(decryptFunc(userName));
    std::wstring decryptedToken = utf8ToWideFunc(decryptFunc(token));

    wcsncpy_s(data.hostname, hostname, _TRUNCATE);
    wcsncpy_s(data.tenant, tenant, _TRUNCATE);
    wcsncpy_s(data.period, period, _TRUNCATE);
    wcsncpy_s(data.username, decryptedUser.c_str(), _TRUNCATE);
    wcsncpy_s(data.token, decryptedToken.c_str(), _TRUNCATE);

    INT_PTR result = DialogBoxParamW(hInstance, MAKEINTRESOURCE(102), NULL, FullInputDlgProc, reinterpret_cast<LPARAM>(&data));
    if (result != IDOK) return;

    ini.SetValue(L"Settings", L"instance", data.tenant);
    ini.SetValue(L"Settings", L"period", data.period);

    ini.SetValue(data.tenant, L"hostname", data.hostname);
    ini.SetValue(data.tenant, L"tenantId", data.tenant);
    ini.SetValue(data.tenant, L"userName", encryptFunc(data.username).c_str());
    ini.SetValue(data.tenant, L"token", encryptFunc(data.token).c_str());

    if (ini.SaveFile(iniPath.c_str()) < 0) {
        MessageBoxW(NULL, L"Failed to save config file.", L"Error", MB_ICONERROR);
    }
}


void ConfigInitializer::processINI(const std::wstring& iniPath, EncryptFunc encryptFunc) {
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

    auto valueUserName = encryptFunc(std::move(user));
    ini.SetValue(instance, L"userName", valueUserName.c_str());

    auto valueToken = encryptFunc(std::move(token));
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
            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK: {
                    wchar_t userBuf[256], tokenBuf[256];
                    GetDlgItemTextW(hwndDlg, 1001, userBuf, 256);
                    GetDlgItemTextW(hwndDlg, 1002, tokenBuf, 256);
                    if (wcslen(userBuf) == 0 || wcslen(tokenBuf) == 0) {
                        MessageBoxW(hwndDlg, L"Both username and token must be provided.", L"Warning", MB_OK | MB_ICONWARNING);
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

std::pair<std::wstring, std::wstring> ConfigInitializer::promptInput() {
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
