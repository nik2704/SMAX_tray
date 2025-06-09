#include "ConfigInitializer.h"
#include <Shlwapi.h>
#include <windowsx.h>

#pragma comment(lib, "Shlwapi.lib")

namespace smax {

inline const wchar_t * DEFAULT_AVIATOR_MODEL = L"ESM-VIRTUAL-AGENT";
inline const wchar_t * DEFAULT_LOG_LEVEL = L"ERROR";

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
            CheckDlgButton(hwndDlg, 1006, input->check_requests ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, 1007, input->check_tasks ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, 1008, input->check_approvals ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, 1009, input->aviator_enabled ? BST_CHECKED : BST_UNCHECKED);

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

                    input->check_requests  = (IsDlgButtonChecked(hwndDlg, 1006) == BST_CHECKED);
                    input->check_tasks     = (IsDlgButtonChecked(hwndDlg, 1007) == BST_CHECKED);
                    input->check_approvals = (IsDlgButtonChecked(hwndDlg, 1008) == BST_CHECKED);
                    input->aviator_enabled = (IsDlgButtonChecked(hwndDlg, 1009) == BST_CHECKED);

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
    ini.SetValue(L"Settings", L"aviator_model", DEFAULT_AVIATOR_MODEL);
    ini.SetValue(L"Settings", L"min_log_level", DEFAULT_LOG_LEVEL);

    ini.SetValue(data.tenant, L"hostname", data.hostname);
    ini.SetValue(data.tenant, L"tenantId", data.tenant);
    ini.SetValue(data.tenant, L"userName", encryptFunc(data.username).c_str());
    ini.SetValue(data.tenant, L"token", encryptFunc(data.token).c_str());

    ini.SetValue(data.tenant, L"check_requests", data.check_requests ? L"1" : L"0");
    ini.SetValue(data.tenant, L"check_tasks", data.check_tasks ? L"1" : L"0");
    ini.SetValue(data.tenant, L"check_approvals", data.check_approvals ? L"1" : L"0");
    ini.SetValue(data.tenant, L"aviator_enabled", data.aviator_enabled ? L"1" : L"0");

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
    const wchar_t* val_requests = ini.GetValue(tenant, L"check_requests", L"1");
    const wchar_t* val_tasks = ini.GetValue(tenant, L"check_tasks", L"1");
    const wchar_t* val_approvals = ini.GetValue(tenant, L"check_approvals", L"1");
    const wchar_t* val_aviator_enabled = ini.GetValue(tenant, L"aviator_enabled", L"1");

    std::wstring userName_encrypted_wstr(userName);
    std::wstring token_encrypted_wstr(token);
  
    std::wstring decryptedUser = utf8ToWideFunc(decryptFunc(userName));
    std::wstring decryptedToken = utf8ToWideFunc(decryptFunc(token));

    wcsncpy_s(data.hostname, hostname, _TRUNCATE);
    wcsncpy_s(data.tenant, tenant, _TRUNCATE);
    wcsncpy_s(data.period, period, _TRUNCATE);
    wcsncpy_s(data.username, decryptedUser.c_str(), _TRUNCATE);
    wcsncpy_s(data.token, decryptedToken.c_str(), _TRUNCATE);

    data.check_requests = wcscmp(val_requests, L"1") == 0 ? 1 : 0;
    data.check_tasks = wcscmp(val_tasks, L"1") == 0 ? 1 : 0;
    data.check_approvals = wcscmp(val_approvals, L"1") == 0 ? 1 : 0;
    data.aviator_enabled = wcscmp(val_aviator_enabled, L"1") == 0 ? 1 : 0;

    INT_PTR result = DialogBoxParamW(hInstance, MAKEINTRESOURCE(102), NULL, FullInputDlgProc, reinterpret_cast<LPARAM>(&data));
    if (result != IDOK) return;

    ini.SetValue(L"Settings", L"instance", data.tenant);
    ini.SetValue(L"Settings", L"period", data.period);
    ini.SetValue(L"Settings", L"aviator_model", DEFAULT_AVIATOR_MODEL);
    ini.SetValue(L"Settings", L"min_log_level", DEFAULT_LOG_LEVEL);

    ini.SetValue(data.tenant, L"hostname", data.hostname);
    ini.SetValue(data.tenant, L"tenantId", data.tenant);
    ini.SetValue(data.tenant, L"userName", encryptFunc(data.username).c_str());
    ini.SetValue(data.tenant, L"token", encryptFunc(data.token).c_str());
    ini.SetValue(data.tenant, L"check_requests", data.check_requests ? L"1" : L"0");
    ini.SetValue(data.tenant, L"check_tasks", data.check_tasks ? L"1" : L"0");
    ini.SetValue(data.tenant, L"check_approvals", data.check_approvals ? L"1" : L"0");
    ini.SetValue(data.tenant, L"aviator_enabled", data.aviator_enabled ? L"1" : L"0");

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
